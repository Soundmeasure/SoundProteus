
/*

����� �������� ��� ������� ������������ ��������� ������
�������� "������"
������ ����� 29.05.2017�.

���������:
��������� ������������ TFT �������.

�������� ������� - �������� ���������� ������.
����� � ����� �� �����������.

��������������� SAMD21G18A 


*/


#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <UTouch.h>

// For the Adafruit shield, these are the default.
#define TFT_DC   9
#define TFT_CS   10
#define TFT_MOSI MOSI
#define TFT_CLK  SCK
#define TFT_RST 8
#define TFT_MISO MISO


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
UTouch      myTouch(6, 5, 4, 3, 2);


#define Serial SERIAL_PORT_USBVIRTUAL     // ���������� Serial � USB �����


int x, y, n;
char stCurrent[20] = "";
int stCurrentLen = 0;
char stLast[20] = "";
//void setRotation(uint8_t rotation);


void test_display()
{

	// read diagnostics (optional but can help debug problems)
	uint8_t x = tft.readcommand8(ILI9341_RDMODE);
	Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDMADCTL);
	Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDPIXFMT);
	Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDIMGFMT);
	Serial.print("Image Format: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDSELFDIAG);
	Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);

	Serial.println(F("Benchmark                Time (microseconds)"));
	delay(10);
	Serial.print(F("Screen fill              "));
	Serial.println(testFillScreen());
	delay(500);

	Serial.print(F("Text                     "));
	Serial.println(testText());
	delay(1000);

	Serial.print(F("Lines                    "));
	Serial.println(testLines(ILI9341_CYAN));
	delay(100);

	Serial.print(F("Horiz/Vert Lines         "));
	Serial.println(testFastLines(ILI9341_RED, ILI9341_BLUE));
	delay(100);

	Serial.print(F("Rectangles (outline)     "));
	Serial.println(testRects(ILI9341_GREEN));
	delay(100);

	Serial.print(F("Rectangles (filled)      "));
	Serial.println(testFilledRects(ILI9341_YELLOW, ILI9341_MAGENTA));
	delay(100);

	Serial.print(F("Circles (filled)         "));
	Serial.println(testFilledCircles(10, ILI9341_MAGENTA));

	Serial.print(F("Circles (outline)        "));
	Serial.println(testCircles(10, ILI9341_WHITE));
	delay(100);

	Serial.print(F("Triangles (outline)      "));
	Serial.println(testTriangles());
	delay(100);

	Serial.print(F("Triangles (filled)       "));
	Serial.println(testFilledTriangles());
	delay(100);

	Serial.print(F("Rounded rects (outline)  "));
	Serial.println(testRoundRects());
	delay(100);

	Serial.print(F("Rounded rects (filled)   "));
	Serial.println(testFilledRoundRects());
	delay(100);

	//testText();
	tft.fillScreen(ILI9341_BLACK);
	tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(2);
	tft.println("Display test end");

	Serial.println(F("Done!"));
}

void drawButtons()
{
	x = 0;
	y = 0;
	n = 0;
	tft.fillScreen(ILI9341_BLACK);                // �������� �����
	//tft.setRotation(1);
	tft.setTextSize(3);

	for (y = 0; y < 4; y++)
	{

		for (x = 0; x < 4; x++)
		{
			tft.fillRoundRect(5 + ((x * 50) + (x * 5)), 5 + ((y * 50) + (y * 5)), 50, 50, 5, tft.color565(0, 0, 255));
			tft.drawRoundRect(5 + ((x * 50) + (x * 5)), 5 + ((y * 50) + (y * 5)), 50, 50, 5, tft.color565(255, 255, 255));
			tft.setCursor(20 + ((x * 50) + (x * 5)), 20 + ((y * 50) + (y * 5)));
			n++;
			tft.print(n);
		}
	}
}


void setup()
{

	Serial.begin(9600);                  // �������� ������
	while (!Serial);                     // ������� ����������� � USB / Serial ����� ����� ����������, ������������ � ���� �� ������.

	Serial.println("Start setup");


	myTouch.InitTouch();
	myTouch.setPrecision(PREC_MEDIUM);

	Serial.println("ILI9341 Test!");

	tft.begin();

	//test_display();
	drawButtons();

	Serial.println("Setup end");
}

void loop()
{



	/*for (uint8_t rotation = 0; rotation<4; rotation++) {
		tft.setRotation(rotation);
		testText();
		delay(1000);
	}*/

	delay(100);
}


unsigned long testFillScreen() {
	unsigned long start = micros();
	tft.fillScreen(ILI9341_BLACK);
	yield();
	tft.fillScreen(ILI9341_RED);
	yield();
	tft.fillScreen(ILI9341_GREEN);
	yield();
	tft.fillScreen(ILI9341_BLUE);
	yield();
	tft.fillScreen(ILI9341_BLACK);
	yield();
	return micros() - start;
}

unsigned long testText() {
	tft.fillScreen(ILI9341_BLACK);
	unsigned long start = micros();
	tft.setCursor(0, 0);
	tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
	tft.println("Hello World!");
	tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
	tft.println(1234.56);
	tft.setTextColor(ILI9341_RED);    tft.setTextSize(3);
	tft.println(0xDEADBEEF, HEX);
	tft.println();
	tft.setTextColor(ILI9341_GREEN);
	tft.setTextSize(5);
	tft.println("Groop");
	tft.setTextSize(2);
	tft.println("I implore thee,");
	tft.setTextSize(1);
	tft.println("my foonting turlingdromes.");
	tft.println("And hooptiously drangle me");
	tft.println("with crinkly bindlewurdles,");
	tft.println("Or I will rend thee");
	tft.println("in the gobberwarts");
	tft.println("with my blurglecruncheon,");
	tft.println("see if I don't!");
	return micros() - start;
}

