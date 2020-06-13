# pheonix-project

##The simplest but most functional GPS-only software:
Upload OnBoard to the Moteino on the rocket.
Upload GroundStation to the Moteino on the ground.
Upload GroundMega to the Arduino Mega on the ground.

This set can transmit the data from one GPS on the rocket, as well as automatically control the motor to track the rocket GPS. However, the motor itself must be manually positioned to center 180 degrees at north or south. Adjust the motor centering on GroundMega accordingly by inputting a command.

##TwoEnds
Upload Rocket to the Moteino on the rocket.
Upload Ground to the Moteino on the ground.
Upload Mega to the Arduino Mega on the ground.

This set sends one message from Rocket at a time, but can handle different data struct formats. Different messages can be requested by inputting a command to Mega, which will be sent to Rocket.

##combinedMessages
Upload rocketCombine to the Moteino on the rocket.
Upload groundCombine to the Moteino on the ground.
Upload megaCombine to the Arduino Mega on the ground.

This set can send multiple messages from Rocket which are combined and displayed by Mega as one data package, but the messages are restricted to a single struct format.
