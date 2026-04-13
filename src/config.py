import numpy as np


source_width = 1296 # 0 - 1920
source_height = 972 # 0 - 1080

speed = 50 # 0 - 100
kp = 0.4 # >0
ki = 0.0 # >0
kd = 0.0 # >0
kdef = 0.0 # -1 - 1

## Lane Finder

lane_finder_source_width = 640 # 0 - source_width
lane_finder_source_height = 480 # 0 - source_height
lane_finder_angle_weights = [0, 3, 2, 1, 0]

lane_finder_transform_source_points = [
    # [0, 0],
    # [1, 0],
    # [0, 1],
    # [1, 1],
    [0, 0],
    [1, 0],
    [0, 1],
    [1, 1],
]
lane_finder_transform_dest_points = [
    # [0, 0],
    # [1, 0],
    # [0, 1],
    # [1, 1],
    [0, 0],
    [1, 0],
    [0.3, 1],
    [0.7, 1],
]

lane_finder_search_interval = 35 # >0
lane_finder_search_range = 460 # > 20% of image size
lane_finder_search_points = 8 # > 5

## canny
lane_finder_canny_min = 200
lane_finder_canny_max = 250

## hough
lane_finder_hough_theta= np.pi/180
lane_finder_hough_thresh = 50
lane_finder_hough_min_line_length = 10
lane_finder_hough_max_line_gap = 50
