FROM alpine:3.6

RUN apk add --update \
    bash \
  && rm -rf /var/cache/apk/*

EXPOSE 7681

COPY run_in_cattlegrid.sh  /bin/run_in_cattlegrid.sh
COPY sh_in_cattlegrid.sh  /bin/sh_in_cattlegrid.sh
COPY cattlegrid  /bin/cattlegrid
COPY prlimit     /bin/prlimit
COPY ttyd  /bin/ishell
COPY start.sh /bin

RUN  mkdir /ishell_bin && \
     cp /bin/sh   /ishell_bin && \
     cp /usr/bin/env /ishell_bin && \
     cp /bin/bash /ishell_bin && \
     cp /bin/ls   /ishell_bin

COPY run_prog.sh /ishell_bin/
RUN  echo "stensal-anon:x:1000:1000:stensal-anonuser,,,:/home/jail:/bin/bash"  >> /etc/passwd

ENTRYPOINT ["/bin/ishell"]

CMD ["bash"]
