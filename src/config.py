source_width = 1920 # 0 - 1920
source_height = 1080 # 0 - 1080

speed = 50 # 0 - 100
kp = 0.4 # >0
ki = 0.0 # >0
kd = 0.0 # >0
kdef = 0.0 # -1 - 1

## Lane Finder

lane_finder_source_width = 640 # 0 - source_width
lane_finder_source_height = 480 # 0 - source_height
lane_finder_angle_weights = [0, 1, 1, 1, 1]

lane_finder_transform_source_points = [
    # [0, 0],
    # [1, 0],
    # [0, 1],
    # [1, 1],
    [0, 0],
    [1, 0],
    [0, 0.9],
    [1, 0.9],
]
lane_finder_transform_dest_points = [
    # [0, 0],
    # [1, 0],
    # [0, 1],
    # [1, 1],
    [0, 0],
    [1, 0],
    [0.4, 1],
    [0.6, 1],
]

lane_finder_search_interval = 50 # >0
lane_finder_search_range = 130 # > 20% of image size
lane_finder_search_points = 12 # > 5




