
/*

������ ����� 21.08.2017�.


�������������� 11.11.2017�.

��������� 14.11.2017�.



*/

#define __SAM3X8E__


#include <SPI.h>
#include <UTFT.h>
#include <UTouch.h>
#include <UTFT_Buttons.h>
#include <DueTimer.h>
#include "Wire.h"
#include <DS3231.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t Dingbats1_XL[];
extern uint8_t SmallSymbolFont[];
extern uint8_t SevenSegNumFont[];

// ��������� ��������

UTFT myGLCD(TFT01_28, 38, 39, 40, 41);     // ��������� ��������
										   //UTFT myGLCD(ITDB28, 38, 39, 40, 41);     // ��������� ��������
UTouch        myTouch(6, 5, 4, 3, 2);          // ��������� ����������

UTFT_Buttons  myButtons(&myGLCD, &myTouch);
boolean default_colors = true;
uint8_t menu_redraw_required = 0;

//******************���������� ���������� ��� �������� � ����� ���� (������)****************************

int but1, but2, but3, but4, butX, pressed_button;


// +++++++++++++++ �������� pin +++++++++++++++++++++++++++++++++++++++++++++

#define intensityLCD      9                         // ���� ���������� �������� ������
#define synhro_pin       66                         // ���� ��� ������������� ���� �  ������������
#define alarm_pin         7                         // ���� ���������� �� �������  DS3231
#define kn_red           43                         // AD4 ������ ������� +
#define kn_blue          42                         // AD6 ������ ����� -
#define vibro            11                         // ����������
#define sounder          53                         // ������
#define LED_PIN13        13                         // ���������


// -------------------   ��������� �������� ������� DS3231 -------------------

int oldsec = 0;
//char* str_mon[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

DS3231 DS3231_clock;
RTCDateTime dt;
//char* daynames[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

boolean isAlarm           = false;                               //
//boolean alarmState        = false;                               //
int alarm_synhro          = 0;                                   //
unsigned long alarm_count = 0;                                   //




//+++++++++++++++++++++++++++++ ������� ������ +++++++++++++++++++++++++++++++++++++++
int deviceaddress = 80;                                          // ����� ���������� ������
int mem_start     = 24;                                          // ������� ������� ��������� ������� ����� ������. �������� ������  
int adr_mem_start = 1023;                                        // ����� �������� �������� ������� ��������� ������� ����� ������
byte hi;                                                         // ������� ���� ��� �������������� �����
byte low;                                                        // ������� ���� ��� �������������� �����


volatile int adr_count1_kn = 2;                                  // ����� �������� ������ ��������� 1 ������ ��������� Sound Test Base 
volatile int adr_count2_kn = 4;                                  // ����� �������� ������ ��������� 2 ������ ��������� Sound Test Base
volatile int adr_count3_kn = 6;                                  // ����� �������� ������ ��������� 1 ������ ������������ 
volatile int adr_count4_kn = 8;                                  // ����� �������� ������ ��������� 2 ������ ������������ 
int adr_set_timeZero     = 20;                                 //





int adr_start_unixtimetime = 100;                                // ����� ������� ������ ������������� �������
int adr_start_year         = 104;                                // ����� ������� ������ ������������� �������                                
int adr_start_month        = 105;                                // ����� ������� ������ ������������� �������
int adr_start_day          = 106;                                // ����� ������� ������ ������������� �������
int adr_start_hour         = 107;                                // ����� ������� ������ ������������� �������
int adr_start_minute       = 108;                                // ����� ������� ������ ������������� �������
int adr_start_second       = 109;                                // ����� ������� ������ ������������� �������

// ++++++++++++++++++++++ + ��������� ������������ ���������++++++++++++++++++++++++++++++++++++ +

#define address_AD5252   0x2C                                    // ����� ���������� AD5252  
#define control_word1    0x07                                    // ���� ���������� �������� �1
#define control_word2    0x87                                    // ���� ���������� �������� �2
byte resistance = 0x00;                                          // ������������� 0x00..0xFF - 0��..100���
//byte level_resist      = 0;                                    // ���� ��������� ������ �������� ���������

//+++++++++++++++++++++++++++++++

RF24 radio(48, 49);                                                        // DUE

unsigned long timeoutPeriod = 3000;                             // Set a user-defined timeout period. With auto-retransmit set to (15,15) retransmission will take up to 60ms and as little as 7.5ms with it set to (1,15).
										                        // With a timeout period of 1000, the radio will retry each payload for up to 1 second before giving up on the transmission and starting over
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };   // Radio pipe addresses for the 2 nodes to communicate.

byte data_in[24];                                               // ����� �������� �������� ������
byte data_out[24];                                              // ����� �������� ������ ��� ��������
volatile unsigned long counter;
unsigned long rxTimer, startTime, stopTime, payloads = 0;
bool TX = 1, RX = 0, role = 0, transferInProgress = 0;

typedef enum { role_ping_out = 1, role_pong_back } role_e;                  // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back" };  // The debug-friendly names of those roles
role_e role_test = role_pong_back;                                          // The role of the current running sketch
byte counter_test = 0;

const char str[] = "My very long string";
extern "C" char *sbrk(int i);
uint32_t message;                                                // ��� ���������� ��� ����� ��������� ��������� �� ���������;
unsigned long tim1 = 0;                                          // ����� ������ ��������������
unsigned long timeStartRadio = 0;                                // ����� ������ ����� ������ ��������
unsigned long timeStopRadio = 0;                                 // ����� ��������� ����� ������ ��������

// +++++++++++++++++++++++ ��������� �������� ������� ++++++++++++++++++++++++++

int time_sound = 50;                                             // ������������ �������� �������
int freq_sound = 1800;                                           // ������� �������� �������
byte volume1 = 100;                                              // 
byte volume2 = 100;                                              //
volatile byte volume_variant = 0;                                // ���������� ������������� ��������� ��. ����������. 

//++++++++++++++++++++++++++++++++ ��������� ���������� ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int x_osc, y_osc;
#define page_max  12
unsigned long Start_Power  = 0;                                  // ������ �������� �������
volatile int count_ms      = 0;                                  // ��������� �����
bool wiev_Start_Menu       = false;                              // ���������� ������ ���� ����� ������������� ������� �����
volatile bool start_synhro = false;                              // ����� ������ ��������
bool synhro_enable         = false;                              // ������� �������� �������������
unsigned long StartSample  = 0;
int Page_count             = 0;
int scale_strob            = 2;
int page                   = 0;
int xpos;
#define ms_info            0                                       // 
#define line_info          1                                       // 
int Sample_osc[240];
int Synhro_osc[240][2];
bool trig_sin = false;                                          // ���� ������������ ��������
int page_trig = 0;
int dgvh;
int mode                  = 3;                                  //����� ���������  
int mode1                 = 0;                                  //������������ ����������������
int dTime                 = 2;
int x_dTime               = 276;
int tmode                 = 5;
int t_in_mode             = 0;
int Trigger               = 0;
float koeff_h             = 7.759 * 4;
volatile int kn           = 0;
byte counter_kn           = 0;
byte Chanal_volume        = 0;
float koeff_volume[]      = { 0.0, 1.0, 1.4, 2.0, 2.8, 4.0, 5.6, 8.0, 11.2, 15.87 };
const char* proc_volume[] = { "0%","6%","8%","12%","17%","25%","35%","50%","70%","99%" };
float StartMeasure         = 0;
float EndMeasure           = 0;
unsigned long set_timeZero = 0;
int ypos_osc1_0;
int ypos_osc2_0;
int ypos_trig;
int ypos_trig_old;
int OldSample_osc[254][20];
const int hpos = 95; //set 0v on horizontal  grid
int Channel_x = 0;
volatile unsigned long StartSynhro = 0;

//***************** ���������� ���������� ��� �������� �������*****************************************************

char  txt_Start_Menu[] = "HA""CTPO""\x87""K""\x86";                                                     // "���������"

char  txt_menu1_1[] = "\x86\x9C\xA1""ep.""\x9C""a""\x99""ep""\x9B\x9F\x9D";                             // "�����.��������"
char  txt_menu1_2[] = "HA""CTPO""\x87""K""\x86";                                                        // "���������"
char  txt_menu1_3[] = "TA""\x87""MEP C""\x86""HXPO.";                                                    // "������ ������."
char  txt_menu1_4[] = "KOHTPO""\x88\x92"" C""\x86""HXPO.";                                              // "�������� ������."
char  txt_menu1_5[] = "B\x91XO\x82";                                                                    // "�����"       

char  txt_delay_measure1[] = "C""\x9D\xA2""xpo ""\xA3""o ""\xA4""a""\x9E\xA1""epy";                     // ������ �� �������
char  txt_delay_measure2[] = "C""\x9D\xA2""xpo ""\xA3""o pa""\x99\x9D""o";                              // ������ �� ����� 
char  txt_delay_measure3[] = "";                                                                        //
char  txt_delay_measure4[] = "B\x91XO\x82";                                                             // �����    

char  txt_tuning_menu1[] = "Oc\xA6\x9D\xA0\xA0o\x98pa\xA5";                                             // "�����������"
char  txt_tuning_menu2[] = "C""\x9D\xA2""xpo""\xA2\x9D\x9C""a""\xA6\x9D\xAF"" ";                        // "�������������"
char  txt_tuning_menu3[] = "PA""\x82\x86""O";                                                           // �����
char  txt_tuning_menu4[] = "B\x91XO\x82";                                                               // ����� 

char  txt_synhro_menu1[] = "C""\x9D\xA2""xpo pa""\x99\x9D""o";                                          // "������ �����"
char  txt_synhro_menu2[] = "C""\x9D\xA2""xpo ""\xA3""po""\x97""o""\x99";                                // "������ ������"
char  txt_synhro_menu3[] = "C""\xA4""o""\xA3"" c""\x9D\xA2""xpo.";                                      // "���� ������."
char  txt_synhro_menu4[] = "B\x91XO\x82";                                                               // �����      

char  txt_info29[] = "Stop->PUSH Disp";
char  txt_info11[] = "ESC->PUSH Display";                                                              // "ESC->PUSH Display"





//++++++++++++++++++++++++++++  ��������� ��� +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
��������� ������� ������ � ���(����������� ����� ��������)
������ ����� ���������� ��������� ��������� ��� � �������������� �������� ADC_MR, ������ ��� ����� ����������� ��������������(8 ��� 10 ���),
����� ������(���������� / ������), ������� ������������ ���, ����� ������� � ������ ���������.
����� ���������� �������� ������, �� ������� ����� ����������� �������������� � ������� �������� ADC_CHER.
���� �������������� �������� � ��� � ������ ��� �����������, ���������� ��������� ���������� ���������� �� ��������� �������� ���������� �� ���,
������� ��������� ��������� ����������, � ����� �������� ������ �������� ���������� �� ������� ��� �� ����������� ��������(�������� �� ��������� ��������������).
����� ������������ ������ ��� � �������������� �������� ADC_CR.
���������� ���������� ��������������(������ �����) ������������ ���� �� �������� ���������� �������������� �� ������(��������, ADC_CDR4 ��� ������ 4),
���� �� �������� ���������� ���������� ��������������(ADC_LCDR).���������� ���������� ������ ����������� �� ����� ��������������!
������� ��� ������ � ��� � ������ ��� ���������� ����� ����������� ���������� ���������� ���������,
���� �� ����������� ��������������� ���� ���������� � �������� ADC_SR.���� ��������������� �������� �� ��������� ���������� �� ��������� ��������������,
��������� ����������� � ����������� ����������.
��������� ��������������, ���������� � ���� ������ ����� �� �������� �������� ���������� �� ����� ���.
��� ��������� ������ �������� ���������� ��������� ��������� �����������������.��� ����� ��������� �������������� ���������� �������� �� �������� ������ ��������������.
�������� ������ ������������ ���
q = Uref / 2n,
��� Uref � ������� ���������� ���, n � ����������� ��������������(������������ ����� LOW_RES �������� ADC_MR).
*/




// ADC speed one channel 480,000 samples/sec (no enable per read)
//           one channel 288,000 samples/sec (enable per read)

#define ADC_MR * (volatile unsigned int *) (0x400C0004)              /*adc mode word*/
#define ADC_CR * (volatile unsigned int *) (0x400C0000)              /*write a 2 to start convertion*/
#define ADC_ISR * (volatile unsigned int *) (0x400C0030)             /*status reg -- bit 24 is data ready*/
#define ADC_ISR_DRDY 0x01000000
																								   //
#define ADC_START 2
#define ADC_LCDR * (volatile unsigned int *) (0x400C0020)            /*last converted low 12 bits*/
#define ADC_DATA 0x00000FFF 
#define ADC_STARTUP_FAST 12
																								   //
#define ADC_CHER * (volatile unsigned int *) (0x400C0010)           /*ADC Channel Enable Register  ������ ������*/
#define ADC_CHSR * (volatile unsigned int *) (0x400C0018)           /*ADC Channel Status Register  ������ ������ */
#define ADC_CDR0 * (volatile unsigned int *) (0x400C0050)           /*ADC Channel ������ ������ */
																								   //#define ADC_ISR_EOC0 0x00000001

//-------------  ��������� ���������� ������  ------------------------
#define Analog_pinA0 ADC_CHER_CH7    // ���� A0
#define Analog_pinA1 ADC_CHER_CH6    // ���� A1
#define Analog_pinA2 ADC_CHER_CH5    // ���� A2
#define Analog_pinA3 ADC_CHER_CH4    // ���� A3
#define Analog_pinA4 ADC_CHER_CH3    // ���� A4
#define Analog_pinA5 ADC_CHER_CH2    // ���� A5
#define Analog_pinA6 ADC_CHER_CH1    // ���� A6
#define Analog_pinA7 ADC_CHER_CH0    // ���� A7



//+++++++++++++++++++++++  ��������� EEPROM +++++++++++++++++++++++++++++++++++++
unsigned long i2c_eeprom_ulong_read(int addr)
{
	byte raw[4];
	for (byte i = 0; i < 4; i++) raw[i] = i2c_eeprom_read_byte(deviceaddress, addr + i);
	unsigned long &num = (unsigned long&)raw;
	return num;
}
// ������
void i2c_eeprom_ulong_write(int addr, unsigned long num)
{
	byte raw[4];
	(unsigned long&)raw = num;
	for (byte i = 0; i < 4; i++) i2c_eeprom_write_byte(deviceaddress, addr + i, raw[i]);
}

void i2c_eeprom_write_byte(int deviceaddress, unsigned int eeaddress, byte data)
{
	int rdata = data;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.write(rdata);
	Wire.endTransmission();
	delay(10);
}
byte i2c_eeprom_read_byte(int deviceaddress, unsigned int eeaddress) {
	byte rdata = 0xFF;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress, 1);
	if (Wire.available()) rdata = Wire.read();
	return rdata;
}
void i2c_eeprom_read_buffer(int deviceaddress, unsigned int eeaddress, byte *buffer, int length)
{

	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress, length);
	int c = 0;
	for (c = 0; c < length; c++)
		if (Wire.available()) buffer[c] = Wire.read();

}
void i2c_eeprom_write_page(int deviceaddress, unsigned int eeaddresspage, byte* data, byte length)
{
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddresspage >> 8)); // MSB
	Wire.write((int)(eeaddresspage & 0xFF)); // LSB
	byte c;
	for (c = 0; c < length; c++)
		Wire.write(data[c]);
	Wire.endTransmission();

}
void i2c_test()
{
	Serial.println("--------  EEPROM Test  ---------");
	char somedata[] = "this data from the eeprom i2c"; // data to write
	i2c_eeprom_write_page(deviceaddress, 0, (byte *)somedata, sizeof(somedata)); // write to EEPROM
	delay(100); //add a small delay
	Serial.println("Written Done");
	delay(10);
	Serial.print("Read EERPOM:");
	byte b = i2c_eeprom_read_byte(deviceaddress, 0); // access the first address from the memory
	char addr = 0; //first address

	while (b != 0)
	{
		Serial.print((char)b); //print content to serial port
		if (b != somedata[addr])
		{
			break;
		}
		addr++; //increase address
		b = i2c_eeprom_read_byte(0x50, addr); //access an address from the memory
	}
	Serial.println();
	Serial.println();
}

