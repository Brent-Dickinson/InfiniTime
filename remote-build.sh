#!/usr/bin/env bash
set -e
LINUX=brent@192.168.1.228
REMOTE=~/InfiniTime

rsync -av --delete ./src/ "$LINUX:$REMOTE/src/"
ssh -t brent@192.168.1.228 "cd ~/InfiniTime && sudo docker run --rm -v \$(pwd):/sources infinitime-build"
