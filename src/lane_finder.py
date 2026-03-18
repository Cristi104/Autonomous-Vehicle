import numpy as np
import cv2 as cv
import math
from src import config
import copy
from controller import Controller
import time

class lane_finder:
    def __init__(self):
        self.source_frame = np.ones((480, 640), dtype=np.uint8)
        self.lanes = []
        self.pidQueue = []
        self.contr = Controller()
        self.contr.setSpeed(config.speed)
        self.contr.setPID(config.kp, config.ki, config.kd, config.kdef)

        self.contr.setDistanceThresh(40)
        self.contr.startThread()
        time.sleep(5)

    def stop_controller(self):
        self.contr.stopThread()

    def getPointAlongLine(self, x, y, theta, distance):
        return [int(x + distance * math.cos(theta)), int(y + distance * math.sin(theta))]

    def getFirstWhitePoint(self, x, y, theta, length):
        for i in range(length):
            nx = int(x + i * math.cos(theta))
            ny = int(y + i * math.sin(theta))

            if not (0 <= nx < config.lane_finder_source_width) or not (0 <= ny < config.lane_finder_source_height):
                break

            if self.source_frame[ny][nx]:
                return [nx, ny]
        return [-1, -1]

    def process_image(self, image):
        self.source_frame = copy.deepcopy(image)
        self.source_frame = (255-self.source_frame)

        ## building the "top-down" view
        pts1 = np.float32([
            [config.source_width * config.lane_finder_transform_source_points[0][0], config.source_height * config.lane_finder_transform_source_points[0][1]],
            [config.source_width * config.lane_finder_transform_source_points[1][0], config.source_height * config.lane_finder_transform_source_points[1][1]],
            [config.source_width * config.lane_finder_transform_source_points[2][0], config.source_height * config.lane_finder_transform_source_points[2][1]],
            [config.source_width * config.lane_finder_transform_source_points[3][0], config.source_height * config.lane_finder_transform_source_points[3][1]],
        ])
        pts2 = np.float32([
            [config.lane_finder_source_width * config.lane_finder_transform_dest_points[0][0], config.lane_finder_source_height * config.lane_finder_transform_dest_points[0][1]],
            [config.lane_finder_source_width * config.lane_finder_transform_dest_points[1][0], config.lane_finder_source_height * config.lane_finder_transform_dest_points[1][1]],
            [config.lane_finder_source_width * config.lane_finder_transform_dest_points[2][0], config.lane_finder_source_height * config.lane_finder_transform_dest_points[2][1]],
            [config.lane_finder_source_width * config.lane_finder_transform_dest_points[3][0], config.lane_finder_source_height * config.lane_finder_transform_dest_points[3][1]],
        ])

        M = cv.getPerspectiveTransform(pts1, pts2)
        self.source_frame = cv.warpPerspective(self.source_frame, M, (config.lane_finder_source_width, config.lane_finder_source_height))
        # return self.source_frame
        self.source_frame = cv.adaptiveThreshold(self.source_frame, 255, cv.ADAPTIVE_THRESH_MEAN_C, cv.THRESH_BINARY, 11, -2)

        ## removing edges created by thresholding the vision cone
        cv.line(self.source_frame,
                [int(config.lane_finder_source_width * config.lane_finder_transform_dest_points[2][0]), int(config.lane_finder_source_height * config.lane_finder_transform_dest_points[2][1])],
                [int(config.lane_finder_source_width * config.lane_finder_transform_dest_points[0][0]), int(config.lane_finder_source_height * config.lane_finder_transform_dest_points[0][1])],
                (0, 0, 0), 10)
        cv.line(self.source_frame,
                [int(config.lane_finder_source_width * config.lane_finder_transform_dest_points[3][0]), int(config.lane_finder_source_height * config.lane_finder_transform_dest_points[3][1])],
                [int(config.lane_finder_source_width * config.lane_finder_transform_dest_points[1][0]), int(config.lane_finder_source_height * config.lane_finder_transform_dest_points[1][1])],
                (0, 0, 0), 10)

        ## noise removal
        kernel = np.ones((5, 5), np.uint8)
        self.source_frame = cv.erode(self.source_frame, kernel)
        kernel = np.ones((5, 5), np.uint8)
        self.source_frame = cv.dilate(self.source_frame, kernel)
        kernel = np.ones((9, 9), np.uint8)

        cv.rectangle(self.source_frame,
                     [int(config.lane_finder_source_width * 0.14), int(config.lane_finder_source_height * 0.85)],
                     [int(config.lane_finder_source_width * 0.15), int(config.lane_finder_source_height * 0.80)],
                     (255, 255, 255), -1)
        cv.rectangle(self.source_frame,
                     [int(config.lane_finder_source_width * 0.85), int(config.lane_finder_source_height * 0.85)],
                     [int(config.lane_finder_source_width * 0.86), int(config.lane_finder_source_height * 0.80)],
                     (255, 255, 255), -1)
        cv.rectangle(self.source_frame,
                     [int(config.lane_finder_source_width * 0.19), int(config.lane_finder_source_height * 1)],
                     [int(config.lane_finder_source_width * 0.20), int(config.lane_finder_source_height * 0.90)],
                     (255, 255, 255), -1)
        cv.rectangle(self.source_frame,
                     [int(config.lane_finder_source_width * 0.80), int(config.lane_finder_source_height * 1)],
                     [int(config.lane_finder_source_width * 0.81), int(config.lane_finder_source_height * 0.90)],
                     (255, 255, 255), -1)

        ## bridging gaps between lane markings
        self.source_frame = cv.morphologyEx(self.source_frame, cv.MORPH_CLOSE, kernel)
        num, labels, stats, centroids = cv.connectedComponentsWithStats(self.source_frame)

        for i in range(len(centroids)):
            # if stats[i][4] < 70:
            #     continue
            c1 = centroids[i]
            if not self.source_frame[int(c1[1])][int(c1[0])]:
                continue
            for j in range(i + 1, len(centroids)):
                c2 = centroids[j]
                # if stats[j][4] < 70:
                #     continue
                if(math.sqrt((c1[0] - c2[0]) ** 2 + (c1[1] - c2[1]) ** 2) < 70):
                    cv.line(self.source_frame, [int(c1[0]), int(c1[1])], [int(c2[0]), int(c2[1])], (255, 0, 0), 3)


        leftPoints = [
            [int(config.lane_finder_source_width * 0.40), int(config.lane_finder_source_height * 1)],
            [int(config.lane_finder_source_width * 0.40), int(config.lane_finder_source_height * 0.99)],
        ]
        rightPoints = [
            [int(config.lane_finder_source_width * 0.60), int(config.lane_finder_source_height * 1)],
            [int(config.lane_finder_source_width * 0.60), int(config.lane_finder_source_height * 0.99)],
        ]
        midPoints = [
            [int(config.lane_finder_source_width * 0.50), int(config.lane_finder_source_height * 1)],
            [int(config.lane_finder_source_width * 0.50), int(config.lane_finder_source_height * 0.99)],
        ]
        angles = []
        dx = midPoints[-2][0] - midPoints[-1][0]
        dy = midPoints[-2][1] - midPoints[-1][1]
        angle = math.atan2(dy, dx) - math.pi
        angle = min(-math.pi / 4, max(angle, -3 * math.pi / 4))
        angles.append(angle)

        tries_without_finds = 0

        for i in range(config.lane_finder_search_points):
            leftPoint = None
            rightPoint = None
            tries_without_finds += 1
            # print(tries_without_finds, midPoints[-1], angles[-1])
            whitePoint = self.getFirstWhitePoint(midPoints[-1][0], midPoints[-1][1], angles[-1], config.lane_finder_search_interval * tries_without_finds)
            if whitePoint[0] == -1:
                leftPoint = self.getPointAlongLine(midPoints[-1][0], midPoints[-1][1], angles[-1], config.lane_finder_search_interval * tries_without_finds)
                rightPoint = leftPoint
            else:
                leftPoint = self.getPointAlongLine(whitePoint[0], whitePoint[1], angles[-1] + math.pi, 5)
                rightPoint = leftPoint


            leftPoint = self.getFirstWhitePoint(leftPoint[0], leftPoint[1], angles[-1] - math.pi/2, config.lane_finder_search_range)
            rightPoint = self.getFirstWhitePoint(rightPoint[0], rightPoint[1], angles[-1] + math.pi/2, config.lane_finder_search_range)

            if rightPoint[0] != -1:
                rightPoints.append(rightPoint)
                tries_without_finds = 0
            if leftPoint[0] != -1:
                leftPoints.append(leftPoint)
                tries_without_finds = 0

            if min(len(leftPoints), len(rightPoints)) >= 1:
                midPoints.append([int((leftPoints[-1][0] + rightPoints[-1][0]) / 2), int((leftPoints[-1][1] + rightPoints[-1][1]) / 2)])
            if len(midPoints) >= 2:
                dx = midPoints[-2][0] - midPoints[-1][0]
                dy = midPoints[-2][1] - midPoints[-1][1]
                angle = math.atan2(dy, dx) - math.pi
                angle = min(-math.pi / 4, max(angle, -3 * math.pi / 4))
                angles.append(angle)

        # if len(angles) >= 2:
        #     # pidQueue.append(-(angles[1] + math.pi / 2)/5)
        #     anglesToAvg = 4
        #     avgAngle = sum([-(angles[i] + math.pi / 2) * (anglesToAvg - i) / anglesToAvg  for i in range(min(len(angles), anglesToAvg))]) / min(len(angles), anglesToAvg)
        #     pidQueue.append(avgAngle)
        # if len(pidQueue) <= 0:
        #     contr.pid(0)
        # else:
        first_angle = 1
        last_angle = 4
        angles = []
        for i in range(len(midPoints) - 1):
            dx = midPoints[i][0] - midPoints[i+1][0]
            dy = midPoints[i][1] - midPoints[i+1][1]
            angle = math.atan2(dy, dx) - math.pi
            # angle = min(-math.pi / 4, max(angle, -3 * math.pi / 4))
            angles.append(angle)
        # avgAngle = sum([-(angles[i] + math.pi / 2) / anglesToAvg  for i in range(min(len(angles), anglesToAvg))]) / min(len(angles), anglesToAvg)
        # avg_angle = 0
        # weight = 0
        # for i in range(min(len(angles), len(config.lane_finder_angle_weights))):
        #     avg_angle += -(angles[i] + math.pi / 2) * config.lane_finder_angle_weights[i]
        #     weight += config.lane_finder_angle_weights[i]
        # avg_angle = avg_angle/weight
        avg_angle = sum([-(angles[i] + math.pi / 2) / (last_angle - first_angle)  for i in range(first_angle, min(len(angles), last_angle))])
        avg_angle = (avg_angle * 2 - (angles[1] + math.pi / 2) * 3) / 5
        print(avg_angle)
        self.pidQueue.append(avg_angle)
        if len(self.pidQueue) >= 6:
            self.contr.pid(self.pidQueue.pop(0))
        # self.contr.pid(avgAngle)

        ## display
        self.source_frame = cv.cvtColor(self.source_frame, cv.COLOR_GRAY2BGR)
        for i in range(config.lane_finder_search_points):
            for p in leftPoints:
                cv.circle(self.source_frame, p, 5, (255, 0, 0), -1)
            for p in rightPoints:
                cv.circle(self.source_frame, p, 5, (0, 255, 0), -1)
            for p in centroids:
                cv.circle(self.source_frame, [int(p[0]), int(p[1])], 5, (255, 255, 0), -1)
            for j in range(len(midPoints) - 1):
                cv.line(self.source_frame, midPoints[j], midPoints[j + 1], (0, 0, 255), 1)
        return self.source_frame
