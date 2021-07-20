```//group project

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define SS_PIN 53
#define RST_PIN 5


//This is for LCD change info button
//define port H Register pointers
volatile unsigned char* portH = (unsigned char*) 0x102;
volatile unsigned char* ddrH = (unsigned char*) 0x101;
volatile unsigned char* pinH = (unsigned char*) 0x100;
// Watersensor
volatile unsigned char* portF = (unsigned char*) 0x31;
volatile unsigned char* ddrF = (unsigned char*) 0x30;
volatile unsigned char* pinF  = (unsigned char*) 0x2F;
// onboard LED
volatile unsigned char* port_b = (unsigned char*) 0x25;
volatile unsigned char* ddr_b  = (unsigned char*) 0x24;
volatile unsigned char* pin_b  = (unsigned char*) 0x23;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

// initialize LCD interface pins
LiquidCrystal lcd(7,8,9,10,11,12);

bool buttonPress();
bool buttonReleased();
byte nuidPICC[4];
volatile bool authorized = false;
volatile int screenstate = 0;

void setup() {
  //code from TENEKO for RFID
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  //printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  //My LCD and Button code
  //begin statement for LCD requiring the number of columns and rows:
  lcd.begin(16, 2);
  //WATER LEVEL SENSOR
  *ddrF &= ~(0x01);
  //BUTTON
  *ddrH &= ~(0x08);
  *portH |= (0x08);
  //LED
  *ddr_b |= 0x80;
}

void loop() {
  
  while(!rfid.PICC_ReadCardSerial()&& !rfid.PICC_IsNewCardPresent()){
    lcd.setCursor(0, 0);
    lcd.print(" Not Authorized ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    while (buttonPress()){
      lcd.setCursor(0, 0);
      lcd.print("   PLEASE USE   ");
      lcd.setCursor(0, 1);
      lcd.print("   RFID CARD    ");
      delay(200);
    }
  }
  //read card
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) 
  {
   Serial.println(F("Your tag is not of type MIFARE Classic."));
   authorized = false;
  } else {
    //Activates LCD for use
    authorized = true;
    Serial.println(F("Authorized User"));
  }
  
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  if (authorized = true){
    delay(100);
    for (int i = 0; i < 3; i++){
      lcd.setCursor(0, 0);
      lcd.print(" Authorizing.   ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      delay(120);
      lcd.setCursor(0, 0);
      lcd.print(" Authorizing..  ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      delay(120);
      lcd.setCursor(0, 0);
      lcd.print(" Authorizing... ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      delay(120);
  }
  screenstate = 1;
  while(authorized = true){
    while(screenstate == 1){
      lcd.setCursor(0, 0);
      lcd.print("Water Level:    ");
      lcd.setCursor(0, 1);
      lcd.print("over 9000       ");
      while(buttonPress()){
        screenstate = 2;
      }
    }
    while(screenstate == 2){
      lcd.setCursor(0, 0);
      lcd.print("Air Temperature ");
      lcd.setCursor(0, 1);
      lcd.print("over 9000       ");
      while(buttonPress()){
        screenstate = 1;
      }
    }
   }
  }
}
   



bool buttonPress(){
  if (*pinH & (0x08)){  
    *port_b |= 0x80; //LED HIGH
    return false;
   } else {
    *port_b &= ~(0x80); //LED LOW
    return true;
   }
}
```