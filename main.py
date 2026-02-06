import numpy as np
import cv2 as cv
from fastapi import FastAPI, WebSocket
from starlette.websockets import WebSocketState
import uvicorn
import base64
import asyncio
import threading
from picamera2 import Picamera2
import io
# from contextlib import redirect_stdout
from controller import Controller
import time
import math

from extern.pybind11.docs.conf import latex_engine, primary_domain

cam = Picamera2()

width = 1024
height = 720
cam.configure(cam.create_video_configuration(main={"format": "XRGB8888", "size": (width, height)}))

cam.start()
frameMutex = threading.Lock()
frame = cam.capture_array()

f = io.StringIO()

app = FastAPI()

@app.websocket("/ws/video")
async def video_stream(websocket: WebSocket):
    await websocket.accept()

    try:
        while True:
            global frame

            # frameMutex.aquire()
            with frameMutex:
                _, buffer = cv.imencode(".jpg", frame)
            # frameMutex.release()
                jpg_as_text = base64.b64encode(buffer).decode("utf-8")

                await websocket.send_text(jpg_as_text)
                await asyncio.sleep(0.03)  # ~30 FPS

    except Exception:
        pass
    finally:
        if websocket.application_state == WebSocketState.CONNECTED:
            await websocket.close()

# @app.websocket("/ws/log")
# async def log(websocket: WebSocket):
#     await websocket.accept()
#
#     try:
#         while True:
#             global f
#             websocket.send_text(f.getValue())
#         # await websocket.send_text("test\n\r")
#             await asyncio.sleep(1)
#         # await websocket.send_text("test\n\r")
#         # await asyncio.sleep(1)
#
#
#     except Exception:
#         pass
#     finally:
#         if websocket.application_state == WebSocketState.CONNECTED:
#             await websocket.close()

def run_api():
    uvicorn.run(app, host="0.0.0.0", port=8000)

threading.Thread(target=run_api, daemon=True).start()
contr = Controller()
contr.setSpeed(70)
contr.setPID(0.3, 0.1, 0.1, -0.03)

contr.setDistanceThresh(40)
contr.startThread()
time.sleep(5)

def component_orientation(mask):
    ys, xs = np.where(mask > 0)
    coords = np.column_stack((xs, ys)).astype(np.float32)

    if len(coords) < 10:
        return None, None

    mean = coords.mean(axis=0)
    _, _, vt = np.linalg.svd(coords - mean)

    direction = vt[0]  # principal axis (unit vector)
    return mean, direction


def getPointAlongLine(x, y, theta, distance):
    return [int(x + distance * math.cos(theta)), int(y + distance * math.sin(theta))]

def getFirstWhitePoint(x, y, theta, length):
    for i in range(length):
        nx = int(x + i * math.cos(theta))
        ny = int(y + i * math.sin(theta))

        if not (0 <= nx < 640) or not (0 <= ny < 480):
            break

        if frame[ny][nx]:
            return [nx, ny]
    return [-1, -1]


