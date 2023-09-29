#include <Wire.h>
#include <RTClib.h>  // RTC library for DS1307 and DS3231
#include "PulseSensor.h"

const int accelerometerXPin = A0;
const int accelerometerYPin = A1;
const int accelerometerZPin = A2;

const int rtcSDAPin = A4;
const int rtcSCLPin = A5;

RTC_DS1307 rtc1307;  // DS1307 RTC
RTC_DS3231 rtc3231;  // DS3231 RTC

const int pulseSensorPin = 2;
PulseSensor pulseSensor;

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;

float healthParameter1 = 0.0;
float healthParameter2 = 0.0;
int pillReminder = 0;
bool waterReminder = false;
bool sleepReminder = false;
bool healthAlert = false;
bool taskCompleted = false;
int inGamePoints = 0;

void updateHealthParameters();
void checkReminders();
void checkHealthStatus();
void updateTaskStatus();
void countStepsAndCalories();
bool checkInactivity();

void setup() {
  // Initialize accelerometer
  pinMode(accelerometerXPin, INPUT);
  pinMode(accelerometerYPin, INPUT);
  pinMode(accelerometerZPin, INPUT);

  // Initialize RTC
  Wire.begin();
  rtc1307.begin();
  rtc3231.begin();

  // Initialize Pulse Sensor
  pulseSensor.start();

  // Initialize TFT display
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(3);
  tft.fillScreen(ILI9486_BLACK);
}

void loop() {
  updateHealthParameters();
  checkReminders();
  checkHealthStatus();
  updateTaskStatus();
  countStepsAndCalories();
  checkInactivity();
}

