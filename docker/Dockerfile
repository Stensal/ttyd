#FROM alpine:3.6
FROM dockerhub.stensalinc.com/i386/stensal_release_sdk:latest
LABEL Maintainer1="Ning <ning@stensalinc.com>"

RUN apk add --no-cache lsof

RUN rm -rf /sjacket/etc/ld-musl-i386.path \
    && rm -rf /sjacket/etc/abuild.conf \
    && rm -rf /sjacket/etc/abuild.conf.orig \
    && rm -rf /sjacket/etc/apk \
    && rm -rf /sjacket/apk \
    && rm -rf /sjacket/lib/apk

RUN rm /sjacket/stensal/bin/klaram.exe \
    && rm /sjacket/stensal/bin/clang-3.5 \
    && rm /sjacket/stensal/bin/stensal-nm \
    && rm /sjacket/stensal/bin/license.json

#RUN apk add --update \
#    bash \
#  && rm -rf /var/cache/apk/*


EXPOSE 7681

COPY run_in_cattlegrid.sh  /bin/run_in_cattlegrid.sh
COPY sh_in_cattlegrid.sh  /bin/sh_in_cattlegrid.sh
COPY gdb.sh /bin/gdb.sh
COPY cattlegrid  /bin/cattlegrid

COPY prlimit     /bin/prlimit
COPY ttyd  /bin/ishell
COPY dumb-init /bin/
COPY start.sh /bin

RUN  mkdir /ishell_bin && \
     cp /bin/sh   /ishell_bin && \
     cp /usr/bin/env /ishell_bin && \
     cp /bin/bash /ishell_bin && \
     cp /bin/ls   /ishell_bin && \
     cp /usr/bin/clear /ishell_bin && \
     cp /bin/more /ishell_bin

COPY run_prog.sh /ishell_bin/
COPY greeting.sh /ishell_bin/
COPY entrypoint.sh /bin/
RUN chmod -R -w /ishell_bin/


RUN chmod -w /ishell_bin /lib /usr/lib /usr/local /usr/share
RUN adduser -s /ishell_bin/sh -h /home/jail -D -H tester -u 2000 -G root

ENTRYPOINT ["/bin/dumb-init", "--", "/bin/entrypoint.sh"]
