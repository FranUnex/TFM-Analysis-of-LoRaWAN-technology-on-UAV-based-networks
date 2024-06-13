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
  // put your setup code here, to run once:115200
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)

  Serial1.begin(9600);

  while (!Serial);
    // change this to your regional band (eg. US915, AS923, ...)
    if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  };

    // If We want to use the pins, we need to insert GPS_MODE_SHIELD in the GPS begin
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
  
  Serial.println("Esperando para obtener señal GPS...");
}

void loop() {

  if (GPS.available()) {

      latitude = GPS.latitude();
      longitude = GPS.longitude();
      altitude = GPS.altitude();
      speed = GPS.speed();
      satellites = GPS.satellites();



      Serial.println("Nuevo mensaje GPS");

      for (int i=0;i<10;i++){
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
        digitalWrite(LED_BUILTIN, HIGH);
      }
      
      // Print the GPS information
      printValues();
  
      // Print the hexadecimal payload message
      printHexMsg();
      
      // Send the message over LoRa
      LoRa_send();

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
    
}


//function to send information over LoRa network
void LoRa_send() {
  modem.beginPacket();  //creates a LoRa packet
  modem.print(latitude, 7);
  modem.print(" ");
  modem.print(longitude, 7);
  modem.print(" ");
  modem.print(altitude);
  modem.print(" ");
  modem.print(speed);
  modem.print(" ");
  modem.print(satellites);
  modem.print(" ");

  // Check if the message was send correctly
  int err;
  err = modem.endPacket(true);
  if (err > 0) {
      Serial.println("Message sent correctly!");
  } else {
    Serial.println("Error sending message!");
  }

  delay(30000); //a 30 second delay to limit the amount of packets sent
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
