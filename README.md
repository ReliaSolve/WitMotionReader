# WitMotionReader

This is a wrapper around the WitMotion SDK sample files for Linux that provides a way
to build them into a library and them use them in a C program to open and read from a
device. It presumes that the device has been configured and calibrated separately.

These instructions are specific to the GPS+inertial unit JY-GPSIMU with an internal
antenna, whose device type is WTGAHRS2.

## Configuring the device

Plug in the leads on the included USB adapter according to the TTL layout,
which has black on GND, red on +5V, yellow on RXD and green on TXD.

There is a Windows program provided by WitMotion that can configure the baud rate
and other settings on the device. There is a software link on their web page that
points to a [Google Drive](https://drive.google.com/drive/folders/1TLutidDBd_tDg5aTXgjvkz63OVt5_8ZZ)
that contains a *WitMotion New Software.zip* file with the program in it.

Follow the instructions in the *Software Instructions Manual.pdf* in that Google Drive to:
- select the appropriate device type (WTGAHRS2) and then search for the device,
- configure the baud rate on the device to be 230400,
- configure the bandwidth to be 188 Hz,
- configure the output rate to be 100 Hz, and
- configure the channels to be read so that they include those needed
(Time, Acceleration, Velocity, Angle, Location, PDOP, and Positioning Accuracy).

## Calibrating the device

Follow the instructions in the *Software Instructions Manual.pdf* in that Google Drive to:
- Calibrate the accelerometer, and
- Calibrate the magnetic field.

## Setting permissions on Linux

Plug the device into a USB port on the Linux system. This should produce a virtual TTY
device, perhaps */dev/ttyS0*. Permissions must be set on the device for the
current user: `sudo chmod 666 /dev/ttyS0` (adjusting the device name as needed).

## Building this example software

The source code for this project is available on Github. One way to download and build
it is as follows:
```
mkdir -p ~/src
cd ~/src
git clone --recursive https://github.com/ReliaSolve/WitMotionReader.git
mkdir -p ~/build/WitMotionReader
cd ~/build/WitMotionReader
cmake ~/src/WitMotionReader
make
```

### Running

The *make* during build will generate a program named *witExample* that can be run with the name of the
virtual serial port to use. For example, `./witExample /dev/ttyS0` (adjusting the
device name as needed).
This program should auto-detect the baud rate on the device and then start
printing the acceleration, gyro rotation rate, angle of orientation, and
magnetometer readings. Press ^C to quit the program.

It will also generate a program named **example** from *example.c* that is both simpler
and has a more general list of data values. It is also more heavily commented. It is
intended to be used as an example for developing a driver that uses this device.

### Additional data fields:

There is a [protocol description document]
(https://drive.google.com/file/d/1xrfK9bAEncgFQYjvT_c6vwSEH0ZhzaUZ/view?usp=drive_link) that
describes the formats and how to convert the registers into values.
