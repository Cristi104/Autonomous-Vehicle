import os
import cv2 as cv
import copy
import numpy as np
import math

# Load images
directory_path = "./data2/"
directory_files = sorted(os.listdir(directory_path))

valid_ext = ('.png', '.jpg', '.jpeg', '.bmp', '.tif', '.tiff')
directory_files = [f for f in directory_files if f.lower().endswith(valid_ext)]

imgs = []
valid_files = []

directory_files = directory_files[21:]

for file in directory_files:
    image = cv.imread(os.path.join(directory_path, file))
    if image is not None:
        imgs.append(image)
        valid_files.append(file)

print(f"Loaded {len(imgs)} images.")

pattern_size = (5, 5)

corners = [cv.findChessboardCorners(img, pattern_size) for img in imgs]
corners_copy = copy.deepcopy(corners)

for file, cor in zip(valid_files, corners):
    print(f"{file}: {'OK' if cor[0] else 'FAIL'}")

imgs_gray = [cv.cvtColor(img, cv.COLOR_BGR2GRAY) for img in imgs]

criteria = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 30, 0.01)

corners_refined = [
    cv.cornerSubPix(gray, cor[1], (8, 6), (-1, -1), criteria) if cor[0] else []
    for gray, cor in zip(imgs_gray, corners_copy)
]

imgs_original = copy.deepcopy(imgs)
imgs_with_corners = copy.deepcopy(imgs)

_ = [
    cv.drawChessboardCorners(img, pattern_size, cor[1], cor[0])
    for img, cor in zip(imgs_with_corners, corners)
    if cor[0]
]
# for i in range(len(imgs_with_corners)):
#
#     cv.imshow("test", cv.cvtColor(imgs_original[i], cv.COLOR_BGR2RGB))
#     cv.waitKey(0)
#     cv.imshow("test", cv.cvtColor(imgs_with_corners[i], cv.COLOR_BGR2RGB))
#     cv.waitKey(0)

def get_chessboard_points(chessboard_shape, dx, dy):
    return [
        [(i % chessboard_shape[0]) * dx,
         (i // chessboard_shape[0]) * dy,
         0]
        for i in range(np.prod(chessboard_shape))
    ]

square_size = 21  # millimeters

cb_points = get_chessboard_points(pattern_size, square_size, square_size)
cb_points[:10]

valid_corners = [cor for found, cor in zip([c[0] for c in corners], corners_refined) if found]
num_valid_images = len(valid_corners)

print(f"Number of valid calibration images: {num_valid_images}")

real_points = get_chessboard_points(pattern_size, square_size, square_size)

object_points = np.asarray(
    [real_points for _ in range(num_valid_images)],
    dtype=np.float32
)

image_points = np.asarray(valid_corners, dtype=np.float32)

image_size = (imgs[0].shape[1], imgs[0].shape[0])

rms, intrinsics, dist_coeffs, rvecs, tvecs = cv.calibrateCamera(
    object_points,
    image_points,
    image_size,
    None,
    None
)

extrinsics = [
    np.hstack((cv.Rodrigues(rvec)[0], tvec))
    for rvec, tvec in zip(rvecs, tvecs)
]

np.savez('calib_left', intrinsic=intrinsics, extrinsic=extrinsics)

print("Intrinsic matrix K:\n", intrinsics)
print("\nDistortion coefficients:\n", dist_coeffs)
print("\nRMS reprojection error:\n", rms)

def py_ang(v1, v2):
    cosang = np.dot(v1, v2)
    sinang = np.linalg.norm(np.cross(v1, v2))
    return np.arctan2(sinang, cosang)

h, w = imgs[0].shape[:2]

v1 = np.linalg.inv(intrinsics) @ np.array([0, 0, 1])
v2 = np.linalg.inv(intrinsics) @ np.array([w, h, 1])

print("Diagonal FOV: {:.2f} degrees".format(math.degrees(py_ang(v1, v2))))

# img = imgs[-1]

img = cv.imread("./test.png")
undistorted = cv.undistort(img, intrinsics, dist_coeffs)
cv.imshow("test", cv.cvtColor(img, cv.COLOR_BGR2RGB))
cv.waitKey(0)
cv.imshow("test", cv.cvtColor(undistorted, cv.COLOR_BGR2RGB))
cv.waitKey(0)
img = undistorted
h_src, w_src = img.shape[:2]
h_dest, w_dest = img.shape[:2]

# Whole source frame
src_pts = np.float32([
    [0, 0],              # top-left
    [w_src - 1, 0],      # top-right
    [w_src - 1, h_src - 1],  # bottom-right
    [0, h_src - 1]       # bottom-left
])

# Region inside destination where source should be projected
dst_pts = np.float32([
    [0, 0],              # top-left
    [w_src - 1, 0],      # top-right
    [int((w_src - 1) * 0.86), h_src - 1],  # bottom-right
    [int((w_src -1) * 0.14), h_src -1]       # bottom-left
])

# Transform whole source -> part of destination
H = cv.getPerspectiveTransform(src_pts, dst_pts)

# Warp into full destination-size canvas
img = cv.warpPerspective(img, H, (w_dest, h_dest))
cv.imshow("test", img)
cv.waitKey(0)
