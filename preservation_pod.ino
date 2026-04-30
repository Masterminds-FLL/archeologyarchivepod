#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_VEML7700.h>
#include <LiquidCrystal_I2C.h>

// -------------------- Pins --------------------
#define FAN_PIN 33
#define SD_CS_PIN 5

// ESP32 default I2C pins
#define SDA_PIN 21
#define SCL_PIN 22

// -------------------- Settings --------------------
const float FAN_ON_TEMP_C  = 22.5;
const float FAN_OFF_TEMP_C = 21.5;

const float FAN_ON_HUMIDITY  = 60.0;
const float FAN_OFF_HUMIDITY = 55.0;

const unsigned long SENSOR_INTERVAL_MS = 2000;
const unsigned long LOG_INTERVAL_MS = 10000;

// -------------------- Devices --------------------
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_VEML7700 veml = Adafruit_VEML7700();
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4);

// -------------------- State --------------------
bool fanOn = false;
bool sdReady = false;
bool rtcReady = false;
bool lightReady = false;

unsigned long lastSensorRead = 0;
unsigned long lastLogWrite = 0;
unsigned long startMillis = 0;

float tempC = 0;
float humidity = 0;
float lux = 0;

File logFile;

// -------------------- Helpers --------------------
float cToF(float c) {
  return (c * 9.0 / 5.0) + 32.0;
}

String twoDigits(int n) {
  if (n < 10) return "0" + String(n);
  return String(n);
}

String timestamp() {
  if (rtcReady) {
    DateTime now = rtc.now();
    return String(now.year()) + "-" +
           twoDigits(now.month()) + "-" +
           twoDigits(now.day()) + " " +
           twoDigits(now.hour()) + ":" +
           twoDigits(now.minute()) + ":" +
           twoDigits(now.second());
  }

  unsigned long seconds = millis() / 1000;
  return "uptime_" + String(seconds) + "s";
}

String runtimeString() {
  unsigned long seconds = (millis() - startMillis) / 1000;
  unsigned long hours = seconds / 3600;
  unsigned long minutes = (seconds % 3600) / 60;
  unsigned long secs = seconds % 60;

  return String(hours) + "h " + String(minutes) + "m " + String(secs) + "s";
}

void setFan(bool on) {
  fanOn = on;
  digitalWrite(FAN_PIN, fanOn ? HIGH : LOW);
}

void updateFanControl() {
  if (!fanOn && (tempC >= FAN_ON_TEMP_C || humidity >= FAN_ON_HUMIDITY)) {
    setFan(true);
  }

  if (fanOn && (tempC <= FAN_OFF_TEMP_C && humidity <= FAN_OFF_HUMIDITY)) {
    setFan(false);
  }
}

void readSensors() {
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (!isnan(t)) tempC = t;
  if (!isnan(h)) humidity = h;

  if (lightReady) {
    lux = veml.readLux();
  }
}

void updateDisplay() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(cToF(tempC), 1);
  lcd.print("F");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity, 1);
  lcd.print("%");

  lcd.setCursor(0, 2);
  lcd.print("Light: ");
  lcd.print(lux, 0);
  lcd.print(" lx");

  lcd.setCursor(0, 3);
  lcd.print(fanOn ? "Fan: ON " : "Fan: OFF");
  lcd.print(" SD:");
  lcd.print(sdReady ? "OK" : "NO");
}

void writeLogHeaderIfNeeded() {
  if (!sdReady) return;

  if (!SD.exists("/pod_log.csv")) {
    logFile = SD.open("/pod_log.csv", FILE_WRITE);
    if (logFile) {
      logFile.println("timestamp,runtime,temp_c,temp_f,humidity_percent,light_lux,fan_on");
      logFile.close();
    }
  }
}

void logData() {
  if (!sdReady) return;

  logFile = SD.open("/pod_log.csv", FILE_APPEND);

  if (logFile) {
    logFile.print(timestamp());
    logFile.print(",");
    logFile.print(runtimeString());
    logFile.print(",");
    logFile.print(tempC, 2);
    logFile.print(",");
    logFile.print(cToF(tempC), 2);
    logFile.print(",");
    logFile.print(humidity, 2);
    logFile.print(",");
    logFile.print(lux, 2);
    logFile.print(",");
    logFile.println(fanOn ? "ON" : "OFF");
    logFile.close();
  } else {
    sdReady = false;
  }
}

void printSerialStatus() {
  Serial.println("----- Preservation Pod -----");
  Serial.print("Time: "); Serial.println(timestamp());
  Serial.print("Runtime: "); Serial.println(runtimeString());
  Serial.print("Temp C: "); Serial.println(tempC);
  Serial.print("Temp F: "); Serial.println(cToF(tempC));
  Serial.print("Humidity: "); Serial.println(humidity);
  Serial.print("Light Lux: "); Serial.println(lux);
  Serial.print("Fan: "); Serial.println(fanOn ? "ON" : "OFF");
  Serial.print("SD: "); Serial.println(sdReady ? "OK" : "NOT READY");
  Serial.println("----------------------------");
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  startMillis = millis();

  pinMode(FAN_PIN, OUTPUT);
  setFan(false);

  Wire.begin(SDA_PIN, SCL_PIN);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Preservation Pod");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  if (!sht31.begin(0x44)) {
    lcd.setCursor(0, 2);
    lcd.print("SHT31 missing");
    Serial.println("ERROR: SHT31 not found.");
  } else {
    Serial.println("SHT31 ready.");
  }

  if (veml.begin()) {
    lightReady = true;
    veml.setGain(VEML7700_GAIN_1);
    veml.setIntegrationTime(VEML7700_IT_100MS);
    Serial.println("VEML7700 ready.");
  } else {
    lightReady = false;
    Serial.println("WARNING: VEML7700 not found.");
  }

  if (rtc.begin()) {
    rtcReady = true;
    Serial.println("RTC ready.");

    if (rtc.lostPower()) {
      Serial.println("RTC lost power. Setting to compile time.");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  } else {
    rtcReady = false;
    Serial.println("WARNING: RTC not found.");
  }

  if (SD.begin(SD_CS_PIN)) {
    sdReady = true;
    writeLogHeaderIfNeeded();
    Serial.println("SD card ready.");
  } else {
    sdReady = false;
    Serial.println("WARNING: SD card not ready.");
  }

  delay(1500);
}

// -------------------- Loop --------------------
void loop() {
  unsigned long now = millis();

  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;

    readSensors();
    updateFanControl();
    updateDisplay();
    printSerialStatus();
  }

  if (now - lastLogWrite >= LOG_INTERVAL_MS) {
    lastLogWrite = now;
    logData();
  }
}
