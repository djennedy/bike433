#!/bin/bash
FILE_NAME=camera_output-$(date +"%d-%m-%Y")
DIR_OUTPUT=/mnt/remote/webcam/output/

if [ -f "$DIR_OUTPUT" ]; then
  echo "$DIR_OUTPUT exists."
else
  mkdir -r $DIR_OUTPUT
fi

ffmpeg -i /dev/shm/dash/live.mpd -vsync 2 -c:v -vcodec libx264  "/mnt/remote/webcam/output/$FILE_NAME.mp4"