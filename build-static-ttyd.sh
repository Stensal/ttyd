#!/bin/bash

docker run --rm -it -v $(pwd):/build dockerhub.stensalinc.com/x86_64/ttyd-static-docker-builder:latest /bin/bash \
    -c "cd /build/cmake-build-debug && rm -rf * && cmake -DCMAKE_BUILD_TYPE=Release -DSTATIC_TTYD=ON .. && make"