// ******************* ������� ���� ********************************
void draw_Start_Menu()
{
//	myGLCD.clrScr();
	myGLCD.setColor(0, 0, 0);
	myGLCD.fillRoundRect(0,15, 319, 239);
	but4 = myButtons.addButton(10, 200, 250, 35, txt_Start_Menu);
	butX = myButtons.addButton(279, 199, 40, 40, "W", BUTTON_SYMBOL); // ������ ���� 
	myGLCD.setColor(VGA_BLACK);
	myGLCD.setBackColor(VGA_WHITE);
	myGLCD.setColor(0, 255, 0);
	myGLCD.setBackColor(0, 0, 0);
	myButtons.drawButtons();
}
void Start_Menu()                    // 
{
	draw_Start_Menu();
	while (1)
	{
		myButtons.setTextFont(BigFont);                      // ���������� ������� ����� ������  
		measure_power();
		if (myTouch.dataAvailable() == true)                 // ��������� ������� ������
		{
		//	sound1();
			pressed_button = myButtons.checkButtons();       // ���� ������ - ��������� ��� ������
			if (pressed_button == butX)                      // ������ ����� ����
			{
				//sound1();
				myGLCD.setFont(BigFont);
				setClockRTC();
				myGLCD.clrScr();
				myButtons.drawButtons();                    // ������������ ������
			}

			//*****************  ���� �1  **************

			//if (pressed_button == but1)                 //  
			//{

			//	myGLCD.clrScr();
			//	myButtons.drawButtons();
			//}

			//if (pressed_button == but2)              / 
			//{

			//	myGLCD.clrScr();
			//	myButtons.drawButtons();
			//}

			//if (pressed_button == but3)             // 
			//{

			//	myGLCD.clrScr();
			//	myButtons.drawButtons();
			//}
			if (pressed_button == but4)             // ������ � SD
			{
				//	draw_Glav_Menu();
				Swich_Glav_Menu();
				myGLCD.clrScr();
				myButtons.drawButtons();
			}
		}
	}
}
void Draw_Glav_Menu()
{
	myGLCD.clrScr();
	myGLCD.setFont(BigFont);
	myGLCD.setBackColor(0, 0, 255);
	for (int x = 0; x<5; x++)
	{
		myGLCD.setColor(0, 0, 255);
		myGLCD.fillRoundRect(20, 15 + (45 * x), 300, 50 + (45 * x));
		myGLCD.setColor(255, 255, 255);
		myGLCD.drawRoundRect(20, 15 + (45 * x), 300, 50 + (45 * x));
	}

	myGLCD.print(txt_menu1_1, CENTER, 25);        // 
	myGLCD.print(txt_menu1_2, CENTER, 70);
	myGLCD.print(txt_menu1_3, CENTER, 115);
	myGLCD.print(txt_menu1_4, CENTER, 160);
	myGLCD.print(txt_menu1_5, CENTER, 205);
	myGLCD.setColor(255, 255, 255);

}
void Swich_Glav_Menu()
{
	Draw_Glav_Menu();
	while (true)
	{
		delay(10);
		if (myTouch.dataAvailable())
		{
			myTouch.read();
			int	x = myTouch.getX();
			int	y = myTouch.getY();

			if ((x >= 20) && (x <= 3000))       // 
			{
				if ((y >= 15) && (y <= 50))    // Button: 1   
				{
					waitForIt(20, 15, 300, 50);
					myGLCD.clrScr();
					menu_delay_measure();
					Draw_Glav_Menu();
				}
				if ((y >= 60) && (y <= 100))   // Button: 2  
				{
					waitForIt(20, 60, 300, 100);
					Draw_menu_tuning();
					menu_tuning();
					Draw_Glav_Menu();
				}
				if ((y >= 105) && (y <= 145))  // Button: 3  
				{
					waitForIt(20, 105, 300, 145);
					synhro_DS3231_clock();
					Draw_Glav_Menu();
				}
				if ((y >= 150) && (y <= 190))  // Button: 4  
				{
					waitForIt(20, 150, 300, 190);
					wiev_synhro();
					Draw_Glav_Menu();
				}
				if ((y >= 195) && (y <= 235))  // Button: 5 "EXIT" �����
				{
					waitForIt(20, 195, 300, 235);
					break;
				}
			}
		}
	}
}
void Draw_menu_delay_measure()                                    // ����������� ���� �������������
{
	myGLCD.clrScr();
	myGLCD.setFont(BigFont);
	myGLCD.setBackColor(0, 0, 255);
	for (int x = 0; x<4; x++)
	{
		myGLCD.setColor(0, 0, 255);
		myGLCD.fillRoundRect(20, 20 + (50 * x), 300, 60 + (50 * x));
		myGLCD.setColor(255, 255, 255);
		myGLCD.drawRoundRect(20, 20 + (50 * x), 300, 60 + (50 * x));
	}
	myGLCD.print(txt_delay_measure1, CENTER, 30);     // 
	myGLCD.print(txt_delay_measure2, CENTER, 80);
	myGLCD.print(txt_delay_measure3, CENTER, 130);
	myGLCD.print(txt_delay_measure4, CENTER, 180);

}
void menu_delay_measure()                                                // ���� ��������� �������������
{
	Draw_menu_delay_measure();
	while (true)
	{
		delay(10);
		if (myTouch.dataAvailable())
		{
			myTouch.read();
			int	x = myTouch.getX();
			int	y = myTouch.getY();

			if ((x >= 20) && (x <= 3000))                                  // 
			{
				if ((y >= 20) && (y <= 60))                                // Button: 1  ������������� �� �������
				{
					waitForIt(20, 20, 300, 60);
					myGLCD.clrScr();
					synhro_by_timer();                                    // ������������� �� ������� 
					Draw_menu_delay_measure();
				}
				if ((y >= 70) && (y <= 110))                               // Button: 2 ������������� �� �����
				{
					waitForIt(20, 70, 300, 110);
					myGLCD.clrScr();
					synhro_by_timer_1();
				//	synhro_by_main();
//					synhro_by_radio();                                    //  "������������� �� �����"
					Draw_menu_delay_measure();
				}
				if ((y >= 120) && (y <= 160))                             // Button: 3  
				{
					waitForIt(20, 120, 300, 160);
					myGLCD.clrScr();
		
					Draw_menu_delay_measure();
				}
				if ((y >= 170) && (y <= 220))                             // Button: 4 "EXIT" �����
				{
					waitForIt(20, 170, 300, 210);
					break;
				}
			}
		}
	}
}
void Draw_menu_tuning()
{
	myGLCD.clrScr();
	myGLCD.setFont(BigFont);
	myGLCD.setBackColor(0, 0, 255);
	for (int x = 0; x<4; x++)
	{
		myGLCD.setColor(0, 0, 255);
		myGLCD.fillRoundRect(30, 20 + (50 * x), 290, 60 + (50 * x));
		myGLCD.setColor(255, 255, 255);
		myGLCD.drawRoundRect(30, 20 + (50 * x), 290, 60 + (50 * x));
	}
	myGLCD.print(txt_tuning_menu1, CENTER, 30);     // 
	myGLCD.print(txt_tuning_menu2, CENTER, 80);
	myGLCD.print(txt_tuning_menu3, CENTER, 130);
	myGLCD.print(txt_tuning_menu4, CENTER, 180);
}
void menu_tuning()   // ���� "���������", ���������� �� �������� ���� 
{
	while (true)
	{
		delay(10);
		if (myTouch.dataAvailable())
		{
			myTouch.read();
			int	x = myTouch.getX();
			int	y = myTouch.getY();

			if ((x >= 30) && (x <= 290))       // 
			{
				if ((y >= 20) && (y <= 60))    // Button: 1  "Oscilloscope"
				{
					waitForIt(30, 20, 290, 60);
					myGLCD.clrScr();
					oscilloscope();
					Draw_menu_tuning();
				}
				if ((y >= 70) && (y <= 110))   // Button: 2 "������������� ���."
				{
					waitForIt(30, 70, 290, 110);
					myGLCD.clrScr();
					Draw_menu_synhro();
					menu_synhro();
					Draw_menu_tuning();
				}
				if ((y >= 120) && (y <= 160))  // Button: 3  
				{
					waitForIt(30, 120, 290, 160);
					myGLCD.clrScr();
				//	menu_radio();
					Draw_menu_tuning();
				}
				if ((y >= 170) && (y <= 220))  // Button: 4 "EXIT" �����
				{
					waitForIt(30, 170, 290, 210);
					break;
				}
			}
		}
	}

} 

void Draw_menu_synhro()
{
	myGLCD.clrScr();
	myGLCD.setFont(BigFont);
	myGLCD.setBackColor(0, 0, 255);
	for (int x = 0; x<4; x++)
	{
		myGLCD.setColor(0, 0, 255);
		myGLCD.fillRoundRect(20, 20 + (50 * x), 300, 60 + (50 * x));
		myGLCD.setColor(255, 255, 255);
		myGLCD.drawRoundRect(20, 20 + (50 * x), 300, 60 + (50 * x));
	}
	myGLCD.print(txt_synhro_menu1, CENTER, 30);     // 
	myGLCD.print(txt_synhro_menu2, CENTER, 80);
	myGLCD.print(txt_synhro_menu3, CENTER, 130);
	myGLCD.print(txt_synhro_menu4, CENTER, 180);
}
void menu_synhro()                                                        // ���� "���������", ���������� �� �������� ���� 
{
	while (true)
	{
		delay(10);
		if (myTouch.dataAvailable())
		{
			myTouch.read();
			int	x = myTouch.getX();
			int	y = myTouch.getY();

			if ((x >= 20) && (x <= 300))       // 
			{
				if ((y >= 20) && (y <= 60))                                 // Button: 1  "������������� �� �����"
				{
					waitForIt(20, 20, 300, 60);
					myGLCD.clrScr();
					data_out[2] = 2;                                        // ��������� ������� ������������� �������(�������� ������ ����������)
			//		tuning_mod();                                           // ������������� �� �����
					Draw_menu_synhro();
				}
				if ((y >= 70) && (y <= 110))                               // Button: 2 "������������� ���������"
				{
					waitForIt(20, 70, 300, 110);
					myGLCD.clrScr();
					data_out[2] = 3;                                        // ��������� ������� ������������� �������(�������� ������ ����������)
					radio_send_command();
			//		synhro_ware();
					Draw_menu_synhro();
				}
				if ((y >= 120) && (y <= 160))  // Button: 3  
				{
					waitForIt(20, 120, 300, 160);
					myGLCD.clrScr();
					data_out[2] = 4;                                        // �������� ������� ����
//					setup_radio();                                          // ��������� ����������
//					delayMicroseconds(500);
					radio_send_command();                                   //  ��������� �� ����� ������� ����
					//Timer4.stop();
					//Timer5.stop();
					//Timer6.stop();
					Draw_menu_synhro();
				}
				if ((y >= 170) && (y <= 220))  // Button: 4 "EXIT" �����
				{
					waitForIt(20, 170, 300, 210);
					break;
				}
			}
		}
	}
}
void waitForIt(int x1, int y1, int x2, int y2)
{
	myGLCD.setColor(255, 0, 0);
	myGLCD.drawRoundRect(x1, y1, x2, y2);
	sound1();
	while (myTouch.dataAvailable())
		myTouch.read();
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect(x1, y1, x2, y2);
}
//++++++++++++++++++++++++++ ����� ���� ������� ++++++++++++++++++++++++
void alarmFunction()
{
	DS3231_clock.clearAlarm1();
	dt = DS3231_clock.getDateTime();
//	Serial.println(DS3231_clock.dateFormat("H:i:s", dt));
	myGLCD.setBackColor(0, 0, 0);                   // ����� ��� ������
	myGLCD.setColor(255, 255, 255);
	myGLCD.setFont(SmallFont);
	if (alarm_synhro >1)
	{
		StartSynhro = micros();                                 // �������� �����
		alarm_synhro = 0;
		alarm_count++;
		myGLCD.print(DS3231_clock.dateFormat("s", dt), 190, 2);
		wiev_Start_Menu = true;
		start_synhro = true;
	}
	alarm_synhro++;
	myGLCD.print(DS3231_clock.dateFormat("d-m-Y H:i:s - ", dt), 10, 2);
}
void sevenHandler()                                         // Timer7 -  ������ ��������� �����  � ������ ��� ������ �� �����
{
	count_ms += scale_strob;
	if(xpos >0) Synhro_osc[xpos-1][ms_info] = count_ms;                          // �������� ��������� ����� � ������ ��� ������ �� �����
	if (xpos >0) Synhro_osc[xpos-1][line_info] = 4095;              // �������� ��������� ����� � ������ ��� ������ �� �����
}



void DrawGrid()
{
	myGLCD.setColor(VGA_GREEN);                                                 // ���� �����                                 
	for (dgvh = 0; dgvh < 7; dgvh++)                          // ���������� �����
	{
		myGLCD.drawLine(dgvh * 40, 0, dgvh * 40, 159);
		if (dgvh < 5)
		{
			myGLCD.drawLine(0, (dgvh * 40), 239, (dgvh * 40));
		}
	}
	myGLCD.setColor(255, 255, 255);                                           // ����� ��������� ������ ������
	myGLCD.drawRoundRect(245, 1, 318, 40);                                    // ���������� ������� ������
	myGLCD.drawRoundRect(245, 45, 318, 85);
	myGLCD.drawRoundRect(245, 90, 318, 130);
	myGLCD.drawRoundRect(245, 135, 318, 175);
}
void DrawGrid1()
{
	myGLCD.setColor(0, 200, 0);
	for (dgvh = 0; dgvh < 7; dgvh++)                          // ���������� �����
	{
		myGLCD.drawLine(dgvh * 40, 0, dgvh * 40, 159);
		if (dgvh < 5)
		{
			myGLCD.drawLine(0, (dgvh * 40), 239, (dgvh * 40));
		}
	}
	myGLCD.setColor(255, 255, 255);                           // �����
}
void buttons_channelNew()                   // ������ ������ ������������ 
{
	myGLCD.setFont(SmallFont);

	// ������ � 1
	myGLCD.setColor(VGA_BLACK);                // ���� ���� ������
	myGLCD.setBackColor(VGA_BLACK);
	myGLCD.fillRoundRect(10, 210, 60, 239);
	myGLCD.setColor(VGA_WHITE);                // ���� ������
											   //myGLCD.print("<-", 28, 214);
	myGLCD.print("Up1", 25, 224);
	//osc_line_off0 = true;

	// ������ � 2
	myGLCD.setColor(VGA_BLACK);                // ���� ���� ������
	myGLCD.setBackColor(VGA_BLACK);
	myGLCD.fillRoundRect(70, 210, 120, 239);
	myGLCD.setColor(VGA_WHITE);                // ���� ������
											   //myGLCD.print("->", 88, 214);
	myGLCD.print("Down1", 78, 224);

	// ������ � 3
	myGLCD.setColor(VGA_BLACK);                // ���� ���� ������
	myGLCD.setBackColor(VGA_BLACK);
	myGLCD.fillRoundRect(130, 210, 180, 239);
	myGLCD.setColor(VGA_WHITE);                // ���� ������

											   //	myGLCD.print("Up2", 148, 214);
	myGLCD.print("Up2", 142, 224);

	//myGLCD.print("<-", 148, 214);
	//myGLCD.print("sec0,1", 134, 224);


	// ������ � 4
	myGLCD.setColor(VGA_BLACK);                // ���� ���� ������
	myGLCD.setBackColor(VGA_BLACK);
	myGLCD.fillRoundRect(190, 210, 240, 239);
	myGLCD.setColor(VGA_WHITE);                // ���� ������
											   //myGLCD.print("Off", 206, 214);
	myGLCD.print("Down2", 197, 224);
	//myGLCD.print("->", 206, 214);
	//myGLCD.print("sec0,1", 193, 224);


	myGLCD.setColor(255, 255, 255);                  // ���� ��������� ������
	myGLCD.drawRoundRect(10, 210, 60, 239);         // ��������� ������ N1
	myGLCD.drawRoundRect(70, 210, 120, 239);        // ��������� ������ N2
	myGLCD.drawRoundRect(130, 210, 180, 239);       // ��������� ������ N3
	myGLCD.drawRoundRect(190, 210, 240, 239);       // ��������� ������ N4
}
void set_volume(int reg_module, byte count_vol)
{
	/*
	reg_module = 1  ���������� ������� ��������� 1 ������ ��������� Sound Test Base
	reg_module = 2  ���������� ������� ��������� 2 ������ ��������� Sound Test Base
	reg_module = 3  ���������� ������� ��������� 1 ������ ��������� ������������
	reg_module = 4  ���������� ������� ��������� 2 ������ ��������� ������������ (�� ������������)
	*/
	kn = 0;
	myGLCD.setFont(SmallFont);
	if (count_vol != 0)
	{
		byte b = 0;
		switch (reg_module)
		{
		case 1:
			if (count_vol == 1)
			{
				b = i2c_eeprom_read_byte(deviceaddress, adr_count1_kn);
				b++;
				if (b > 9) b = 9;
				i2c_eeprom_write_byte(deviceaddress, adr_count1_kn, b);
			}
			else if (count_vol == 2)
			{
				b = i2c_eeprom_read_byte(deviceaddress, adr_count1_kn);
				if (b > 0) b--;
				if (b <= 0) b = 0;
				i2c_eeprom_write_byte(deviceaddress, adr_count1_kn, b);
			}
			resistor(1, 16 * koeff_volume[b]);
			myGLCD.setBackColor(0, 0, 0);
			myGLCD.print(proc_volume[b], 38, 185);
			break;
		case 2:
			if (count_vol == 1)
			{
				b = i2c_eeprom_read_byte(deviceaddress, adr_count2_kn);
				b++;
				if (b > 9) b = 9;
				i2c_eeprom_write_byte(deviceaddress, adr_count2_kn, b);
			}
			else if (count_vol == 2)
			{
				b = i2c_eeprom_read_byte(deviceaddress, adr_count2_kn);
				if (b > 0) b--;
				if (b <= 0) b = 0;
				i2c_eeprom_write_byte(deviceaddress, adr_count2_kn, b);
			}
			resistor(2, 16 * koeff_volume[b]);
			myGLCD.setBackColor(0, 0, 0);
			myGLCD.print(proc_volume[b], 124, 185);
			break;
		case 3:
			if (count_vol == 1)
			{
				b = i2c_eeprom_read_byte(deviceaddress, adr_count3_kn);
				b++;
				if (b > 9) b = 9;
				i2c_eeprom_write_byte(deviceaddress, adr_count3_kn, b);
			}
			else if (count_vol == 2)
			{
				b = i2c_eeprom_read_byte(deviceaddress, adr_count3_kn);
				if (b > 0) b--;
				if (b <= 0) b = 0;
				i2c_eeprom_write_byte(deviceaddress, adr_count3_kn, b);
			}
			volume1 = 16 * koeff_volume[b];
			data_out[2] = 6;
			radio_send_command();
			myGLCD.print(proc_volume[b], 208, 185);
			break;
		case 4:
			if (count_vol == 1)
			{
				b = i2c_eeprom_read_byte(deviceaddress, adr_count4_kn);
				b++;
				if (b > 9) b = 9;
				i2c_eeprom_write_byte(deviceaddress, adr_count4_kn, b);
			}
			else if (count_vol == 2)
			{
				b = i2c_eeprom_read_byte(deviceaddress, adr_count4_kn);
				if (b > 0) b--;
				if (b <= 0) b = 0;
				i2c_eeprom_write_byte(deviceaddress, adr_count4_kn, b);
			}
			volume2 = 16 * koeff_volume[b];
			kn = 0;
			break;
		case 5:
			b = i2c_eeprom_read_byte(deviceaddress, adr_count1_kn);
			myGLCD.setBackColor(0, 0, 0);
			myGLCD.print(proc_volume[b], 38, 185);
			b = i2c_eeprom_read_byte(deviceaddress, adr_count2_kn);
			myGLCD.print(proc_volume[b], 124, 185);
			b = i2c_eeprom_read_byte(deviceaddress, adr_count3_kn);
			myGLCD.print(proc_volume[b], 208, 185);
			break;
		default:
			break;
		}
		//kn = 0;
	}
	//kn = 0;
}
void restore_volume()
{
	// ������������ ��������� ��������� ������������ ���������
	int b = 0;
	b = i2c_eeprom_read_byte(deviceaddress, adr_count1_kn);     // ������������ ������� ��������� 1 ������ ��������� Sound Test Base
	resistor(1, 16 * koeff_volume[b]);
	b = i2c_eeprom_read_byte(deviceaddress, adr_count2_kn);     // ������������ ������� ��������� 2 ������ ��������� Sound Test Base
	resistor(2, 16 * koeff_volume[b]);
	b = i2c_eeprom_read_byte(deviceaddress, adr_count3_kn);     // ������������ ������� ��������� 1 ������ ��������� ������������
	volume1 = 16 * koeff_volume[b];
	b = i2c_eeprom_read_byte(deviceaddress, adr_count4_kn);     // ������������ ������� ��������� 2 ������ ��������� ������������ (�� ������������)
	volume2 = 16 * koeff_volume[b];

}


