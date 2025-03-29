#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "senhaWifi.h"

// Estabelecendo conexão com a internet via Wi-Fi
const char* ssid = ssidSecret;
const char* password = passwordSecret;

// Configuração do MQTT
const char* mqttServer = "test.mosquitto.org";
const char topico[36] = "projeto/tanque/216590/215446/214707";

const int trigPin = 5;
const int echoPin = 18;
const int bombaPin = 33;
const int solenoidePin = 32;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034

WiFiClient espClient;
PubSubClient client(espClient);

static unsigned long lastMsg = 0;

long duration;
float distanceCm;

// Função para publicar um JSON
void sendJson() {
  JsonDocument doc;  // Cria um JSON de até 200 bytes
  
  doc["nivel"] = distanceCm;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // Converte JSON para string

  client.publish(topico, jsonBuffer);  // Envia JSON via MQTT
  Serial.println("JSON enviado:");
  Serial.println(jsonBuffer);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  Serial.print("Mensagem: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect("andreassss")) {  // Nome do cliente MQTT
      Serial.println("Conectado!");
      client.subscribe(topico);  // Inscreve-se em um tópico
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void ConectWifi() {

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(bombaPin, OUTPUT);
  pinMode(solenoidePin, OUTPUT);

  ConectWifi();

  // Configurar o cliente MQTT
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);

  reconnect();
}

void hcsr04() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;

  delay(200);
}

void loop() {
  client.loop();
  // Publica uma mensagem a cada 5 segundos
  if ((millis() - lastMsg) > 5000) {
    lastMsg = millis();
    hcsr04();
    sendJson();
  }

  if (distanceCm < 10) {
    digitalWrite(bombaPin, HIGH);
  } else {
    digitalWrite(bombaPin, LOW);
  }
}