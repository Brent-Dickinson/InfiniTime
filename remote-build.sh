#!/usr/bin/env bash
set -e
LINUX=brent@192.168.1.228
REMOTE=~/InfiniTime

# Sync source, but do NOT touch submodules under src/libs
# Use -rt instead of -a to avoid permission/owner churn from /mnt/c.
rsync -rtvi --delete \
  --exclude 'libs/**' \
  ./src/ "$LINUX:$REMOTE/src/"

ssh -t "$LINUX" "cd $REMOTE && sudo docker run --rm -v \$(pwd):/sources infinitime-build"

