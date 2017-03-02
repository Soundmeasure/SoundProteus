/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: MattairTech MT-D21E (SAM-BA bootloader) w/ SAMD21E18A, Platform=samd, Package=MattairTech_Arduino
*/

#define ARDUINO 10801
#define ARDUINO_MAIN
#define F_CPU 48000000L
#define printf iprintf
#define __SAMD__
#define F_CPU 48000000L
#define ARDUINO 10801
#define ARDUINO_CDC_UART
#define ARDUINO_ARCH_SAMD
#define __SAMD21E18A__
#define __8KB_BOOTLOADER__
#define USB_VID 0x16D0
#define USB_PID {build.pid}
#define USBCON

//
//

#include "pins_arduino.h" 
#include "variant.h" 
#include "arduino.h"
#include "TestArduinoSAMD21E.ino"