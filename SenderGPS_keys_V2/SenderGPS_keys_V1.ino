/*
  Lora Send And Receive
  This sketch demonstrates how to send and receive GPS data with the MKR WAN 1300/1310 LoRa module.
*/

#include <MKRWAN.h>
#include "arduino_secrets.h"

#include <SPI.h>
#include <Arduino_MKRGPS.h>

float latitude;
float longitude;
float altitude;
float speed;
float satellites;

LoRaModem modem;

byte localAddress = 0xBB;  //address of this device
byte destination = 0xFF;   //where we are sending data to

// Please enter your sensitive data in the Secret tab or arduino_secrets.h
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;
String msg;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  while (!Serial);
    // change this to your regional band (eg. US915, AS923, ...)
    if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  };

    // We are using the pins
    if (!GPS.begin(GPS_MODE_SHIELD)) {
    Serial.println("Failed to initialize GPS!");
    while (1);
  }
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  int connected = modem.joinOTAA(appEui, appKey);

  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }

  // Set poll interval to 60 secs.
  modem.minPollInterval(60);
  // NOTE: independent of this setting, the modem will
  // not allow sending more than one message every 2 minutes,
  // this is enforced by firmware and can not be changed.
}

void loop() {
    // check if there is new GPS data available
    if (GPS.available()) {

        // read GPS values
        latitude   = GPS.latitude();
        longitude  = GPS.longitude();
        altitude   = GPS.altitude();
        speed      = GPS.speed();
        satellites = GPS.satellites();

        Serial.println("Nuevo mensaje GPS");

        // Print the GPS information
        printValues();

        // Print the hexadecimal payload message
        printHexMsg();

        // Send the message over LoRa
        LoRa_send();

        delay(1000);
        
        // Check for downlink messages
        if (!modem.available()) {
            Serial.println("No downlink message received at this time.");
            return;
        }

        char rcv[64];
        int i = 0;
        while (modem.available()) {
            rcv[i++] = (char)modem.read(); 
        }

        Serial.print("Received: ");
        for (unsigned int j = 0; j < i; j++) {
            Serial.print(rcv[j] >> 4, HEX);
            Serial.print(rcv[j] & 0xF, HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
    delay(1);
}


//function to send information over LoRa network
void LoRa_send() {
    modem.beginPacket();  //creates a LoRa packet
    modem.write(destination);  //destination address
    modem.print("LAT: ");
    modem.print(latitude);
    modem.print(" LONG: ");
    modem.print(longitude);
    modem.print(" ALT: ");
    modem.print(altitude);
    modem.print(" SPEED: ");
    modem.print(speed);
    modem.print(" SAT: ");
    modem.print(satellites);
    modem.endPacket(); //sends the LoRa packet
    delay(10000); //a 10 second delay to limit the amount of packets sent

  // Check if the message was send correctly
  int err;
  err = modem.endPacket(true);
  if (err > 0) {
      Serial.println("Message sent correctly!");
  } else {
    Serial.println("Error sending message!");
  }

  delay(60000); //a 60 second delay to limit the amount of packets sent
}

//function that prints all readings in the Serial Monitor
void printValues() {
  Serial.print("Location: ");
  Serial.print(latitude, 7);
  Serial.print(", ");
  Serial.print(longitude, 7);
  Serial.print("; Altitude: ");
  Serial.print(altitude);
  Serial.print(" m; ");
  Serial.print("Ground speed: ");
  Serial.print(speed);
  Serial.print(" km/h; ");
  Serial.print("Number of satellites: ");
  Serial.print(satellites);
  Serial.println();
}

void printHexMsg(){
  // Clear the message string
  msg = "";

  // Convert each GPS value to string and concatenate them
  msg += String(latitude, 7);
  msg += String(longitude, 7);
  msg += String(altitude);
  msg += String(speed);
  msg += String(satellites);

  for (unsigned int i = 0; i < msg.length(); i++) {
    Serial.print(msg[i] >> 4, HEX);
    Serial.print(msg[i] & 0xF, HEX);
    Serial.print(" ");
  }
  Serial.println();
}
