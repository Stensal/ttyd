FROM i386/alpine:3.6

RUN apk add --update \
    bash \
  && rm -rf /var/cache/apk/*

EXPOSE 7681

COPY run_in_cattlegrid.sh  /bin/run_in_cattlegrid.sh
COPY cattlegrid  /bin/cattlegrid
COPY prlimit     /bin/prlimit
COPY ./cmake-build-debug/ttyd  /bin/ishell

ENTRYPOINT ["ishell"]

CMD ["bash"]
