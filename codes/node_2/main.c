#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL3C-TXmwXy"
#define BLYNK_TEMPLATE_NAME "CPS"
#define BLYNK_AUTH_TOKEN "REPLACE_WITH_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

char ssid[] = "REPLACE_WITH_WIFI_SSID";
char pass[] = "REPLACE_WITH_WIFI_PASSWORD";

// ── Motor Pins ─────────────────────────────────────────
#define IN1 26
#define IN2 27
#define IN3 14
#define IN4 12
#define ENA 25
#define ENB 33

// ✅ No channel defines needed — v3.x assigns them automatically

// ── Ultrasonic ─────────────────────────────────────────
#define TRIG_PIN 5
#define ECHO_PIN 4

// ── Servo ──────────────────────────────────────────────
#define SERVO_PIN 18

// ── Settings ───────────────────────────────────────────
#define SPEED        120
#define TURN_SPEED   200
#define DEADZONE      30
#define OBSTACLE_CM   25
#define PING_INTERVAL 60

// ── State ──────────────────────────────────────────────
int  joyX = 128, joyY = 128;
long          distanceCM   = 999;
unsigned long lastPingTime = 0;

Servo myServo;

// ── Motor PWM ──────────────────────────────────────────
// ✅ v3.x: ledcWrite takes PIN, not channel
void setMotorPWM(int a, int b) {
  ledcWrite(ENA, a);
  ledcWrite(ENB, b);
}

// ── Ultrasonic ─────────────────────────────────────────
long readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 15000);
  return (dur == 0) ? 999 : dur * 0.034 / 2;
}

// ── Motor Control ──────────────────────────────────────
void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  setMotorPWM(0, 0);
}

void moveForward() {
  long d = readDistance();
  distanceCM = d;
  if (d < OBSTACLE_CM) {
    stopMotors();
    Serial.println("BLOCKED");
    return;
  }
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  setMotorPWM(SPEED, SPEED);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  setMotorPWM(SPEED, SPEED);
}

void turnClockwise() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  setMotorPWM(TURN_SPEED, TURN_SPEED);
}

void turnAntiClockwise() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  setMotorPWM(TURN_SPEED, TURN_SPEED);
}

// ── Joystick Processing ────────────────────────────────
void processJoystick() {
  int xVal = joyX - 128;
  int yVal = joyY - 128;

  if (abs(xVal) <= DEADZONE && abs(yVal) <= DEADZONE) {
    stopMotors(); return;
  }

  if (abs(yVal) >= abs(xVal)) {
    if (yVal > 0) moveForward();
    else          moveBackward();
  } else {
    if (xVal > 0) turnClockwise();
    else          turnAntiClockwise();
  }
}

// ── Blynk Callbacks ───────────────────────────────────
BLYNK_WRITE(V4) { joyX = param.asInt(); processJoystick(); }
BLYNK_WRITE(V5) { joyY = param.asInt(); processJoystick(); }

BLYNK_WRITE(V6) {
  int angle = constrain(atoi(param.asStr()), 0, 180);
  myServo.write(angle);
  Serial.print("Servo → "); Serial.print(angle); Serial.println("°");
}

// ── Setup ──────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // ✅ v3.x: single function replaces ledcSetup + ledcAttachPin
  ledcAttach(ENA, 1000, 8);   // pin, freq, resolution
  ledcAttach(ENB, 1000, 8);
  stopMotors();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Servo — allocate timers not used by LEDC motor channels
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400);
  myServo.write(90);
  Serial.println("Servo centred at 90°");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

// ── Loop ───────────────────────────────────────────────
void loop() {
  Blynk.run();

  unsigned long now = millis();
  if (now - lastPingTime >= PING_INTERVAL) {
    lastPingTime = now;
    distanceCM   = readDistance();
    Serial.print("Dist: "); Serial.print(distanceCM); Serial.println(" cm");

    if (distanceCM < OBSTACLE_CM) {
      int yVal = joyY - 128, xVal = joyX - 128;
      if (abs(yVal) >= abs(xVal) && yVal > DEADZONE) {
        stopMotors();
        Serial.println("AUTO-STOP");
      }
    }
  }
}
