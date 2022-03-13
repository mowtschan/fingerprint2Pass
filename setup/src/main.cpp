#include <stdint.h>
#include <Adafruit_Fingerprint.h>

#define RXD2 16 // hardware serial2 on esp32 (receive)
#define TXD2 17 // hardware serial2 on esp32 (transmit)

#define FingerprintPassword 0x00000000    // Current Sensor Passwort
#define FingerprintPasswordNeu 0x00000000 // New Sensor Password

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2, FingerprintPassword);
uint8_t id;
int FingerID = -1;

int getFingerprintIDez();
uint8_t deleteFingerprint(uint8_t id);
uint8_t getFingerprintEnroll();
bool checkFingerPrint(uint8_t p);
uint8_t readNumber();

void setup(void)
{
  Serial.begin(115200);
  finger.begin(57600);
  while (!Serial)
  {
    Serial.println("Waiting for Serial terminal...");
    delay(1000);
  }

  Serial.println("Start!");
  if (finger.verifyPassword() and Serial)
    Serial.println("Sensor Passwort verified!");
}

void loop(void)
{
  FingerID = getFingerprintIDez();
  if (FingerID >= 0)
  {
    Serial.print("Found ID #");
    Serial.println(FingerID);
    delay(10);
  }

  if (Serial.available())
  {
    char Wahl = Serial.read();
    Serial.flush();
    Serial.println(Wahl);
    switch (Wahl)
    {
    case 'r':
      Serial.println("Please set ID");
      id = readNumber();
      Serial.print("Save ID #");
      Serial.println(id);
      while (!getFingerprintEnroll())
      {
        delay(100);
      }
      break;

    case 'd':
      Serial.println("Set ID you want to remove");
      id = readNumber();
      Serial.print("ID #");
      Serial.print(id);
      Serial.println(" Are you sure? y/n");
      Serial.flush();
      while (!Serial.available())
      {
        // wait for serial is available
      }

      Wahl = Serial.read();
      Serial.println(Wahl);
      if (Wahl == 'y')
      {
        deleteFingerprint(id);
      }
      break;

    case 'p':
      Serial.print("Setting password...");
      uint8_t p = finger.setPassword(FingerprintPasswordNeu);
      if (p == FINGERPRINT_OK)
      {
        Serial.println("OK. Password set to: ");
        Serial.println(FingerprintPasswordNeu, HEX);
        Serial.println("Don't forget to update 'FingerprintPassword' variable!");
      }
      else
      {
        Serial.println("Error");
      }
      break;
    }
  }
}

uint8_t deleteFingerprint(uint8_t id)
{
  uint8_t p = -1;
  p = finger.deleteModel(id);
  checkFingerPrint(p);
  return p;
}

uint8_t getFingerprintEnroll()
{
  int p = -1;
  Serial.println("Put your finger on the sensor");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    checkFingerPrint(p);
  }

  p = finger.image2Tz(1);
  if (!checkFingerPrint(p))
  {
    return p;
  }

  Serial.println("Remove your finger from the sensor");
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }

  delay(1000);
  Serial.print("ID ");
  Serial.println(id);
  p = -1;

  Serial.println("Please put the same finger again");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    checkFingerPrint(p);
    checkFingerPrint(p);
  }

  p = finger.image2Tz(2);
  if (!checkFingerPrint(p))
  {
    return p;
  }

  Serial.print("Create a modell for #");
  Serial.println(id);
  p = finger.createModel();
  if (!checkFingerPrint(p))
  {
    return p;
  }

  Serial.println("Fingerprint model created!");
  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (!checkFingerPrint(p))
  {
    return p;
  }

  Serial.println("Fingerprint saved!");
  return true;
}

int getFingerprintIDez()
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
  {
    return -1;
  }

  finger.LEDcontrol(FINGERPRINT_LED_GRADUAL_OFF, 200, FINGERPRINT_LED_PURPLE);
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK)
  {
    finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_PURPLE);
    finger.LEDcontrol(FINGERPRINT_LED_GRADUAL_OFF, 255, FINGERPRINT_LED_RED);
    return -1;
  }

  finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_PURPLE);
  finger.LEDcontrol(FINGERPRINT_LED_GRADUAL_OFF, 255, FINGERPRINT_LED_BLUE);
  return finger.fingerID;
}

uint8_t readNumber()
{
  uint8_t num = 0;

  while (num == 0)
  {
    while (!Serial.available())
    {
      // wait for serial
    }
    num = Serial.parseInt();
  }
  return num;
}

bool checkFingerPrint(uint8_t p)
{
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Fingerprint transformed");
    return true;
  case FINGERPRINT_NOFINGER:
    Serial.println("No Fingerprint");
    return false;
  case FINGERPRINT_IMAGEFAIL:
    Serial.println("Imaging error");
    return false;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Fingerprint has bad quality");
    return false;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return false;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return false;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Invalid fingerprint image");
    return false;
  case FINGERPRINT_BADLOCATION:
    Serial.println("Invalid fingerprint location");
    return false;
  case FINGERPRINT_FLASHERR:
    Serial.println("Flash fingerprint failed");
    return false;
  case FINGERPRINT_ENROLLMISMATCH:
    Serial.println("Fingerprint doesn't match");
    return false;
  default:
    Serial.print("Unknown error: 0x");
    Serial.println(p, HEX);
    return false;
  }
}
