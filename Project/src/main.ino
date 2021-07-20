//group project
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define SS_PIN 53
#define RST_PIN 5

#define A0_PIN 0

//This is for LCD change info button
//define port H Register pointers
volatile unsigned char* port_h = (unsigned char*) 0x102;
volatile unsigned char* ddr_h  = (unsigned char*) 0x101;
volatile unsigned char* pin_h  = (unsigned char*) 0x100;
// Watersensor
volatile unsigned char* port_f = (unsigned char*) 0x31;
volatile unsigned char* ddr_f  = (unsigned char*) 0x30;
volatile unsigned char* pin_f  = (unsigned char*) 0x2F;
// onboard LED
volatile unsigned char* port_b = (unsigned char*) 0x25;
volatile unsigned char* ddr_b  = (unsigned char*) 0x24;
volatile unsigned char* pin_b  = (unsigned char*) 0x23;
// define port E register pointers
volatile unsigned char* port_e = (unsigned char*) 0x2E;
volatile unsigned char* ddr_e  = (unsigned char*) 0x2D;
volatile unsigned char* pin_e  = (unsigned char*) 0x2C;
// define port G register pointers
volatile unsigned char* port_g = (unsigned char*) 0x34;
volatile unsigned char* ddr_g  = (unsigned char*) 0x33;
volatile unsigned char* pin_g  = (unsigned char*) 0x32;


MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

// initialize LCD interface pins
LiquidCrystal lcd(7,8,9,10,11,12);

bool buttonPress();
bool buttonReleased();
void adc_init();
uint8_t adc_read(uint8_t adc_channel);
byte nuidPICC[4];
volatile uint8_t water_level = 0;
volatile bool authorized = false;
volatile int screenstate = 0;

void setup() {
  //code from TENEKO for RFID
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  // initialize ADC for 8 bit resolution
  adc_init();

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
  //*ddr_f &= ~(0x01);
  //BUTTON
  *ddr_h &= ~(0x08);
  *port_h |= (0x08);
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
  if (*pin_h & (0x08)){  
    *port_b |= 0x80; //LED HIGH
    return false;
   } else {
    *port_b &= ~(0x80); //LED LOW
    return true;
   }
}

void adc_init() {
  // Set reference voltage to voltage supply and left justify the output
  ADMUX |= (1<<REFS0) | (1 << ADLAR);
  // Set prescaler division  factor oto 128
  ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
}

uint8_t adc_read(uint8_t adc_channel) {
  // Clears DIDR0 and DIDR2
  DIDR0 = 0x00;
  DIDR2 = 0x00;
  // Disables appropriate digital input buffer
  if (adc_channel < 8)
    DIDR0 |= (1 << adc_channel);
  else 
    DIDR2 |= (1 << (adc_channel - 8));
  // Start sample
  ADCSRA |= (1 << ADSC);
  // Wait till sampling complete
  while(ADCSRA & (1 << ADIF == 0));
  // return ADC result
  return ADCH;
}