void synhro_by_main()                                    // ������������� �� �������
{
	myGLCD.clrScr();
	myGLCD.setBackColor(0, 0, 0);
	//buttons_right();                                      // ���������� ������ ������ ������;
	//buttons_channelNew();                                 // ���������� ������ ����� ������;
	//myGLCD.setBackColor(0, 0, 0);
	myGLCD.setFont(BigFont);
/*	myGLCD.setColor(VGA_LIME);
	myGLCD.print(txt_info29, 5, 190);        */              // "Stop->PUSH Disp"
	mode = 3;                                             // ����� ���������  
	count_ms = 0;                                         // ����� ������� �� � ��������� ���������
	long time_temp = 0;
	bool start_info = false;
	unsigned long StartTime = 0;                          // ����������� ������� ��������� ������� (2 ������� )
//	set_volume(5, 1);                                     // ���������� ������� ���������  ������� ��������� �� ����

	//myGLCD.setColor(255, 255, 255);
	//myGLCD.drawCircle(200, 10, 10);


	for (xpos = 0; xpos < 240; xpos++)                                            // ������� ������ ������ ����������� � ��������� �����
	{
		Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
		Synhro_osc[xpos][line_info] = 0;                                          // ������� ������ ������ ��������� �����
	}

	while (1)                                                                        //                
	{
		// ������ ������ ��������������
		bool synhro_Off = false;                                                     // ������� ������������� � �������� ���������. ������������ ��� ������ ���������������
		trig_sin = false;                                                            // ������� �������� � �������� ���������   
		StartSample = micros();
		while (!start_synhro)                                                        // �������� ������� ������ ��������������.  ������ ����������� �������� Timer5
		{
			if (micros() - StartSample > 6000000)                                    // ������� ������������� � ������� 3 ������
			{
				synhro_Off = true;
				break;                                                               // ��������� �������� ��������������
			}

			// �������� ������ ����
			if (myTouch.dataAvailable())
			{
				delay(10);
				myTouch.read();
				x_osc = myTouch.getX();
				y_osc = myTouch.getY();

				if ((x_osc >= 2) && (x_osc <= 240))               // ����� �� ���������. ������ �� ������� ������
				{
					if ((y_osc >= 1) && (y_osc <= 160))           // 
					{
						Timer7.stop();                            //  ���������� ������ �� ����� ��������� �����
						return;
					}                                             //
				}

				//myGLCD.setBackColor(0, 0, 255);                   // ����� ��� ������
				//myGLCD.setColor(255, 255, 255);
				//myGLCD.setFont(SmallFont);

				//if ((x_osc >= 245) && (x_osc <= 280))               // ������� ������
				//{
				//	if ((y_osc >= 1) && (y_osc <= 40))              // ������  ������
				//	{
				//		waitForIt(245, 1, 318, 40);
				//		//chench_mode(0);                             //
				//	}

				//	if ((y_osc >= 45) && (y_osc <= 85))             // ������ - �������
				//	{
				//		waitForIt(245, 45, 318, 85);
				//		trigger_volume(0);
				//	}
				//	if ((y_osc >= 90) && (y_osc <= 130))           // ������ - ��������
				//	{
				//		waitForIt(245, 90, 318, 130);
				//		chench_mode1(0);
				//	}
				//}

				//if ((x_osc >= 282) && (x_osc <= 318))              // ������� ������
				//{
				//	if ((y_osc >= 1) && (y_osc <= 40))             // ������  ������
				//	{
				//		waitForIt(245, 1, 318, 40);
				//		//chench_mode(1);                            //  
				//	}

				//	if ((y_osc >= 45) && (y_osc <= 85))            // ������ - �������
				//	{
				//		waitForIt(245, 45, 318, 85);
				//		trigger_volume(1);
				//	}
				//	if ((y_osc >= 90) && (y_osc <= 130))          // ������ - ��������
				//	{
				//		waitForIt(245, 90, 318, 130);
				//		chench_mode1(1);
				//	}
				//}

				if ((x_osc >= 245) && (x_osc <= 318))                  // ������� ������
				{
					if ((y_osc >= 135) && (y_osc <= 175))              // ��������� ����������
					{
						waitForIt(245, 135, 318, 175);
						i2c_eeprom_ulong_write(adr_set_timeZero, EndMeasure - StartMeasure);  // �������� ����������� ����� 
					}
				}

				//if ((y_osc >= 205) && (y_osc <= 239))                 // ������ ������ ������������ 
				//{
				//	if ((x_osc >= 10) && (x_osc <= 60))               //  ������ �1
				//	{
				//		waitForIt(10, 210, 60, 239);
				//		set_volume(1, 1);                             // ����������� "+" ��������� �1

				//	}
				//	if ((x_osc >= 70) && (x_osc <= 120))              //  ������ �2
				//	{
				//		waitForIt(70, 210, 120, 239);
				//		set_volume(1, 2);                            // ����������� "-" ��������� �1

				//	}
				//	if ((x_osc >= 130) && (x_osc <= 180))            //  ������ �3
				//	{
				//		waitForIt(130, 210, 180, 239);
				//		set_volume(2, 1);                            // ����������� "+" ��������� �2

				//	}
				//	if ((x_osc >= 190) && (x_osc <= 240))            //  ������ �4
				//	{
				//		waitForIt(190, 210, 240, 239);
				//		set_volume(2, 2);                            // ����������� "-" ��������� �1

				//	}
				//}
			}
			// ��������� �������� ������  ����

		}

		if (synhro_Off)                                                              // ���� ��� ��������������� - ���������
		{
			myGLCD.setFont(BigFont);
			myGLCD.setColor(VGA_YELLOW);
			myGLCD.print("He""\xA4", 100, 30);                                       // ���
			myGLCD.print("c""\x9D\xA2""xpo""\xA2\x9D\x9C""a""\xA6\x9D\x9D", 20, 50); // �������������
			delay(2000);
			break;                                                                   // ��������� ���������. �����  � ����
		}

		if (start_synhro == true)		                                                 // ������������� ������. �������� ��������� ��������� �������
		{                                                                             // ��� ���������. �������� !!
			start_synhro = false;                                                    // ����� ������ �������������� ������. ���������� ���� ������ � �������� ��� �������� ���������� ������ ��������
			Timer7.start(scale_strob * 1000);                                        // �������� ������������  ��������� ����� �� ������
			StartMeasure = micros();                            // ��������� ����� ��������� ��������������
																//		Control_synhro = micros();                            // ��������� ����� ��������� ��������������
			page = 0;
			StartTime = micros();                                                                     //  ��������� ������ ������ ���. �������� ���������� ������ � ���� ������
			ADC_CHER = 0;                                                            // �������� ���������� ���� "0"          (1<<7) | (1<<6) for adc 7= A0, 6=A1 , 5=A2, 4 = A3

			while (!trig_sin)                                                        // �������� ������� ������ ��������������.  ������ ����������� �������� Timer5
			{
				if (micros() - StartTime > 2000000)                                    // ������� ������������� � ������� 1 ������
				{
					EndMeasure = micros();                                        // �������� ����� ������������ �������� ������
					trig_sin = true;                                             // ���������� ���� ������������ �������� ������
					break;                                                               // ��������� �������� ��������������
				}
				for (xpos = 0; xpos < 240; xpos++)                                   // ����� �� 240 ������
				{
					ADC_CR = ADC_START; 	                                         // ��������� ��������������
					while (!(ADC_ISR_DRDY));                                         // �������� ���������� ��������������
					Sample_osc[xpos] = ADC->ADC_CDR[7];                        // �������� ������ �0 � �������� � ������
					Synhro_osc[xpos][ms_info] = 0;                                      // ������� ������ ������ ��������� �����
					if ((Sample_osc[xpos] > Trigger) && (trig_sin == false) && (xpos > 2))   // ����� ���������� ������ ������
					{
						EndMeasure = micros();                                        // �������� ����� ������������ �������� ������
						trig_sin = true;                                             // ���������� ���� ������������ �������� ������
						//page_trig = page;                                            // ���������� ����� ����� � ������� �������� �������
					}
					delayMicroseconds(dTime);                                        // dTime ��������� �������� (��������) ��������� 
				}
				if (trig_sin == true) break;                                       // ��������� ������������ �� ����� � ������� �������� �����
			}
			Timer7.stop();                                                           // ��������� ������������ ��������� �����

			count_ms = 0;                                                            // ����� ������� �� � ��������� ���������
			//myGLCD.printNumI(page, 255, 177);                                        // ������� �� ����� ����� ���������� �������
			//myGLCD.printNumI(page_trig, 300, 177);                                   // ������� �� ����� �������� ������������ �������� ������
			//																		 // ������� ���������� �� ���������
			//myGLCD.setColor(0, 0, 0);
			//myGLCD.fillRoundRect(0, 0, 240, 176);                                    // �������� ������� ������ ��� ������ ��������� �����. 
			//DrawGrid1();
			//myGLCD.setBackColor(0, 0, 255);                                          // ��� ������ �����
			//myGLCD.setColor(255, 255, 255);                                          // ���� ������ �����
			//myGLCD.print("        ", 255, 160);                                      // �������� ������� ������ ���������� 
			//myGLCD.printNumI(set_timeZero, 255, 160);
			//myGLCD.print("        ", 255, 150);
			//time_temp = EndMeasure - StartSynhro;
			//myGLCD.printNumI(time_temp / 1000, 255, 150);                            // ����� ��������� ������
			myGLCD.setBackColor(0, 0, 0);                                            // ��� ������ ������
			myGLCD.setFont(BigFont);
			myGLCD.print("\x85""a""\x99""ep""\x9B\x9F""a", 5, 20);             // "��������"
			myGLCD.print("c""\x9D\x98\xA2""a""\xA0""a", 5, 45);               // "�������"
		/*	myGLCD.print("      ", LEFT, 44);*/

			//myGLCD.setFont(SmallFont);
			//myGLCD.setBackColor(0, 0, 255);
			if (trig_sin)
			{
				//myGLCD.setColor(255, 0, 0);
				//myGLCD.drawRoundRect(245, 135, 318, 175);
				//myGLCD.setBackColor(0, 0, 255);
				//myGLCD.setColor(255, 255, 255);
				//myGLCD.print("        ", 255, 140);

				myGLCD.setColor(0, 0, 0);
				myGLCD.drawRoundRect(0, 80, 318, 130);
				myGLCD.setFont(SevenSegNumFont);

				time_temp = EndMeasure - StartSynhro - set_timeZero;
			//	myGLCD.printNumF(time_temp / 1000.00, 2, 255, 140);  // ����� ����������� ����������
				//myGLCD.setFont(BigFont);
				//myGLCD.setBackColor(0, 0, 0);
				//myGLCD.printNumF(time_temp / 1000.00, 2, 5, 80); // ����� �������� ����������
				myGLCD.setColor(255, 255, 255);

				if(time_temp >0)myGLCD.printNumI(time_temp / 1000.00, 5, 80); // ����� �������� ����������
				//myGLCD.print(" ms", 88, 44);
				myGLCD.setFont(SmallFont);
				//myGLCD.setBackColor(0, 0, 255);
				//myGLCD.setColor(VGA_RED);                                                       // ������� ��������� ������������ �������� ������
				//myGLCD.fillCircle(227, 10, 10);                                                 // ������� ��������� ������������ �������� ������
			}
			else
			{


				myGLCD.setColor(0, 0, 0);
				myGLCD.drawRoundRect(0, 80, 318, 130);
				//myGLCD.setColor(255, 255, 255);
				//myGLCD.drawRoundRect(245, 135, 318, 175);
				//myGLCD.setBackColor(0, 0, 255);
				//myGLCD.setColor(255, 255, 255);
				//myGLCD.print("        ", 255, 140);
				//myGLCD.printNumI(0, 255, 140);
				//myGLCD.setFont(BigFont);
				//myGLCD.setBackColor(0, 0, 0);
				//myGLCD.printNumI(0, 5, 44);
				//myGLCD.print(" ms", 88, 44);
				//myGLCD.setFont(SmallFont);
				//myGLCD.setBackColor(0, 0, 255);
				//myGLCD.setColor(0, 0, 0);
				//myGLCD.fillCircle(227, 10, 10);
			}
			//myGLCD.setBackColor(0, 0, 0);
			//myGLCD.setColor(255, 255, 255);
			//myGLCD.print("     ", 250, 212);
			////	myGLCD.printNumI(i_trig_syn, 250, 212);
			//myGLCD.print("     ", 250, 224);
			//myGLCD.printNumI(Trigger, 250, 224);
			//myGLCD.drawCircle(227, 10, 10);

			myGLCD.setBackColor(0, 0, 0);
			set_timeZero = i2c_eeprom_ulong_read(adr_set_timeZero);
			//	timePeriod = i2c_eeprom_ulong_read(adr_timePeriod);
			//myGLCD.printNumF(set_timeSynhro / 1000000.00, 2, 250, 200, ',');
			//myGLCD.print("sec", 285, 200);
			//int b = i2c_eeprom_read_byte(deviceaddress, adr_count3_kn);                // ����������  ������� ��������� 1 ������ ��������� ������������
			//myGLCD.print("     ", 250, 188);                                           // ����������  ������� ��������� 1 ������ ��������� ������������
			//myGLCD.print(proc_volume[b], 250, 188);                                    // ����������  ������� ��������� 1 ������ ��������� ������������

		//	ypos_trig = 255 - (Trigger / koeff_h) - hpos;                              // �������� ������� ������
			//if (ypos_trig != ypos_trig_old)                                            // ������� ������ ������� ������ ��� ���������
			//{
			//	myGLCD.setColor(0, 0, 0);
			//	myGLCD.drawLine(1, ypos_trig_old, 240, ypos_trig_old);
			//	ypos_trig_old = ypos_trig;
			//}
			//myGLCD.setColor(255, 0, 0);
			//myGLCD.drawLine(1, ypos_trig, 240, ypos_trig);                             // ���������� ����� ����� ������ ������
			//myGLCD.setColor(0, 0, 0);
			//myGLCD.fillRoundRect(0, 162, 242, 176);                                    // �������� ������� ������ ������ �����
	
			//myGLCD.setColor(0, 0, 0);
			//myGLCD.fillCircle(200, 10, 10);
			//myGLCD.setColor(255, 255, 255);
			//myGLCD.drawCircle(200, 10, 10);
		}

		// �������� ������ ����
		if (myTouch.dataAvailable())
		{
			delay(10);
			myTouch.read();
			x_osc = myTouch.getX();
			y_osc = myTouch.getY();

			if ((x_osc >= 2) && (x_osc <= 240))               // ����� �� ���������. ������ �� ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 160))           // 
				{
					Timer7.stop();                            //  ���������� ������ �� ����� ��������� �����
					break;                                    //
				}                                             //
			}

			//myGLCD.setBackColor(0, 0, 255);                   // ����� ��� ������
			//myGLCD.setColor(255, 255, 255);
			//myGLCD.setFont(SmallFont);

			//if ((x_osc >= 245) && (x_osc <= 280))               // ������� ������
			//{
			//	if ((y_osc >= 1) && (y_osc <= 40))              // ������  ������
			//	{
			//		waitForIt(245, 1, 318, 40);
			//		//chench_mode(0);                             //
			//	}

			//	if ((y_osc >= 45) && (y_osc <= 85))             // ������ - �������
			//	{
			//		waitForIt(245, 45, 318, 85);
			//		trigger_volume(0);
			//	}
			//	if ((y_osc >= 90) && (y_osc <= 130))           // ������ - ��������
			//	{
			//		waitForIt(245, 90, 318, 130);
			//		chench_mode1(0);
			//	}
			//}

			//if ((x_osc >= 282) && (x_osc <= 318))              // ������� ������
			//{
			//	if ((y_osc >= 1) && (y_osc <= 40))             // ������  ������
			//	{
			//		waitForIt(245, 1, 318, 40);
			//		//chench_mode(1);                            //  
			//	}

			//	if ((y_osc >= 45) && (y_osc <= 85))            // ������ - �������
			//	{
			//		waitForIt(245, 45, 318, 85);
			//		trigger_volume(1);
			//	}
			//	if ((y_osc >= 90) && (y_osc <= 130))          // ������ - ��������
			//	{
			//		waitForIt(245, 90, 318, 130);
			//		chench_mode1(1);
			//	}
			//}

			if ((x_osc >= 245) && (x_osc <= 318))                  // ������� ������
			{
				if ((y_osc >= 135) && (y_osc <= 175))              // ��������� ����������
				{
					waitForIt(245, 135, 318, 175);
					i2c_eeprom_ulong_write(adr_set_timeZero, EndMeasure - StartSynhro);  // �������� ����������� ����� 
				}
			}

			//if ((y_osc >= 205) && (y_osc <= 239))                 // ������ ������ ������������ 
			//{
			//	if ((x_osc >= 10) && (x_osc <= 60))               //  ������ �1
			//	{
			//		waitForIt(10, 210, 60, 239);
			//		set_volume(1, 1);                             // ����������� "+" ��������� �1

			//	}
			//	if ((x_osc >= 70) && (x_osc <= 120))              //  ������ �2
			//	{
			//		waitForIt(70, 210, 120, 239);
			//		set_volume(1, 2);                            // ����������� "-" ��������� �1

			//	}
			//	if ((x_osc >= 130) && (x_osc <= 180))            //  ������ �3
			//	{
			//		waitForIt(130, 210, 180, 239);
			//		set_volume(2, 1);                            // ����������� "+" ��������� �2

			//	}
			//	if ((x_osc >= 190) && (x_osc <= 240))            //  ������ �4
			//	{
			//		waitForIt(190, 210, 240, 239);
			//		set_volume(2, 2);                            // ����������� "-" ��������� �1

			//	}
			//}
		}
		// ��������� �������� ������  ����

		//attachInterrupt(kn_red, volume_up, FALLING);
		//attachInterrupt(kn_blue, volume_down, FALLING);
		//data_out[2] = 6;                                    //
		//if (kn != 0) radio_send_command();                                             //  ��������� ����� ������ ��������� ������ ���������;
	}
	koeff_h = 7.759 * 4;
	mode1 = 0;             // �/���
	mode = 3;              // ����� ���������  
	tmode = 5;             // ������� �������� ������
	trigger_volume(tmode); // ������� �������� ������
	myGLCD.setFont(BigFont);
	while (myTouch.dataAvailable()) {}
}


