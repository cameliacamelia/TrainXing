#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

#include <SPI.h>
#include <LoRa.h>
// #include "SSD1306.h"
#include<Arduino.h>


String trainDetectorMessage = "TD01";

//OLED pins to ESP32 GPIOs via this connecthin:
//OLED_SDA — GPIO4
//OLED_SCL — GPIO15
//OLED_RST — GPIO16

SSD1306 display(0x3c, 4, 15);

 

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

#define spreadingFactor 12
#define SignalBandwidth 7.8E3

#define codingRateDenominator 5

#define preambleLength 8

int counter = 0;

void setup() {
  pinMode(25,OUTPUT); //Send success, LED will bright 1 second
  
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW); // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH);
  
  Serial.begin(115200);
  while (!Serial); //If just the the basic function, must connect to a computer

// Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(5,5,"LoRa Sender");
  display.display();
  
  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);
  Serial.println("LoRa Sender");

    Serial.print("LoRa Spreading Factor: ");
  Serial.println(spreadingFactor);
  LoRa.setSpreadingFactor(spreadingFactor);
  
  Serial.print("LoRa Signal Bandwidth: ");
  Serial.println(SignalBandwidth);
  LoRa.setSignalBandwidth(SignalBandwidth);

  LoRa.setCodingRate4(codingRateDenominator);

  LoRa.setPreambleLength(preambleLength);
  LoRa.enableCrc();
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  Serial.println("LoRa Initial OK!");
  display.drawString(5,20,"LoRa Initializing OK!");
  display.display();
  delay(2000);
}

int overValuesCounter = 0;
int clearCounter = 0;

void loop() {
  int analog_value = analogRead(12);
  Serial.println(analog_value);
  if (analog_value > 2000){
    overValuesCounter++;
    if (overValuesCounter > 3){
      LoRa.beginPacket();
      LoRa.print(trainDetectorMessage);
      LoRa.print(counter);
      LoRa.endPacket();
      Serial.println("Train detected!");
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.drawString(5,20, "Train detected!");
      display.display();
      overValuesCounter = 0;
      clearCounter = 0;
    }
  }
  else{
    overValuesCounter = 0;
    clearCounter++;
    if (clearCounter > 180){
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.display();
    }
  }
  delay(30);
}
//
//void sendTestPacket(){
//  Serial.print("Sending packet: ");
//  Serial.println(counter);
//  
//  display.clear();
//  display.setFont(ArialMT_Plain_16);
//  display.drawString(3, 5, "Sending packet ");
//  display.drawString(50, 30, String(counter));
//  display.display();
//  
//  // send packet
//  LoRa.beginPacket();
//  LoRa.print("Hello..");
//  LoRa.print(counter);
//  LoRa.endPacket();
//  
//  counter++;
//  digitalWrite(25, HIGH); // turn the LED on (HIGH is the voltage level)
//  delay(1000); // wait for a second
//  digitalWrite(25, LOW); // turn the LED off by making the voltage LOW
//  delay(1000); // wait for a second
//}

