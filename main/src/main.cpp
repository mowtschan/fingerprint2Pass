#include <stdint.h>
#include <Adafruit_Fingerprint.h>
#include <BleKeyboard.h>
#include "MacrosPass.h"

#define SensorPassword 0x00000000
#define TouchSensor 2
#define active_low 1 // 0- FPC1021A, 1- R503
#define RXD2 16      // hardware serial2 on esp32 (receive)
#define TXD2 17      // hardware serial2 on esp32 (transmit)

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2, SensorPassword);
BleKeyboard bleKeyboard("BLE Keyboard", "ESP32", 100);
uint8_t id;

int getFingerprintIDez();
unsigned int hexToDec(String hexString);
void KeyOutput(String str);

void setup(void)
{
  Serial.begin(115200);
  finger.begin(57600);
  bleKeyboard.begin();
  pinMode(TouchSensor, INPUT);

  while (!Serial)
  {
    Serial.println("Waiting for Serial terminal...");
    delay(1000);
  }

  while (!finger.verifyPassword())
  {
    Serial.println("Fingerprint module not connected.");
    delay(5000);
  }

  Serial.println("Fingerprint module connected!");
}

void loop(void)
{
  while (!(active_low ^ digitalRead(TouchSensor)))
  {
    // wait for touch event
  }
  Serial.println("touch event received");

  int FingerID = getFingerprintIDez();
  if (FingerID > 0)
  {
    KeyOutput(data[FingerID - 1]);
  }
}

int getFingerprintIDez()
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
  {
    Serial.println("finger.getImage failed");
    return -1;
  }

  finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
  p = finger.image2Tz();
  finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_PURPLE);
  if (p != FINGERPRINT_OK)
  {
    Serial.println("finger.image2Tz failed");
    return -1;
  }

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK)
  {
    finger.LEDcontrol(FINGERPRINT_LED_GRADUAL_OFF, 250, FINGERPRINT_LED_RED);
    Serial.println("finger.fingerSearch failed");
    return -1;
  }

  finger.LEDcontrol(FINGERPRINT_LED_GRADUAL_OFF, 120, FINGERPRINT_LED_BLUE);
  Serial.println("fingerprint found");
  return finger.fingerID;
}

unsigned int hexToDec(String hexString)
{

  unsigned int decValue = 0;
  int nextInt;
  hexString.toUpperCase();
  for (int i = 0; i < hexString.length(); i++)
  {
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57)
      nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70)
      nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102)
      nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    decValue = (decValue * 16) + nextInt;
  }
  return decValue;
}

void KeyOutput(String str)
{
  int pos = str.indexOf('|');
  while (pos >= 0)
  {
    if (pos > 0)
    {
      bleKeyboard.print(str.substring(0, pos - 1));
    }

    str.remove(0, pos + 1);
    char code = str.charAt(0);
    pos = str.indexOf(' ');
    String text = str.substring(1, pos);
    byte z;
    switch (code)
    {
    case 'w':
      z = hexToDec(text);
      bleKeyboard.write(z);
      break;
    case 't':
      delay(text.toInt());
      break;
    case 'p':
      z = hexToDec(text);
      bleKeyboard.press(z);
      break;
    case 'r':
      z = hexToDec(text);
      bleKeyboard.release(z);
      break;
    case 'a':
      bleKeyboard.releaseAll();
      break;
    }
    str.remove(0, pos + 1);
    pos = str.indexOf('|');
  }
  if (str.length() > 0)
    bleKeyboard.print(str);
}
