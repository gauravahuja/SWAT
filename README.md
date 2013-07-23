SWAT,Semi Autonomous Robotics Competition - IIT, Guwahati 2009

The aim of the competition was to build a semi-autonomous robot which goes to specific locations (white circles), and using a mounted laser pointer and an overhead camera, shoots various targets(boxes) of different heights (marked by different colors). See Arena.jpg for more info.

I used opencv to process the images and locate the position and orientation of the robot. The robot had two red leds mounted on the top which could be controlled by the software running on the PC. The leds were turned on and off and this was used to figure out the orientation of the bot with respect to the circle it had to navigate to. The robot had an ATmega32 micro controller which handled most of the low level stuff, like controlling the motors, the LEDs, the laser and communication with the PC parallel port.

The code was written in C using Microsoft Visual Studio 2008 and OpenCV 1.0 SDK.

Folder Descriptions

1. Set Threshold
Contains code that would capture image from the overhead camera and help in setting the (RGB) threshold of the colours of the boxes in the arena. The thresholds are stored in a file which then needs to be passed on to the SWATMain executable.

2.SWATMain
Contains code that would control the robot throught the run. It takes in the threshold file as input.

3. SWATManualControl
Contains code that helps to manually control the robot and check all functions on the robot.

4. SWATMCU
Contains code for the ATmega32 micro controller onboard the robot. The code was written using the WinAVR library and compiled on AVRStudio.

Videos:

https://www.youtube.com/watch?v=hM6YZntjH0U

https://www.youtube.com/watch?v=EEb2jc-eafA
