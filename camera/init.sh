#!/bin/bash

FILE=/etc/systemd/system/bike_camera.service

  cp bike_camera.service /etc/systemd/system/bike_camera.service
  echo "$FILE copied to service"
  systemctl daemon-reload
#if [ -f "$FILE" ]; then
#  echo "$FILE exists."
#else
#
#fi

systemctl start bike_camera.service

node webcam_server/webcam.js