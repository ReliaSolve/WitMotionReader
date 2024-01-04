#include "serial.h"
#include "wit_c_sdk.h"
#include "REG.h"
#include <stdint.h>

// Please see the protocol description for details of the data parsing:
// https://drive.google.com/file/d/1xrfK9bAEncgFQYjvT_c6vwSEH0ZhzaUZ/view?usp=drive_link

// This is called whenever the WitSerialDataIn() function has enough
// data to update one or more registers.  The values are updated in
// the sReg global array and this callback function is called to let
// us know which ones have been updated.  The calculation and response
// happens inside this callback function as each report is completed.
static void SensorDataUpdate(uint32_t uiReg, uint32_t uiRegNum)
{
  // Loop through all of the registers, starting with the
  // one in the first parameter and going through as many
  // as are specified in the second.
  // The sReg array is an array of registers declared in the WitMotion
  // library whose values are changed when new data comes in.
  // We convert raw values into appropriate units.
  int i;
  for(i = 0; i < uiRegNum; i++) {
    switch(uiReg) {

//    case YYMM:
//    case DDHH:
//    case MMSS:
      case MS:
        {
          int YY = sReg[YYMM] & 0xff;
          int Mo = (sReg[YYMM] & 0xff00) >> 8;
          int DD = sReg[DDHH] & 0xff;
          int HH = (sReg[DDHH] & 0xff00) >> 8;
          int Mi = sReg[MMSS] & 0xff;
          int SS = (sReg[MMSS] & 0xff00) >> 8;
          int Ms = sReg[MS];
          printf("Date: %02d/%02d/%02d, Time: %02d:%02d:%02d.%03d\n",
            YY,Mo,DD, HH,Mi,SS, Ms);
        }
        break;

//    case AX:
//    case AY:
      case AZ:
        {
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
        {
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
        {
          // Scaling is to +/-4900 uTesla.
          printf("mag:%d %d %d\n", sReg[HX], sReg[HY], sReg[HZ]);
        }
        break;

//    case Roll:
//    case Pitch:
      case Yaw:
        {
          float fAngle[3];
          int i;
          for(i = 0; i < 3; i++) {
            fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
          }
          printf("angle:%.3f %.3f %.3f\n", fAngle[0], fAngle[1], fAngle[2]);
        }
        break;

//    case LonL:
//    case LonH:
//    case LatL:
//    case LatH:
      case GPSHeight:
        // The NMEA8013 standard stipulates that the GPS longitude output format is ddmm.mmmmm
        //  (dd is degrees, mm.mmmmm is minutes). The decimal point is removed when
        //  longitude/latitude is output, so the longitude/latitude degrees can be calculated as follows:
        // dd=Lon[31:0]/10000000;
        // dd=Lat[31:0]/10000000;
        // The longitude/latitude fraction can be calculated like this:
        // mm.mmmmm=(Lon[31:0]%10000000)/100000; (% represents the remainder operation)
        // mm.mmmmm=(Lat[31:0]%10000000)/100000; (% represents the remainder operation)
        // Note: Positive numbers represent northern latitude, negative numbers represent southern latitude;
        // Positive numbers represent east longitude, negative numbers represent west longitude.

        // Please see the protocol description for details:
        // https://drive.google.com/file/d/1xrfK9bAEncgFQYjvT_c6vwSEH0ZhzaUZ/view?usp=drive_link
        {
          double longitude, latitude, height;
          {
            int32_t value = sReg[LonL] + (sReg[LonH] << 16);
            int degrees = value / 10000000;
            double minutes = (value%10000000)/100000.0;
            longitude = degrees + minutes / 60;
          }
          {
            int32_t value = sReg[LatL] + (sReg[LatH] << 16);
            int degrees = value / 10000000;
            double minutes = (value%10000000)/100000.0;
            latitude = degrees + minutes / 60;
          }
          {
            int32_t value = sReg[GPSHeight];
            height = value / 10.0;
          }
          printf("longitude:%11.6lf, latitude:%11.6lf, height: %6.2lf\n", longitude, latitude, height);
        }
        break;

//    case HeightL:
      case HeightH:
        // Height=((int)HeightH[15:0]<<16)|HeightL×100 (unit is meter)
        // Please see the protocol description for details:
        // https://drive.google.com/file/d/1xrfK9bAEncgFQYjvT_c6vwSEH0ZhzaUZ/view?usp=drive_link
        {
          int32_t value = sReg[HeightL] + (sReg[HeightH] << 16);
          double height = value * 100;
          printf("height:%.2lf\n", height);
        }
        break;

      default:
        break;
    }
    uiReg++;
  }
}

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

