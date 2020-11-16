export HISTFILE=~/.history
export HISTSIZE=500
export PATH=.:/xshell/bin:/bin
export PS1='$ '

alias ls='ls --color'
umask 022

printf " The following shell commands are available.\n"
printf " \t\033[1;32mls\033[0m -- list all files\n"
printf " \t\033[1;32mmv\033[0m -- move (rename) files\n"
printf " \t\033[1;32mrm\033[0m -- remove files or directories\n"
printf " \t\033[1;32mclear\033[0m -- clear this terminal screen\n"
printf " \t\033[1;32mexit\033[0m -- exit this terminal\n"
printf " \t\033[1;32mCtrl-D\033[0m  -- terminate a running process or this terminal\n"
printf "\n\n\n"
