#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wi-Fi Credentials
const char* ssid = "WIFI_SSD";
const char* password = "Password";

// Adafruit MQTT Credentials
const char* mqtt_server = "io.adafruit.com";
const char* aio_username = "Username";
const char* aio_key = "Key";
const char* aio_feed_turbidity = "feed";  

WiFiClient espClient;
PubSubClient client(espClient);

// ADC Parameters
#define ADC_PIN A0  // ESP8266 ADC Pin
#define LOW_VAL 0
#define HIGH_VAL 1024
#define LED_PIN 4

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Connect to MQTT
  client.setServer(mqtt_server, 1883);
  connectMQTT();
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,LOW);
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP8266_Client", aio_username, aio_key)) {
      Serial.println("Connected!");
    } else {
      Serial.print("Failed. Error: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Read ADC Value
  int raw = analogRead(ADC_PIN);
  // Clamp Raw Value
  raw = constrain(raw, LOW_VAL, HIGH_VAL);
  float voltage = (raw / 1023.0) * 5;  // Convert to voltage


  // Print Raw Value
  Serial.print("Raw ADC Value: ");
  Serial.print(raw);
  Serial.print(" | Voltage: ");
  Serial.print(voltage);
  Serial.println("V");

  // Convert to Percentage
  int percentage = map(raw, LOW_VAL, HIGH_VAL, 0, 100);
  percentage = constrain(percentage, 0, 100);

  // Print Percentage
  Serial.print("Processed Percentage: ");
  Serial.print(percentage);
  Serial.println("%");


  if(percentage>=55) digitalWrite(LED_PIN,HIGH);
  else digitalWrite(LED_PIN,LOW);
  // Publish to MQTT
  char payload[10];
  sprintf(payload, "%d", percentage);

  if (client.publish(aio_feed_turbidity, payload)) {
    Serial.println("MQTT Publish Success!");
  } else {
    Serial.println("MQTT Publish Failed!");
  }

  delay(5000);  // Delay before next reading
}
