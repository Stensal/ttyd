#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <sys/select.h>
#include <sys/types.h>
#include <pthread.h>

#if defined(__OpenBSD__) || defined(__APPLE__)
#include <util.h>
#elif defined(__FreeBSD__)
#include <libutil.h>
#else
#include <pty.h>
#endif

#include <libwebsockets.h>
#include <json.h>

#include "server.h"
#include "utils.h"

/*
int
send_initial_message(struct lws *wsi, int index) {
    unsigned char message[LWS_PRE + 1 + 4096];
    unsigned char *p = &message[LWS_PRE];
    char buffer[128];
    int n = 0;

    char cmd = initial_cmds[index];
    switch(cmd) {
        case SET_WINDOW_TITLE:
            gethostname(buffer, sizeof(buffer) - 1);
            n = sprintf((char *) p, "%c%s (%s)", cmd, server->command, buffer);
            break;
        case SET_RECONNECT:
            n = sprintf((char *) p, "%c%d", cmd, server->reconnect);
            break;
        case SET_PREFERENCES:
            n = sprintf((char *) p, "%c%s", cmd, server->prefs_json);
            break;
        default:
            break;
    }

    return lws_write(wsi, p, (size_t) n, LWS_WRITE_BINARY);
}
*/

bool
parse_window_size(const char *json, struct winsize *size) {
    int columns, rows;
    json_object *obj = json_tokener_parse(json);
    struct json_object *o = NULL;

    if (!json_object_object_get_ex(obj, "columns", &o)) {
        lwsl_err("columns field not exists, json: %s\n", json);
        return false;
    }
    columns = json_object_get_int(o);
    if (!json_object_object_get_ex(obj, "rows", &o)) {
        lwsl_err("rows field not exists, json: %s\n", json);
        return false;
    }
    rows = json_object_get_int(o);
    json_object_put(obj);

    memset(size, 0, sizeof(struct winsize));
    size->ws_col = (unsigned short) columns;
    size->ws_row = (unsigned short) rows;

    return true;
}

bool
check_host_origin(struct lws *wsi) {
    int origin_length = lws_hdr_total_length(wsi, WSI_TOKEN_ORIGIN);
    char buf[origin_length + 1];
    memset(buf, 0, sizeof(buf));
    int len = lws_hdr_copy(wsi, buf, sizeof(buf), WSI_TOKEN_ORIGIN);
    if (len <= 0) {
        return false;
    }

    const char *prot, *address, *path;
    int port;
    if (lws_parse_uri(buf, &prot, &address, &port, &path))
        return false;
    if (port == 80 || port == 443) {
        sprintf(buf, "%s", address);
    } else {
        sprintf(buf, "%s:%d", address, port);
    }

    int host_length = lws_hdr_total_length(wsi, WSI_TOKEN_HOST);
    if (host_length != strlen(buf))
        return false;
    char host_buf[host_length + 1];
    memset(host_buf, 0, sizeof(host_buf));
    len = lws_hdr_copy(wsi, host_buf, sizeof(host_buf), WSI_TOKEN_HOST);

    return len > 0 && strcasecmp(buf, host_buf) == 0;
}

