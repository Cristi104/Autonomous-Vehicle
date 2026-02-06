# The Controller

The controller has the role of communicating to hardware on behalf of the main python app it can communicate to the following hardware:
- MPU6500 through the i2c-1 driver (i2c must be enabled in raspi-config)
- Ultrasonic distance sensor through pins 7 and 8 (gpiod digital pins)
- Motor drivers through pins 16, 19, 20, 26 (gpiod digital pins) and pins 12 and 13 (hardware pwm controlled by pwmchip0 directly must be enabled in /boot/firmware/config.txt)

The controller class is multithreaded, there is one controlThread that is constantly running to read and write from and to the hardware even if there is no commands from the API.

