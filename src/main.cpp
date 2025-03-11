#include <WiFi.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "senhaWifi.h"

// Configuração do IP estático:
IPAddress local_IP(192, 168, 146, 184);
IPAddress gateway(192, 168, 146, 1);        // Geralmente o IP do seu roteador
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);            // Opcional: servidor DNS primário
IPAddress secondaryDNS(8, 8, 4, 4);          // Opcional: servidor DNS secundário

// Estabelecendo conexão com a internet via Wi-Fi
const char* ssid = ssidSecret;
const char* password = passwordSecret;

// Criando AsyncWebServer object na porta 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Função para tratar eventos do WebSocket (conexão, mensagem, etc.)
void onWebSocketEvent(AsyncWebSocket *server, 
  AsyncWebSocketClient *client, 
  AwsEventType type, 
  void *arg, 
  uint8_t *data, 
  size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("Cliente conectado ao WebSocket");
  } else if(type == WS_EVT_DISCONNECT) {
    Serial.println("Cliente desconectado do WebSocket");
  } else if(type == WS_EVT_DATA) {
    // Aqui você pode processar dados recebidos do cliente, se necessário
    Serial.println("Dados recebidos no WebSocket");
  }
}

// Função para enviar dados para todos os clientes conectados via WebSocket
void broadcastData() {
  // Exemplo: cria um objeto JSON com uma leitura fictícia
  JsonDocument doc;
  doc["sensor"] = "temperatura";
  doc["valor"] = random(20, 30); // Valor fictício
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  ws.textAll(jsonString);  // Envia a string JSON para todos os clientes conectados
}

void ConectWifi() {
  // Define o IP estático
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Falha na configuração do IP estático");
  }

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

  // Configura o endpoint REST (exemplo: retorna status em JSON)
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    doc["status"] = "ok";
    doc["uptime"] = millis() / 1000;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Configura o WebSocket
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  
  // Inicia o servidor
  server.begin();
}

void loop() {
  // Envie dados via WebSocket periodicamente (ex: a cada 5 segundos)
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 5000) {
    broadcastData();
    lastTime = millis();
  }
}