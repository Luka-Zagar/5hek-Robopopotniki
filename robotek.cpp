#include <Arduino.h>

/* Simple drive-forward function for ESP32 + L298N
   Hardware assumptions (from your wiring):
   - Left motor:  ENA = 25  (PWM)  IN1 = 26  IN2 = 27
   - Right motor: ENB = 14  (PWM)  IN3 = 12  IN4 = 13

   How it works:
   - driveForward(distance_cm, speed_percent) computes a time from distance_cm
     using ms_per_cm (calibration constant).
   - It sets motor direction to forward, applies PWM speed, waits that time,
     then stops motors.

   NOTE: This is an open-loop (time-based) approach. Calibrate ms_per_cm
   by running small distances and adjusting the constant until measured
   travel matches requested distance.
*/

void driveForward(float distance_cm, int speed_percent);
void rotate(float angle_deg, int speed_percent);
void stopMotors();
int speedPercentToPwm(int speedPercent);

const int ENA = 25; // left motor PWM
const int IN1 = 26;
const int IN2 = 27;

const int ENB = 14; // right motor PWM
const int IN3 = 12;
const int IN4 = 13;

// PWM settings for ESP32 ledc
const int PWM_FREQ = 2000;     // 2 kHz PWM frequency
const int PWM_RESOLUTION = 8;  // 8-bit resolution (0-255)
const int PWM_CHANNEL_LEFT  = 0;
const int PWM_CHANNEL_RIGHT = 1;

// Calibration constant: milliseconds to drive per centimeter.
// YOU MUST CALIBRATE THIS for your specific motors/wheels/battery.
// Example starting value (very rough): 120 ms per cm.
// If the car travels too far, reduce this value. If not far enough, increase it.
float ms_per_cm = 80.0;

// Calibration constant: milliseconds per degree of rotation.
// Start with a rough guess and tune it by testing 90° rotations.
// If the robot rotates too much -> decrease
// If rotates too little -> increase

float ms_per_degree = 8.4;  // <-- GOOD STARTING VALUE for small 2-motor robots

// Helper: convert 0-100% speed to 0-255 PWM value
int speedPercentToPwm(int speedPercent) {
  speedPercent = constrain(speedPercent, 0, 100);
  return map(speedPercent, 0, 100, 0, 255);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Drive test starting...");

  // Setup motor direction pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Setup PWM channels and attach to EN pins
  ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(ENA, PWM_CHANNEL_LEFT);

  ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(ENB, PWM_CHANNEL_RIGHT);

  // Ensure motors are stopped initially
  stopMotors();
}

void loop() {
  // Example uses: drive 50 cm at 60% speed, then wait and drive 20 cm at 80%
  //Serial.println("Driving 30 cm at 60% speed...");
  //driveForward(30.0, 60);
  //delay(1000);

  Serial.println("Rotate 90 degrees right...");
  rotate(90, 60);
  delay(1000);

  Serial.println("Rotate 90 degrees right...");
  rotate(-90, 60);
  delay(1000);

  // Don't loop constantly in demo - stop here
  while (true) {
    delay(1000);
  }
}

/* driveForward(distance_cm, speed_percent)
   - distance_cm: requested travel distance in centimeters (float)
   - speed_percent: 0..100 percent of max speed
*/
void driveForward(float distance_cm, int speed_percent) {
  if (distance_cm <= 0.0 || speed_percent <= 0) {
    // nothing to do
    return;
  }

  // Compute how long to run motors (ms) from distance using calibration constant
  unsigned long runTimeMs = (unsigned long) round(distance_cm * ms_per_cm);

  Serial.print("Requested distance (cm): ");
  Serial.print(distance_cm);
  Serial.print("  -> run time (ms): ");
  Serial.print(runTimeMs);
  Serial.print("  speed: ");
  Serial.print(speed_percent);
  Serial.println("%");

  // Set both motors to forward direction
  // For the L298N typical wiring:
  // - forward for a motor could be INx HIGH, INy LOW (depends on wiring; test first)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  // Apply PWM to enable pins
  int pwmVal = speedPercentToPwm(speed_percent);
  ledcWrite(PWM_CHANNEL_LEFT, pwmVal);
  ledcWrite(PWM_CHANNEL_RIGHT, pwmVal);

  // Run for computed time
  unsigned long start = millis();
  while (millis() - start < runTimeMs) {
    // Could add obstacle detection here (HC-SR04) to break early
    delay(5);
  }

  // Stop motors after the run
  stopMotors();
}

/* rotate(angle_deg, speed_percent)
   - angle_deg: positive = turn right, negative = turn left
   - speed_percent: motor power (0–100%)
*/
void rotate(float angle_deg, int speed_percent) {
  if (angle_deg == 0 || speed_percent <= 0) {
    return;
  }

  // Compute time to rotate
  unsigned long runTimeMs = (unsigned long) round(abs(angle_deg) * ms_per_degree);

  Serial.print("Rotating ");
  Serial.print(angle_deg);
  Serial.print(" degrees -> run time (ms): ");
  Serial.print(runTimeMs);
  Serial.print("  speed: ");
  Serial.print(speed_percent);
  Serial.println("%");

  int pwmVal = speedPercentToPwm(speed_percent);

  if (angle_deg > 0) {
    // Turn RIGHT
    // Left motor forward, right motor backward
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

  } else {
    // Turn LEFT
    // Left motor backward, right motor forward
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }

  // Apply PWM
  ledcWrite(PWM_CHANNEL_LEFT, pwmVal);
  ledcWrite(PWM_CHANNEL_RIGHT, pwmVal);

  // Run motors for the required rotation time
  unsigned long start = millis();
  while (millis() - start < runTimeMs) {
    delay(5);
  }

  // Stop after turning
  stopMotors();
}

/* stopMotors() - safely stops both motors (brake)
   To brake, set both inputs LOW (coast) or set IN1=IN2=HIGH (brake) depending on behavior you want.
*/
void stopMotors() {
  // Quick stop by setting PWM to zero and IN pins LOW (coast)
  ledcWrite(PWM_CHANNEL_LEFT, 0);
  ledcWrite(PWM_CHANNEL_RIGHT, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