void synhro_by_timer()
{
	myGLCD.setBackColor(0, 0, 0);
	myGLCD.clrScr();
	buttons_right();                                              // ���������� ������ ������ ������;
	button_down();                                                // ���������� ������ ����� ������;

	tmode = 4;                                                    // ������� �������� ������
	trigger_volume(tmode);                                        // ������� �������� ������
	int x_dTime;
	scale_strob = 2;                                              // ����� ���������
	count_ms = 0;
	for (xpos = 0; xpos < 240; xpos++)                            // ������� ������ ������ ����������� � ��������� �����
	{
		Synhro_osc[xpos][ms_info] = 0;                            // ������� ������ ������ ��������� �����
		Synhro_osc[xpos][line_info] = 0;                                          // ������� ������ ������ ��������� �����
	}
	unsigned long Control_synhro = micros();                      // ���������� ��� �������� ������� ��������������
	data_out[2] = 2;                                              // ��������� ������� ��������� �������� �� ��������������
	radio_send_command();                                         // ���������� ������� ��������� �������� �� ��������������   

	DrawGrid1();                                                  // ���������� �����
	int Trigger1 = 2047;
	while (1)
	{
		if (myTouch.dataAvailable())
		{
			delay(10);
			myTouch.read();
			x_osc = myTouch.getX();
			y_osc = myTouch.getY();

			if ((x_osc >= 2) && (x_osc <= 240))                 // ����� �� ���������. ������ �� ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 160))         // 
				{
					Timer7.stop();                      //  ���������� ������������� ������ �� �����
					break;                              //
				}                                       //
			}                                           //
			myGLCD.setBackColor(0, 0, 255);
			myGLCD.setFont(SmallFont);
			myGLCD.setColor(255, 255, 255);

			myGLCD.drawRoundRect(245, 1, 318, 40);             // ���������� ���������� ������ ����� ������
			myGLCD.drawRoundRect(245, 45, 318, 85);
			myGLCD.drawRoundRect(245, 90, 318, 130);
			myGLCD.drawRoundRect(245, 135, 318, 175);

			if ((x_osc >= 245) && (x_osc <= 280))               // ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 40))              // ������  ������
				{
					waitForIt(245, 1, 318, 40);
					chench_mode(0);                             //
				}

				if ((y_osc >= 45) && (y_osc <= 85))             // ������ - �������
				{
					waitForIt(245, 45, 318, 85);
					trigger_volume(tmode--);
				}
				if ((y_osc >= 90) && (y_osc <= 130))           // ������ - ��������
				{
					waitForIt(245, 90, 318, 130);
					chench_mode1(0);
				}
			}

			if ((x_osc >= 282) && (x_osc <= 318))              // ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 40))             // ������  ������
				{
					waitForIt(245, 1, 318, 40);
					chench_mode(1);                            //  
				}

				if ((y_osc >= 45) && (y_osc <= 85))            // ������ - �������
				{
					waitForIt(245, 45, 318, 85);
					trigger_volume(tmode++);
				}
				if ((y_osc >= 90) && (y_osc <= 130))          // ������ - ��������
				{
					waitForIt(245, 90, 318, 130);
					chench_mode1(1);
				}
			}

			if ((x_osc >= 245) && (x_osc <= 318))               // ������� ������
			{
				if ((y_osc >= 135) && (y_osc <= 175))           // ��������� ����������
				{
					waitForIt(245, 135, 318, 175);
					i2c_eeprom_ulong_write(adr_set_timeZero, EndMeasure - StartMeasure);  // �������� ����������� ����� ��� ������� ������������ ������
				}
			}

			if ((y_osc >= 200) && (y_osc <= 225))                 // ������ ������ ������������ 
			{
				if ((x_osc >= 10) && (x_osc <= 42))               //  ������ �1
				{
					waitForIt(10, 200, 42, 225);
					set_volume(1, 1);

				}
				if ((x_osc >= 52) && (x_osc <= 84))               //  ������ �2
				{
					waitForIt(52, 200, 84, 225);
					set_volume(1, 2);

				}
				if ((x_osc >= 94) && (x_osc <= 126))               //  ������ �3
				{
					waitForIt(94, 200, 126, 225);
					set_volume(2, 1);

				}
				if ((x_osc >= 136) && (x_osc <= 168))               //  ������ �4
				{
					waitForIt(136, 200, 168, 225);
					set_volume(2, 2);

				}
				if ((x_osc >= 178) && (x_osc <= 210))               //  ������ �5   ��������� ��������� ��������
				{
					waitForIt(178, 200, 210, 225);
					set_volume(3, 1);                               // ��������� ��������� ��������

				}
				if ((x_osc >= 220) && (x_osc <= 252))               //  ������ �6 ��������� ��������� ��������
				{
					waitForIt(220, 200, 252, 225);
					set_volume(3, 2);                               // ��������� ��������� ��������

				}
			}
		}

		trig_sin = false;

		if (micros() - Control_synhro > 6000000)                  // ������� ������������� � ������� 6 ������
		{
			myGLCD.setColor(255, 255, 255);
			myGLCD.setBackColor(0, 0, 0);
			myGLCD.print("He""\xA4", 160, 20);                    // ���
			myGLCD.print("c""\x9D\xA2""xpo", 140, 45);            // ������
			break;                                                // ��������� ����� ��������������, �� ������
		}
		if (start_synhro)                                         // ������������� �������
		{
			StartMeasure    = micros();                           // ��������� ����� ��������� ��������������
			Control_synhro = micros();                            // ��������� ����� ��������� ��������������
			start_synhro   = false;                               // ����� ������ �������������� ������. ��������� ��������� ���������.
			Timer7.start(scale_strob * 1000.0);                   // �������� ��������� ����� �� ������

			while (!trig_sin)                                                        // �������� ������� ������ ��������������.  ������ ����������� �������� Timer5
			{
				if (micros() - StartMeasure > 3000000)                                    // ������� ������������� � ������� 3 ������
				{
					EndMeasure = micros();                                        // �������� ����� ������������ �������� ������
					trig_sin = true;                                             // ���������� ���� ������������ �������� ������
					break;                                                               // ��������� �������� ��������������
				}
				for (xpos = 0; xpos < 240; xpos++)                                   // ����� �� 240 ������
				{
					ADC_CR = ADC_START; 	                                         // ��������� ��������������
					while (!(ADC_ISR_DRDY));                                         // �������� ���������� ��������������
					Sample_osc[xpos] = 0;
					Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
					Synhro_osc[xpos][line_info] = 0;
					Sample_osc[xpos] = ADC->ADC_CDR[7];                        // �������� ������ �0 � �������� � ������
				//	Synhro_osc[xpos][ms_info] = 0;                                      // ������� ������ ������ ��������� �����
					if ((Sample_osc[xpos] > Trigger) && (trig_sin == false) && (xpos > 2))   // ����� ���������� ������ ������
					{
						EndMeasure = micros();                                        // �������� ����� ������������ �������� ������
						trig_sin = true;                                             // ���������� ���� ������������ �������� ������
						//page_trig = page;                                            // ���������� ����� ����� � ������� �������� �������
					}
					delayMicroseconds(dTime);                                        // dTime ��������� �������� (��������) ��������� 
				}
				if (trig_sin == true) break;                                       // ��������� ������������ �� ����� � ������� �������� �����
			}

			Timer7.stop();                                        // ���������� ������������ ��������� �����
			count_ms = 0;

			myGLCD.setColor(0, 0, 0);
			myGLCD.fillRoundRect(0, 20, 240, 176);
			myGLCD.setColor(255, 255, 255);
			myGLCD.setBackColor(0, 0, 0);

			set_timeZero = i2c_eeprom_ulong_read(adr_set_timeZero);

			myGLCD.setFont(BigFont);
			myGLCD.print("\x85""a""\x99""ep""\x9B\x9F""a", 2, 20); // "��������"
			myGLCD.print("c""\x9D\x98\xA2""a""\xA0""a", 2, 40);    // "�������"
			myGLCD.print("   ", 160, 20);                          // �������� ������� "���"
			myGLCD.print("      ", 140, 45);                       // �������� ������� "������"
			DrawGrid1();
			if (trig_sin)                                          // ������ ���������������                              
			{
				myGLCD.printNumF(((EndMeasure - StartMeasure) - set_timeZero) / 1000.00, 3, 5, 60);
				myGLCD.print(" ms", 88, 60);
			}
			else                                                   // ������ �� ���������������        
			{
				myGLCD.print("      ", 5, 60);                     // �������� ���� ������

			}

			ypos_trig = 255 - (Trigger / koeff_h) - hpos;                             // �������� ������� ������ ��� �����������
			if (ypos_trig != ypos_trig_old)                                            // ������� ������ ������� ������ ��� ���������
			{
				myGLCD.setColor(0, 0, 0);
				myGLCD.drawLine(1, ypos_trig_old, 240, ypos_trig_old);
				ypos_trig_old = ypos_trig;
				Serial.println(Trigger);
			}
			myGLCD.setColor(255, 0, 0);
			myGLCD.drawLine(1, ypos_trig, 240, ypos_trig);                             // ���������� ����� ����� ������ ������
			myGLCD.setColor(0, 0, 0);
			myGLCD.fillRoundRect(0, 162, 242, 176);                                    // �������� ������� ������ ������ �����

			for (int xpos = 0; xpos < 240; xpos++)                                     // ����� �� �����
			{
				// ���������� �����  �������������
				myGLCD.setFont(SmallFont);
				myGLCD.setColor(255, 255, 255);
				ypos_osc1_0 = 255 - (Sample_osc[xpos] / koeff_h) - hpos;
				ypos_osc2_0 = 255 - (Sample_osc[xpos + 1] / koeff_h) - hpos;
				if (ypos_osc1_0 < 0) ypos_osc1_0 = 0;
				if (ypos_osc2_0 < 0) ypos_osc2_0 = 0;
				if (ypos_osc1_0 > 220) ypos_osc1_0 = 220;
				if (ypos_osc2_0 > 220) ypos_osc2_0 = 220;
				myGLCD.drawLine(xpos, ypos_osc1_0, xpos + 1, ypos_osc2_0);


				//myGLCD.setFont(SmallFont);
				myGLCD.setColor(VGA_WHITE);
				myGLCD.setColor(VGA_YELLOW);

				if (Synhro_osc[xpos][line_info] == 4095)
				{
					myGLCD.drawLine(xpos, 80, xpos, 160);

				}
				if (Synhro_osc[xpos][ms_info] > 0)
				{
					if (xpos > 230)
					{
						myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos - 12, 165);
					}
					else
					{
						myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos, 165);

					}
				}

				Sample_osc[xpos] = 0;
				Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
				Synhro_osc[xpos][line_info] = 0;


				/*
				if (Synhro_osc[xpos] > 0)
				{
					myGLCD.setColor(VGA_YELLOW);
					if (Synhro_osc[xpos][line_info] == 4095)
					{
						myGLCD.drawLine(xpos, 80, xpos, 160);
					}
					else
					{
						myGLCD.setColor(255, 255, 255);
						if (xpos > 230)
						{
							myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos - 12, 165);
						}
						else
						{
							myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos, 165);

						}
					}
					myGLCD.setColor(255, 255, 255);
					Synhro_osc[xpos][ms_info] = 0;
				}


				*/
				//myGLCD.setColor(0, 0, 0);
				//myGLCD.fillCircle(200, 10, 10);
				//myGLCD.setColor(255, 255, 255);
				//myGLCD.drawCircle(200, 10, 10);
			}
		}
	}
	koeff_h = 7.759 * 4;
	mode1 = 0;                                          // �/���
	mode = 3;                                           // ����� ���������  
//	tmode = 5;                                          // ������� �������� ������
	while (myTouch.dataAvailable()) {}                  // ����� �� ���������
}

void synhro_by_timer_1()
{
	myGLCD.setBackColor(0, 0, 0);
	myGLCD.clrScr();
	//buttons_right();                                              // ���������� ������ ������ ������;
	//button_down();                                                // ���������� ������ ����� ������;

	tmode = 4;                                                    // ������� �������� ������
	trigger_volume(tmode);                                        // ������� �������� ������
	int x_dTime;
	scale_strob = 2;                                              // ����� ���������
	count_ms = 0;
	long time_temp = 0;
	for (xpos = 0; xpos < 240; xpos++)                            // ������� ������ ������ ����������� � ��������� �����
	{
		Synhro_osc[xpos][ms_info] = 0;                            // ������� ������ ������ ��������� �����
		Synhro_osc[xpos][line_info] = 0;                                          // ������� ������ ������ ��������� �����
	}
	unsigned long Control_synhro = micros();                      // ���������� ��� �������� ������� ��������������
	data_out[2] = 2;                                              // ��������� ������� ��������� �������� �� ��������������
	radio_send_command();                                         // ���������� ������� ��������� �������� �� ��������������   

	DrawGrid1();                                                  // ���������� �����
	int Trigger1 = 2047;
	while (1)
	{
		if (myTouch.dataAvailable())
		{
			delay(10);
			myTouch.read();
			x_osc = myTouch.getX();
			y_osc = myTouch.getY();

			if ((x_osc >= 2) && (x_osc <= 240))                 // ����� �� ���������. ������ �� ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 160))         // 
				{
					Timer7.stop();                      //  ���������� ������������� ������ �� �����
					break;                              //
				}                                       //
			}                                           //
			//myGLCD.setBackColor(0, 0, 255);
			//myGLCD.setFont(SmallFont);
			//myGLCD.setColor(255, 255, 255);

			//myGLCD.drawRoundRect(245, 1, 318, 40);             // ���������� ���������� ������ ����� ������
			//myGLCD.drawRoundRect(245, 45, 318, 85);
			//myGLCD.drawRoundRect(245, 90, 318, 130);
			//myGLCD.drawRoundRect(245, 135, 318, 175);

			//if ((x_osc >= 245) && (x_osc <= 280))               // ������� ������
			//{
			//	if ((y_osc >= 1) && (y_osc <= 40))              // ������  ������
			//	{
			//		waitForIt(245, 1, 318, 40);
			//		chench_mode(0);                             //
			//	}

			//	if ((y_osc >= 45) && (y_osc <= 85))             // ������ - �������
			//	{
			//		waitForIt(245, 45, 318, 85);
			//		trigger_volume(tmode--);
			//	}
			//	if ((y_osc >= 90) && (y_osc <= 130))           // ������ - ��������
			//	{
			//		waitForIt(245, 90, 318, 130);
			//		chench_mode1(0);
			//	}
			//}

			//if ((x_osc >= 282) && (x_osc <= 318))              // ������� ������
			//{
			//	if ((y_osc >= 1) && (y_osc <= 40))             // ������  ������
			//	{
			//		waitForIt(245, 1, 318, 40);
			//		chench_mode(1);                            //  
			//	}

			//	if ((y_osc >= 45) && (y_osc <= 85))            // ������ - �������
			//	{
			//		waitForIt(245, 45, 318, 85);
			//		trigger_volume(tmode++);
			//	}
			//	if ((y_osc >= 90) && (y_osc <= 130))          // ������ - ��������
			//	{
			//		waitForIt(245, 90, 318, 130);
			//		chench_mode1(1);
			//	}
			//}

			//if ((x_osc >= 245) && (x_osc <= 318))               // ������� ������
			//{
			//	if ((y_osc >= 135) && (y_osc <= 175))           // ��������� ����������
			//	{
			//		waitForIt(245, 135, 318, 175);
			//		i2c_eeprom_ulong_write(adr_set_timeZero, EndMeasure - StartMeasure);  // �������� ����������� ����� ��� ������� ������������ ������
			//	}
			//}

			//if ((y_osc >= 200) && (y_osc <= 225))                 // ������ ������ ������������ 
			//{
			//	if ((x_osc >= 10) && (x_osc <= 42))               //  ������ �1
			//	{
			//		waitForIt(10, 200, 42, 225);
			//		set_volume(1, 1);

			//	}
			//	if ((x_osc >= 52) && (x_osc <= 84))               //  ������ �2
			//	{
			//		waitForIt(52, 200, 84, 225);
			//		set_volume(1, 2);

			//	}
			//	if ((x_osc >= 94) && (x_osc <= 126))               //  ������ �3
			//	{
			//		waitForIt(94, 200, 126, 225);
			//		set_volume(2, 1);

			//	}
			//	if ((x_osc >= 136) && (x_osc <= 168))               //  ������ �4
			//	{
			//		waitForIt(136, 200, 168, 225);
			//		set_volume(2, 2);

			//	}
			//	if ((x_osc >= 178) && (x_osc <= 210))               //  ������ �5   ��������� ��������� ��������
			//	{
			//		waitForIt(178, 200, 210, 225);
			//		set_volume(3, 1);                               // ��������� ��������� ��������

			//	}
			//	if ((x_osc >= 220) && (x_osc <= 252))               //  ������ �6 ��������� ��������� ��������
			//	{
			//		waitForIt(220, 200, 252, 225);
			//		set_volume(3, 2);                               // ��������� ��������� ��������

			//	}
			//}
		}

		trig_sin = false;

		if (micros() - Control_synhro > 6000000)                  // ������� ������������� � ������� 6 ������
		{
			myGLCD.setColor(255, 255, 255);
			myGLCD.setBackColor(0, 0, 0);
			myGLCD.print("He""\xA4", 160, 20);                    // ���
			myGLCD.print("c""\x9D\xA2""xpo", 140, 45);            // ������
			break;                                                // ��������� ����� ��������������, �� ������
		}
		if (start_synhro)                                         // ������������� �������
		{
			StartMeasure = micros();                           // ��������� ����� ��������� ��������������
			Control_synhro = micros();                            // ��������� ����� ��������� ��������������
			start_synhro = false;                               // ����� ������ �������������� ������. ��������� ��������� ���������.
			Timer7.start(scale_strob * 1000.0);                   // �������� ��������� ����� �� ������

			while (!trig_sin)                                                        // �������� ������� ������ ��������������.  ������ ����������� �������� Timer5
			{
				if (micros() - StartMeasure > 3000000)                                    // ������� ������������� � ������� 3 ������
				{
					EndMeasure = micros();                                        // �������� ����� ������������ �������� ������
					trig_sin = true;                                             // ���������� ���� ������������ �������� ������
					break;                                                               // ��������� �������� ��������������
				}
				for (xpos = 0; xpos < 240; xpos++)                                   // ����� �� 240 ������
				{
					ADC_CR = ADC_START; 	                                         // ��������� ��������������
					while (!(ADC_ISR_DRDY));                                         // �������� ���������� ��������������
					Sample_osc[xpos] = 0;
					Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
					Synhro_osc[xpos][line_info] = 0;
					Sample_osc[xpos] = ADC->ADC_CDR[7];                        // �������� ������ �0 � �������� � ������
																			   //	Synhro_osc[xpos][ms_info] = 0;                                      // ������� ������ ������ ��������� �����
					if ((Sample_osc[xpos] > Trigger) && (trig_sin == false) && (xpos > 2))   // ����� ���������� ������ ������
					{
						EndMeasure = micros();                                        // �������� ����� ������������ �������� ������
						trig_sin = true;                                             // ���������� ���� ������������ �������� ������
																					 //page_trig = page;                                            // ���������� ����� ����� � ������� �������� �������
					}
					delayMicroseconds(dTime);                                        // dTime ��������� �������� (��������) ��������� 
				}
				if (trig_sin == true) break;                                       // ��������� ������������ �� ����� � ������� �������� �����
			}

			Timer7.stop();                                        // ���������� ������������ ��������� �����
			count_ms = 0;

		//	myGLCD.setColor(255, 255, 255);
			myGLCD.setBackColor(0, 0, 0);

			set_timeZero = i2c_eeprom_ulong_read(adr_set_timeZero);

			//myGLCD.setFont(BigFont);
			//myGLCD.print("\x85""a""\x99""ep""\x9B\x9F""a c""\x9D\x98\xA2""a""\xA0""a ms", 2, 180); // "��������"
			//myGLCD.print("   ", 160, 20);                          // �������� ������� "���"
			//myGLCD.print("      ", 140, 45);                       // �������� ������� "������"
			DrawGrid1();
			if (trig_sin)                                          // ������ ���������������                              
			{

				myGLCD.setColor(0, 0, 0);
				myGLCD.drawRoundRect(0, 180, 318, 239);
				myGLCD.setFont(SevenSegNumFont);
				time_temp = EndMeasure - StartSynhro - set_timeZero;
				if (time_temp < 0) time_temp = 0;
				myGLCD.setColor(255, 255, 255);
				myGLCD.printNumI(time_temp / 1000.00, 5, 180); // ����� �������� ����������
			}
			else                                                   // ������ �� ���������������        
			{
				myGLCD.setColor(0, 0, 0);
				myGLCD.drawRoundRect(0, 180, 318, 239);

				//myGLCD.print("      ", 5, 60);                     // �������� ���� ������

			}

			ypos_trig = 255 - (Trigger / koeff_h) - hpos;                             // �������� ������� ������ ��� �����������
			if (ypos_trig != ypos_trig_old)                                            // ������� ������ ������� ������ ��� ���������
			{
				myGLCD.setColor(0, 0, 0);
				myGLCD.drawLine(1, ypos_trig_old, 240, ypos_trig_old);
				ypos_trig_old = ypos_trig;
				Serial.println(Trigger);
			}
			myGLCD.setColor(255, 0, 0);
			myGLCD.drawLine(1, ypos_trig, 240, ypos_trig);                             // ���������� ����� ����� ������ ������
			myGLCD.setColor(0, 0, 0);
			myGLCD.fillRoundRect(0, 162, 242, 176);                                    // �������� ������� ������ ������ �����

			for (int xpos = 0; xpos < 240; xpos++)                                     // ����� �� �����
			{
				// ���������� �����  �������������
				myGLCD.setFont(SmallFont);
				myGLCD.setColor(255, 255, 255);
				ypos_osc1_0 = 255 - (Sample_osc[xpos] / koeff_h) - hpos;
				ypos_osc2_0 = 255 - (Sample_osc[xpos + 1] / koeff_h) - hpos;
				if (ypos_osc1_0 < 0) ypos_osc1_0 = 0;
				if (ypos_osc2_0 < 0) ypos_osc2_0 = 0;
				if (ypos_osc1_0 > 220) ypos_osc1_0 = 220;
				if (ypos_osc2_0 > 220) ypos_osc2_0 = 220;
				myGLCD.drawLine(xpos, ypos_osc1_0, xpos + 1, ypos_osc2_0);


				//myGLCD.setFont(SmallFont);
			//	myGLCD.setColor(VGA_WHITE);
				myGLCD.setColor(VGA_YELLOW);

				if (Synhro_osc[xpos][line_info] == 4095)
				{
					myGLCD.drawLine(xpos, 80, xpos, 160);

				}
				if (Synhro_osc[xpos][ms_info] > 0)
				{
					if (xpos > 230)
					{
						myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos - 12, 165);
					}
					else
					{
						myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos, 165);

					}
				}

				//Sample_osc[xpos] = 0;
				//Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
				//Synhro_osc[xpos][line_info] = 0;


				/*
				if (Synhro_osc[xpos] > 0)
				{
				myGLCD.setColor(VGA_YELLOW);
				if (Synhro_osc[xpos][line_info] == 4095)
				{
				myGLCD.drawLine(xpos, 80, xpos, 160);
				}
				else
				{
				myGLCD.setColor(255, 255, 255);
				if (xpos > 230)
				{
				myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos - 12, 165);
				}
				else
				{
				myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos, 165);

				}
				}
				myGLCD.setColor(255, 255, 255);
				Synhro_osc[xpos][ms_info] = 0;
				}


				*/
	/*			myGLCD.setColor(0, 0, 0);
				myGLCD.fillCircle(200, 10, 10);
				myGLCD.setColor(255, 255, 255);
				myGLCD.drawCircle(200, 10, 10);*/
			}
		}
	}
	koeff_h = 7.759 * 4;
	mode1 = 0;                                          // �/���
	mode = 3;                                           // ����� ���������  
														//	tmode = 5;                                          // ������� �������� ������
	while (myTouch.dataAvailable()) {}                  // ����� �� ���������
}


