#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>


const char* ssid = "Vishvak A22";
const char* password = "1342005@";


const String botToken = "7756219083:AAEba_zb1XP0qDQ3uub2EboiiT0CsO6iIUI";
const String chatID = "5598505366";


TinyGPSPlus gps;
HardwareSerial gpsSerial(1);  


Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);


float heavyVibrationThreshold = 10.0; 
const int stableReadingCount = 1;      


void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi!");
}


void sendMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + botToken + "/sendMessage";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"chat_id\":\"" + chatID + "\",\"text\":\"" + message + "\"}";
    Serial.println("Sending payload: " + payload);

    int httpCode = http.POST(payload);
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("Message sent successfully! Response:");
      Serial.println(response);
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(http.errorToString(httpCode));
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void setup() {
  Serial.begin(115200);
  

  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  

 
  connectToWiFi();

  
  if (!accel.begin()) {
    Serial.println("Could not find a valid ADXL345 sensor, check wiring!");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G); 

 
  sendMessage("GPS Tracker with accelerometer is online and ready");

  
  Serial.println("Allowing GPS module to acquire signal...");
  delay(15000); 
}

bool isGPSLocationValid() {
  
  for (int i = 0; i < 300; i++) {
    if (gps.location.isValid()) {
      return true; 
    }
    delay(100); 
  }
  return false; 
}

void loop() {
 
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  
  sensors_event_t event;
  accel.getEvent(&event);

  
  static int heavyVibrationCount = 0;

  
  if (abs(event.acceleration.x) > heavyVibrationThreshold || 
      abs(event.acceleration.y) > heavyVibrationThreshold || 
      abs(event.acceleration.z) > heavyVibrationThreshold) {
    heavyVibrationCount++;
  } else {
    heavyVibrationCount = 0; 
  }

 
  if (heavyVibrationCount >= stableReadingCount) {
    heavyVibrationCount = 0; 

   
    if (isGPSLocationValid()) {
      float latitude = gps.location.lat();
      float longitude = gps.location.lng();
      float altitude = gps.altitude.meters();

      
      String googleMapsLink = "https://maps.google.com/?q=" + String(latitude, 6) + "," + String(longitude, 6);
      
     
      String locationMessage = String("Heavy vibration detected!\nLocation:\n") +
                               "Latitude: " + String(latitude, 6) + 
                               ", Longitude: " + String(longitude, 6) +
                               ", Altitude: " + String(altitude) + " m\n" +
                               "Google Maps: " + googleMapsLink;

     
      sendMessage(locationMessage);
    } else {
      sendMessage("Heavy vibration detected, but GPS location is still unavailable after retries.");
    }

    delay(10000);  
  }

  delay(100);  
}