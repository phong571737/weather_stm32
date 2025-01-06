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

const char* ssidList[] = { "UIT Public", "Coke", "Free wifi" };
const char* passwordList[] = { "", "92797980", "12345678" };

// Cấu hình NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600);  // GMT+7 cho Việt Nam

unsigned long lastTime = 0;
unsigned long timerDelay = 60000000;  // 1 hour in milliseconds
int lastHour = -1;                   // To track the last hour we requested data for
SoftwareSerial mySerial = SoftwareSerial(UART_RX, UART_TX);

String jsonBuffer;

struct data_rc {
  int nhietdoHT[7][24];
  int ngay[7];
  int nhietdoMax[7];
  int nhietdoMin[7];
  int doam[7][24];
  double rain[7][24];
  int gioHT;
  int sendNumber;
  int flagSend;
};

struct city {
  float lati;
  float loni;
  int NumberPro;
  int flagReceive;
};

city data_recieve;
data_rc datagh;

String latitude;
String longtitude;
int NumberProvince;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mySerial.begin(9600, SWSERIAL_8N1);

  // Thử kết nối lần lượt các mạng Wi-Fi
  for (int i = 0; i < 3; i++) {
    WiFi.begin(ssidList[i], passwordList[i]);
    Serial.println("Connecting");

    int timeout = 10;  // Thời gian thử kết nối tối đa (giây)
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
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

void Read_UART() {
  mySerial.read((uint8_t*)&data_recieve, sizeof(city));
  latitude = String(data_recieve.lati);
  longtitude = String(data_recieve.loni);
  NumberProvince = data_recieve.NumberPro;
}

void loop() {
  if (mySerial.available()) {
    Read_UART();
    Serial.println(latitude);
    Serial.println(longtitude);
    Serial.println(data_recieve.flagReceive);
    delay(100);
  }
  timeClient.update();  // Cập nhật thời gian từ NTP
  // Lấy thứ trong tuần
  int dayOfWeek = timeClient.getDay();

  // Send an HTTP GET request
  unsigned long currentMillis = millis();
  int currentHour = hour();  // Get the current hour
  if ((millis() - lastTime) > timerDelay || currentHour != lastHour) {
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      
      String serverPath = "https://api.open-meteo.com/v1/forecast?latitude=" + latitude + "&longitude=" + longtitude + "&current=temperature_2m&hourly=temperature_2m,relative_humidity_2m,rain&daily=temperature_2m_max,temperature_2m_min&timezone=Asia%2FBangkok";

      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);

      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      if(data_recieve.flagReceive == 1){
        if(NumberProvince == 1) 
          datagh.sendNumber = 1;
        else if(NumberProvince == 2){
          datagh.sendNumber = 2;
        }
        else if(NumberProvince == 3){
          datagh.sendNumber = 3;
        }
        else{
          datagh.sendNumber = 4;
          datagh.flagSend = 0;
        }
          String Current = String(myObject["current"]["time"]);
          int HourCurrent = Current.substring(11, 13).toInt();
          datagh.gioHT = HourCurrent;
          for (int i = 0; i < 7; i++) {
            int currentday = (dayOfWeek + i) % 7;
            for (int j = HourCurrent; j < 24; j++) {
              datagh.ngay[i] = currentday;
              datagh.nhietdoMax[i] = (int)((double)myObject["daily"]["temperature_2m_max"][i]);
              datagh.nhietdoMin[i] = (int)((double)myObject["daily"]["temperature_2m_min"][i]);
              datagh.doam[i][j] = (int)((double)myObject["hourly"]["relative_humidity_2m"][j]);
              datagh.nhietdoHT[i][j] = (int)((double)myObject["hourly"]["temperature_2m"][j]);
              datagh.rain[i][j] = ((double)myObject["hourly"]["rain"][j]);
              datagh.flagSend = 1;
              Serial.print(datagh.flagSend);
              Serial.print(datagh.gioHT);
              Serial.print(datagh.ngay[i]);
              Serial.print(datagh.nhietdoHT[i][j]);
              Serial.print(datagh.nhietdoMax[i]);  // Print data to Serial monitor
              Serial.print(datagh.nhietdoMin[i]);
              Serial.print(datagh.doam[i][j]);
              Serial.print(datagh.rain[i][j]);
              Serial.print("\n");
            }
          }
          Serial.print(datagh.sendNumber);
          if(NumberProvince == 1){
            Serial.println("\nData sent HCM to STM32");    
            mySerial.write((uint8_t*)&datagh, sizeof(data_rc));  // Send data to STM32 via UART 
            delay(2000);
           // millis();
          }
          else if( NumberProvince == 2){
            Serial.println("\nData sent Hue to STM32");  
            mySerial.write((uint8_t*)&datagh, sizeof(data_rc));  // Send data to STM32 via UART 
            delay(2000);
           // millis();
          }
          else if( NumberProvince == 3){
            Serial.println("\nData sent HaNoi to STM32");
            mySerial.write((uint8_t*)&datagh, sizeof(data_rc));  // Send data to STM32 via UART 
          }
      }
      else {
        // Nếu flagReceive == 0, ngừng gửi dữ liệu và xóa bộ đệm
        Serial.println("FlagReceive is 0, no data sent.");
        memset(&datagh, 0, sizeof(datagh)); // Xóa bộ đệm dữ liệu
      }
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
    //delay(2000); 
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

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