void oscilloscope()                                     // �������� � �������� �������. ������������� �� �������������
{
	myGLCD.clrScr();
	myGLCD.setBackColor(0, 0, 0);
	buttons_right();                                      // ���������� ������ ������ ������;
	buttons_channelNew();                                 // ���������� ������ ����� ������;
	myGLCD.setBackColor(0, 0, 0);
	myGLCD.setFont(BigFont);
	myGLCD.setColor(VGA_LIME);
	myGLCD.print(txt_info29, 5, 190);                    // "Stop->PUSH Disp"

	mode = 3;                                            // ����� ���������  
	count_ms = 0;                                        // ����� ������� �� � ��������� ���������
	set_volume(5, 1);                                    // ���������� ������� ���������  ������� ��������� �� ����
	page = 0;                                            // ������ 1 ��������
	for (xpos = 0; xpos < 240; xpos++)                   // ������� ������ ������ ����������� � ��������� �����
	{
		OldSample_osc[xpos][page] = 0;                   // ������� ������ ������ �����������
		Synhro_osc[xpos][ms_info] = 0;                          // �������� ��������� ����� � ������ ��� ������ �� �����
		Synhro_osc[xpos][line_info] = 0;              // �������� ��������� ����� � ������ ��� ������ �� �����;            // ������� ����� ������ ��������� �����
	}

	//for (xpos = 0; xpos < 240; xpos++)                                            // ������� ������ ������ ����������� � ��������� �����
	//{
	//	Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
	//	Synhro_osc[xpos][line_info] = 0;                                          // ������� ������ ������ ��������� �����
	//}



	while (1)                                            //                
	{
		Timer7.start(scale_strob * 1000);                       // �������� ������������  ��������� ����� �� ������

																// ��������� ������ ������ ���. �������� ���������� ������ � ���� ������
		ADC_CHER = 0;                                           // �������� ���������� ���� "0"          (1<<7) | (1<<6) for adc 7= A0, 6=A1 , 5=A2, 4 = A3
		for (xpos = 0; xpos < 240; xpos++)                                  // ����� �� 240 ������
		{
			ADC_CR = ADC_START; 	                                        // ��������� ��������������
			while (!(ADC_ISR_DRDY));                                        // �������� ���������� ��������������
			Sample_osc[xpos] = ADC->ADC_CDR[7];                              // �������� ������ �0 � �������� � ������
			Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
			delayMicroseconds(dTime);                                       // dTime ��������� �������� (��������) ��������� 
		}
		Timer7.stop();                                                          // ��������� ������������ ��������� �����
		count_ms = 0;                                                           // ����� ������� �� � ��������� ���������


		ypos_trig = 255 - (Trigger / koeff_h) - hpos;                           // �������� ������� ������
		if (ypos_trig != ypos_trig_old)                                         // ������� ������ ������� ������ ��� ���������
		{
			myGLCD.setColor(0, 0, 0);
			myGLCD.drawLine(1, ypos_trig_old, 240, ypos_trig_old);
			ypos_trig_old = ypos_trig;
		}
		myGLCD.setColor(255, 0, 0);
		myGLCD.drawLine(1, ypos_trig, 240, ypos_trig);                           // ���������� ����� ����� ������ ������


		for (int xpos = 0; xpos < 239; xpos++)                                   // ����� �� �����
		{
			// ������� ���������� �����

			myGLCD.setColor(0, 0, 0);
			myGLCD.setBackColor(0, 0, 0);
			ypos_osc1_0 = 255 - (OldSample_osc[xpos][page] / koeff_h) - hpos;
			ypos_osc2_0 = 255 - (OldSample_osc[xpos + 1][page] / koeff_h) - hpos;
			if (ypos_osc1_0 < 0) ypos_osc1_0 = 0;
			if (ypos_osc2_0 < 0) ypos_osc2_0 = 0;
			if (ypos_osc1_0 > 220) ypos_osc1_0 = 220;
			if (ypos_osc2_0 > 220) ypos_osc2_0 = 220;
			myGLCD.drawLine(xpos, ypos_osc1_0, xpos + 1, ypos_osc2_0);
			myGLCD.drawLine(xpos + 1, ypos_osc1_0, xpos + 2, ypos_osc2_0);

			if (Synhro_osc[xpos] > 0)
			{
				if (Synhro_osc[xpos] != Synhro_osc[xpos])
				{
					if (Synhro_osc[xpos][line_info] == 4095)
					{
						myGLCD.drawLine(xpos, 60, xpos, 160);
					}
					else
					{
						if (xpos > 230)
						{
							myGLCD.print("  ", xpos-10, 165);                    // ������� ��������� �����
						}
						else
						{
							myGLCD.print("  ", xpos-2, 165);                    // ������� ��������� �����

						}
					}
				}
			}

			// ���������� �����  �������������
			myGLCD.setColor(255, 255, 255);
			ypos_osc1_0 = 255 - (Sample_osc[xpos] / koeff_h) - hpos;
			ypos_osc2_0 = 255 - (Sample_osc[xpos + 1] / koeff_h) - hpos;
			if (ypos_osc1_0 < 0) ypos_osc1_0 = 0;
			if (ypos_osc2_0 < 0) ypos_osc2_0 = 0;
			if (ypos_osc1_0 > 220) ypos_osc1_0 = 220;
			if (ypos_osc2_0 > 220) ypos_osc2_0 = 220;
			myGLCD.drawLine(xpos, ypos_osc1_0, xpos + 1, ypos_osc2_0);

			if (Synhro_osc[xpos] > 0)
			{
				myGLCD.setColor(VGA_YELLOW);
				if (Synhro_osc[xpos][line_info] == 4095)
				{
					myGLCD.drawLine(xpos, 60, xpos, 160);
				}
				else
				{
					if (Synhro_osc[xpos] != Synhro_osc[xpos])
					{
						myGLCD.setColor(255, 255, 255);
						if (xpos > 230)
						{
							myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos - 12, 165);
						}
						else
						{
							myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos, 165);

						}
					}
				}
				myGLCD.setColor(255, 255, 255);
			}
			OldSample_osc[xpos][page] = Sample_osc[xpos];
			//Synhro_osc[xpos] = Synhro_osc[xpos];
		}
		DrawGrid1();
		if (myTouch.dataAvailable())
		{
			delay(10);
			myTouch.read();
			x_osc = myTouch.getX();
			y_osc = myTouch.getY();

			if ((x_osc >= 2) && (x_osc <= 240))               // ����� �� ���������. ������ �� ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 160))           // 
				{
					Timer7.stop();                            //  ���������� ������ �� ����� ��������� �����
					break;                                    //
				}                                             //
			}                                                 //
			myGLCD.setBackColor(0, 0, 255);                   // ����� ��� ������
			myGLCD.setColor(255, 255, 255);
			myGLCD.setFont(SmallFont);

			if ((x_osc >= 245) && (x_osc <= 280))               // ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 40))              // ������  ������
				{
					waitForIt(245, 1, 318, 40);
					chench_mode(0);                             //
				}

				if ((y_osc >= 45) && (y_osc <= 85))             // ������ - �������
				{
					waitForIt(245, 45, 318, 85);
					trigger_volume(0);
				}
				if ((y_osc >= 90) && (y_osc <= 130))           // ������ - ��������
				{
					waitForIt(245, 90, 318, 130);
					chench_mode1(0);
				}
			}

			if ((x_osc >= 282) && (x_osc <= 318))              // ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 40))             // ������  ������
				{
					waitForIt(245, 1, 318, 40);
					chench_mode(1);                            //  
				}

				if ((y_osc >= 45) && (y_osc <= 85))            // ������ - �������
				{
					waitForIt(245, 45, 318, 85);
					trigger_volume(1);
				}
				if ((y_osc >= 90) && (y_osc <= 130))          // ������ - ��������
				{
					waitForIt(245, 90, 318, 130);
					chench_mode1(1);
				}
			}

			if ((x_osc >= 245) && (x_osc <= 318))                  // ������� ������
			{
				if ((y_osc >= 135) && (y_osc <= 175))              // ��������� ����������
				{
					waitForIt(245, 135, 318, 175);
				}
			}

			if ((y_osc >= 205) && (y_osc <= 239))                 // ������ ������ ������������ 
			{
				if ((x_osc >= 10) && (x_osc <= 60))               //  ������ �1
				{
					waitForIt(10, 210, 60, 239);
					set_volume(1, 1);                             // ����������� "+" ��������� �1

				}
				if ((x_osc >= 70) && (x_osc <= 120))              //  ������ �2
				{
					waitForIt(70, 210, 120, 239);
					set_volume(1, 2);                            // ����������� "-" ��������� �1

				}
				if ((x_osc >= 130) && (x_osc <= 180))            //  ������ �3
				{
					waitForIt(130, 210, 180, 239);
					set_volume(2, 1);                            // ����������� "+" ��������� �2

				}
				if ((x_osc >= 190) && (x_osc <= 240))            //  ������ �4
				{
					waitForIt(190, 210, 240, 239);
					set_volume(2, 2);                            // ����������� "-" ��������� �1

				}
			}
		}
	}

	koeff_h = 7.759 * 4;
	mode1 = 0;                                                   // �/���
	mode = 3;                                                    // ����� ���������  
	tmode = 5;                                                   // ������� �������� ������
	trigger_volume(tmode); // ������� �������� ������
	myGLCD.setFont(BigFont);
	while (myTouch.dataAvailable()) {}
}


