#!/bin/bash
#Documentation:
# Text on video:
#   https://stackoverflow.com/questions/17623676/text-on-video-ffmpeg
#   https://ottverse.com/ffmpeg-drawtext-filter-dynamic-overlays-timecode-scrolling-text-credits/
# MPEG-DASH:
#   https://www.codeinsideout.com/blog/pi/stream-ffmpeg-hls-dash/#use-mpeg-dash-streaming

FILE=/dev/shm/dash/

if [ -f "$FILE" ]; then
  echo "$FILE exists."
else
  mkdir $FILE
fi


ffmpeg -y \
    -input_format mjpeg \
    -f video4linux2 \
    -use_wallclock_as_timestamps 1 \
    -copyts \
    -start_at_zero \
    -framerate 30 \
    -i /dev/video0 \
    -c:v copy \
    -preset ultrafast -tune zerolatency \
    -f dash \
    -ldash 1 \
    -seg_duration 1 \
    -frag_duration 1 \
    -streaming 1 \
    -window_size 300000 -remove_at_exit 1 \
    -strict experimental -lhls 1 \
    -write_prft 1 \
    -target_latency 1 \
    /dev/shm/dash/live.mpd


#ffmpeg -i /dev/shm/dash/live.mpd \
#  "/mnt/remote/webcam_server/output/output.mjpeg"

#ffmpeg -i /dev/shm/dash/live.mpd -vsync 2 -c:v mjpeg  "/mnt/remote/webcam/output/output.mjpeg"