#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"


// 0 - error or other than 1-3
// 1 - normal
// 2 - train from left
// 3 - train from right
int state = 1;

long lastTimeHeartBeat01;
long lastTimeHeartBeat02;

SSD1306 display(0x3c, 4, 15);

//OLED pins to ESP32 GPIOs via this connection:
//OLED_SDA — GPIO4
//OLED_SCL — GPIO15
//OLED_RST — GPIO16

 

// WIFI_LoRa_32 ports

// GPIO5 — SX1278’s SCK
// GPIO19 — SX1278’s MISO
// GPIO27 — SX1278’s MOSI
// GPIO18 — SX1278’s CS
// GPIO14 — SX1278’s RESET
// GPIO26 — SX1278’s IRQ(Interrupt Request)

#define SS 18
#define RST 14
#define DI0 26
#define BAND 866E6

// LoRa Settings

#define spreadingFactor 7
#define SignalBandwidth 125E3

#define codingRateDenominator 5

#define preambleLength 8

// ADC? Battery voltage
// const uint8_t vbatPin = 34;
// float VBAT;  // battery voltage from ESP32 ADC read



// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0
#define LEDC_CHANNEL_1     1

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     1000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED1_PIN           13
#define LED2_PIN           12

int brightness1 = 0;    // how bright the LED is
int fadeAmount1 = 20;    // how many points to fade the LED by

int brightness2 = 0;    // how bright the LED is
int fadeAmount2 = 20;    // how many points to fade the LED by
long now, lastLED1Time, lastLED2Time, deltaTime;

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}


void setupLEDChanngel0(){
    // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED1_PIN, LEDC_CHANNEL_0);
}

void setupLEDChanngel1(){
    // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED2_PIN, LEDC_CHANNEL_1);
}

void loopLEDChannel0(){
    // set the brightness on LEDC channel 0
  ledcAnalogWrite(LEDC_CHANNEL_0, brightness1);

  // change the brightness for next time through the loop:
  brightness1 = brightness1 + fadeAmount1;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness1 <= 0 || brightness1 >= 255) {
    fadeAmount1 = -fadeAmount1;
  }
}

void loopLEDChannel1(){
    // set the brightness on LEDC channel 0
  ledcAnalogWrite(LEDC_CHANNEL_1, brightness2);

  // change the brightness for next time through the loop:
  brightness2 = brightness2 + fadeAmount2;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness2 <= 0 || brightness2 >= 255) {
    fadeAmount2 = -fadeAmount2;
  }
}

void stopLED1(){
  ledcAnalogWrite(LEDC_CHANNEL_0, 0);
  brightness1 = 0;
}

void stopLED2(){
  ledcAnalogWrite(LEDC_CHANNEL_1, 0);
  brightness2 = 0;
}
void setup() {
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW); // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
  Serial.begin(115200);
  while (!Serial); //if just the the basic function, must connect to a computer
  delay(1000);
  
  Serial.println("LoRa Receiver");
  display.drawString(5,5,"LoRa Receiver");
  display.display();
  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);

/*     
  pinMode(vbatPin, INPUT);
  VBAT = (120.0/20.0) * (float)(analogRead(vbatPin)) / 1024.0; // LiPo battery voltage in volts
  Serial.println("Vbat = "); Serial.print(VBAT); Serial.println(" Volts");
*/

  Serial.print("LoRa Frequency: ");
  Serial.println(BAND);
  
  Serial.print("LoRa Spreading Factor: ");
  Serial.println(spreadingFactor);
  LoRa.setSpreadingFactor(spreadingFactor);
  
  Serial.print("LoRa Signal Bandwidth: ");
  Serial.println(SignalBandwidth);
  LoRa.setSignalBandwidth(SignalBandwidth);

  LoRa.setCodingRate4(codingRateDenominator);
  
  if (!LoRa.begin(BAND)) {
    display.drawString(5,25,"Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initial OK!");
  

  
  display.drawString(5,25,"LoRa Initializing OK!");
  display.display();

  setupLEDChanngel0();
  setupLEDChanngel1();
}


void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packets
    Serial.print("Received packet. ");
    
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(3, 0, "Received packet ");
    display.display();
    
    // read packet
    while (LoRa.available()) {
      String data = LoRa.readString();
      Serial.print("Received: "); Serial.println(data);

        
      if (data.startsWith("HB01")){
        
        lastTimeHeartBeat01 = millis();
      }

      if (data.startsWith("HB02")){
        lastTimeHeartBeat02 = millis();
      }
// 0 - error or other than 1-3
// 1 - normal
// 2 - train from left
// 3 - train from right

      if (state == 1){
        if (data.startsWith("TD01")){
          state = 2;
        }  

        if (data.startsWith("TD02")){
          state = 3;
        }
      }

      if (state == 2){
        if (data.startsWith("TD02")){
          state = 1;
        }  
      }
      
      if (state == 3){
        if (data.startsWith("TD01")){
          state = 1;
        }  
      }
      
      Serial.print(data);
      display.drawString(20,22, data);
      display.display();
    }
    
    // print RSSI of packet
    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());
    Serial.print(" with SNR ");
    Serial.println(LoRa.packetSnr());
    // display.drawString(0, 45, "RSSI: ");
    // display.drawString(50, 45, (String)LoRa.packetRssi());
    
    display.drawString(0, 45, (String)LoRa.packetRssi() + "dB (" + (String)LoRa.packetSnr() +"dB)");
        
    display.display();
  }

  now = millis();

  deltaTime = now - lastTimeHeartBeat01;
  if (deltaTime > 5000){
    state = 0;
  }

  deltaTime = now - lastTimeHeartBeat02;
  if (deltaTime > 5000){
    state = 0;
  }
  
  if (state == 0){
    stopLED1();
    stopLED2();
  }
  else if (state == 2 || state == 3){
    stopLED1();
    deltaTime = now - lastLED2Time;
    if (deltaTime > 50){
       loopLEDChannel1();
       lastLED2Time = now;
    }
  }
  else if (state == 1) {
    stopLED2();
    deltaTime = now - lastLED1Time;

    if (deltaTime > 100){
       loopLEDChannel0();
       lastLED1Time = now;
    }
  } else {
    stopLED1();
    stopLED2();
  }
 
}
