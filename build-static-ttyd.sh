#!/bin/bash

echo "build ttyd .."
docker run --rm -it -v $(pwd):/build dockerhub.stensalinc.com/x86_64/ttyd-static-docker-builder:latest /bin/bash \
    -c "cd /build/static-build && rm -rf * && cmake -DCMAKE_BUILD_TYPE=Release -DSTATIC_TTYD=ON .. && make"


echo "build dumb-init .."
docker run --rm -it -v $(pwd):/build dockerhub.stensalinc.com/x86_64/ttyd-static-docker-builder:latest /bin/bash \
    -c "cd /build/dumb-init && gcc -static dumb-init.c -o dumb-init"
