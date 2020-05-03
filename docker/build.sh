#!/bin/bash
docker build . -f Dockerfile -t dockerhub.stensalinc.com/i386/ishell:latest
#docker push dockerhub.stensalinc.com/i386/ishell:latest
docker image prune -f

docker build . -f Dockerfile_cs50 -t dockerhub.stensalinc.com/i386/ishell-cs50:latest
#docker push dockerhub.stensalinc.com/i386/ishell-cs50:latest
docker image prune -f
