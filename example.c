#include "serial.h"
#include "wit_c_sdk.h"
#include "REG.h"
#include <stdint.h>

#define ACC_UPDATE		0x01
#define GYRO_UPDATE		0x02
#define ANGLE_UPDATE	0x04
#define MAG_UPDATE		0x08
#define READ_UPDATE		0x80

static volatile char s_cDataUpdate = 0;

static void SensorDataUpdate(uint32_t uiReg, uint32_t uiRegNum);

int main(int argc,char* argv[])
{
	if (argc < 2) {
		printf("Usage: %s DEVICE_NAME\n", argv[0]);
    printf("       DEVICE_NAME is the name of the serial device, like /dev/ttyUSB0\n");
		return 1;
	}
  char *dev = argv[1];

	int i , ret;
	char cBuff[1];

  // Initialize the WitMotion library and register a callback for data values coming
  // from the device.  The callback function is called whenever a range of registers
  // has been updated, telling the application about the fact that they have been
  // updated.  The changed values are stored in a global sReg structure that is
  // exposed by the library, indexed by the same values as those passed to the
  // callback function.
	WitInit(WIT_PROTOCOL_NORMAL, 0x50);
	WitRegisterCallBack(SensorDataUpdate);
	
  // Try opening the device with all possible baud rates to see if it responds to any
  // of them. @todo This can be simplified to only open the device at the expected
  // baud rate.
  int fd = -1;
  {
    const int c_uiBaud[] = {2400 , 4800 , 9600 , 19200 , 38400 , 57600 , 115200 , 230400 , 460800 , 921600};
    int i;
    char cBuff[1];
    int found = 0;

    for(i = 0; !found && (i < sizeof(c_uiBaud) / sizeof(int)); i++) {
      serial_close(fd);
      fd = serial_open(dev, c_uiBaud[i]);
 
      int iRetry = 2;
      do {
        s_cDataUpdate = 0;
        usleep(50000);
        WitReadReg(AX, 3);
        // Continually loop, reading and interpreting characters until the
        // callback is called with new data updates.
        while(serial_read_data(fd, cBuff, 1)) {
          WitSerialDataIn(cBuff[0]);
        }
        if(s_cDataUpdate != 0) {
          found = 1;
          break;
        }
      } while(--iRetry > 0);		
    }

    if (!found) {
      printf("can not find sensor\n");
      printf("please check your connection\n");
      return 2;
    }
  }

	printf("\n********************** Found device ************************\n");

  // Continually loop, reading and interpreting characters until the
  // callback is called with new data updates.
	while (1) {
    float fAcc[3], fGyro[3], fAngle[3];

    // Get all available characters and send them to the library
    // for processing. When a report is available for a particular
    // value, the callback function will be called to update it.
    while(serial_read_data(fd, cBuff, 1)) {
      WitSerialDataIn(cBuff[0]);
    }

    // The sReg array is an array of registers declared in the WitMotion
    // library whose values are changed when new data comes in.
    if(s_cDataUpdate) {
      for(i = 0; i < 3; i++) {
        fAcc[i] = sReg[AX+i] / 32768.0f * 16.0f;
        fGyro[i] = sReg[GX+i] / 32768.0f * 2000.0f;
        fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
      }

      if(s_cDataUpdate & ACC_UPDATE) {
        printf("acc:%.3f %.3f %.3f\n", fAcc[0], fAcc[1], fAcc[2]);
        s_cDataUpdate &= ~ACC_UPDATE;
      }
      if(s_cDataUpdate & GYRO_UPDATE) {
        printf("gyro:%.3f %.3f %.3f\n", fGyro[0], fGyro[1], fGyro[2]);
        s_cDataUpdate &= ~GYRO_UPDATE;
      }
      if(s_cDataUpdate & ANGLE_UPDATE) {
        printf("angle:%.3f %.3f %.3f\n", fAngle[0], fAngle[1], fAngle[2]);
        s_cDataUpdate &= ~ANGLE_UPDATE;
      }
      if(s_cDataUpdate & MAG_UPDATE) {
        printf("mag:%d %d %d\n", sReg[HX], sReg[HY], sReg[HZ]);
        s_cDataUpdate &= ~MAG_UPDATE;
      }
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
  // Loop through all of the registers, starting with the
  // one in the first parameter and going through as many
  // as are specified in the second.
  //  Only inform the main program that an update has been made
  // when the last value in a given data field has been updated.
  int i;
  for(i = 0; i < uiRegNum; i++) {
    switch(uiReg) {
//      case AX:
//      case AY:
      case AZ:
        s_cDataUpdate |= ACC_UPDATE;
        break;
//      case GX:
//      case GY:
      case GZ:
        s_cDataUpdate |= GYRO_UPDATE;
        break;
//      case HX:
//      case HY:
      case HZ:
        s_cDataUpdate |= MAG_UPDATE;
        break;
//      case Roll:
//      case Pitch:
      case Yaw:
        s_cDataUpdate |= ANGLE_UPDATE;
        break;
      default:
        s_cDataUpdate |= READ_UPDATE;
        break;
    }
    uiReg++;
  }
}

