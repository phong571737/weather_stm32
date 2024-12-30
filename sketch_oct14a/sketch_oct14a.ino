#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>  // Include WiFiClientSecure for HTTPS
#include <TimeLib.h>
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define UART_TX 17
#define UART_RX 16

const char* ssidList[] = {"UIT Public", "Coke", "Free wifi"};
const char* passwordList[] = {"", "92797980", "12345678"};

// Cấu hình NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600); // GMT+7 cho Việt Nam

unsigned long lastTime = 0;
unsigned long timerDelay = 3600000;  // 1 hour in milliseconds
int lastHour = -1;  // To track the last hour we requested data for
SoftwareSerial mySerial = SoftwareSerial(UART_RX, UART_TX);

String jsonBuffer;

struct data_rc 
{
  int ngay[7];
	int nhietdoMax[7];
  int nhietdoMin[7];
	int doam[7];
};
struct city
{ 
  float lati;
  float loni;
};

city data_recieve;
data_rc datagh;

String latitude;
String longtitude;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mySerial.begin(9600, SWSERIAL_8N1);

  // Thử kết nối lần lượt các mạng Wi-Fi
  for(int i = 0;i < 3;i++){
    WiFi.begin(ssidList[i], passwordList[i]);
    Serial.println("Connecting");
    
    int timeout = 10; // Thời gian thử kết nối tối đa (giây)
    while(WiFi.status() != WL_CONNECTED && timeout > 0) {
      delay(500);
      Serial.print(".");
      timeout--;
    }
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Connected to WiFi SSID: ");
  Serial.println(WiFi.SSID());
}

void Read_UART()
{
  mySerial.read((uint8_t*)&data_recieve, sizeof(city));
  latitude = String(data_recieve.lati); 
  longtitude = String(data_recieve.loni);
}

void loop() {
  if(mySerial.available())  
  {
    Read_UART();
    Serial.println(latitude);
    Serial.println(longtitude);
    delay(1000);
  }
  timeClient.update(); // Cập nhật thời gian từ NTP
  // Lấy thứ trong tuần
  int dayOfWeek = timeClient.getDay();

  // Send an HTTP GET request
  unsigned long currentMillis = millis();
  int currentHour = hour(); // Get the current hour
  if ((millis() - lastTime) > timerDelay || currentHour != lastHour) {
    // Check WiFi connection status
    if(WiFi.status() == WL_CONNECTED){
      String serverPath = "https://api.open-meteo.com/v1/forecast?latitude=" + latitude + "&longitude=" + longtitude + "&hourly=temperature_2m,relative_humidity_2m&daily=temperature_2m_max,temperature_2m_min&timezone=Asia%2FBangkok";
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      for (int i = 0; i < 7; i++) {
        int currentday = (dayOfWeek + i) % 7;
        datagh.ngay[i] = currentday;
        datagh.nhietdoMax[i] = (int)((double)myObject["daily"]["temperature_2m_max"][i]);
        datagh.nhietdoMin[i] = (int)((double)myObject["daily"]["temperature_2m_min"][i]);
        datagh.doam[i] = (int)((double)myObject["hourly"]["relative_humidity_2m"][i]);   
        Serial.print(datagh.ngay[i]); 
        Serial.print(datagh.nhietdoMax[i]);  // Print data to Serial monitor
        Serial.print(datagh.nhietdoMin[i]);
        Serial.print(datagh.doam[i]);  
        mySerial.write((uint8_t*)&datagh, sizeof(data_rc));  // Send data to STM32 via UART                         
      }
      delay(600);
    }
      //mySerial.write((uint8_t*)&datagh, sizeof(data_rc));  // Send data to STM32 via UART   
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClientSecure client;  // Use WiFiClientSecure for HTTPS
  HTTPClient http;

  client.setInsecure();
    
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
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;

}

