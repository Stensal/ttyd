#!/bin/bash
docker build . -f Dockerfile -t dockerhub.stensalinc.com/i386/ishell:latest
docker push dockerhub.stensalinc.com/i386/ishell:latest
docker image prune -f
