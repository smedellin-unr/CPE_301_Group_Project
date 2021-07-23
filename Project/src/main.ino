//group project
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT11
#define DHT_SENSOR_PIN 2
#define SS_PIN 53
#define RST_PIN 13
#define A0_PIN 0
#define ENABLE 5
#define DIRA 3
#define DIRB 4

// Defining states
#define RFID_READING_STATE 2000
#define AUTHORIZING_STATE 2100
#define WATER_LEVEL_SCREEN_STATE 2200
#define AIR_TEMPERATURE_SCREEN_STATE 2300

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
// define port A register pointers
volatile unsigned char* port_a = (unsigned char*) 0x22;
volatile unsigned char* ddr_a  = (unsigned char*) 0x21;
volatile unsigned char* pin_a  = (unsigned char*) 0x20;

DHT dht( DHT_SENSOR_PIN, DHTTYPE );
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

LiquidCrystal lcd(7,8,9,10,11,12);
void badgeIn();
void Cyclic();
void getTempAndWater();
void startYourEngines();
bool buttonPress();
bool buttonReleased();
void adc_init();
uint8_t adc_read(uint8_t adc_channel);

// Globals
byte nuidPICC[4];
volatile float temperature = 0.00;
volatile uint8_t water_level = 0;
volatile bool authorized = false;
volatile int screenstate = 0;
volatile int state = 0;
volatile int next_state = 2000;

const float threshold = 80.70f; // set at this temperature for demonstration purposes

void setup() {
  Serial.begin(9600);
  dht.begin();
  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  adc_init();      // initialize ADC for 8 bit resolution
  lcd.begin(16, 2);
  *ddr_a &= ~(1 << PA1); // Set Digital pin 23 as output
  *port_a |= (1 << PA1); // Set internal pullup resistor
  //LED
  *ddr_b |= 0x80;
  // Motor IO setup
  pinMode(ENABLE,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);
}

void loop() {

  Cyclic();
  state = next_state;
  switch(state) {
    case RFID_READING_STATE: 
      badgeIn();
      break;
    case AUTHORIZING_STATE:
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
      next_state = WATER_LEVEL_SCREEN_STATE;
      break;
    case WATER_LEVEL_SCREEN_STATE:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Water Level:    ");
      lcd.setCursor(0, 1);
      lcd.print(water_level);
      if(buttonPress())
        next_state = AIR_TEMPERATURE_SCREEN_STATE;
      break;
    case AIR_TEMPERATURE_SCREEN_STATE:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Air Temperature ");
      lcd.setCursor(0, 1);
      lcd.print(temperature);
      if(buttonPress())
        next_state = WATER_LEVEL_SCREEN_STATE;
      break;
  }
}

void Cyclic() {

  getTempAndWater();
  startYourEngines();

}


bool buttonPress(){
  if (*pin_a & (1 << PA1)){  
    delay(300);
    *port_b |= 0x80; //LED HIGH
    return false;
   } else {
     delay(300);
    *port_b &= ~(0x80); //LED LOW
    return true;
   }
}

void badgeIn() {
  if(!rfid.PICC_ReadCardSerial()&& 
  !rfid.PICC_IsNewCardPresent()) {
    lcd.setCursor(0, 0);
    lcd.print(" Not Authorized ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    while (buttonPress()) {
      lcd.setCursor(0, 0);
      lcd.print("   PLEASE USE   ");
      lcd.setCursor(0, 1);
      lcd.print("   RFID CARD    ");
      delay(200);
    }
  }
  else 
    next_state = AUTHORIZING_STATE;
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

void startYourEngines()
{
   if(temperature > threshold)
  {
    analogWrite(ENABLE, 75);
    digitalWrite(DIRA,HIGH);
    digitalWrite(DIRB,LOW);
  }
  else
  {
    analogWrite(ENABLE, 0);
    digitalWrite(ENABLE, LOW);
  }
}

void getTempAndWater()
{
  temperature = dht.readTemperature(true);
  if (isnan(temperature))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  water_level = adc_read(A0_PIN);
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