void synhro_by_timer_old()                                     // �������� � �������� �������
{
	//uint32_t bgnBlock, endBlock;
	//block_t block[BUFFER_BLOCK_COUNT];
//	myGLCD.clrScr();
	myGLCD.setBackColor(0, 0, 0);
	myGLCD.setColor(0, 0, 0);
	myGLCD.fillRoundRect(20, 162, 240, 196);
	//myGLCD.clrScr();
	buttons_right();                                    // ���������� ������ ������ ������;
	buttons_channelNew();                               // ���������� ������ ����� ������;
	//myGLCD.setBackColor(0, 0, 0);
	//myGLCD.setFont(BigFont);
	//myGLCD.setColor(VGA_LIME);
	//myGLCD.print(txt_info29, 5, 190);                      //
	int x_dTime;
	count_ms = 0;
	set_volume(5, 1);
	bool osc_line_off0 = false;

	unsigned long Control_synhro = micros();             // ���������� ��� �������� ������� ��������������


	drawUpButton(286, 180);                              // ���������� ������ ��������� ������ ������;
	drawDownButton(286, 211);                            // ���������� ������ ��������� ������ ������;
	data_out[2] = 2;
	radio_send_command();

	//for (page = 0; page < page_max; page++)                     // ��������� ������ ������ ���
	//{
	//	for (xpos = 0; xpos < 240; xpos++)                 // ������� ������ ������ ����������� 

	//	{
	//		OldSample_osc[xpos][page] = 0;
	//		//Synhro_osc[xpos][0] = 0.1;
	//	}
	//}
	while (1)
	{
		DrawGrid1();
		if (myTouch.dataAvailable())
		{
			delay(10);
			myTouch.read();
			x_osc = myTouch.getX();
			y_osc = myTouch.getY();

			if ((x_osc >= 2) && (x_osc <= 240))                 // ����� �� ���������. ������ �� ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 160))         // 
				{
					Timer7.stop();                      //  ���������� ������������� ������ �� �����
					break;                              //
				}                                       //
			}                                           //
			myGLCD.setBackColor(0, 0, 255);
			myGLCD.setFont(SmallFont);
			myGLCD.setColor(255, 255, 255);

			myGLCD.drawRoundRect(245, 1, 318, 40);             // ���������� ���������� ������ ����� ������
			myGLCD.drawRoundRect(245, 45, 318, 85);
			myGLCD.drawRoundRect(245, 90, 318, 130);
			myGLCD.drawRoundRect(245, 135, 318, 175);

			if ((x_osc >= 245) && (x_osc <= 280))               // ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 40))              // ������  ������
				{
					waitForIt(245, 1, 318, 40);
					chench_mode(0);                             //
				}

				if ((y_osc >= 45) && (y_osc <= 85))             // ������ - �������
				{
					waitForIt(245, 45, 318, 85);
					trigger_volume(0);
				}
				if ((y_osc >= 90) && (y_osc <= 130))           // ������ - ��������
				{
					waitForIt(245, 90, 318, 130);
					chench_mode1(0);
				}
			}

			if ((x_osc >= 282) && (x_osc <= 318))              // ������� ������
			{
				if ((y_osc >= 1) && (y_osc <= 40))             // ������  ������
				{
					waitForIt(245, 1, 318, 40);
					chench_mode(1);                            //  
				}

				if ((y_osc >= 45) && (y_osc <= 85))            // ������ - �������
				{
					waitForIt(245, 45, 318, 85);
					trigger_volume(1);
				}
				if ((y_osc >= 90) && (y_osc <= 130))          // ������ - ��������
				{
					waitForIt(245, 90, 318, 130);
					chench_mode1(1);
				}
			}

			if ((x_osc >= 245) && (x_osc <= 318))               // ������� ������
			{
				if ((y_osc >= 135) && (y_osc <= 175))          // ��������� ����������
				{
					waitForIt(245, 135, 318, 175);
					i2c_eeprom_ulong_write(adr_set_timeZero, EndMeasure - StartMeasure);  // �������� ����������� ����� ��� ������� ������������ ������
				}

				if ((y_osc >= 180) && (y_osc <= 206))          // ��������� ����������
				{
					waitForIt(286, 180, 318, 206);
					set_volume(3, 1);                           // ��������� ��������� ��������
				}

				if ((y_osc >= 211) && (y_osc <= 237))          // ��������� ����������
				{
					waitForIt(286, 211, 318, 237);
					set_volume(3, 2);                          // ��������� ��������� ��������
				}
			}

			if ((y_osc >= 205) && (y_osc <= 239))                 // ������ ������ ������������ 
			{
				if ((x_osc >= 10) && (x_osc <= 60))               //  ������ �1
				{
					waitForIt(10, 210, 60, 239);
					set_volume(1, 1);

				}
				if ((x_osc >= 70) && (x_osc <= 120))               //  ������ �2
				{
					waitForIt(70, 210, 120, 239);
					set_volume(1, 2);

				}
				if ((x_osc >= 130) && (x_osc <= 180))               //  ������ �3
				{
					waitForIt(130, 210, 180, 239);
					set_volume(2, 1);

				}
				if ((x_osc >= 190) && (x_osc <= 240))               //  ������ �4
				{
					waitForIt(190, 210, 240, 239);
					set_volume(2, 2);

				}
			}
		}
		bool synhro_Off = false;                                   // ������� ������������� � �������� ���������   
		trig_sin = false;
	//	StartSample = micros();

		if (micros() - Control_synhro > 6000000)                     // ������� ������������� � ������� 6 ������
		{
		//	synhro_Off = true;
			break;                                                   // ��������� ��������� 
		}
		if (start_synhro) Control_synhro = micros();                 // ������������� �������


		//while (!start_synhro)                                      // �������� ������� ������ ��������������
		//{
		//	if (micros() - StartSample > 6000000)                    // ������� ������������� � ������� 6 ������
		//	{
		//		synhro_Off = true;
		//		break;                                               // ��������� ��������� 
		//	}
		//}
		start_synhro = false;                                        // ����� ������ �������������� ������
	//	Timer7.start(scale_strob * 1000.0);                          // �������� ��������� ����� �� ������

	// ������  ��������� �������
	while (!trig_sin)                                                // �������� ������������ ��������
	{
		ADC_CHER = 0;    // this is (1<<7) | (1<<6) for adc 7= A0, 6=A1 , 5=A2, 4 = A3
		// ADC_CHER = Channel_x;    // this is (1<<7) | (1<<6) for adc 7= A0, 6=A1 , 5=A2, 4 = A3

		for (page = 0; page < page_max; page++)               // ��������� ������ ������ ���
		{

			for (xpos = 0; xpos < 240; xpos++)                // ��������� ������ ������ ���
			{
				ADC_CR = ADC_START; 	                      // ��������� ��������������
				while (!(ADC_ISR_DRDY));                      // �������� ���������� ��������������
				Sample_osc[xpos] = ADC->ADC_CDR[7];     // �������� ������ �0
				if (ADC->ADC_CDR[7] > Trigger)
				{
					EndMeasure = micros();                     // �������� ����� ������������ �������� ������
					trig_sin = true;                          // ���������� ���� ������������ �������� ������
				}
				delayMicroseconds(dTime);                     // dTime  ����� ���������
			}
			//if (micros() - StartSample > 3000000)    break;   // �������  ������������ �������� ������ � ������� 3 ������
			//myGLCD.printNumI(page, 255, 177);

		}


		//}



		Timer7.stop();
		//if (kn != 0) radio_transfer();                    //  ��������� ����� ������ ������;


		// ������� ���������� �� ���������
	//	myGLCD.setColor(0, 0, 0);
	////	myGLCD.fillRoundRect(0, 162, 240, 176);
	//	myGLCD.setBackColor(0, 0, 255);
	//	myGLCD.setColor(255, 255, 255);
		//	myGLCD.print("            ", 1, 165);
	//	myGLCD.print("        ", 255, 160);
	////	myGLCD.printNumI(set_timeZero, 255, 160);
	//	myGLCD.print("        ", 255, 150);
	//	//	myGLCD.printNumI((EndMeasure - timeStartTrig)/1000, 255, 150);        // ����� ��������� ������
	//	myGLCD.printNumI((EndMeasure - StartMeasure) / 1000, 255, 150);            // ����� ��������� ������

		myGLCD.setColor(255, 255, 255);
		myGLCD.setBackColor(0, 0, 0);

		myGLCD.setFont(BigFont);
		myGLCD.print("\x85""a""\x99""ep""\x9B\x9F""a", 2, 20);         // "��������"
		myGLCD.print("c""\x9D\x98\xA2""a""\xA0""a", 2, 45);           // "�������"
		myGLCD.print("      ", LEFT, 44);
		if (synhro_Off)
		{
			myGLCD.print("He""\xA4", 160, 20);                 // ���
			myGLCD.print("c""\x9D\xA2""xpo", 140, 45);        // ������
		}
		else
		{
			myGLCD.print("   ", 160, 20);                 //  
			myGLCD.print("      ", 140, 45);        //  
		}
		myGLCD.setFont(SmallFont);
		myGLCD.setBackColor(0, 0, 255);
		if (trig_sin)
		{
			//myGLCD.setColor(255, 0, 0);
			//myGLCD.drawRoundRect(245, 135, 318, 175);
			//myGLCD.setBackColor(0, 0, 255);
			//myGLCD.setColor(255, 255, 255);
			//myGLCD.print("        ", 255, 140);
			/*			if (Filter_enable)
			{
			if (trig_sinF)
			{
			myGLCD.printNumF(((EndMeasure - StartMeasure) - set_timeZero)/1000.00, 3,255, 140);
			myGLCD.setFont(BigFont);
			myGLCD.setBackColor(0, 0, 0);
			myGLCD.printNumF(((EndMeasure - StartMeasure) - set_timeZero) / 1000.00, 3, LEFT+5, 44);
			myGLCD.print(" ms", 88, 44);
			myGLCD.setFont(SmallFont);
			myGLCD.setBackColor(0, 0, 255);
			}
			else
			{
			myGLCD.printNumI(0, 255, 140);
			myGLCD.setFont(BigFont);
			myGLCD.setBackColor(0, 0, 0);
			myGLCD.printNumI(0, LEFT+5, 44);
			myGLCD.print(" ms", 88, 44);
			myGLCD.setFont(SmallFont);
			myGLCD.setBackColor(0, 0, 255);

			}
			}
			else
			{*/
			/*
			����� �������� �������:
			timeStartTrig - ����� ������������ ��������
			StartMeasure - ����� ������ ���������
			EndMeasure - ����� ���������� ���������
			set_timeZero - ����� �������� ���������� ���������

			*/
		//	myGLCD.printNumF(((EndMeasure - StartMeasure) - set_timeZero) / 1000.00, 2, 255, 140);  // ����� ����������� ����������
			myGLCD.setFont(BigFont);
			myGLCD.setBackColor(0, 0, 0);
			myGLCD.printNumF(((EndMeasure - StartMeasure) - set_timeZero) / 1000.00, 2, 5, 64); // ����� �������� ����������
			myGLCD.print(" ms", 80, 64);
			//myGLCD.setFont(SmallFont);
			//myGLCD.setBackColor(0, 0, 255);
			////}

			//myGLCD.setColor(VGA_RED);
			//myGLCD.fillCircle(227, 12, 10);
		}
		else
		{
	/*		myGLCD.setColor(255, 255, 255);
			myGLCD.drawRoundRect(245, 135, 318, 175);
			myGLCD.setBackColor(0, 0, 255);
			myGLCD.setColor(255, 255, 255);
			myGLCD.print("        ", 255, 140);
			myGLCD.printNumI(0, 255, 140);*/
			myGLCD.setFont(BigFont);
			myGLCD.setBackColor(0, 0, 0);
			myGLCD.printNumI(0, 5, 44);
			myGLCD.print(" ms", 88, 44);
			myGLCD.setFont(SmallFont);
			//myGLCD.setBackColor(0, 0, 255);
			//myGLCD.setColor(0, 0, 0);
			//myGLCD.fillCircle(227, 12, 10);
		}
	//	myGLCD.setBackColor(0, 0, 0);
	//	myGLCD.setColor(255, 255, 255);
	//	myGLCD.print("     ", 250, 212);
	////	myGLCD.printNumI(i_trig_syn, 250, 212);
	//	myGLCD.print("     ", 250, 224);
	//	myGLCD.printNumI(Trigger, 250, 224);
	//	myGLCD.drawCircle(227, 12, 10);

	//	myGLCD.setBackColor(0, 0, 0);
	//	set_timeZero = i2c_eeprom_ulong_read(adr_set_timeZero);
	//	//timePeriod = i2c_eeprom_ulong_read(adr_timePeriod);
	//	//myGLCD.printNumF(timePeriod / 1000000.00, 2, 250, 200, ',');
	//	myGLCD.print("sec", 285, 200);
	//	int b = i2c_eeprom_read_byte(deviceaddress, adr_count3_kn);                // ����������  ������� ��������� 1 ������ ��������� ������������
	//	myGLCD.print("     ", 250, 188);                                           // ����������  ������� ��������� 1 ������ ��������� ������������
	//	myGLCD.print(proc_volume[b], 250, 188);                                    // ����������  ������� ��������� 1 ������ ��������� ������������

	//	osc_line_off0 = true;                                                      // ��������� �������� ���������� �����
		//for (page = 0; page < page_max; page++)                                    // ��������� 
		//{
			for (int xpos = 0; xpos < 240; xpos++)                                     // ����� �� �����
			{
				//  ������� ���������� �����
				myGLCD.setColor(0, 0, 0);

				//if (osc_line_off0)
				//{
					ypos_osc1_0 = 255 - (OldSample_osc[xpos + 1][page] / koeff_h) - hpos;
					ypos_osc2_0 = 255 - (OldSample_osc[xpos + 2][page] / koeff_h) - hpos;
					if (ypos_osc1_0 < 0) ypos_osc1_0 = 0;
					if (ypos_osc2_0 < 0) ypos_osc2_0 = 0;
					if (ypos_osc1_0 > 220) ypos_osc1_0 = 220;
					if (ypos_osc2_0 > 220) ypos_osc2_0 = 220;
					myGLCD.drawLine(xpos + 1, ypos_osc1_0+20, xpos + 2, ypos_osc2_0+20);
					myGLCD.drawLine(xpos + 2, ypos_osc1_0 + 1+20, xpos + 3, ypos_osc2_0 + 1+20);

					if (xpos == 239)
					{
					//	osc_line_off0 = false;
						DrawGrid1();
					}
			//	}
				if (xpos < 1)
				{
					//myGLCD.drawLine(xpos + 1, 1, xpos + 1, 220);          // ������� �������� ����� � ������
					//myGLCD.drawLine(xpos + 2, 1, xpos + 2, 220);          // ������� �������� ����� � ������
					//myGLCD.drawLine(xpos + 3, 1, xpos + 3, 220);          // ������� �������� ����� � ������
					ypos_trig = 255 - (Trigger / koeff_h) - hpos;         // �������� ������� ������
					if (ypos_trig != ypos_trig_old)                        // ������� ������ ������� ������ ��� ���������
					{
						myGLCD.setColor(0, 0, 0);
						myGLCD.drawLine(1, ypos_trig_old+20, 240, ypos_trig_old+20);
						ypos_trig_old = ypos_trig;
					}
					myGLCD.setColor(255, 0, 0);
					myGLCD.drawLine(1, ypos_trig+20, 240, ypos_trig+20);         // ���������� ����� ������ ������
				}
				// ���������� �����  �������������
				myGLCD.setColor(255, 255, 255);
				ypos_osc1_0 = 255 - (Sample_osc[xpos] / koeff_h) - hpos;
				ypos_osc2_0 = 255 - (Sample_osc[xpos + 1] / koeff_h) - hpos;
				if (ypos_osc1_0 < 0) ypos_osc1_0 = 0;
				if (ypos_osc2_0 < 0) ypos_osc2_0 = 0;
				if (ypos_osc1_0 > 220) ypos_osc1_0 = 220;
				if (ypos_osc2_0 > 220) ypos_osc2_0 = 220;
				myGLCD.drawLine(xpos, ypos_osc1_0+20, xpos + 1, ypos_osc2_0+20);
				myGLCD.drawLine(xpos + 1, ypos_osc1_0 + 1+20, xpos + 2, ypos_osc2_0 + 1+20);

				//if (Synhro_osc[xpos][0] > 0.1)
				//{
				//	if (mode < 2)
				//	{
				//		if (xpos < 5)
				//		{
				//			myGLCD.printNumI(0, 10, 165);
				//		}
				//		else if (xpos > 230)
				//		{
				//			myGLCD.printNumF(Synhro_osc[xpos][0], 1, xpos - 12, 165);
				//		}
				//		else
				//		{
				//			myGLCD.printNumF(Synhro_osc[xpos][0], 1, xpos, 165);
				//		}
				//	}
				//	else
				//	{
				//		if (xpos < 5)
				//		{
				//			myGLCD.printNumI(0, 10, 165);
				//		}
				//		else if (xpos > 230)
				//		{
				//			myGLCD.printNumI(Synhro_osc[xpos][0], xpos - 12, 165);
				//		}
				//		else
				//		{
				//			myGLCD.printNumI(Synhro_osc[xpos][0], xpos, 165);
				//		}
				//	}
				//}
				OldSample_osc[xpos][0] = Sample_osc[xpos];
				//Synhro_osc[xpos][0] = 0;
			}
		}

		//attachInterrupt(kn_red, volume_up, FALLING);
		//attachInterrupt(kn_blue, volume_down, FALLING);
		//data_out[2] = 6;                                    //
		//if (kn != 0) radio_send_command();                                             //  ��������� ����� ������ ��������� ������ ���������;
	}
	koeff_h = 7.759 * 4;
	mode1 = 0;             // �/���
	mode = 3;              // ����� ���������  
	tmode = 5;             // ������� �������� ������
	trigger_volume(tmode); // ������� �������� ������
	myGLCD.setFont(BigFont);
	while (myTouch.dataAvailable()) {}
}

void buttons_right()  //  ������ ������  oscilloscope
{
	myGLCD.setColor(0, 0, 255);
	myGLCD.fillRoundRect(245, 1, 318, 40);                 // ��������� ���� ������ ����� ������
	myGLCD.fillRoundRect(245, 45, 318, 85);
	myGLCD.fillRoundRect(245, 90, 318, 130);
	myGLCD.fillRoundRect(245, 135, 318, 175);
	//myGLCD.fillRoundRect(292, 180, 318, 206);              // ��������� �����
	//myGLCD.fillRoundRect(292, 211, 318, 237);              // ��������� ����

	myGLCD.setBackColor(0, 0, 255);
	myGLCD.setFont(SmallFont);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect(245, 1, 318, 40);                 // ���������� ���������� ������ ����� ������
	myGLCD.drawRoundRect(245, 45, 318, 85);
	myGLCD.drawRoundRect(245, 90, 318, 130);
	myGLCD.drawRoundRect(245, 135, 318, 175);
	//myGLCD.drawRoundRect(292, 180, 318, 206);              // ��������� �����
	//myGLCD.drawRoundRect(292, 211, 318, 237);              // ��������� ����

	myGLCD.setBackColor(0, 0, 255);
	myGLCD.setColor(255, 255, 255);

	myGLCD.setFont(BigFont);
	myGLCD.print("-  +", 248, 20);
	myGLCD.print("-  +", 248, 63);
	myGLCD.print("-  +", 248, 108);


	myGLCD.setFont(SmallFont);

	chench_mode(0);
//	trigger_volume(0);
	chench_mode1(0);

	myGLCD.print("Delay", 260, 6);
	myGLCD.print("Trig.", 265, 50);
	myGLCD.print("V/del.", 265, 95);
}
void button_down()
{
	drawUpButton(10, 200);                                        // ���������� ������ ����������� �������� 1 ������� ��������;
	drawDownButton(52, 200);                                      // ���������� ������ ����������� �������� 1 ������� ��������;
	drawUpButton(94, 200);                                        // ���������� ������ ����������� �������� 2 ������� ��������;
	drawDownButton(136, 200);                                     // ���������� ������ ����������� �������� 2 ������� ��������;
	drawUpButton(178, 200);                                       // ���������� ������ ��������� ������ ������;
	drawDownButton(220, 200);                                     // ���������� ������ ��������� ������ ������;

	myGLCD.setFont(SmallFont);
	myGLCD.setColor(255, 255, 255);
	myGLCD.setBackColor(0, 0, 0);
	set_volume(5, 1);
	myGLCD.print("\x8A""H""\x8D""1 -M""\x86""KPO""\x8B""OH- ""\x8A""H""\x8D""2 ", 11, 228);     // "���1 -��������- ���2 "
	myGLCD.print("\x82\x86""HAM""\x86""K", 190, 228);              // "�������"
}
void drawUpButVolume(int x, int y)
{
	myGLCD.setColor(64, 64, 128);
	myGLCD.fillRoundRect(x, y, x + 32, y + 25);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect(x, y, x + 32, y + 25);
	myGLCD.setColor(128, 128, 255);
	for (int i = 0; i<15; i++)
		myGLCD.drawLine(x + 6 + (i / 1.5), y + 20 - i, x + 27 - (i / 1.5), y + 20 - i);
}
void drawDownButVolume(int x, int y)
{
	myGLCD.setColor(64, 64, 128);
	myGLCD.fillRoundRect(x, y, x + 32, y + 25);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect(x, y, x + 32, y + 25);
	myGLCD.setColor(128, 128, 255);
	for (int i = 0; i<15; i++)
		myGLCD.drawLine(x + 6 + (i / 1.5), y + 5 + i, x + 27 - (i / 1.5), y + 5 + i);
}


