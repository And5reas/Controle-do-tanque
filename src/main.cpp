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

WiFiClient espClient;
PubSubClient client(espClient);

static unsigned long lastMsg = 0;

// Função para publicar um JSON
void sendJson() {
  JsonDocument doc;  // Cria um JSON de até 200 bytes
  
  // Simula dados de sensores
  float temperatura = 25.6;
  float umidade = 60.4;

  doc["temperatura"] = temperatura;
  doc["umidade"] = umidade;

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

  ConectWifi();

  // Configurar o cliente MQTT
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);

  reconnect();
}

void loop() {
  client.loop();
  // Publica uma mensagem a cada 5 segundos
  if ((millis() - lastMsg) > 5000) {
    lastMsg = millis();
    sendJson();
  }
}