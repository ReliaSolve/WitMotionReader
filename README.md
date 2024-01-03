# WitMotionReader

This is a wrapper around the WitMotion SDK sample files for Linux that provides a way
to build them into a library and them use them in a C program to open and read from a
device. It presumes that the device has been configured and calibrated separately.

## Configuring the device

There is a Windows program provided by WitMotion that can configure the baud rate
and other settings on the device. There is a software link on their web page that
points to a [Google Drive](https://drive.google.com/drive/folders/1TLutidDBd_tDg5aTXgjvkz63OVt5_8ZZ)
that contains a *WitMotion New Software.zip* file with the program in it.

Follow the instructions in the *Software Instructions Manual.pdf* in that Google Drive to:
- select the appropriate device type and then search for the device,
- configure the baud rate on the device to be 230400,
- configure the read rate to be 200 Hz,
- configure the channels to be read so that they include those needed.

## Building this example software

The source code for this project is available on Github. One way to download and build
it is as follows:
```
mkdir -p ~/src
cd ~/src
git clone --recursive https://github.com/ReliaSolve/WitMotionReader.git
mkdir -p ~/src/WitMotionReader
cd ~/src/WitMotionReader
cmake ~/src/WitMotionReader
make
```

This will build a program named *witExample* that can be run with the name of the
virtual serial port to use. For example, `./witExample /dev/ttyS0'
This program should auto-detect the baud rate on the device and then start
printing the acceleration, gyro rotation rate, angle of orientation, and
magnetometer readings. Press ^C to quit the program.

