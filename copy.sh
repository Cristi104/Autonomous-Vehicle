#!/bin/bash

# Usage: ./script.sh user@host folder_name

REMOTE="$1"
FOLDER="$2"

if [ -z "$REMOTE" ] || [ -z "$FOLDER" ]; then
  echo "Usage: $0 user@host folder_name"
  exit 1
fi

# Optional: remove local copy safely
if [ -d "./$FOLDER" ]; then
  rm -rf "./$FOLDER"
fi

# Copy from remote
scp -r "$REMOTE:/home/cristi/Git/Autonomous-Vehicle/$FOLDER" "./$FOLDER"