try:
    pidQueue = []
    while True:
        with frameMutex:
            frame = cam.capture_array()
            frame = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)
            frame = cv.rotate(frame, cv.ROTATE_180)
            frame = (255-frame)

            pts1 = np.float32([[width * (0/640), height * (90/480)], [width, height * (90/480)], [0, height], [width * (500/640), height]])
            pts2 = np.float32([[0, 0], [640, 0], [300, 480], [340, 480]])

            M = cv.getPerspectiveTransform(pts1, pts2)
            frame = cv.warpPerspective(frame, M, (640, 480))
            # frame = cv.fastNlMeansDenoising(frame, None, 3, 7, 21)
            frame = cv.adaptiveThreshold(frame, 255, cv.ADAPTIVE_THRESH_MEAN_C, cv.THRESH_BINARY, 11, -2)
            cv.line(frame, (300, 480), (0, 0), (0, 0, 0), 10)
            cv.line(frame, (350, 480), (640, 0), (0, 0, 0), 10)
            kernel = np.ones((5, 5), np.uint8)
            frame = cv.erode(frame, kernel)
            kernel = np.ones((5, 5), np.uint8)
            frame = cv.dilate(frame, kernel)
            kernel = np.ones((9, 9), np.uint8)
            # cv.rectangle(frame, (0, 480), (640, 475), (255, 255, 255), -1)
            # cv.rectangle(frame, (0, 480), (260, 420), (255, 255, 255), -1)
            # cv.rectangle(frame, (400, 480), (640, 420), (255, 255, 255), -1)
            # cv.rectangle(frame, (0, 480), (20, 0), (255, 255, 255), -1)
            # cv.rectangle(frame, (620, 480), (640, 0), (255, 255, 255), -1)
            # cv.rectangle(frame, (0, 0), (640, 20), (255, 255, 255), -1)
            # cv.rectangle(frame, (250, 410), (260, 390), (255, 255, 255), -1)
            # cv.rectangle(frame, (360, 410), (370, 390), (255, 255, 255), -1)
            cv.rectangle(frame, (250, 440), (260, 420), (255, 255, 255), -1)
            cv.rectangle(frame, (360, 440), (370, 420), (255, 255, 255), -1)
            cv.rectangle(frame, (250, 480), (260, 450), (255, 255, 255), -1)
            cv.rectangle(frame, (360, 480), (370, 450), (255, 255, 255), -1)
            # cv.rectangle(frame, (0, 440), (240, 420), (255, 255, 255), -1)
            # cv.rectangle(frame, (420, 440), (640, 420), (255, 255, 255), -1)
            # # kernel = cv.getStructuringElement(cv.MORPH_RECT, (95, 95))
            frame = cv.morphologyEx(frame, cv.MORPH_CLOSE, kernel)
            num, labels, stats, centroids = cv.connectedComponentsWithStats(frame)
            # print(num, labels, stats, centroids)


            for i in range(len(centroids)):
                # if stats[i][4] < 70:
                #     continue
                c1 = centroids[i]
                if not frame[int(c1[1])][int(c1[0])]:
                    continue
                for j in range(i + 1, len(centroids)):
                    c2 = centroids[j]
                    # if stats[j][4] < 70:
                    #     continue
                    if(math.sqrt((c1[0] - c2[0]) ** 2 + (c1[1] - c2[1]) ** 2) < 70):
                        cv.line(frame, [int(c1[0]), int(c1[1])], [int(c2[0]), int(c2[1])], (255, 0, 0), 3)


            leftPoints = [[300, 480], [300, 475]]
            rightPoints = [[320, 480], [320, 475]]
            midPoints = [[310, 480], [310, 475]]
            pointInterval = 40
            searchRange = 130
            points = 12
            angles = []
            dx = midPoints[-2][0] - midPoints[-1][0]
            dy = midPoints[-2][1] - midPoints[-1][1]
            angle = math.atan2(dy, dx) - math.pi
            angle = min(-math.pi / 4, max(angle, -3 * math.pi / 4))
            angles.append(angle)
            # print("start")
            for i in range(points):
                leftPoint = None
                rightPoint = None
                whitePoint = getFirstWhitePoint(midPoints[-1][0], midPoints[-1][1], angles[-1], pointInterval)
                if whitePoint[0] == -1:
                    leftPoint = getPointAlongLine(midPoints[-1][0], midPoints[-1][1], angles[-1], pointInterval)
                    rightPoint = leftPoint
                else:
                    leftPoint = getPointAlongLine(whitePoint[0], whitePoint[1], angles[-1] + math.pi, 5)
                    rightPoint = leftPoint

                # print(angles)
                # print(leftPoint, rightPoint)

                leftPoint = getFirstWhitePoint(leftPoint[0], leftPoint[1], angles[-1] - math.pi/2, searchRange)
                rightPoint = getFirstWhitePoint(rightPoint[0], rightPoint[1], angles[-1] + math.pi/2, searchRange)

                if rightPoint[0] != -1:
                    rightPoints.append(rightPoint)
                if leftPoint[0] != -1:
                    leftPoints.append(leftPoint)

                if min(len(leftPoints), len(rightPoints)) >= 1:
                    midPoints.append([int((leftPoints[-1][0] + rightPoints[-1][0]) / 2), int((leftPoints[-1][1] + rightPoints[-1][1]) / 2)])
                if len(midPoints) >= 2:
                    dx = midPoints[-2][0] - midPoints[-1][0]
                    dy = midPoints[-2][1] - midPoints[-1][1]
                    angle = math.atan2(dy, dx) - math.pi
                    angle = min(0, max(angle, -math.pi))
                    angles.append(angle)


            # print(angles)
            # print(midPoints)

            if len(angles) >= 2:
                # pidQueue.append(-(angles[1] + math.pi / 2)/5)
                anglesToAvg = 4
                avgAngle = sum([-(angles[i] + math.pi / 2) * (anglesToAvg - i) / anglesToAvg  for i in range(min(len(angles), anglesToAvg))]) / min(len(angles), anglesToAvg)
                pidQueue.append(avgAngle)
            if len(pidQueue) <= 0:
                contr.pid(0)
            else:
                contr.pid(pidQueue.pop(0))
            frame = cv.cvtColor(frame, cv.COLOR_GRAY2BGR)
            for i in range(points):
                for p in leftPoints:
                    cv.circle(frame, p, 5, (255, 0, 0), -1)
                for p in rightPoints:
                    cv.circle(frame, p, 5, (0, 255, 0), -1)
                for p in centroids:
                    cv.circle(frame, [int(p[0]), int(p[1])], 5, (255, 255, 0), -1)
                for j in range(len(midPoints) - 1):
                    cv.line(frame, midPoints[j], midPoints[j + 1], (0, 0, 255), 1)
                    # cv.circle(frame, (int((leftPoints[j][0] + rightPoints[j][0]) / 2), int((leftPoints[j][1] + rightPoints[j][1]) / 2)), 5, (255, 0, 0), -1)
                # cv.circle(frame, leftPoint, 5, (255, 0, 0), -1)
                # cv.circle(frame, rightPoint, 5, (255, 0, 0), -1)
                # cv.line(frame, leftPoint, (leftPoint[0] - searchRange, leftPoint[1]), (255, 0, 0), 1)
                # cv.line(frame, rightPoint, (rightPoint[0] + searchRange, rightPoint[1]), (255, 0, 0), 1)
                # cv.line(frame, leftPoint, (leftPoint[0] - searchRange, leftPoint[1] + searchRange), (255, 0, 0), 1)
                # cv.line(frame, rightPoint, (rightPoint[0] + searchRange, rightPoint[1] + searchRange), (255, 0, 0), 1)
                # cv.line(frame, leftPoint, (leftPoint[0] - searchRange, leftPoint[1] - searchRange), (255, 0, 0), 1)
                # cv.line(frame, rightPoint, (rightPoint[0] + searchRange, rightPoint[1] - searchRange), (255, 0, 0), 1)

            # frame = cv.Canny(frame, 100, 200)


except KeyboardInterrupt:
    pass
finally:
    contr.stopThread()