unsigned long testLines(uint16_t color) {
	unsigned long start, t;
	int           x1, y1, x2, y2,
		w = tft.width(),
		h = tft.height();

	tft.fillScreen(ILI9341_BLACK);
	yield();

	x1 = y1 = 0;
	y2 = h - 1;
	start = micros();
	for (x2 = 0; x2<w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
	x2 = w - 1;
	for (y2 = 0; y2<h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
	t = micros() - start; // fillScreen doesn't count against timing

	yield();
	tft.fillScreen(ILI9341_BLACK);
	yield();

	x1 = w - 1;
	y1 = 0;
	y2 = h - 1;
	start = micros();
	for (x2 = 0; x2<w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
	x2 = 0;
	for (y2 = 0; y2<h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
	t += micros() - start;

	yield();
	tft.fillScreen(ILI9341_BLACK);
	yield();

	x1 = 0;
	y1 = h - 1;
	y2 = 0;
	start = micros();
	for (x2 = 0; x2<w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
	x2 = w - 1;
	for (y2 = 0; y2<h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
	t += micros() - start;

	yield();
	tft.fillScreen(ILI9341_BLACK);
	yield();

	x1 = w - 1;
	y1 = h - 1;
	y2 = 0;
	start = micros();
	for (x2 = 0; x2<w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
	x2 = 0;
	for (y2 = 0; y2<h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);

	yield();
	return micros() - start;
}

unsigned long testFastLines(uint16_t color1, uint16_t color2) {
	unsigned long start;
	int           x, y, w = tft.width(), h = tft.height();

	tft.fillScreen(ILI9341_BLACK);
	start = micros();
	for (y = 0; y<h; y += 5) tft.drawFastHLine(0, y, w, color1);
	for (x = 0; x<w; x += 5) tft.drawFastVLine(x, 0, h, color2);

	return micros() - start;
}

unsigned long testRects(uint16_t color) {
	unsigned long start;
	int           n, i, i2,
		cx = tft.width() / 2,
		cy = tft.height() / 2;

	tft.fillScreen(ILI9341_BLACK);
	n = min(tft.width(), tft.height());
	start = micros();
	for (i = 2; i<n; i += 6) {
		i2 = i / 2;
		tft.drawRect(cx - i2, cy - i2, i, i, color);
	}

	return micros() - start;
}

unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
	unsigned long start, t = 0;
	int           n, i, i2,
		cx = tft.width() / 2 - 1,
		cy = tft.height() / 2 - 1;

	tft.fillScreen(ILI9341_BLACK);
	n = min(tft.width(), tft.height());
	for (i = n; i>0; i -= 6) {
		i2 = i / 2;
		start = micros();
		tft.fillRect(cx - i2, cy - i2, i, i, color1);
		t += micros() - start;
		// Outlines are not included in timing results
		tft.drawRect(cx - i2, cy - i2, i, i, color2);
		yield();
	}

	return t;
}

unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
	unsigned long start;
	int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

	tft.fillScreen(ILI9341_BLACK);
	start = micros();
	for (x = radius; x<w; x += r2) {
		for (y = radius; y<h; y += r2) {
			tft.fillCircle(x, y, radius, color);
		}
	}

	return micros() - start;
}

unsigned long testCircles(uint8_t radius, uint16_t color) {
	unsigned long start;
	int           x, y, r2 = radius * 2,
		w = tft.width() + radius,
		h = tft.height() + radius;

	// Screen is not cleared for this one -- this is
	// intentional and does not affect the reported time.
	start = micros();
	for (x = 0; x<w; x += r2) {
		for (y = 0; y<h; y += r2) {
			tft.drawCircle(x, y, radius, color);
		}
	}

	return micros() - start;
}

unsigned long testTriangles() {
	unsigned long start;
	int           n, i, cx = tft.width() / 2 - 1,
		cy = tft.height() / 2 - 1;

	tft.fillScreen(ILI9341_BLACK);
	n = min(cx, cy);
	start = micros();
	for (i = 0; i<n; i += 5) {
		tft.drawTriangle(
			cx, cy - i, // peak
			cx - i, cy + i, // bottom left
			cx + i, cy + i, // bottom right
			tft.color565(i, i, i));
	}

	return micros() - start;
}

unsigned long testFilledTriangles() {
	unsigned long start, t = 0;
	int           i, cx = tft.width() / 2 - 1,
		cy = tft.height() / 2 - 1;

	tft.fillScreen(ILI9341_BLACK);
	start = micros();
	for (i = min(cx, cy); i>10; i -= 5) {
		start = micros();
		tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
			tft.color565(0, i * 10, i * 10));
		t += micros() - start;
		tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
			tft.color565(i * 10, i * 10, 0));
		yield();
	}

	return t;
}

unsigned long testRoundRects() {
	unsigned long start;
	int           w, i, i2,
		cx = tft.width() / 2 - 1,
		cy = tft.height() / 2 - 1;

	tft.fillScreen(ILI9341_BLACK);
	w = min(tft.width(), tft.height());
	start = micros();
	for (i = 0; i<w; i += 6) {
		i2 = i / 2;
		tft.drawRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(i, 0, 0));
	}

	return micros() - start;
}

unsigned long testFilledRoundRects() {
	unsigned long start;
	int           i, i2,
		cx = tft.width() / 2 - 1,
		cy = tft.height() / 2 - 1;

	tft.fillScreen(ILI9341_BLACK);
	start = micros();
	for (i = min(tft.width(), tft.height()); i>20; i -= 6) {
		i2 = i / 2;
		tft.fillRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(0, i, 0));
		yield();
	}

	return micros() - start;
}