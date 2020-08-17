# pheonix-project

## Base, functional software
Upload OnBoard.ino to the Moteino on the rocket.
Upload GroundStation.ino to the Moteino on the ground.
Upload GroundMega.ino to the Arduino Mega on the ground.

This set can transmit the data from one GPS on the rocket, as well as automatically control the motor to track the rocket GPS. However, the motor itself must be manually positioned to center 180 degrees at north or south. Adjust the motor centering on GroundMega accordingly by inputting a command.

## TwoEnds
Upload Rocket.ino to the Moteino on the rocket.
Upload Ground.ino to the Moteino on the ground.
Upload Mega.ino to the Arduino Mega on the ground.

NOTE: This software currently needs version 1.4.0 of the RFM69 library in order to compile.

This set sends one message from Rocket at a time, but can handle different data struct formats. Different messages can be requested by inputting a command to Mega, which will be sent to Rocket.

## combinedMessages
Upload rocketCombine.ino to the Moteino on the rocket.
Upload groundCombine.ino to the Moteino on the ground.
Upload megaCombine.ino to the Arduino Mega on the ground.

This set can send multiple messages from Rocket which are combined and displayed by Mega as one data package, but the messages are restricted to a single struct format.
