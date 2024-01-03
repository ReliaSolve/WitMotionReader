#include "serial.h"
#include "wit_c_sdk.h"
#include "REG.h"
#include <stdint.h>

static void SensorDataUpdate(uint32_t uiReg, uint32_t uiRegNum);

int main(int argc,char* argv[])
{
	if (argc != 3) {
		printf("Usage: %s DEVICE_NAME BAUD\n", argv[0]);
    printf("       DEVICE_NAME is the name of the serial device, like /dev/ttyUSB0\n");
    printf("       BAUD is the baud rate of the serial device, like 230400\n");
		return 1;
	}
  char *dev = argv[1];
  int baud = atoi(argv[2]);

  // Initialize the WitMotion library and register a callback for data values coming
  // from the device.  The callback function is called whenever a range of registers
  // has been updated, telling the application about the fact that they have been
  // updated.  The changed values are stored in a global sReg structure that is
  // exposed by the library, indexed by the same values as those passed to the
  // callback function.
	WitInit(WIT_PROTOCOL_NORMAL, 0x50);
	WitRegisterCallBack(SensorDataUpdate);
	
  int fd = -1;
  fd = serial_open(dev, baud);
  if (fd < 0) {
    printf("Could not open %s with baud %d\n", dev, baud);
    return 2;
  }

	printf("\n********************** Found device ************************\n");

  // These counts are used to determine when there has been a change on one
  // of the values in the loop below.  Whenever the corresponding _COUNT
  // global variable changes, we can perform the appropriate action and then
  // set the count to match so we don't redo an action.
  // An alternate approach would be to handle the action in the callback
  // directly.
  int lastAccCount = -1, lastGyroCount = -1, lastAngleCount = -1, lastMagCount = -1;

  // Continually loop, reading and interpreting characters until the
  // callback is called with new data updates.
	while (1) {

    // Get all available characters and send them to the library
    // for processing. When a report is available for a particular
    // value, the callback function will be called to let us know.
    // The response happens in the callback handler, which deals with each report
    // type as it arrives.
    char cBuff[1];
    while(serial_read_data(fd, cBuff, 1)) {
      WitSerialDataIn(cBuff[0]);
    }
  }

  serial_close(fd);
	return 0;
}

// This is called whenever the WitSerialDataIn() function has enough
// data to update one or more registers.  The values are updated in
// the sReg global array and this callback function is called to let
// us know which ones have been updated.
static void SensorDataUpdate(uint32_t uiReg, uint32_t uiRegNum)
{
  // Provides a way for us to only print things every once in a while.
  // @todo this will not be needed for a program that handles each event.
  static volatile int ACC_COUNT = 0;
  static volatile int GYRO_COUNT = 0;
  static volatile int ANGLE_COUNT = 0;
  static volatile int MAG_COUNT = 0;

  // Loop through all of the registers, starting with the
  // one in the first parameter and going through as many
  // as are specified in the second.
  // The sReg array is an array of registers declared in the WitMotion
  // library whose values are changed when new data comes in.
  // We convert raw values into appropriate units.
  int i;
  for(i = 0; i < uiRegNum; i++) {
    switch(uiReg) {

//    case AX:
//    case AY:
      case AZ:
        ACC_COUNT++;
        if (ACC_COUNT % 100 == 0) {
          float fAcc[3];
          int i;
          for(i = 0; i < 3; i++) {
            fAcc[i] = sReg[AX+i] / 32768.0f * 16.0f;
          }
          printf("acc:%.3f %.3f %.3f\n", fAcc[0], fAcc[1], fAcc[2]);
        }
        break;

//    case GX:
//    case GY:
      case GZ:
        GYRO_COUNT++;
        if (GYRO_COUNT % 100 == 0) {
          float fGyro[3];
          int i;
          for(i = 0; i < 3; i++) {
            fGyro[i] = sReg[GX+i] / 32768.0f * 2000.0f;
          }
          printf("gyro:%.3f %.3f %.3f\n", fGyro[0], fGyro[1], fGyro[2]);
        }
        break;

//    case HX:
//    case HY:
      case HZ:
        MAG_COUNT++;
        if (MAG_COUNT % 100 == 0) {
          printf("mag:%d %d %d\n", sReg[HX], sReg[HY], sReg[HZ]);
        }
        break;

//    case Roll:
//    case Pitch:
      case Yaw:
        ANGLE_COUNT++;
        if (ANGLE_COUNT % 100 == 0) {
          float fAngle[3];
          int i;
          for(i = 0; i < 3; i++) {
            fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
          }
          printf("angle:%.3f %.3f %.3f\n", fAngle[0], fAngle[1], fAngle[2]);
        }
        break;

      default:
        break;
    }
    uiReg++;
  }
}