void updateHealthParameters() {
  // Read accelerometer data
  int xAcceleration = analogRead(accelerometerXPin);
  int yAcceleration = analogRead(accelerometerYPin);
  int zAcceleration = analogRead(accelerometerZPin);

  // Read heart rate from Pulse Sensor
  int heartRate = pulseSensor.getBeatsPerMinute();

  // Map the values to health parameters
  healthParameter1 = map(xAcceleration, 0, 1023, 0, 100);
  healthParameter2 = map(yAcceleration, 0, 1023, 0, 100);
  float healthParameter3 = map(heartRate, 60, 180, 0, 100);
}
void checkReminders() {
  if (pillReminder == 1) {
    tft.fillScreen(ILI9486_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(ILI9486_WHITE);
    tft.setTextSize(2);
    tft.println("Reminder: Take pills!");
    delay(5000);
    pillReminder = 0;
  }

  if (waterReminder) {
    tft.fillScreen(ILI9486_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(ILI9486_WHITE);
    tft.setTextSize(2);
    tft.println("Reminder: Drink water!");
    delay(5000);
    waterReminder = false;
  }

  if (sleepReminder) {
    tft.fillScreen(ILI9486_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(ILI9486_WHITE);
    tft.setTextSize(2);
    tft.println("Reminder: Go to sleep!");
    delay(5000);
    sleepReminder = false;
  }
}

void checkHealthStatus() {
    if (healthParameter1 < 30.0) {
    tft.fillScreen(ILI9486_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(ILI9486_WHITE);
    tft.setTextSize(2);
    tft.println("Health Alert: You are not feeling well!");
    delay(5000);
    healthAlert = true;
  }
  if (healthParameter2 > 70.0) {
    tft.fillScreen(ILI9486_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(ILI9486_WHITE);
    tft.setTextSize(2);
    tft.println("Health Alert: You are stressed!");
    delay(5000);
    healthAlert = true;
  }
}

void updateTaskStatus() {
  if (taskCompleted) {
    inGamePoints += 10;
    
    tft.fillScreen(ILI9486_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(ILI9486_WHITE);
    tft.setTextSize(2);
    tft.println("Task Completed!");
    tft.setCursor(10, 40);
    tft.print("In-Game Points: ");
    tft.print(inGamePoints);
    delay(5000);
    
    taskCompleted = false;
  }


  int stepsTaken = countSteps();
  if (stepsTaken > 0) {
    float caloriesBurnt = calculateCaloriesBurnt(stepsTaken);
    tft.fillScreen(ILI9486_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(ILI9486_WHITE);
    tft.setTextSize(2);
    tft.print("Steps Taken: ");
    tft.println(stepsTaken);
    tft.setCursor(10, 40);
    tft.print("Calories Burnt: ");
    tft.print(caloriesBurnt);
    delay(5000);
  }

  if (checkInactivity()) {
    tft.fillScreen(ILI9486_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(ILI9486_WHITE);
    tft.setTextSize(2);
    tft.println("Inactive for 30 mins!");
  }
}

int countSteps() {
  static int prevAccelValue = 0;
  static bool isPeak = false;
  int stepCount = 0;

  int currentAccelValue = analogRead(accelerometerXPin);

  int peakThreshold = 600;

  if (currentAccelValue > peakThreshold && !isPeak) {
    isPeak = true;
  }
  else if (currentAccelValue < peakThreshold && isPeak) {
    isPeak = false;
    stepCount++;
  }
  prevAccelValue = currentAccelValue;

  return stepCount;
}


float calculateCaloriesBurnt(int steps) {
  const float caloriesPerStep = 0.05;
  const float userWeightKg = 70.0;
  
  float caloriesBurnt = steps * caloriesPerStep;

  float bmr = 88.362 + (13.397 * userWeightKg) + (4.799 * 170.0) - (5.677 * 30.0);
  float adjustedCaloriesBurnt = caloriesBurnt * bmr / 24.0;

  return adjustedCaloriesBurnt;
}

bool checkInactivity() {
  static unsigned long lastActivityTime = 0;
  const unsigned long inactivityThreshold = 30 * 60 * 1000;

  unsigned long currentTime = millis();

  if (currentTime - lastActivityTime >= inactivityThreshold) {
    return true;
  } else {
    lastActivityTime = currentTime;
    return false;
  }
}


void countStepsAndCalories() {
    static int prevStepCount = 0;
  static unsigned long lastStepTime = 0;
  const unsigned long stepInterval = 1000;
  const float caloriesPerStep = 0.05;

  int currentStepCount = detectSteps();

  unsigned long currentTime = millis();
  unsigned long timeSinceLastStep = currentTime - lastStepTime;

  if (currentStepCount > prevStepCount && timeSinceLastStep >= stepInterval) {
    prevStepCount = currentStepCount;
    lastStepTime = currentTime;

    float caloriesBurnt = currentStepCount * caloriesPerStep;

    tft.fillScreen(ILI9486_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(ILI9486_WHITE);
    tft.setTextSize(2);
    tft.print("Steps Taken: ");
    tft.println(currentStepCount);
    tft.setCursor(10, 40);
    tft.print("Calories Burnt: ");
    tft.print(caloriesBurnt, 1);
    delay(5000);
  }
}

int detectSteps() {
  int currentAccX = analogRead(accelerometerXPin);
  int currentAccY = analogRead(accelerometerYPin);
  int currentAccZ = analogRead(accelerometerZPin);

  int thresholdX = 500;
  int thresholdY = 500;
  int thresholdZ = 500;

  static bool isStepDetected = false;
  static int stepCount = 0;

  if (!isStepDetected) {
    if (currentAccX > thresholdX || currentAccY > thresholdY || currentAccZ > thresholdZ) {
      isStepDetected = true;
    }
  } else {
    if (currentAccX <= thresholdX && currentAccY <= thresholdY && currentAccZ <= thresholdZ) {
      isStepDetected = false;
      stepCount++;
    }
  }

  return stepCount;
}

bool checkInactivity() {
  static unsigned long lastActivityTime = 0;
  const unsigned long inactivityThreshold = 30 * 60 * 1000;

  unsigned long currentTime = millis();

  if (currentTime - lastActivityTime >= inactivityThreshold) {
    return true;
  } else {
    lastActivityTime = currentTime;
    return false;
  }
}