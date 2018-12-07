// Adding Libraries
// Adding a comment to check the Git
//New Comment
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // Hardware-specific library
MCUFRIEND_kbv tft;
#include <Keypad.h>
#include <string.h>
#include <stdio.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <FreeDefaultFonts.h>
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410

// Defining Variables
const byte ROWS = 4;
const byte COLS = 3;
bool start = true;
bool filled = false;
bool reset = "false";
long time;
char strval[10];
byte statusLed    = 13;
int numval;
byte sensorInterrupt = 2;  // 0 = digital pin 2
byte sensorPin       = 53;
byte solonoid = 36;

// The hall-effect flow sensor outputs approximately 6.52 pulses per second per
// litre/minute of flow.
float calibrationFactor = 6.52;
volatile byte pulseCount;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {22, 24, 26, 28};
byte colPins[COLS] = {30, 32, 34};
String num = "";
String val = "";
String temp = "";
String bottles = "";
int n = 0;
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
void setup(void)
{
  pinMode(solonoid, OUTPUT);
  digitalWrite(solonoid, HIGH);
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;
  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  if (ID == 0xD3) ID = 0x9481;
  tft.begin(ID);
  tft.setRotation(0);
  tft.fillScreen(WHITE);
  showmsgXY(50, 150, 2, &FreeSans9pt7b, "JENTEK");
  showmsgXY(50, 170, 1, &FreeSans9pt7b, "Smart Solutions");
  delay(3000);
  tft.fillScreen(WHITE);
}

void loop(void)
{

  char customKey = customKeypad.getKey();

  if (start) {

    showmsgXY(50, 40, 1, &FreeSans9pt7b, "Auto Filling System");
    tft.drawFastHLine(0, 100, tft.width(), BLACK);
    tft.drawFastHLine(0, 220, tft.width(), BLACK);
    tft.setTextColor(RED, WHITE);
    tft.setTextSize(2);
    tft.setCursor(30, 150);
    tft.print("Enter Filling \n   Amount");
    tft.setTextSize(2);
    tft.setCursor(200, 290);
    tft.print("ml");
    start = false;
  } else {
    if (customKey) {
      if (customKey == '*') {
        val = "";
        tft.fillScreen(WHITE);
        def();
        if (num[0] == '\0') {
          tft.setTextColor(RED, GREY);
          showmsgXY(30, 120, 2, &FreeSans9pt7b, "Number Of \n   Bottles");
          num = temp;

          numval = num.toInt();

          temp = "";
        }
        else if (bottles[0] == '\0') {
          bottles = temp;
          temp = "";
        } 

      } else {
        if (num[0] == '\0') {
          temp = enterNum(customKey);
        }
        else  if (bottles[0] == '\0') {
          temp = enterNum(customKey);
        } 
        Serial.println(num);

        Serial.println(customKey);
      }
    }
    if (!num[0] == '\0' && !bottles[0] == '\0') {
      if (!filled) {
        digitalWrite(solonoid, LOW);

        Serial.println("here coming");
        filled = true;

      }

      while (totalMilliLitres < numval - 50) {

        if ((millis() - oldTime) > 1000)   // Only process counters once per second
        {
          // Disable the interrupt while calculating flow rate and sending the value to
          // the host
          detachInterrupt(sensorInterrupt);

          // Because this loop may not complete in exactly 1 second intervals we calculate
          // the number of milliseconds that have passed since the last execution and use
          // that to scale the output. We also apply the calibrationFactor to scale the output
          // based on the number of pulses per second per units of measure (litres/minute in
          // this case) coming from the sensor.
          flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;

          // Note the time this processing pass was executed. Note that because we've
          // disabled interrupts the millis() function won't actually be incrementing right
          // at this point, but it will still return the value it was set to just before
          // interrupts went away.
          oldTime = millis();

          // Divide the flow rate in litres/minute by 60 to determine how many litres have
          // passed through the sensor in this 1 second interval, then multiply by 1000 to
          // convert to millilitres.
          flowMilliLitres = (flowRate / 60) * 1000;

          // Add the millilitres passed in this second to the cumulative total
          totalMilliLitres += flowMilliLitres;

          unsigned int frac;

          // Reset the pulse counter so we can start incrementing again
          pulseCount = 0;

          // Enable the interrupt again now that we've finished sending output
          attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
          Serial.println(totalMilliLitres);
        }
      }
      Serial.println("Stop Val **************************************************************");
      Serial.print(totalMilliLitres);
      digitalWrite(solonoid, HIGH);

      Serial.println("bottles" + bottles);
      Serial.println("num" + num);

      showmsgXY(30, 100, 2, &FreeSans9pt7b, "Filling up ...");
      sprintf(strval, "%d", n);
      tft.setTextColor(RED, WHITE);
      tft.setTextSize(1);
      tft.setCursor(30, 170);
      tft.setFont(&FreeSevenSegNumFont);
      tft.fillRect(30, 120, 120, 50, WHITE) ;
      tft.print(totalMilliLitres);
   
      showmsgXY(30, 220, 1, &FreeSans9pt7b, "Number of \n      Bottles :");
      tft.setTextColor(RED, WHITE);
      tft.setTextSize(1);
      tft.setFont(&FreeSevenSegNumFont);
      tft.setCursor(10, 300);
      tft.print("10");
      Serial.println(time);
      tft.drawFastVLine( 130, 220, 70, BLACK);
      showmsgXY(160, 220, 1, &FreeSans9pt7b, "Seconds");
      showmsgXY(160, 235, 1, &FreeSans9pt7b, "Proceed");
    }
  }
}
void showmsgXY(int x, int y, int sz, const GFXfont * f, const char *msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  //tft.drawFastHLine(0, y, tft.width(), WHITE);
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(BLACK);
  tft.setTextSize(sz);
  tft.print(msg);

}
void screenOne() {
  showmsgXY(50, 40, 1, &FreeSans9pt7b, "Auto Filling System");

  tft.drawFastHLine(30, 60, 170, BLACK);
  tft.drawFastHLine(30, 180, 170, BLACK);
}
void def() {
  showmsgXY(50, 40, 1, &FreeSans9pt7b, "Auto Filling System");

  tft.drawFastHLine(30, 60, 170, BLACK);
  tft.drawFastHLine(30, 180, 170, BLACK);
}
String enterNum(char customKey) {

  val = val + customKey;
  tft.setTextColor(RED, GREY);
  tft.setTextSize(1);
  tft.setFont(&FreeSevenSegNumFont);
  tft.setCursor(30, 300);
  tft.print(val);
  return val;
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
