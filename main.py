import numpy as np
import cv2 as cv
from picamera2 import Picamera2
import threading
import io
import time
from src import config
from src.lane_finder import lane_finder
from src.web_API import web_API
from controller import Controller

from extern.pybind11.docs.conf import latex_engine, primary_domain

cam = Picamera2()

cam.configure(cam.create_video_configuration(main={"format": "XRGB8888", "size": (config.source_width, config.source_height)}))

cam.start()
frameMutex = threading.Lock()
frame = cam.capture_array()

api = web_API()
lane = lane_finder()

contr = Controller()
contr.setSpeed(config.speed)
contr.setPID(config.kp, config.ki, config.kd, config.kdef)

contr.setDistanceThresh(40)
contr.startThread()
time.sleep(5)

enabled = False

def process_web_input():
    global enabled
    command = api.control_queue_pop().split()
    if command[0] != '[NONE]':
        print(command)
    match command[0]:
        case "[STOP]":
            enabled = False
            print(enabled)
            contr.forwardCm(1)
        case "[GO]":
            enabled = True
            print(enabled)
        case "[SPEED]":
            config.speed = int(command[1])
            contr.setSpeed(config.speed)
        case "[KP]":
            config.kp = float(command[1])
            contr.setPID(config.kp, config.ki, config.kd, config.kdef)
        case "[KI]":
            config.ki = float(command[1])
            contr.setPID(config.kp, config.ki, config.kd, config.kdef)
        case "[KD]":
            config.kd = float(command[1])
            contr.setPID(config.kp, config.ki, config.kd, config.kdef)
        case "[KDEF]":
            config.kdef = float(command[1])
            contr.setPID(config.kp, config.ki, config.kd, config.kdef)
        case "[CANNY MIN]":
            config.lane_finder_canny_min = int(command[1])
        case "[CANNY MAX]":
            config.lane_finder_canny_max = int(command[1])
        case "[HOUGH THETA]":
            config.lane_finder_hough_theta = float(command[1])
        case "[HOUGH THRESH]":
            config.lane_finder_hough_thresh = int(command[1])
        case "[HOUGH MIN LINE LENGTH]":
            config.lane_finder_hough_min_line_length = int(command[1])
        case "[HOUGH MAX LINE GAP]":
            config.lane_finder_hough_max_line_gap = int(command[1])
        case _:
            pass


try:
    pidQueue = []
    while True:
        frame = cam.capture_array()
        frame = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)
        frame = cv.rotate(frame, cv.ROTATE_180)
        process_web_input()
        # frame = cv.resize(frame, None, fx=0.5, fy=0.5)
        frame = lane.process_image(frame)
        if len(lane.pidQueue) >= 1:
            if enabled:
                contr.pid(lane.pidQueue.pop(0))
            else:
                lane.pidQueue.pop(0)
        api.send_image(frame)
        # frame = lane.process_image(frame)

except KeyboardInterrupt:
    pass
finally:
    contr.stopThread()
