/**
  * Version: SCX ESP32 C3 Mini Lap Counter
  *
  * Creator: Carlos MC
  *
  * Project instructions: 
    - none
  *  
  * GitHub Project repository:
    - https://github.com/Peyutron/SH1106-DCCpp-Wifi-controller
  * GitHub repository:
    - https://github.com/Peyutron
  * Web:
    - https://www.infotronikblog.com
  *
  * External Library:
    - LiquidCrystal_I2C:  https://github.com/markub3327/LiquidCrystal_I2C
    - ToneESP32:          By Larry Bernstone 1.0.0 (Use the Library Manager)
  *
  * ESP32 Library:
    - WiFi:         https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFi.h
    - WebServer:    https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClient/WiFiClient.ino
  * 
  * 
  * Display -> I2C LCD 16x02   
  * Board -> ESP32C3 Dev Module
  * Use CDC on boot "enabled"
  * CPU Frequency "80MHz (wifi)"
  * Board Version 2.0.18
  *
  * NOTE!!: If you're having Wi-Fi connection problems, 
  * please check the antenna on your ESP32C2 Mini module.
  * Many problems have been reported due to ESP32C3 mini 
  * boards with a faulty antenna component. 
**/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ToneESP32.h>  // For tone() en ESP32-C3
#include "web_server.h"

// ===========
// WIFI CONFIG
// ===========
const char* ssid = "MOVISTAR_D310";           // Change to your SSID
const char* password = "puPdDU4tXafYmjcpaoGo";   // your Wifi Password
// char serverIP[16] = "192.168.1.2";
// const int serverPort = 2560;

// =================================
// PIN CONFIGURATION (ESP32-C3 Mini)
// =================================
const int SENSOR_PIN = 3;     // GPIO2 - IR Sensor
const int BUZZER_PIN = 5;     // GPIO3 - Buzzer
const int START_BUTTON = 9;   // GPIO9 - BOOT button (Start)
const int RESET_BUTTON = 10;  // GPIO10 - Reset (opcional)

// =====================
// I2C LCD CONFIGURATION
// =====================
const int I2C_SDA = 6;
const int I2C_SCL = 7;
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Try 0x3F if that doesn't work

// =======================
// CONFIGURATION VARIABLES
// =======================
int countdownSeconds = 3;   // Countdown time (3 or 5 seconds)
int totalLaps = 3;          // Number of laps in the race
bool firstDetection = true;
bool showUpdateLap = false;

// ============
// SYSTEM STATE
// ============
enum SystemState 
{ 
  STATE_IDLE, 
  STATE_COUNTDOWN, 
  STATE_RACING, 
  STATE_LAP_FINISHED, 
  STATE_RACE_FINISHED 
};
SystemState state = STATE_IDLE;

// ==============
// TIME VARIABLES
// ==============
volatile unsigned long startTime = 0;
unsigned long lapTimes[100];     // Stores up to 100 laps
int currentLap = 0;
unsigned long countdownStart = 0;
int countdownCounter = 0;
float bestLap = 999999.0;
unsigned long lastDetectionTime = 0;
const unsigned long DEBOUNCE_US = 1000000;  // 1 seg

// Volatile variables for interruption
volatile bool sensorTriggered = false;
volatile unsigned long sensorTimestamp = 0;

// ==========
// WEB SERVER
// ==========
WebServer server(80);

// ===================
// FUNCTION PROTOTYPES
// ===================
void playSoundReset();
void playSoundCountdownStart();
void playSoundCountdownTick();
void playSoundRaceStart();
void playSoundLap();
void playSoundRaceFinish();
void resetRace();
void startRace();
void finishLap();
void updateLCDReady();
void updateLCDCountdown(int seconds);
void updateLCDRacing();
void updateLCDRacingTime(unsigned long elapsedMs);
void updateLCDLapComplete(int lap, int total, float lapTime, float best);
void updateLCDRaceFinished();
void handleRoot();
void handleData();
void handleStart();
void handleReset();
void handleConfig();

// ===================
// WEB SERVER MANAGERS
// ===================
void handleRoot() 
{
  server.send(200, "text/html", webpage);
}

