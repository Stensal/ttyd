#!/bin/bash

docker run --rm -it -v $(pwd):/build ttyd-static-docker-builder /bin/bash -c "cd /build/cmake-build-debug && make"
