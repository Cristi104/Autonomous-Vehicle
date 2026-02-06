# The computer vision app

This is the main and most complex part of the whole system it has the role of reading camera data and processing it to get data such as position on the lane, necessary turns and obstacles that can be avoided (to be added)

Currently, the lane detection (and communication with the pid controller based on the detected lane) is the only implemented part here is how it is implemented:
- first a grayscale image is read from the camera
- an afine transformation is applied to get a top-down view of the lane
- a mean adaptive threshold is applied
- noise is reduced by using erosion dilation and closing the result is a top-down view of the front lane of the car with white pixels on parts where a lane marking is and black everywhere else
- dotted lines are merged by drawing lines to nearby centroids detected
- now we have 2 lines one on the left and one on the right we can detect the middle of the lane by sampling dots until we get a white pixel on the left one on the right, and we can get the middle point.
- following middle points are searched by using the angle created by the last 2 middle points so that the point searching adapts to the road direction
- using multiple middle point we can get the turning angle/robot offset from the middle of the road, based on which we can send steering commands to the PID of the controller API.