void handleData() 
{
  String json = "{\"state\":\"";
  switch(state) {
    case STATE_IDLE: json += "IDLE"; break;
    case STATE_COUNTDOWN: json += "COUNTDOWN"; break;
    case STATE_RACING: json += "RACING"; break;
    case STATE_LAP_FINISHED: json += "LAP FINISHED"; break;
    case STATE_RACE_FINISHED: json += "RACE FINISHED"; break;
  }
  json += "\",\"laps\":[";
  
  for(int i = 0; i < totalLaps && lapTimes[i] > 0; i++) {
    if(i > 0) json += ",";
    json += String(lapTimes[i] / 1000000.0, 3);
  }
  json += "],\"best\":";
  if(bestLap < 999999.0) json += String(bestLap, 3);
  else json += "0";
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleStart() 
{
  startRace();
  server.send(200, "text/plain", "OK");
}

void handleReset() 
{
  resetRace();
  server.send(200, "text/plain", "OK");
}

void handleConfig() 
{
  if(server.hasArg("cd")) 
  {
    countdownSeconds = server.arg("cd").toInt();
    if(countdownSeconds < 1) countdownSeconds = 1;
    if(countdownSeconds > 10) countdownSeconds = 10;
  }
  if(server.hasArg("laps")) 
  {
    totalLaps = server.arg("laps").toInt();
    if(totalLaps < 1) totalLaps = 1;
    if(totalLaps > 30) totalLaps = 30;
  }
  resetRace();
  server.send(200, "text/plain", "OK");
}

// ====================
//  SENSOR INTERRUPTION
// ====================
void IRAM_ATTR sensorISR() 
{
  if (state != STATE_RACING) return;
  
  unsigned long now = micros();
  if ((now - lastDetectionTime) > DEBOUNCE_US) 
  {
    lastDetectionTime = now;
    sensorTriggered = true;
    sensorTimestamp = now;
  }
}

// =====
// SETUP
// =====
void setup() 
{
  Serial.begin(115200);
  
  // Configure pins
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(START_BUTTON, INPUT_PULLUP);
  if (RESET_BUTTON >= 0) pinMode(RESET_BUTTON, INPUT_PULLUP);
  delay(1000);
  Serial.println(F("Iniciando LCD"));
  
  // Configure LCD I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();
  
  // Configure sensor interrupt
  // attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), sensorISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), sensorISR, RISING);
  
  // Welcome message on LCD
  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print(F("Contador Vueltas"));
  lcd.setCursor(0, 1);
  lcd.print("v3.0");
  delay(1500);
  
  // Connect to WiFi
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");
  
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) 
  {
    delay(500);
    Serial.print(F("."));
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) 
  {
    Serial.println(F("\nWiFi conectado"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Web:");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
    delay(2000);
  } 
  else 
  {
    Serial.println(F("\nError WiFi"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error WiFi!");
    lcd.setCursor(0, 1);
    lcd.print("Revisa config");
    delay(3000);
  }
  
  // Configure web server
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/start", handleStart);
  server.on("/reset", handleReset);
  server.on("/config", handleConfig);
  server.begin();
  
  updateLCDReady();
  
  Serial.print(F("Servidor web en: http://"));
  Serial.println(WiFi.localIP());
  Serial.print(F("Total vueltas: "));
  Serial.println(totalLaps);
  Serial.print(F("Cuenta atrás: "));
  Serial.println(countdownSeconds);
}

// =========
// MAIN LOOP
// =========
void loop() 
{
  // Handle web requests
  server.handleClient();
  ReadButtons();
  // State machine
  switch (state) 
  {
    case STATE_COUNTDOWN: 
    {
      unsigned long now = millis();
      int remaining = countdownSeconds - ((now - countdownStart) / 1000);
      if (remaining != countdownCounter && remaining >= 0) 
      {
        countdownCounter = remaining;
        updateLCDCountdown(countdownCounter);
        if (countdownCounter > 0) playSoundCountdownTick();
      }
      if (now - countdownStart >= (countdownSeconds * 1000)) 
      {
        state = STATE_RACING;
        startTime = micros();
        playSoundRaceStart();
        updateLCDRacing();
        Serial.println(F("Carrera iniciada!"));
      }
      break;
    }
    
    case STATE_RACING: 
    {
      if (sensorTriggered) 
      {
        sensorTriggered = false;
        finishLap();
      }
      unsigned long elapsed = (micros() - startTime) / 1000;  // in milliseconds
      updateLCDRacingTime(elapsed);
      break;
    }
    
    default:
      break;
  }
  delay(10);
}