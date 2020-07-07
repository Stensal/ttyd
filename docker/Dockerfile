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

COPY run_in_cattlegrid.sh  /sbin/run_in_cattlegrid.sh
COPY sh_in_cattlegrid.sh   /sbin/sh_in_cattlegrid.sh
COPY gdb.sh                /sbin/gdb.sh
COPY cattlegrid            /sbin/cattlegrid

COPY prlimit               /sbin/prlimit
COPY ttyd                  /sbin/ishell
COPY dumb-init             /sbin/
COPY start.sh              /sbin

RUN  mkdir -p /xshell/bin && \
     cp /bin/sh   /xshell/bin && \
     cp /usr/bin/env /xshell/bin && \
     cp /bin/bash /xshell/bin && \
     cp /bin/ls   /xshell/bin && \
     cp /bin/rm   /xshell/bin && \
     cp /bin/mv   /xshell/bin && \
     cp /usr/bin/clear /xshell/bin && \
     cp /bin/more /xshell/bin

COPY pbd  /xshell/bin/
COPY run_prog.sh /xshell/bin/
COPY greeting.sh /xshell/bin/
COPY entrypoint.sh /sbin/
RUN chmod -R -w /xshell/bin/
RUN chmod -R -w /bin

RUN chmod -w /xshell/bin /lib /usr/lib /usr/local /usr/share
RUN adduser -s /xshell/bin/sh -h /home/jail -D -H tester -u 2000 -G root

ENTRYPOINT ["/sbin/dumb-init", "--", "/sbin/entrypoint.sh"]
