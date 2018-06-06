#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
//include "esp_deep_sleep.h"

const char* ssid = "default";
const char* password =  "";
const char* mqttServer = "";
const int mqttPort =  [];
const char* mqttUser = "";
const char* mqttPassword = "";

bool greenIn = 0;
bool redIn = 0;
const char* inpMsg;

#define GREEN_PIN 25
#define RED_PIN 26

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  Serial.begin(115200);
  //  esp_sleep_enable_ext0_wakeup(RED_PIN, 1);
  //  esp_err_t esp_deep_sleep_enable_timer_wakeup(uint64_t 600000000)

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  pinMode(LED_BUILTIN, OUTPUT);
  
//  Serial.println("Entering Deep Sleep");
//  esp_deep_sleep_start();
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String runTest;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    runTest += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/output") {
    Serial.print("Waiting for command ");
    if (runTest == "on") {
      Serial.println("on");
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (runTest == "off") {
      Serial.println("off");
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT Broker...");
    if (client.connect("ConvexLens", mqttUser, mqttPassword )) {
      Serial.println("connected");
      client.subscribe("esp/iadtMediaCube" );
    } else {
      Serial.print("failed, failState=");
      Serial.print(client.state());
      Serial.println(" retrying");
      delay(2000);
    }
  }
}

void digInRead() {
  greenIn = digitalRead(GREEN_PIN);
  redIn = digitalRead(RED_PIN);
  Serial.print("Green LED: ");
  Serial.println(greenIn);
  Serial.print("Red LED: ");
  Serial.println(redIn);
  if (greenIn == HIGH) {
    inpMsg = "Luminaire UPS Charging - GREEN";
  }
  else if (redIn == HIGH) {
    inpMsg = "UPS Not Charging - RED";
  }
  else {
    inpMsg = "Power Out - LED OFF";
  }
  client.publish("esp/iadtMediaCube", inpMsg);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 60000) {
    lastMsg = now;

    digInRead();
  }
}
