#include <Arduino.h>

// Ultrasonic sensor pins
const int TRIG_PIN = 4;
const int ECHO_PIN = 5;

void setupUltrasonic() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

float measureDistanceCm() {
  // Ensure trigger is low
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send a 10 µs pulse to trigger
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure duration of echo HIGH pulse (in microseconds)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000UL); 
  // 30,000 µs = 30 ms timeout ≈ max reliable distance ~5 meters

  if (duration == 0) {
    // No echo received (timeout)
    return -1.0;
  }

  // Convert time to distance:
  // Sound speed ≈ 0.0343 cm/µs, but since pulse travels to target and back:
  // distance = (duration * 0.0343) / 2
  float distance_cm = (duration * 0.0343f) / 2.0f;

  return distance_cm;
}

void setup() {
  Serial.begin(115200);
  setupUltrasonic();
}

void loop() {
  float d = measureDistanceCm();
  if (d < 0) {
    Serial.println("No echo detected.");
  } else {
    Serial.print("Distance: ");
    Serial.print(d);
    Serial.println(" cm");
  }

  delay(200);
}