void chench_mode(bool mod)   // ������������ ���������
{
	if (mod)
	{
		mode++;
	}
	else
	{
		mode--;
	}
	if (mode < 0) mode = 0;
	if (mode > 9) mode = 9;
	// Select delay times you can change values to suite your needs
	if (mode == 0) { dTime = 1;    x_dTime = 278; scale_strob = 1; }
	if (mode == 1) { dTime = 10;   x_dTime = 274; scale_strob = 1; }
	if (mode == 2) { dTime = 20;   x_dTime = 274; scale_strob = 1; }
	if (mode == 3) { dTime = 50;   x_dTime = 274; scale_strob = 2; }
	if (mode == 4) { dTime = 100;  x_dTime = 272; scale_strob = 5; }
	if (mode == 5) { dTime = 200;  x_dTime = 272; scale_strob = 10; }
	if (mode == 6) { dTime = 300;  x_dTime = 272; scale_strob = 20; }
	if (mode == 7) { dTime = 500;  x_dTime = 272; scale_strob = 30; }
	if (mode == 8) { dTime = 1000; x_dTime = 267; scale_strob = 50; }
	if (mode == 9) { dTime = 5000; x_dTime = 267; scale_strob = 100; }
	myGLCD.print("    ", 262, 22);
	myGLCD.printNumI(dTime, x_dTime, 22);
}
void trigger_volume(int volume_trigger)                    // ������� ������
{
	if (volume_trigger < 0)volume_trigger = 0;
	if (volume_trigger > 7)volume_trigger = 7;

	if (volume_trigger == 1) { Trigger = 512; myGLCD.print(" 10% ", 264, 65); }
	if (volume_trigger == 2) { Trigger = 768;  myGLCD.print(" 18% ", 264, 65); }
	if (volume_trigger == 3) { Trigger = 1024;  myGLCD.print(" 25% ", 264, 65); }
	if (volume_trigger == 4) { Trigger = 1536;  myGLCD.print(" 38% ", 264, 65); }
	if (volume_trigger == 5) { Trigger = 2047;  myGLCD.print(" 50% ", 264, 65); }
	if (volume_trigger == 6) { Trigger = 3071;  myGLCD.print(" 75% ", 264, 65); }
	if (volume_trigger == 7) { Trigger = 4080; myGLCD.print("100%", 265, 65); }
	if (volume_trigger == 0) { Trigger = 0; myGLCD.print(" Off ", 265, 65); }
	Serial.println(Trigger);
}
//void trigger_volume(int mode)                    // ������� ������
//{
//	if (mode)
//	{
//		tmode++;
//	}
//	else
//	{
//		tmode--;
//	}
//	
//	if (tmode < 0)tmode = 0;
//	if (tmode > 7)tmode = 7;
//
//	if (tmode == 1) { Trigger = 512; myGLCD.print(" 10% ", 264, 65); }
//	if (tmode == 2) { Trigger = 768;  myGLCD.print(" 18% ", 264, 65); }
//	if (tmode == 3) { Trigger = 1024;  myGLCD.print(" 25% ", 264, 65); }
//	if (tmode == 4) { Trigger = 1536;  myGLCD.print(" 38% ", 264, 65); }
//	if (tmode == 5) { Trigger = 2047;  myGLCD.print(" 50% ", 264, 65); }
//	if (tmode == 6) { Trigger = 3071;  myGLCD.print(" 75% ", 264, 65); }
//	if (tmode == 7) { Trigger = 4080; myGLCD.print("100%", 265, 65); }
//	if (tmode == 0) { Trigger = 0; myGLCD.print(" Off ", 265, 65); }
//	Serial.println(Trigger);
//}
void chench_mode1(bool mod)  // ���������� ������
{
	if (mod)
	{
		mode1++;
	}
	else
	{
		mode1--;
	}
	if (mode1 < 0) mode1 = 0;
	if (mode1 > 3) mode1 = 3;
	myGLCD.setColor(0, 0, 0);
	myGLCD.fillRoundRect(0, 0, 240, 160);
	DrawGrid();
	myGLCD.setColor(255, 255, 255);
	if (mode1 == 0) { koeff_h = 7.759 * 4; myGLCD.print("  1 ", 262, 110); }
	if (mode1 == 1) { koeff_h = 3.879 * 4; myGLCD.print(" 0.5", 262, 110); }
	if (mode1 == 2) { koeff_h = 1.939 * 4; myGLCD.print("0.25", 264, 110); }
	if (mode1 == 3) { koeff_h = 0.969 * 4; myGLCD.print(" 0.1", 262, 110); }
}


void synhro_DS3231_clock()
{
	detachInterrupt(alarm_pin);
	DS3231_clock.clearAlarm1();
	DS3231_clock.clearAlarm2();
	pinMode(alarm_pin, INPUT);
	digitalWrite(alarm_pin, HIGH);
	myGLCD.clrScr();
	myGLCD.setBackColor(0, 0, 0);
	myGLCD.setColor(255, 255, 255);
	myGLCD.setFont(BigFont);
	count_ms = 0;
	synhro_enable = false;
	alarm_count = 0;
	wiev_Start_Menu = false;
	start_synhro = false;

	if (digitalRead(synhro_pin) != LOW)
	{
		myGLCD.setColor(VGA_RED);
		myGLCD.print("C""\x9D\xA2""xpo ""\xA2""e ""\xA3""o""\x99\x9F\xA0\xAE\xA7""e""\xA2", CENTER, 80);   // ������ �� ���������
		delay(2000);
	}
	else
	{
		//adr_set_time = 100;                                          // ����� ������� ������������� �������
		//unsigned long Start_unixtime = dt.unixtime;


		dt = DS3231_clock.getDateTime();
		data_out[2] = 8;                                      // ��������� ������� ������������� �����
		data_out[12] = dt.year - 2000;                      // 
		data_out[13] = dt.month;                            // 
		data_out[14] = dt.day;                              // 
		data_out[15] = dt.hour;                             //
		data_out[16] = dt.minute;                           // 

		radio_send_command();

		unsigned long StartMeasure = micros();               // �������� �����
	
		while (!synhro_enable) 
		{
			if (micros() - StartMeasure >= 20000000)
			{
				myGLCD.setFont(BigFont);
				myGLCD.setColor(VGA_RED);
				myGLCD.print("He""\xA4"" c""\x9D\xA2""xpo""\xA2\x9D\x9C""a""\xA6\x9D\x9D", CENTER, 80);   // "��� �������������"
				delay(2000);
				break;
			}
			if (digitalRead(synhro_pin) == HIGH)
			{
				do {} while (digitalRead(synhro_pin));                                       // ����� ��������� ��������������
				DS3231_clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, 00); // ���������� ����� ������������������ �����
				alarm_synhro = 0;                                                            // ���������� ������� ���������������
				DS3231_clock.setAlarm1(0, 0, 0, 1, DS3231_EVERY_SECOND);                     // DS3231_EVERY_SECOND //���������� ������ �������
				while (digitalRead(alarm_pin) != HIGH){}                                     // �������������� �������� ����������
				while (digitalRead(alarm_pin) == HIGH){}                                     // �������������� �������� ����������

				while (true)                                                                 // ������������� ����� ������ ������ �������������
				{
					dt = DS3231_clock.getDateTime();
					if (oldsec != dt.second)
					{
						myGLCD.print(DS3231_clock.dateFormat("H:i:s", dt), 5, 0);
						oldsec = dt.second;
					}
					if (dt.second == 10)
					{
						break;
					}
				}
				if(!synhro_enable) attachInterrupt(alarm_pin, alarmFunction, FALLING);      // ���������� ���������� ������ ��� ����� �������� �� ����� � LOW �� HIGH
				unsigned long Start_unixtime = dt.unixtime;
				i2c_eeprom_ulong_write(adr_start_unixtimetime, Start_unixtime);             // �������� ����� ������ ������������� �������
				i2c_eeprom_write_byte(deviceaddress, adr_start_year, dt.year - 2000);       // �������� ����� ������ ������������� �������
				i2c_eeprom_write_byte(deviceaddress, adr_start_month, dt.month);            // �������� ����� ������ ������������� �������
				i2c_eeprom_write_byte(deviceaddress, adr_start_day, dt.day);                // �������� ����� ������ ������������� �������
				i2c_eeprom_write_byte(deviceaddress, adr_start_hour, dt.hour);              // �������� ����� ������ ������������� �������
				i2c_eeprom_write_byte(deviceaddress, adr_start_minute, dt.minute);          // �������� ����� ������ ������������� �������
				i2c_eeprom_write_byte(deviceaddress, adr_start_second, dt.second);          // �������� ����� ������ ������������� �������
	
				myGLCD.setFont(BigFont);
				myGLCD.setColor(VGA_LIME);
				myGLCD.print("C""\x9D\xA2""xpo OK!", CENTER, 80);                           // ������ ��!
				myGLCD.setColor(255, 255, 255);
				delay(1000);
				synhro_enable = true;
			}
		}  // ������������� ���������, ��������� ���������
	}
}
void wiev_synhro()
{
	myGLCD.clrScr();
	myGLCD.setBackColor(0, 0, 0);
	count_ms = 0;
	myGLCD.setFont(BigFont);
	start_synhro                   = false;
	int count_synhro               = 0;
	unsigned long TrigSynhro       = 0;
	long time_temp                 = 0;                                         // ��� ����������� ����������� ����������� ���������
	dTime                          = 23;                                        //  
	scale_strob                    = 2;                                         // 
	unsigned long Start_unixtime   = 0;                                         //
	unsigned long Current_unixtime = 0;                                         // 
	int start_year                 = 0;                                         // ����� ������ ������������� �������
	int start_month                = 0;                                         // ����� ������ ������������� �������
	int start_day                  = 0;                                         // ����� ������ ������������� �������
	int start_hour                 = 0;                                         // ����� ������ ������������� �������
	int start_minute               = 0;                                         // ����� ������ ������������� �������
	int start_second               = 0;                                         // ����� ������ ������������� �������


	for (xpos = 0; xpos < 240; xpos++)                                            // ������� ������ ������ ����������� � ��������� �����
	{
		Sample_osc[xpos] = 0;
		Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
		Synhro_osc[xpos][line_info] = 0;                                          // ������� ������ ������ ��������� �����
	}

	Start_unixtime = i2c_eeprom_ulong_read(adr_start_unixtimetime);                // �������� ����� ������ ������������� �������
	start_year     = i2c_eeprom_read_byte(deviceaddress, adr_start_year);          // ����� ������ ������������� �������
	start_month    = i2c_eeprom_read_byte(deviceaddress, adr_start_month);         // ����� ������ ������������� �������
	start_day      = i2c_eeprom_read_byte(deviceaddress, adr_start_day);           // ����� ������ ������������� �������
	start_hour     = i2c_eeprom_read_byte(deviceaddress, adr_start_hour);          // ����� ������ ������������� �������
	start_minute   = i2c_eeprom_read_byte(deviceaddress, adr_start_minute);        // ����� ������ ������������� �������
	start_second   = i2c_eeprom_read_byte(deviceaddress, adr_start_second);        // ����� ������ ������������� �������

	myGLCD.setFont(SmallFont);

	myGLCD.print("Start", 250, 10);                                                //  
	myGLCD.print("H:", 250, 25);                                                   //  
	myGLCD.printNumI(start_hour, 280, 25, 2);                                      // ������� �� ����� ����� ���
	myGLCD.print("M:", 250, 40);                                                   //  
	myGLCD.printNumI(start_minute, 280, 40, 2);                                    // ������� �� ����� ����� ���
	myGLCD.print("S:", 250, 55);                                                   //  
	myGLCD.printNumI(start_second, 280, 55, 2);                                    // ������� �� ����� ����� ���

	DrawGrid1();
	while (1)
	{
		bool synhro_Off = false;                                                   // ������� ������������� � �������� ���������   
		StartSample = micros();
		while (!start_synhro)                                                      // �������� ������� ������ ��������������
		{
			if (micros() - StartSample > 6000000)                                  // ������� ������������� � ������� 6 ������
			{
				//synhro_Off = true;
				myGLCD.setFont(BigFont);
				myGLCD.setColor(VGA_YELLOW);
				myGLCD.print("He""\xA4", 100, 30);                                         // ���
				myGLCD.print("c""\x9D\xA2""xpo""\xA2\x9D\x9C""a""\xA6\x9D\x9D", 20, 50);   // �������������

				delay(2000);
				break;                                                             // ��������� �������� ��������������
			}

			if (myTouch.dataAvailable())   break;                                  //���������� ������ �� �����
		}
    	if (start_synhro)
		{
			if (digitalRead(synhro_pin))
			{
				myGLCD.setFont(BigFont);
				myGLCD.setColor(VGA_YELLOW);
				myGLCD.print("He""\xA4", 100, 30);                                         // ���
				myGLCD.print("c""\x9D\xA2""xpo""\xA2\x9D\x9C""a""\xA6\x9D\x9D", 20, 50);   // �������������
				delay(2000);
				break;                                                            // ��������� �������� ��������������
			}

			// ��� ���������. �������� !!
			StartSample = micros();
			start_synhro = false;                                                    // ����� ������ �������������� ������. ���������� ���� ������ � �������� ��� �������� ���������� ������ ��������
			Timer7.start(2 * 1000);                                                  // �������� ������������  ��������� ����� �� ������
			trig_sin = false;                                                         // ���������� ���� ������������ �������� ������
			while (1)                                                                // �������� ������� ������ ��������������.  ������ ����������� �������� Timer5
			{
				if (micros() - StartSample > 3000000)                                // ������� ������������� � ������� 3 ������
				{
					EndMeasure = micros();                                           // �������� ����� ������������ �������� ������
					trig_sin = true;                                                 // ���������� ���� ������������ �������� ������
					break;                                                           // ��������� �������� ��������������
				}
				for (xpos = 0; xpos < 240; xpos++)                                   // ����� �� 240 ������
				{
					Sample_osc[xpos] = 0;
					Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
					Synhro_osc[xpos][line_info] = 0;
					//Sample_osc[xpos] = 0;
					Sample_osc[xpos] = digitalRead(synhro_pin);                      // �������� ������ synhro_pin � �������� � ������
					if ((Sample_osc[xpos] > Trigger) && (trig_sin == false))         // ����� ���������� ������ ������
					{
						EndMeasure = micros();                                       // �������� ����� ������������ �������� ������
						trig_sin = true;                                             // ���������� ���� ������������ �������� ������
					}
					delayMicroseconds(dTime);                                        // dTime ��������� �������� (��������) ��������� 
				}
				if (trig_sin == true) break;                                         // ��������� ������������ �� ����� � ������� �������� �����
			}

			Timer7.stop();
			count_ms = 0;
			count_synhro++;
			myGLCD.setColor(0, 0, 0);
			myGLCD.fillRoundRect(0, 20, 240, 176);                                 // �������� ������� ������ ��� ������ ��������� �����. 
			DrawGrid1();
			myGLCD.setColor(255, 255, 255);
			myGLCD.setFont(SmallFont);
			myGLCD.printNumI(count_synhro, 250, 190);                              // ������� �� ����� ������� ���������������

			myGLCD.print("Current", 250, 80);                                      //  
			dt = DS3231_clock.getDateTime();
			myGLCD.print("H:", 250, 95);                                           //  
			myGLCD.printNumI(dt.hour, 280, 95, 2);                                 // ������� �� ����� ����� ���
			myGLCD.print("M:", 250, 110);                                          //  
			myGLCD.printNumI(dt.minute, 280, 110, 2);                              // ������� �� ����� ����� ���
			myGLCD.print("S:", 250, 125);                                          //  
			myGLCD.printNumI(dt.second, 280, 125, 2);                              // ������� �� ����� ����� ���
			myGLCD.print("Duration", 250, 145);                                    //  
			myGLCD.print("of time", 253, 160);

			Current_unixtime = dt.unixtime;
			myGLCD.print("Bpe""\xA1\xAF"" o""\xA4"" c""\xA4""ap""\xA4""a c""\x9D\xA2""xpo""\xA2\x9D\x9C""a""\xA6\x9D\x9D", 0, 175);  //"����� �� ������ �������������"
			myGLCD.printNumI((Current_unixtime - Start_unixtime) / 60, 260, 175);      // ������� �� ����� ����� ���
			myGLCD.print("min", 295, 175);
			myGLCD.setFont(BigFont);

			for (int xpos = 0; xpos < 239; xpos++)                                  // ����� �� �����
			{
				// ���������� �����  �������������
				if (xpos == 119)
				{
					myGLCD.setColor(VGA_YELLOW);
					myGLCD.drawLine(xpos, 80, xpos, 160);
					myGLCD.drawLine(xpos + 1, 80, xpos + 1, 160);
					myGLCD.setColor(255, 255, 255);
				}

				if (Sample_osc[xpos] == HIGH)
				{
					myGLCD.setColor(255, 255, 255);
					myGLCD.drawLine(xpos, 100, xpos, 160);
					myGLCD.drawLine(xpos + 1, 100, xpos + 1, 160);
				}

	
				myGLCD.setFont(SmallFont);
				myGLCD.setColor(VGA_WHITE);


				if (Synhro_osc[xpos][line_info] == 4095)
				{
					myGLCD.drawLine(xpos, 120, xpos, 160);

				}
				if (Synhro_osc[xpos][ms_info] > 0)
				{
					if (xpos > 230)
					{
						myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos - 12, 165);
					}
					else
					{
						myGLCD.printNumI(Synhro_osc[xpos][ms_info], xpos, 165);

					}
				}

				//Sample_osc[xpos] = 0;
				//Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
				//Synhro_osc[xpos][line_info] = 0;

			}

			//for (xpos = 0; xpos < 240; xpos++)                                            // ������� ������ ������ ����������� � ��������� �����
			//{
			//	Sample_osc[xpos] = 0;
			//	Synhro_osc[xpos][ms_info] = 0;                                            // ������� ������ ������ ��������� �����
			//	Synhro_osc[xpos][line_info] = 0;                                          // ������� ������ ������ ��������� �����
			//}

		}
		if (myTouch.dataAvailable())   break;                                    //���������� ����� �� �����
	}
	while (myTouch.dataAvailable()) {}
	delay(200);
//	while(myTouch.dataAvailable()) {}
}

