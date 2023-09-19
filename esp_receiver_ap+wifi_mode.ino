// ----------------------------------------------------------------------------
// ESP-NOW network + WiFi gateway to the Internet
// ----------------------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>

// ----------------------------------------------------------------------------
// WiFi handling
// ----------------------------------------------------------------------------

char* WIFI_AP_SSID = "esp32";       //REPLACE_WITH_YOUR_AP_SSID
char* WIFI_AP_PASS = "12345678";    //REPLACE_WITH_YOUR_AP_PASSWORD

char* WIFI_SSID = "********";       //REPLACE_WITH_YOUR_AP_SSID
char* WIFI_PASS = "********";       //REPLACE_WITH_YOUR_AP_PASSWORD

// To check the parameters passed on the URL 
const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

// Structure example to receive data Must match the sender structure
typedef struct struct_message {
  int id;
  float temp;
  float hum;
  int mois;
  unsigned int readingId;
} struct_message;

struct_message incomingReadings; // incomingReadings is variable for struct_message

JSONVar board; //Create a JSON variable called board

AsyncWebServer server(80);
AsyncEventSource events("/events");

void initWiFi() {
  WiFi.mode(WIFI_MODE_APSTA);
  esp_wifi_set_ps(WIFI_PS_NONE); // Otherwise wifi card goes to sleep if inactive
  connect_esp32_wifi_network(WIFI_SSID, WIFI_PASS);
  WiFi.printDiag(Serial);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS); // Start local access point
}

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

void onReceive(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  Serial.printf("received: %3u from %02x:%02x:%02x:%02x:%02x:%02x\n",
    (uint8_t) *incomingData,
    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]
  );

  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  board["id"] = incomingReadings.id;
  board["temperature"] = incomingReadings.temp;
  board["humidity"] = incomingReadings.hum;
  board["moisture"] = incomingReadings.mois;
  board["readingId"] = String(incomingReadings.readingId);
  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());
  
  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);
  Serial.printf("t value: %4.2f \n", incomingReadings.temp);
  Serial.printf("h value: %4.2f \n", incomingReadings.hum);
  Serial.printf("m value: %d \n", incomingReadings.mois);
  Serial.printf("readingID value: %d \n", incomingReadings.readingId);
  Serial.println();
} // You can use boolean variables to trigger wifi-related events in the loop function following onReceive() call.

void initEspNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP NOW failed to initialize");
        while (1);
    }
    esp_now_register_recv_cb(onReceive);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>SMART AGRI DASHBOARD</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://kit.fontawesome.com/da7dfccddb.js" crossorigin="anonymous"></script>
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 3rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
    .packets { color: #b08d3c; }
    .card.temperature { color: #fd7e14; }
    .card.humidity { color: #1b78e2; }
    .card.moisture { color: #3cb019; }
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
 </style>
</head>
<body>
  <div class="topnav">
    <h3>SMART AGRI DASHBOARD</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #1 - TEMPERATURE</h4>
        <p><span class="reading"><span id="t1"></span> &deg;C</span></p>
        <p class="packet">Reading ID: <span id="rt1"></span></p>
        <p class="packets">Time: <span id="ttime1"></span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> BOARD #1 - HUMIDITY</h4>
        <p><span class="reading"><span id="h1"></span> &percnt;</span></p>
        <p class="packet">Reading ID: <span id="rh1"></span></p>
        <p class="packets">Time: <span id="htime1"></span></p>
      </div>
      <div class="card moisture">
        <h4><i class="fas fa-plant-wilt"></i> BOARD #1 - SOIL MOISTURE</h4>
        <p><span class="reading"><span id="m1"></span></span></p>
        <p>0 - Soil moisture is perfect</p>
        <p>1 - Soil is too dry - time to water!</p>
        <p class="packet">Reading ID: <span id="rm1"></span></p>
        <p class="packets">Time: <span id="mtime1"></span></p>
      </div>
      <div class="card buttons">
        <h4><i class="fas fa-power-off"></i> BOARD #1 - MOTOR PUMP ON/OFF </h4>
        <label class="switch">
        <input type="checkbox" onchange="toggleCheckbox(this)" id="2" checked>
        <span class="slider"></span>
        </label>
      </div>
    </div>
  </div>
<script>
function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if (element.checked) {
    xhr.open("GET", "/update?output=2&state=1", true);
  } else {
    xhr.open("GET", "/update?output=2&state=0", true);
  }
  xhr.send();
}

if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('new_readings', function(e) {
  console.log("new_readings", e.data);

  var date = new Date();
  var current_date = date.getFullYear()+"-"+(date.getMonth()+1)+"-"+ date.getDate();
  var current_time = date.getHours()+":"+date.getMinutes()+":"+ date.getSeconds();
  var date_time = current_date+" "+current_time;
  
  var obj = JSON.parse(e.data);
  document.getElementById("t"+obj.id).innerHTML = obj.temperature.toFixed(2);
  document.getElementById("h"+obj.id).innerHTML = obj.humidity.toFixed(2);
  document.getElementById("m"+obj.id).innerHTML = obj.moisture;
  document.getElementById("rt"+obj.id).innerHTML = obj.readingId;
  document.getElementById("rh"+obj.id).innerHTML = obj.readingId;
  document.getElementById("rm"+obj.id).innerHTML = obj.readingId;
  document.getElementById("ttime"+obj.id).innerHTML = date_time;
  document.getElementById("htime"+obj.id).innerHTML = date_time;
  document.getElementById("mtime"+obj.id).innerHTML = date_time;
 }, false);
}
</script>
</body>
</html>)rawliteral";

String outputState(int pin){
  if(digitalRead(pin)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}


// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(500);
    initWiFi();
    initEspNow();

    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

// Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    Serial.print("GPIO: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  });

  server.addHandler(&events);
  server.begin();    
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
 static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping",NULL,millis());
    lastEventTime = millis();
  }
} 
