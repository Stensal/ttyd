#!/bin/bash
docker build -f Dockerfile-static . -t dockerhub.stensalinc.com/x86_64/ttyd-static-docker-builder
docker push dockerhub.stensalinc.com/x86_64/ttyd-static-docker-builder:latest
