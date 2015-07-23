#!/bin/bash
gst-launch multifilesrc location=preview_000150.yuv ! "video/x-raw-yuv, format=(fourcc)YV12, width=640, height=480, framerate=(fraction)0/1" ! ffmpegcolorspace ! autovideosink

