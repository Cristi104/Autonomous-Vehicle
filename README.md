# Autonomous Vehicle
Autonomous vehicle is a robotics project that tries to recreate the self-driving features present on modern cars (Tesla, BYD and others), with the goal of creating a robot car capable of SAE Level 2/3 automation.

## Parts Used
- Raspberry Pi 4B
- Raspberry Pi compatible camera
- HC-SR04 ultrasonic distance sensor
- MPU6500 6-axis accelerometer and gyrroscope
- 2x TC1508A 2 motor driver
- 4x DC motor with 48:1 gearbox and wheel
- 12A DC-DC adjustable buck converter
- 2 cell 18650 li-ion battery holder
- 2x 18650 3.6V 2600mAh li-ion battery
- 3D printed baseplate (https://www.printables.com/model/186945-4wd-buggy-chassis-arduino/files)
- cabels and connectors as needed

### Q1 - What is the system boundry?
The Raspberry Pi with the above sensors and camera it may also receive simple commands (such as GO, STOP, etc.) over wifi from an external device connected to the wifi hotspot of the pi.

### Q2 - Where does the inteligence live?
Everything is processed and read by the Raspberry Pi.

### Q3 - What is the hardest technical problem?
Extracting data such as position on the road, intersections and others from the camera using computer vision.

### Q4 - What is the minimum demo?
A robot that moves along the middle of a lane and can do a simple 90 degree turn on its own.

### Q5 - Why is this not just a tutorial?
The addition of a camera and computer vision extends this project far past a simple tutorial/robotics kit and allows for more complex interactions such as overtakes or navigating intersections with odd shapes.