void radio_send_command()                                   // ������������ �������
{
	//detachInterrupt(kn_red);
	//detachInterrupt(kn_blue);
	tim1 = 0;                                                 // ����� ������ ��������������
//	volume_variant = 3;                                       // ���������� ������� ��������� 1 ������ ��������� ������������
	radio.stopListening();                                    // ��-������, ����������� �������, ����� �� ����� ����������.
	myGLCD.setBackColor(0, 0, 0);
	myGLCD.setFont(SmallFont);
//	if (kn != 0) set_volume(volume_variant, kn);
	data_out[0] = counter_test;
	data_out[1] = 1;                                          //  
	//data_out[2] = 2;                                        // ������� ������ ����� ��� ������ ���������
	data_out[3] = 1;                                          //
	data_out[4] = highByte(time_sound);                       // ������� ���� ��������� ������������ �������� �������
	data_out[5] = lowByte(time_sound);                        // ������� ���� ��������� ������������ �������� �������
	data_out[6] = highByte(freq_sound);                       // ������� ���� ��������� ������� �������� �������
	data_out[7] = lowByte(freq_sound);                        // ������� ���� ��������� ������� �������� ������� 
	data_out[8] = volume1;                                    // 
	data_out[9] = volume2;                                    // 

	timeStartRadio = micros();                                // �������� ����� ������ ����� ������ ��������

	if (!radio.write(&data_out, sizeof(data_out)))
	{
		//myGLCD.setColor(VGA_LIME);                            // ������� �� ������� ����� ������ ��������������
		//myGLCD.print("     ", 90 - 40, 178);                  // ������� �� ������� ����� ������ ��������������
		//myGLCD.printNumI(0, 90 - 32, 178);                    // ������� �� ������� ����� ������ ��������������
		//myGLCD.setColor(255, 255, 255);
	}
	else
	{
		if (!radio.available())
		{
			//	Serial.println(F("Blank Payload Received."));
		}
		else
		{
			while (radio.isAckPayloadAvailable())
			{
				timeStopRadio = micros();                     // �������� ����� ��������� ������.  
				radio.read(&data_in, sizeof(data_in));
				tim1 = timeStopRadio - timeStartRadio;        // �������� ����� ������ ������ ��������
				//myGLCD.print("Delay: ", 5, 178);              // ������� �� ������� ����� ������ ��������������
				//myGLCD.print("     ", 90 - 40, 178);          // ������� �� ������� ����� ������ ��������������
				//myGLCD.setColor(VGA_LIME);                    // ������� �� ������� ����� ������ ��������������
				if (tim1 < 999)                               // ���������� ����� ����� �� ������� 
				{
					//myGLCD.printNumI(tim1, 90 - 32, 178);     // ������� �� ������� ����� ������ ��������������
				}
				else
				{
					//myGLCD.printNumI(tim1, 90 - 40, 178);     // ������� �� ������� ����� ������ ��������������
				}
				//myGLCD.setColor(255, 255, 255);
				//myGLCD.print("microsec", 90, 178);            // ������� �� ������� ����� ������ ��������������
				counter_test++;
			}
		}
	}
}
void radio_test_ping()
{
	detachInterrupt(kn_red);
	detachInterrupt(kn_blue);
	myGLCD.clrScr();
	Serial.println(F("Test ping."));
	myGLCD.setFont(BigFont);
	myGLCD.setBackColor(0, 0, 0);
	myGLCD.setColor(VGA_YELLOW);
	myGLCD.print("TEST PING", CENTER, 10);
	myGLCD.print(txt_info11, CENTER, 221);            // ������ "ESC -> PUSH"
	myGLCD.setBackColor(0, 0, 0);
	setup_radio();
	role_test = role_ping_out;
	tim1 = 0;
	volume_variant = 3;                                       //  ���������� ������� ��������� 1 ������ ��������� ������������
	int x_touch, y_touch;

	while (1)
	{
		if (myTouch.dataAvailable())
		{
			delay(10);
			myTouch.read();
			x_touch = myTouch.getX();
			y_touch = myTouch.getY();

			if ((x_touch >= 2) && (x_touch <= 319))               //  ������� ������
			{
				if ((y_touch >= 1) && (y_touch <= 239))           // Delay row
				{
					sound1();
					break;
				}
			}
		}
		if (role_test == role_ping_out)
		{
			radio.stopListening();                                  // ��-������, ����������� �������, ����� �� ����� ����������.
			myGLCD.print("Sending", 1, 40);
			myGLCD.print("    ", 125, 40);
			myGLCD.setColor(VGA_LIME);
			if (counter_test<10)
			{
				myGLCD.printNumI(counter_test, 155, 40);
			}
			else if (counter_test>9 && counter_test<100)
			{
				myGLCD.printNumI(counter_test, 155 - 16, 40);
			}
			else if (counter_test > 99)
			{
				myGLCD.printNumI(counter_test, 155 - 32, 40);
			}
			myGLCD.setColor(255, 255, 255);
			myGLCD.print("payload", 178, 40);
	//		if (kn != 0) set_volume(volume_variant, kn);

			data_out[0] = counter_test;
			data_out[1] = 1;                                       //1= ��������� ������� ping �������� ������� 
			data_out[2] = 1;                    //
			data_out[3] = 1;                    //
			data_out[4] = highByte(time_sound);                     //1= ��������� �������� ������� 
			data_out[5] = lowByte(time_sound);                    //1= ��������� �������� ������� 
			data_out[6] = highByte(freq_sound);                    //1= ��������� �������� ������� 
			data_out[7] = lowByte(freq_sound);                   //1= ��������� �������� ������� 
			data_out[8] = volume1;
			data_out[9] = volume2;


			//int value = 3000;
			//// ���������
			//byte hi = highByte(value);
			//byte low = lowByte(value);

			//// ��� �� ��� hi,low ����� ��������, ��������� �� eePROM

			//int value2 = (hi << 8) | low; // �������� ��� "��������� �����������"
			//int value3 = word(hi, low); // ��� �������� ��� "����������"

			//int time_sound = 200;
			//int freq_sound = 1850;



			unsigned long time = micros();                          // Take the time, and send it.  This will block until complete   

			if (!radio.write(&data_out, sizeof(data_out)))
				//if (!radio.write(&data_out, 2)) 
			{
				Serial.println(F("failed."));
			}
			else
			{
				if (!radio.available())
				{
					Serial.println(F("Blank Payload Received."));
				}
				else
				{
					while (radio.isAckPayloadAvailable())
					{
						unsigned long tim = micros();
						radio.read(&data_in, sizeof(data_in));

						//	radio.read(&data_in, sizeof(data_in));
						tim1 = tim - time;
						myGLCD.print("Respons", 1, 60);
						myGLCD.print("    ", 125, 60);
						myGLCD.setColor(VGA_LIME);
						if (data_in[0]<10)
						{
							myGLCD.printNumI(data_in[0], 155, 60);
						}
						else if (data_in[0]>9 && data_in[0]<100)
						{
							myGLCD.printNumI(data_in[0], 155 - 16, 60);
						}
						else if (data_in[0] > 99)
						{
							myGLCD.printNumI(data_in[0], 155 - 32, 60);
						}
						myGLCD.setColor(255, 255, 255);
						myGLCD.print("round", 178, 60);
						myGLCD.print("Delay: ", 1, 80);
						myGLCD.print("     ", 100, 80);
						myGLCD.setColor(VGA_LIME);
						if (tim1<999)
						{
							myGLCD.printNumI(tim1, 155 - 32, 80);
						}
						else
						{
							myGLCD.printNumI(tim1, 155 - 48, 80);
						}
						myGLCD.setColor(255, 255, 255);
						myGLCD.print("microsec", 178, 80);
						counter_test++;
					}
				}
			}
			//attachInterrupt(kn_red, volume_up, FALLING);
			//attachInterrupt(kn_blue, volume_down, FALLING);
			delay(1000);
		}
	}
	while (!myTouch.dataAvailable()) {}
	delay(50);
	while (myTouch.dataAvailable()) {}
}


void measure_power()
{                                                     // ��������� ��������� ���������� ������� � ��������� 1/2 
												      // ���������� ����������� �������� +15� ��� 10� �� ������ �������
	uint32_t logTime1 = 0;
	logTime1 = millis();
	if (logTime1 - Start_Power > 500)                 //  ��������� 
	{
		Start_Power = millis();
		int m_power = 0;
		float ind_power = 0;
		ADC_CHER = Analog_pinA3;                      // ���������� ����� �3, ����������� 12
		ADC_CR = ADC_START; 	                      // ��������� ��������������
		while (!(ADC_ISR_DRDY));                      // �������� ����� ��������������
		m_power = ADC->ADC_CDR[4];                    // ������� ������ � ������ A3
		ind_power = m_power *(3.2 / 4096 * 2);        // �������� ���������� � �������
		myGLCD.setBackColor(0, 0, 0);
		myGLCD.setFont(SmallSymbolFont);
		if (ind_power > 4.1)
		{
			myGLCD.setColor(VGA_LIME);
			myGLCD.drawRoundRect(279, 149, 319, 189);
			myGLCD.setColor(VGA_WHITE);
			myGLCD.print("\x20", 295, 155);
		}
		else if (ind_power > 4.0 && ind_power < 4.1)
		{
			myGLCD.setColor(VGA_LIME);
			myGLCD.drawRoundRect(279, 149, 319, 189);
			myGLCD.setColor(VGA_WHITE);
			myGLCD.print("\x21", 295, 155);
		}
		else if (ind_power > 3.9 && ind_power < 4.0)
		{
			myGLCD.setColor(VGA_WHITE);
			myGLCD.drawRoundRect(279, 149, 319, 189);
			myGLCD.setColor(VGA_WHITE);
			myGLCD.print("\x22", 295, 155);
		}
		else if (ind_power > 3.8 && ind_power < 3.9)
		{
			myGLCD.setColor(VGA_YELLOW);
			myGLCD.drawRoundRect(279, 149, 319, 189);
			myGLCD.setColor(VGA_WHITE);
			myGLCD.print("\x23", 295, 155);
		}
		else if (ind_power < 3.8)
		{
			myGLCD.setColor(VGA_RED);
			myGLCD.drawRoundRect(279, 149, 319, 189);
			myGLCD.setColor(VGA_WHITE);
			myGLCD.print("\x24", 295, 155);
		}
		myGLCD.setFont(SmallFont);
		myGLCD.setColor(VGA_WHITE);
		myGLCD.printNumF(ind_power, 1, 289, 172);
	}
}

void resistor(int resist, int valresist)
{
	resistance = valresist;
	switch (resist)
	{
	case 1:
		Wire.beginTransmission(address_AD5252);     // transmit to device
		Wire.write(byte(control_word1));            // sends instruction byte  
		Wire.write(resistance);                     // sends potentiometer value byte  
		Wire.endTransmission();                     // stop transmitting
		break;
	case 2:
		Wire.beginTransmission(address_AD5252);     // transmit to device
		Wire.write(byte(control_word2));            // sends instruction byte  
		Wire.write(resistance);                     // sends potentiometer value byte  
		Wire.endTransmission();                     // stop transmitting
		break;
	}
}
void setup_resistor()
{
	Wire.beginTransmission(address_AD5252);        // transmit to device
	Wire.write(byte(control_word1));               // sends instruction byte  
	Wire.write(0);                                 // sends potentiometer value byte  
	Wire.endTransmission();                        // stop transmitting
	Wire.beginTransmission(address_AD5252);        // transmit to device
	Wire.write(byte(control_word2));               // sends instruction byte  
	Wire.write(0);                                 // sends potentiometer value byte  
	Wire.endTransmission();                        // stop transmitting
}
void chench_Channel()
{
	//���������� ������ ����������� �������, ���������� ������� � ���� ��������� ���
	Channel_x = 0;                                  // ���������� ���� �0
//	count_pin = 1;                                  // ���������� ������
	Channel_x |= 0x80;                              // ������������ ��� ��������� ������
	ADC_CHER = Channel_x;                           // �������� ��� ��������� ������ � ���
	//SAMPLES_PER_BLOCK = DATA_DIM16 / count_pin;     // ������ ����������� ������
}



void clean_mem()
{
	byte b = i2c_eeprom_read_byte(deviceaddress, adr_mem_start);                                  // �������� ������� ������� ��������� ������� ����� ������
	if (b != mem_start)
	{
		myGLCD.setBackColor(0, 0, 0);
		myGLCD.print("O""\xA7\x9D""c""\xA4\x9F""a ""\xA3""a""\xA1\xAF\xA4\x9D", CENTER, 60);      // "������� ������"
		for (int i = 0; i < 1024; i++)
		{
			i2c_eeprom_write_byte(deviceaddress, i, 0x00);
			delay(10);
		}
		i2c_eeprom_write_byte(deviceaddress, adr_mem_start, mem_start);
//		i2c_eeprom_ulong_write(adr_set_timeSynhro, set_timeSynhro);                              // ��������  ����� 
		myGLCD.print("                 ", CENTER, 60);                                           // "������� ������"
	}
}
void sound1()
{
	digitalWrite(sounder, HIGH);
	delay(100);
	digitalWrite(sounder, LOW);
	delay(100);
}
void vibro1()
{
	digitalWrite(vibro, HIGH);
	delay(100);
	digitalWrite(vibro, LOW);
	delay(100);
}

void setup_pin()
{
	pinMode(kn_red, INPUT);
	pinMode(kn_blue, INPUT);
	pinMode(intensityLCD, OUTPUT);
	pinMode(synhro_pin, INPUT);
	pinMode(LED_PIN13, OUTPUT);
	digitalWrite(LED_PIN13, LOW);
	digitalWrite(intensityLCD, LOW);
	digitalWrite(synhro_pin, HIGH);
	pinMode(alarm_pin, INPUT);
	digitalWrite(alarm_pin, HIGH);
	pinMode(vibro, OUTPUT);
	pinMode(sounder, OUTPUT);
	digitalWrite(vibro, LOW);
	digitalWrite(sounder, LOW);
	pinMode(kn_red, OUTPUT);
	pinMode(kn_blue, OUTPUT);
	digitalWrite(kn_red, HIGH);
	digitalWrite(kn_blue, HIGH);
}

void setup_radio()
{
	radio.begin();
	radio.setAutoAck(1);                    // Ensure autoACK is enabled
											//radio.setPALevel(RF24_PA_MAX);
	radio.setPALevel(RF24_PA_HIGH);
	//radio.setPALevel(RF24_PA_LOW);           // Set PA LOW for this demonstration. We want the radio to be as lossy as possible for this example.
	radio.setDataRate(RF24_1MBPS);
	//	radio.setDataRate(RF24_250KBPS);
	radio.enableAckPayload();               // Allow optional ack payloads
	radio.setRetries(0, 1);                 // Smallest time between retries, max no. of retries
	radio.setPayloadSize(sizeof(data_out));                // Here we are sending 1-byte payloads to test the call-response speed
														   //radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed
	radio.openWritingPipe(pipes[1]);        // Both radios listen on the same pipes by default, and switch when writing
	radio.openReadingPipe(1, pipes[0]);
	radio.startListening();                 // Start listening
	radio.printDetails();                   // Dump the configuration of the rf unit for debugging

	role = role_ping_out;                  // Become the primary transmitter (ping out)
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1, pipes[1]);
}


void setup()
{
	Serial.begin(115200);
	printf_begin();
	Serial.println(F("Setup start!"));
	Serial.println(F("Version - SoundMeasureDUE_08"));
	setup_pin();
	Wire.begin();
	DS3231_clock.begin();
	myGLCD.InitLCD();
	myGLCD.clrScr();
	myGLCD.setBackColor(0, 0, 0);                   // ����� ��� ������
	myGLCD.setColor(255, 255, 255);
	myGLCD.setFont(BigFont);

	myTouch.InitTouch();
	myTouch.setPrecision(PREC_MEDIUM);
	//myTouch.setPrecision(PREC_HI);
	myButtons.setTextFont(BigFont);
	myButtons.setSymbolFont(Dingbats1_XL);
	setup_radio();                                    // ��������� ����� ������
	Timer7.attachInterrupt(sevenHandler);             // Timer7 - ������ ��������� �����  � ������ ��� ������ �� �����

	DS3231_clock.begin();
	// Disarm alarms and clear alarms for this example, because alarms is battery backed.
	// Under normal conditions, the settings should be reset after power and restart microcontroller.
	DS3231_clock.armAlarm1(false);
	DS3231_clock.armAlarm2(false);
	DS3231_clock.clearAlarm1();
	DS3231_clock.clearAlarm2();

	//DS3231_clock.setDateTime(__DATE__, __TIME__);
	// Enable output

	DS3231_clock.setOutput(DS3231_1HZ);
	DS3231_clock.enableOutput(true);
	// Check config

	if (DS3231_clock.isOutput())
	{
		Serial.println("Oscilator is enabled");
	}
	else
	{
		Serial.println("Oscilator is disabled");
	}

	switch (DS3231_clock.getOutput())
	{
	case DS3231_1HZ:     Serial.println("SQW = 1Hz"); break;
	case DS3231_4096HZ:  Serial.println("SQW = 4096Hz"); break;
	case DS3231_8192HZ:  Serial.println("SQW = 8192Hz"); break;
	case DS3231_32768HZ: Serial.println("SQW = 32768Hz"); break;
	default: Serial.println("SQW = Unknown"); break;
	}

// �������� ������������ ��������� setup 
	sound1();
	delay(100);
	sound1();
	delay(100);
	vibro1();
	delay(100);
	vibro1();


	clean_mem();                                                     // ������� ������ ��� ������ ��������� �������  
	alarm_synhro = 0;

	myGLCD.setFont(SmallFont);
	ADC_MR |= 0x00000100; // ADC full speed
	chench_Channel();

	while (digitalRead(alarm_pin) != HIGH){}
	while (digitalRead(alarm_pin) == HIGH){}
	DS3231_clock.setAlarm1(0, 0, 0, 1, DS3231_EVERY_SECOND);         //DS3231_EVERY_SECOND //������ �������
	while (true)
	{
		dt = DS3231_clock.getDateTime();
		if (oldsec != dt.second)
		{
			myGLCD.print(DS3231_clock.dateFormat("H:i:s", dt), 10, 2);
			oldsec = dt.second;
		}
		if (dt.second == 0 || dt.second == 10 || dt.second == 20 || dt.second == 30 || dt.second == 40 || dt.second == 50)
		{
			break;
		}
	} 


	//delayMicroseconds(100000);
	attachInterrupt(alarm_pin, alarmFunction, FALLING);      // ���������� ���������� ������ ��� ����� �������� �� ����� � LOW �� HIGH
  //  attachInterrupt(alarm_pin, alarmFunction, RISING);     // ���������� ���������� ������ ��� ����� �������� �� ����� � HIGH �� LOW
	Serial.println(F("Setup Ok!"));
}

void loop()
{
	if (wiev_Start_Menu)
	{
		Start_Menu();
	}
}
