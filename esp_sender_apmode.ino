#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1

// Digital pin connected to the DHT sensor
#define DHTPIN 15  

// Digital pin connected to the moisture sensor
#define sensorPower 26
#define sensorPin 27

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

float temper;
float humid;
int plant;

// ----------------------------------------------------------------------------
// WiFi handling
// ----------------------------------------------------------------------------

char* WIFI_AP_SSID = "esp32";
char* WIFI_AP_PASS = "12345678";

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int id;
    float temp;
    float hum;
    int mois;
    int readingId;
} struct_message;

//Create a struct_message called myData
struct_message myData;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

unsigned int readingId = 0;

void connect_esp32_wifi_network(char* ssid, char* password) {
  Serial.println("------------------------------------------------------------");
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Ok\nConnected!");
  Serial.print("IP Address: ");
  Serial.print(WiFi.localIP());
  Serial.print("; MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("------------------------------------------------------------\n");
  Serial.print("WiFi channel: ");
  Serial.println(WiFi.channel());
}

// ----------------------------------------------------------------------------
// ESP-NOW handling
// ----------------------------------------------------------------------------

constexpr uint8_t ESP_NOW_RECEIVER[] = { 0x30, 0xC6, 0xF7, 0x2A, 0x0F, 0xCC};
esp_now_peer_info_t peerInfo;
bool channelFound = false;
uint8_t channel = 1;

void initEspNow() {
  if (esp_now_init() != ESP_OK) {
      Serial.println("ESP NOW failed to initialize");
      while (1);
  }

  memcpy(peerInfo.peer_addr, ESP_NOW_RECEIVER, 6);
  // peerInfo.ifidx   = ESP_IF_WIFI_STA;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("ESP NOW pairing failure");
      while (1);
  }
  
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  
  esp_now_register_send_cb(onDataSent);
}


// ----------------------------------------------------------------------------
// Offsets wifi channel by 1 when called
// ----------------------------------------------------------------------------
void tryNextChannel() {
  Serial.println("Changing channel from " + String(channel) + " to " + String(channel+1));
  channel = channel % 13 + 1;
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
}

// ----------------------------------------------------------------------------
// Callback function for when data is sent  
// ----------------------------------------------------------------------------
void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  if (!channelFound && status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery Fail because channel" + String(channel) + " does not match receiver channel.");
    tryNextChannel(); // If message was not delivered, it tries on another wifi channel.
  }
  else {
    Serial.println("Delivery Successful ! Using channel : " + String(channel));
    channelFound = true;
  }
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);
  connect_esp32_wifi_network(WIFI_AP_SSID, WIFI_AP_PASS);
  esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.printDiag(Serial);
  initEspNow();
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

uint32_t last;

void loop() {
//  if (millis() - last > 2000) {
//      uint8_t data = random(1, 256);
//      esp_now_send(ESP_NOW_RECEIVER, (uint8_t *) &data, sizeof(uint8_t));
//      Serial.printf("sent: %3u on channel: %u\n", data, WiFi.channel());
//      last = millis();
//  }
    
 // Read DHT22 module values
  temper = dht.readTemperature();
  delay(10);
  humid = dht.readHumidity();
  delay(10);
 
  Serial.print("Temp: ");
  Serial.println(temper);
  Serial.print("Humid: ");
  Serial.println(humid);

  //get the reading from the function below and print it
  int val = readSensor();
  Serial.print("Digital Output: ");
  Serial.println(val);

  // Determine status of our soil moisture situation
  if (val) {
    plant = 1; 
    Serial.println("Status: Soil is too dry - time to water!");
  } else {
    plant = 0;
    Serial.println("Status: Soil moisture is perfect");
  }


  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    //Set values to send
    myData.id = BOARD_ID;
    myData.temp = temper;
    myData.hum = humid;
    myData.mois = plant;
    myData.readingId = readingId++;
     
    //Send message via ESP-NOW
    esp_now_send(ESP_NOW_RECEIVER, (uint8_t *) &myData, sizeof(myData));
  }

  delay(2000);
}

//  This function returns the analog soil moisture measurement
int readSensor() {
  digitalWrite(sensorPower, HIGH);  // Turn the sensor ON
  delay(10);              // Allow power to settle
  int val = digitalRead(sensorPin); // Read the analog value form sensor
  digitalWrite(sensorPower, LOW);   // Turn the sensor OFF
  return val;             // Return analog moisture value
}
