#!/bin/bash

# Start the first process
./aberd -k
status=$?
if [ $status -ne 0 ]; then
  echo "Failed to start my_first_process: $status"
  exit $status
fi

while sleep 60; do
  ps aux |grep aberd |grep -q -v grep
  PROCESS_1_STATUS=$?
  # If the greps above find anything, they exit with 0 status
  # If they are not both 0, then something is wrong
  if [ $PROCESS_1_STATUS -ne 0]; then
    echo "aberd process has already exited."
    exit 1
  fi
done
