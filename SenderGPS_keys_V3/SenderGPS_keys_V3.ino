#include <MKRWAN.h>
#include "arduino_secrets.h"
#include <SPI.h>
#include <Arduino_MKRGPS.h>

float latitudeSum = 0;
float longitudeSum = 0;
float altitudeSum = 0;
float speedSum = 0;
float satellitesSum = 0;
int readingsCount = 0;

float latitude;
float longitude;
float altitude;
float speed;
float satellites;

int CountGPSMessage = 0;
int CountLoRaMessage = 1;

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
  
  Serial.println("Esperando para obtener señal GPS...");
  delay(60000); // Esperar 1 minuto para obtener señal GPS
}

void loop() {
  // check if there is new GPS data available
  if (GPS.available()) {

      CountGPSMessage++;

      latitudeSum = 0;
      longitudeSum = 0;
      altitudeSum = 0;
      speedSum = 0;
      satellitesSum = 0;
      readingsCount = 0;

      while (readingsCount < 5) {
        latitudeSum += GPS.latitude();
        longitudeSum += GPS.longitude();
        altitudeSum += GPS.altitude();
        speedSum += GPS.speed();
        satellitesSum += GPS.satellites();
        readingsCount++;
        delay(1000); // Delay between readings
      }

      readingsCount = 0;

      latitude = latitudeSum / 5;
      longitude = longitudeSum / 5;
      altitude = altitudeSum / 5;
      speed = speedSum / 5;
      satellites = satellitesSum / 5;

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

      delay(60000);
      
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
  
  else{
    Serial.println("Sin señal GPS");
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
  modem.print(CountGPSMessage);
  modem.print(" ");
  modem.print(CountLoRaMessage);

  // Check if the message was send correctly
  int err;
  err = modem.endPacket(true);
  if (err > 0) {
      Serial.println("Message sent correctly!");
      CountLoRaMessage++;
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
  Serial.print(" Contador GPS: ");
  Serial.print(speed);
  Serial.print(" Contador LoRa: ");
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
