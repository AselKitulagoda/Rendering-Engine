ffmpeg -framerate 40 -i rasteriser_frames/%d.ppm -c:v libx264 -profile:v high -crf 18 -pix_fmt yuv420p -q 4 rasteriser.mp4
