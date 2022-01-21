#!/bin/bash
ffmpeg  -framerate 60 -start_number 0 -i ./composite_pngs/composited_img%d.png -c:v vp9 -crf 9 -b:v 5M -vf "format=rgba" ./final_output_movie/unitTest.webm

