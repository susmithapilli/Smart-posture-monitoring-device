#include <BTAddress.h>
#include <BTAdvertisedDevice.h>
#include <BTScan.h>
#include <BluetoothSerial.h>

#include <MPU6050.h>

#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BluetoothSerial.h"

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// MPU6050
MPU6050 mpu;

// Bluetooth
BluetoothSerial SerialBT;

// Buzzer pin
const int buzzerPin = 13;

String lastMessage = "";

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Init MPU6050
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connected!");
  } else {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed!");
    while (1);
  }
  display.clearDisplay();
  display.display();

  // Init Bluetooth
  SerialBT.begin("ESP32_Posture_Monitor");
  Serial.println("Bluetooth ready!");

  // Buzzer pin setup
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;

  float pitch = atan2(ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180 / PI;

  String message;
  if (pitch < 75) {
    message = "Wrong Position";
    digitalWrite(buzzerPin, LOW);
  } else {
    message = "Good Position";
    digitalWrite(buzzerPin, HIGH);
  }

  // Display on Serial
  Serial.print("Pitch: ");
  Serial.print(pitch);
  Serial.print("° → ");
  Serial.println(message);

  // Display on OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.println(message);
  display.display();

  // Send over Bluetooth if message changed
  if (message != lastMessage) {
    SerialBT.println(message);
    lastMessage = message;
  }

  delay(500);
}