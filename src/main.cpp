#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <SPI.h>
#include <Adafruit_MotorShield.h>
#include <Arduino.h>

#define LED 13

const char *ssid = "";
const char *password = "";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *L_MOTOR = AFMS.getMotor(1);
Adafruit_DCMotor *R_MOTOR = AFMS.getMotor(2);

String message = "";
String sliderValue1 = "0";
String sliderValue2 = "0";
u_int8_t leftMotorDirection;
u_int8_t rightMotorDirection;

int dutyCycle1;
int dutyCycle2;

bool turboMode = false;

JSONVar sliderValues;

String getSliderValues()
{
  sliderValues["sliderValue1"] = String(sliderValue1);
  sliderValues["sliderValue2"] = String(sliderValue2);

  String jsonString = JSON.stringify(sliderValues);
  return jsonString;
}

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

void notifyClients(String sliderValues)
{
  ws.textAll(sliderValues);
}

void stopMotors()
{
  leftMotorDirection = RELEASE;
  rightMotorDirection = RELEASE;
  dutyCycle1 = 0;
  dutyCycle2 = 0;
  sliderValue1 = "0";
  sliderValue2 = "0";
  L_MOTOR->setSpeed(dutyCycle1);
  L_MOTOR->run(leftMotorDirection);

  R_MOTOR->setSpeed(dutyCycle2);
  R_MOTOR->run(rightMotorDirection);

  notifyClients(getSliderValues());
  digitalWrite(LED, HIGH);
  delay(300);
  digitalWrite(LED, LOW);
  delay(300);
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    message = (char *)data;
    Serial.println(message);
    if (message.indexOf("turbotrue") >= 0)
    {
      turboMode = true;
      dutyCycle1 = map(abs(sliderValue1.toInt()), 0, 100, 0, 255);
      dutyCycle2 = map(abs(sliderValue2.toInt()), 0, 100, 0, 255);
    }
    else if (message.indexOf("turbofalse") >= 0)
    {
      turboMode = false;
      dutyCycle1 = map(abs(sliderValue1.toInt()), 0, 100, 0, 127);
      dutyCycle2 = map(abs(sliderValue2.toInt()), 0, 100, 0, 127);
    }
    if (message.indexOf("stopMotors") >= 0)
    {
      stopMotors();
    }
    if (message.indexOf("1s") >= 0)
    {
      sliderValue1 = message.substring(2);
      if (sliderValue1.toInt() < 0)
      {
        leftMotorDirection = BACKWARD;
      }
      else if (sliderValue1.toInt() > 0)
      {
        leftMotorDirection = FORWARD;
      }
      else
      {
        leftMotorDirection = RELEASE;
      }
      if (turboMode)
      {
        dutyCycle1 = map(abs(sliderValue1.toInt()), 0, 100, 0, 255);
      }
      else
      {
        dutyCycle1 = map(abs(sliderValue1.toInt()), 0, 100, 0, 127);
      }
      Serial.println(dutyCycle1);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (message.indexOf("2s") >= 0)
    {
      sliderValue2 = message.substring(2);
      if (sliderValue2.toInt() < 0)
      {
        rightMotorDirection = BACKWARD;
      }
      else if (sliderValue2.toInt() > 0)
      {
        rightMotorDirection = FORWARD;
      }
      else
      {
        rightMotorDirection = RELEASE;
      }
      if (turboMode)
      {
        dutyCycle2 = map(abs(sliderValue2.toInt()), 0, 100, 0, 255);
      }
      else
      {
        dutyCycle2 = map(abs(sliderValue2.toInt()), 0, 100, 0, 127);
      }
      Serial.println(dutyCycle2);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (strcmp((char *)data, "getValues") == 0)
    {
      notifyClients(getSliderValues());
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
  L_MOTOR->setSpeed(dutyCycle1);
  L_MOTOR->run(leftMotorDirection);

  R_MOTOR->setSpeed(dutyCycle2);
  R_MOTOR->run(rightMotorDirection);

  ws.cleanupClients();
}