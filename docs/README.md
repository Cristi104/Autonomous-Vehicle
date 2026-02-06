# Overview of the application

The application is split into two main parts:

## The Controller

The Controller low level python binding written in C++ it manages sensors and motors through direct gpio and device driver access.
The only external dependencies used here are: 
- gpiod for reading and writing to gpio pins(digital pins only)
- pybind for compiling the code into a single .so file readable by python

## The computer vision app

The main python app that has the role of fetching camera data processing it and sending commands to the controller API.
Currently the computer vision part is only normal image processing through OpenCV no advanced/cutting edge image detection is used at the moment (planned for future versions)