void
tty_client_remove(struct tty_client *client) {
    pthread_mutex_lock(&server->mutex);
    struct tty_client *iterator;
    LIST_FOREACH(iterator, &server->clients, list) {
        if (iterator == client) {
            LIST_REMOVE(iterator, list);
            server->client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&server->mutex);
}

#define EXTRA_ARGV  2
void
tty_client_destroy(struct tty_client *client) {
    if (!client->running || client->pid <= 0)
        goto cleanup;

    client->running = false;

    if (pthread_mutex_trylock(&client->mutex)) {
        client->state = STATE_DONE;
        pthread_cond_signal(&client->cond);
        pthread_mutex_unlock(&client->mutex);
    }

    // kill process (group) and free resource
    int pgid = getpgid(client->pid);
    int pid = pgid > 0 ? -pgid : client->pid;
    lwsl_notice("sending %s (%d) to process (group) %d\n", server->sig_name, server->sig_code, pid);
    if (kill(pid, server->sig_code) != 0) {
        lwsl_err("kill: %d, errno: %d (%s)\n", pid, errno, strerror(errno));
    }
    pid_t pid_out;
    int status = wait_proc(client->pid, &pid_out);
    if (pid_out > 0) {
        lwsl_notice("process exited with code %d, pid: %d\n", status, pid_out);
    }
    close(client->pty);

cleanup:
    // free the buffer
    if (client->buffer != NULL)
        free(client->buffer);

    if (client->argc > 0) {
        for (int i = 0; i < client->argc + EXTRA_ARGV; i++) {
            free(client->argv[i]);
        }
    }
    free(client->argv);

    pthread_mutex_destroy(&client->mutex);

    // remove from client list
    tty_client_remove(client);
}

#define BIN_PATH_NAME    "/sbin"
#define SH_CMD   "sh_in_cattlegrid.sh"
#define EXE_CMD  "run_in_cattlegrid.sh"
#define DEBUG_CMD "test.sh"
#define GDB_CMD "gdb.sh"

int
handle_execution_command(struct tty_client *client) {
    struct json_object* json_obj;
    enum json_tokener_error jerror;
    struct json_tokener* tok;
    int result = 0;

    if (!client || !client->buffer || client->len <= 0)
        return 0;

    tok = json_tokener_new();

    json_obj = json_tokener_parse_ex(tok, client->buffer, client->len);
    jerror = json_tokener_get_error(tok);
    if (jerror == json_tokener_success && json_object_is_type(json_obj, json_type_object)) {
        // {
        //     "exec": <pathname>,
        //     "argv": [ "arg1", "arg2", ... , "argN" ],
        // }
        struct json_object* argv_obj = NULL;
        struct json_object* exec_path_obj = NULL;
        int is_sh = 0, is_exe = 0, is_debug = 0, is_gdb = 0;

        if (json_object_object_get_ex(json_obj, "exec", &exec_path_obj))
            is_exe = 1;
        else if (json_object_object_get_ex(json_obj, "shell", &exec_path_obj))
            is_sh = 1;
        else if (json_object_object_get_ex(json_obj, "debug", &exec_path_obj))
            is_debug = 1;
        else if (json_object_object_get_ex(json_obj, "gdb", &exec_path_obj))
            is_gdb = 1;

        if (is_exe || is_sh || is_debug || is_gdb) {
            char * path = json_object_get_string(exec_path_obj);
            if (is_exe)
                client->exe_path_name = strdup(BIN_PATH_NAME "/" EXE_CMD);
            else if (is_sh)
                client->exe_path_name = strdup(BIN_PATH_NAME "/" SH_CMD);
            else if (is_debug)
                client->exe_path_name = strdup(BIN_PATH_NAME "/" DEBUG_CMD);
            else if (is_gdb)
                client->exe_path_name = strdup(BIN_PATH_NAME "/" GDB_CMD);

            lwsl_notice("exe_path_name %s\n", client->exe_path_name);

            //client->exe_path_name = strdup(json_object_get_string(exec_path_obj));
            if (json_object_object_get_ex(json_obj, "argv", &argv_obj)) {
                if (json_object_is_type(argv_obj, json_type_array))
                    client->argc = json_object_array_length(argv_obj);
            }

            client->argv = (char**)xmalloc((client->argc + EXTRA_ARGV + 1) * sizeof(char*));

            // argv[0] is executable name
            if (is_exe)
                client->argv[0] = strdup(EXE_CMD);
            else if (is_sh)
                client->argv[0] = strdup(SH_CMD);
            else if (is_debug)
                client->argv[0] = strdup(DEBUG_CMD);
            else if (is_gdb)
                client->argv[0] = strdup(GDB_CMD);

            lwsl_notice("cmd %s\n", client->argv[0]);
            client->argv[1] = strdup(path);
            lwsl_notice("cattlecell %s\n", client->argv[1]);

            for (int i = EXTRA_ARGV; i < client->argc + EXTRA_ARGV; i++) {
                struct json_object* elem_obj = json_object_array_get_idx(argv_obj, i - EXTRA_ARGV);
                client->argv[i] = strdup(json_object_get_string(elem_obj));
                lwsl_notice("argv[%d]: %s\n", i, client->argv[i]);
            }
            client->argv[client->argc + EXTRA_ARGV] = NULL;

            if (argv_obj)
                json_object_put(argv_obj);
            json_object_put(exec_path_obj);

            result = 1;
        }
        json_object_put(json_obj);
    }

    json_tokener_free(tok);

    return result;
}

void *
thread_run_command(void *args) {
    struct tty_client *client = (struct tty_client *) args;

    int pty;
    fd_set des_set;

    lwsl_notice("\nrun_command: forkpty, path=%s\n", client->exe_path_name); 
    for (int i = 0; i < client->argc + EXTRA_ARGV; i++)
        printf("client->argv[%d]=%s\n", i, client->argv[i]);
    pid_t pid = forkpty(&pty, NULL, NULL, NULL);
    if (pid < 0) { /* error */
        lwsl_err("forkpty failed: %d (%s)\n", errno, strerror(errno));
        pthread_exit((void *) 1);
    } else if (pid == 0) { /* child */
        setenv("TERM", server->terminal_type, true);
        // Don't pass the web socket onto child processes
        close(lws_get_socket_fd(client->wsi));
        if (execv(client->exe_path_name, client->argv) < 0) {
            perror("execvp failed\n");
            _exit(-errno);
        }
    }

    // set the pty file descriptor non blocking
    int status_flags = fcntl(pty, F_GETFL);
    if (status_flags != -1) {
        fcntl(pty, F_SETFL, status_flags | O_NONBLOCK);
    }

    lwsl_notice("started process, pid: %d\n", pid);
    client->pid = pid;
    client->pty = pty;
    client->running = true;
    client->size.ws_col = 80;
    client->size.ws_row = 40;
    if (client->size.ws_row > 0 && client->size.ws_col > 0)
        ioctl(client->pty, TIOCSWINSZ, &client->size);

    while (client->running) {
        FD_ZERO (&des_set);
        FD_SET (pty, &des_set);
        struct timeval tv = { 1, 0 };
        int ret = select(pty + 1, &des_set, NULL, NULL, &tv);
        if (ret == 0) continue;
        if (ret < 0) break;

        if (FD_ISSET (pty, &des_set)) {
            while (client->running) {
                pthread_mutex_lock(&client->mutex);
                while (client->state == STATE_READY) {
                    pthread_cond_wait(&client->cond, &client->mutex);
                }
                memset(client->pty_buffer, 0, sizeof(client->pty_buffer));
                client->pty_len = read(pty, client->pty_buffer + LWS_PRE + 1, BUF_SIZE);
                lwsl_notice("[recv] %s\n", client->pty_buffer + LWS_PRE + 1);
                client->state = STATE_READY;
                pthread_mutex_unlock(&client->mutex);
                break;
            }
        }

        if (client->pty_len <= 0) break;
    }

    pthread_exit((void *) 0);
}

int
callback_tty(struct lws *wsi, enum lws_callback_reasons reason,
             void *user, void *in, size_t len) {
    struct tty_client *client = (struct tty_client *) user;
    char buf[256];
    size_t n = 0;

    switch (reason) {
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
            if (server->once && server->client_count > 0) {
                lwsl_warn("refuse to serve WS client due to the --once option.\n");
                return 1;
            }
            if (server->max_clients > 0 && server->client_count == server->max_clients) {
                lwsl_warn("refuse to serve WS client due to the --max-clients option.\n");
                return 1;
            }
            if (lws_hdr_copy(wsi, buf, sizeof(buf), WSI_TOKEN_GET_URI) <= 0 || strcmp(buf, WS_PATH) != 0) {
                lwsl_warn("refuse to serve WS client for illegal ws path: %s\n", buf);
                return 1;
            }

            if (server->check_origin && !check_host_origin(wsi)) {
                lwsl_warn("refuse to serve WS client from different origin due to the --check-origin option.\n");
                return 1;
            }
            break;

        case LWS_CALLBACK_ESTABLISHED:
            client->running = false;
            client->initialized = false;
            client->authenticated = false;
            client->wsi = wsi;
            client->buffer = NULL;
            client->state = STATE_INIT;
            client->pty_len = 0;
            client->argc = 0;

            pthread_mutex_init(&client->mutex, NULL);
            pthread_cond_init(&client->cond, NULL);
            lws_get_peer_addresses(wsi, lws_get_socket_fd(wsi),
                                   client->hostname, sizeof(client->hostname),
                                   client->address, sizeof(client->address));

            pthread_mutex_lock(&server->mutex);
            LIST_INSERT_HEAD(&server->clients, client, list);
            server->client_count++;
            pthread_mutex_unlock(&server->mutex);

            lws_hdr_copy(wsi, buf, sizeof(buf), WSI_TOKEN_GET_URI);
            lwsl_notice("WS   %s - %s (%s), clients: %d\n", buf, client->address, client->hostname, server->client_count);
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
#if 0
            if (!client->initialized) {
                if (client->initial_cmd_index == sizeof(initial_cmds)) {
                    client->initialized = true;
                    break;
                }
                if (send_initial_message(wsi, client->initial_cmd_index) < 0) {
                    lws_close_reason(wsi, LWS_CLOSE_STATUS_UNEXPECTED_CONDITION, NULL, 0);
                    return -1;
                }
                client->initial_cmd_index++;
                lws_callback_on_writable(wsi);
                return 0;
            }
#endif
            if (client->state != STATE_READY)
                break;

            // read error or client exited, close connection
            if (client->pty_len <= 0) {
                lws_close_reason(wsi,
                                 client->pty_len == 0 ? LWS_CLOSE_STATUS_NORMAL
                                                       : LWS_CLOSE_STATUS_UNEXPECTED_CONDITION,
                                 NULL, 0);
                return -1;
            }

//            client->pty_buffer[LWS_PRE] = OUTPUT;
            n = (size_t) (client->pty_len + 1);
            lwsl_notice("[send] %s\n", client->pty_buffer + LWS_PRE);
            //if (lws_write(wsi, (unsigned char *) client->pty_buffer + LWS_PRE, n, LWS_WRITE_TEXT) < n) {
            if (lws_write(wsi, (unsigned char *) client->pty_buffer + LWS_PRE, n, LWS_WRITE_BINARY) < n) {
                lwsl_err("write data to WS\n");
            }
            client->state = STATE_DONE;
            break;

        case LWS_CALLBACK_RECEIVE:
            if (client->buffer == NULL) {
                client->buffer = xmalloc(len);
                client->len = len;
                memcpy(client->buffer, in, len);
            } else {
                client->buffer = xrealloc(client->buffer, client->len + len);
                memcpy(client->buffer + client->len, in, len);
                client->len += len;
            }

            const char command = client->buffer[0];

            // check auth
            if (server->credential != NULL && !client->authenticated && command != JSON_DATA) {
                lwsl_warn("WS client not authenticated\n");
                return 1;
            }

            // check if there are more fragmented messages
            if (lws_remaining_packet_payload(wsi) > 0 || !lws_is_final_fragment(wsi)) {
                return 0;
            }

            switch (command) {
                case INPUT:
                    if (client->pty == 0)
                        break;
                    if (server->readonly)
                        return 0;
                    if (write(client->pty, client->buffer + 1, client->len - 1) == -1) {
                        lwsl_err("write INPUT to pty: %d (%s)\n", errno, strerror(errno));
                        lws_close_reason(wsi, LWS_CLOSE_STATUS_UNEXPECTED_CONDITION, NULL, 0);
                        return -1;
                    }
                    break;
                case RESIZE_TERMINAL:
                    if (parse_window_size(client->buffer + 1, &client->size) && client->pty > 0) {
                        if (ioctl(client->pty, TIOCSWINSZ, &client->size) == -1) {
                            lwsl_err("ioctl TIOCSWINSZ: %d (%s)\n", errno, strerror(errno));
                        }
                    }
                    break;
                case JSON_DATA:
                    if (client->pid > 0)
                        break;
                    if (!client->authenticated) {
                        if (server->credential != NULL) {
                            json_object *obj = json_tokener_parse(client->buffer);
                            struct json_object *o = NULL;
                            if (json_object_object_get_ex(obj, "AuthToken", &o)) {
                                const char *token = json_object_get_string(o);
                                if (token != NULL && !strcmp(token, server->credential))
                                    client->authenticated = true;
                                else
                                    lwsl_warn("WS authentication failed with token: %s\n", token);
                            }
                            if (!client->authenticated) {
                                lws_close_reason(wsi, LWS_CLOSE_STATUS_POLICY_VIOLATION, NULL, 0);
                                return -1;
                            }
                            break;
                        }
                        else {
                            client->authenticated = true;
                        }
                    }

                    if (!handle_execution_command(client))
                        return 1;

                    lwsl_debug("exe_path %s\n", client->exe_path_name);
                    int err = pthread_create(&client->thread, NULL, thread_run_command, client);
                    if (err != 0) {
                        lwsl_err("pthread_create return: %d\n", err);
                        return 1;
                    }
                    break;
                default:
                    lwsl_warn("ignored unknown message type: command %c %s\n", command, client->buffer);
                    break;
            }

            if (client->buffer != NULL) {
                free(client->buffer);
                client->buffer = NULL;
            }
            break;

        case LWS_CALLBACK_CLOSED:
            tty_client_destroy(client);
            lwsl_notice("WS closed from %s (%s), clients: %d\n", client->address, client->hostname, server->client_count);
            if (server->once && server->client_count == 0) {
                lwsl_notice("exiting due to the --once option.\n");
                force_exit = true;
                lws_cancel_service(context);
                exit(0);
            }
            break;

        default:
            break;
    }

    return 0;
}
