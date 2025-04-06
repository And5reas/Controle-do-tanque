#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "senhaWifi.h"

// Estabelecendo conexão com a internet via Wi-Fi
const char* ssid = ssidSecret;
const char* password = passwordSecret;

// Configuração do MQTT
const char* mqttServer = "test.mosquitto.org";
const char topico[36] = "projeto/tanque/216590";

const int trigPin = 5;
const int echoPin = 18;
const int bombaPin = 33;
const int solenoidePin = 32;
const int statusOnlinePin = 2;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034

WiFiClient espClient;
PubSubClient client(espClient);

static unsigned long lastMsg = 0;

long duration;
float distanceCm;
float funPorcentagem;
bool automatico;

void calc() {
  funPorcentagem = ((45/11)*distanceCm)-(250/11);

  if (funPorcentagem > 85 && automatico) {
    digitalWrite(bombaPin, HIGH);
    digitalWrite(solenoidePin, LOW);
  } else
  if (funPorcentagem < 15 && automatico) {
    digitalWrite(bombaPin, LOW);
  }
}

// Função para publicar um JSON
void sendJson() {
  // JsonDocument doc;  // Cria um JSON de até 200 bytes
  
  // doc["nivel"] = distanceCm;

  // char jsonBuffer[512];
  // serializeJson(doc, jsonBuffer); // Converte JSON para string

  char stringNivel[10]; 
  sprintf(stringNivel, "%.0f", funPorcentagem);
  client.publish(topico, stringNivel);
  
  //Serial.println("JSON enviado:");
  //Serial.println(jsonBuffer
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Serial.print("Mensagem recebida no tópico: ");
  // Serial.println(topic);

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  // Serial.print("Mensagem: " + message);

  if (message == "toggleBomba" && !automatico) {
    if (digitalRead(bombaPin)) {
      digitalWrite(bombaPin, LOW);
    } else {
      digitalWrite(bombaPin, HIGH);
    }
  }

  if (message == "toggleSolenoide" && !automatico) {
    if (digitalRead(solenoidePin)) {
      digitalWrite(solenoidePin, LOW);
    } else {
      digitalWrite(solenoidePin, HIGH);
    }
  }

  if (message == "toggleAuto") {
    automatico = !automatico;
  }

  // Serial.println();
  // Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    digitalWrite(2, LOW);
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

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(bombaPin, OUTPUT);
  pinMode(solenoidePin, OUTPUT);
  pinMode(statusOnlinePin, OUTPUT);

  automatico = true;

  ConectWifi();

  // Configurar o cliente MQTT
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);

  reconnect();
}

void loop() {
  client.loop();

  if (client.connected()) {
    digitalWrite(2, HIGH);
  }

  reconnect();

  // Publica uma mensagem a cada 7 segundos
  if ((millis() - lastMsg) > 3000) {
    lastMsg = millis();
    hcsr04();
    calc();
    // Serial.println("  |  " + (String)distanceCm);
    if (client.connected()) {
      sendJson();
    } else {
      digitalWrite(2, LOW);
    }
  }
}