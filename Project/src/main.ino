//group project
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <dht_nonblocking.h>

#define DHT_SENSOR_TYPE DHT_TYPE_11
#define DHT_SENSOR_PIN 2
#define SS_PIN 53
#define RST_PIN 13
#define A0_PIN 0

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

DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
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
volatile int state = 0;
volatile int next_state = 2000;

void setup() {
  Serial.begin(9600);
  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  adc_init();      // initialize ADC for 8 bit resolution

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  lcd.begin(16, 2);
  //WATER LEVEL SENSOR
  //*ddr_f &= ~(0x01);
  //BUTTON
  *ddr_a &= ~(1 << PA1);
  *port_a |= (1 << PA1);
  //LED
  *ddr_b |= 0x80;
}

void loop() {

  state = next_state;

  switch(state) {

    case RFID_READING_STATE: {
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
      else {
        next_state = AUTHORIZING_STATE;
      }
      // Halt PICC
      rfid.PICC_HaltA();
      // Stop encryption on PCD
      rfid.PCD_StopCrypto1();
      break;
    }
    case AUTHORIZING_STATE: {
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
      next_state = WATER_LEVEL_SCREEN_STATE;
      break;
    }
    case WATER_LEVEL_SCREEN_STATE: {
      lcd.setCursor(0, 0);
      lcd.print("Water Level:    ");
      lcd.setCursor(0, 1);
      lcd.print("over 9000       ");
      if(buttonPress()) {
        next_state = AIR_TEMPERATURE_SCREEN_STATE;
      }
      break;
    }
    case AIR_TEMPERATURE_SCREEN_STATE: {
      lcd.setCursor(0, 0);
      lcd.print("Air Temperature ");
      lcd.setCursor(0, 1);
      lcd.print("over 9000       ");
      if(buttonPress()) {
        next_state = WATER_LEVEL_SCREEN_STATE;
      }
      break;
    }
  }
}

bool buttonPress(){
  if (*pin_a & (1 << PA1)){  
    delay(200);
    *port_b |= 0x80; //LED HIGH
    return false;
   } else {
     delay(200);
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
