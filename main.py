import numpy as np
import cv2 as cv
from picamera2 import Picamera2
import threading
import io
import time
from src import config
from src.lane_finder import lane_finder
from src.web_API import web_API

from extern.pybind11.docs.conf import latex_engine, primary_domain

cam = Picamera2()

cam.configure(cam.create_video_configuration(main={"format": "XRGB8888", "size": (config.source_width, config.source_height)}))

cam.start()
frameMutex = threading.Lock()
frame = cam.capture_array()

api = web_API()
lane = lane_finder()

# contr = Controller()
# contr.setSpeed(70)
# contr.setPID(0.3, 0.1, 0.1, -0.03)
#
# contr.setDistanceThresh(40)
# contr.startThread()
# time.sleep(5)




try:
    pidQueue = []
    while True:
        frame = cam.capture_array()
        frame = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)
        frame = cv.rotate(frame, cv.ROTATE_180)
        # frame = cv.resize(frame, None, fx=0.5, fy=0.5)
        frame = lane.process_image(frame)
        api.send_image(frame)
        # frame = lane.process_image(frame)

except KeyboardInterrupt:
    pass
# finally:
#     contr.stopThread()
