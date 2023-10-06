#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <SPI.h>
#include <Adafruit_MotorShield.h>
#include <Arduino.h>
#include "WiFiCredentials.h"
#include <iostream>
#include <sstream>

#define LED 13

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define STOP 0

const char *ssid = SSID;
const char *password = PASSWORD;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *L_MOTOR = AFMS.getMotor(1);
Adafruit_DCMotor *R_MOTOR = AFMS.getMotor(2);

void initFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }
}

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println(".");
  }
  Serial.println(WiFi.localIP());
}

void stopMotors()
{
  L_MOTOR->setSpeed(0);
  L_MOTOR->run(RELEASE);

  R_MOTOR->setSpeed(0);
  R_MOTOR->run(RELEASE);

  digitalWrite(LED, HIGH);
  delay(300);
  digitalWrite(LED, LOW);
  delay(300);
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
}

void rampMotorSpeed()
{
  int var = 0;
  while (var < 65)
  {
    L_MOTOR->setSpeed(var);
    R_MOTOR->setSpeed(var);
    var++;
  }
}

void moveCar(int inputValue)
{
  Serial.printf("Got value as %d\n", inputValue);

  // if (!(horizontalScreen))
  // {
  switch (inputValue)
  {

  case UP:
    R_MOTOR->run(FORWARD);
    L_MOTOR->run(FORWARD);
    rampMotorSpeed();
    break;

  case DOWN:
    R_MOTOR->run(BACKWARD);
    L_MOTOR->run(BACKWARD);
    rampMotorSpeed();
    break;

  case LEFT:
    R_MOTOR->run(FORWARD);
    L_MOTOR->run(BACKWARD);
    rampMotorSpeed();
    break;

  case RIGHT:
    R_MOTOR->run(BACKWARD);
    L_MOTOR->run(FORWARD);
    rampMotorSpeed();
    break;

  case STOP:
    L_MOTOR->setSpeed(0);
    R_MOTOR->setSpeed(0);
    break;

  default:
    stopMotors();
    break;
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    std::string myData = "";
    myData.assign((char *)data, len);
    std::istringstream ss(myData);
    std::string key, value;
    std::getline(ss, key, ',');
    std::getline(ss, value, ',');
    Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str());
    int valueInt = atoi(value.c_str());
    if (key == "MoveCar")
    {
      moveCar(valueInt);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  initFS();
  initWiFi();
  AFMS.begin();
  stopMotors();

  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  server.serveStatic("/", SPIFFS, "/");

  server.begin();
  digitalWrite(LED, LOW);
}

void loop()
{
  ws.cleanupClients();
}
