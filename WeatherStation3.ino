#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "time.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
Adafruit_PCD8544 display = Adafruit_PCD8544(12,11,3,2,1);

#include "DHT.h"
#define DHTPIN 0     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);


const char* ssid = "hurmee";
const char* password = "133773311337";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;

// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "48a0fe71fbe106ad2887fcf32591906e";


// Replace with your country code and city
String city = "Helsinki";
String countryCode = "FI";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 5 minutes (300000)
unsigned long timerDelay = 300000;
// Set timer to 10 seconds (10000)
//unsigned long timerDelay = 10000;

String jsonBuffer;
String DisplayTime;
float OutsideTemp;
float InsideTemp;


void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");

    //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  display.begin();
  display.setContrast(60);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);

  Serial.println(F("DHTxx test!"));
  dht.begin();
  
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  display.setCursor(4,2);
  display.println(&timeinfo, "%B %d %Y");
  
  display.setCursor(4,12);
  display.println(&timeinfo, "%H:%M:%S");

  display.setCursor(4,21);
  display.println("OutTemp  ");

  display.setCursor(52,21);
  display.println(OutsideTemp);

  display.setCursor(4,30);
  display.println("InTemp  ");

  display.setCursor(48,30);
  display.println(InsideTemp);
  display.display();
  delay(500);
  display.clearDisplay();
}


void loop() {


  // For getting temp from openweater
  // Send an HTTP GET request
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
//      Serial.print("JSON object = ");
//      Serial.println(myObject);
      OutsideTemp = double(myObject["main"]["temp"]) - 272.15;
      Serial.print("Temperature: ");
      Serial.println(OutsideTemp);
//      Serial.print("Pressure: ");
//      Serial.println(myObject["main"]["pressure"]);
//       Serial.print("Humidity: ");
//       Serial.println(myObject["main"]["humidity"]);
//      Serial.print("Wind Speed: ");
//      Serial.println(myObject["wind"]["speed"]);



//  also printing data from dht11 sensors here. 
    InsideTemp = dht.readTemperature();
    Serial.print(F("%  Temperature: "));
    Serial.print(InsideTemp);
    Serial.println(F("°C "));

    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
      Serial.print("Am I here?");
      printLocalTime();

}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
//    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
  

}
