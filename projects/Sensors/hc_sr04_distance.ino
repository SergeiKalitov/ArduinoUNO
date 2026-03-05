// HC-SR04 Ultrasonic Sensor + UNO R4 WiFi LED Matrix output (inches)
//
// Wiring (matches this sketch):
//   HC-SR04 VCC  -> 5V
//   HC-SR04 GND  -> GND
//   HC-SR04 TRIG -> D9
//   HC-SR04 ECHO -> D10
//
// What this sketch does:
//  1) Measures distance with HC-SR04
//  2) Prints distance to Serial Monitor
//  3) Shows distance IN INCHES on the built-in 12x8 LED matrix (UNO R4)
//
// Notes:
//  - The LED matrix is small, so we show a short string like "12.3in".
//  - If the number is long, it will scroll.

#include <Arduino.h>

// Built-in LED matrix library for UNO R4 boards
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

const uint8_t TRIG_PIN = 9;
const uint8_t ECHO_PIN = 10;

const unsigned long PULSE_TIMEOUT_US = 30000; // ~5 meters max
const unsigned long SAMPLE_DELAY_MS = 100;

// -------- Distance measurement --------
float readDistanceCm() {
  // Ensure clean trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // 10us trigger pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo pulse width (HIGH)
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT_US);
  if (duration == 0) {
    return -1.0f; // Timeout (no echo)
  }

  // Speed of sound: 343 m/s => 0.0343 cm/us
  // Divide by 2 because pulse travels to object and back
  return (duration * 0.0343f) / 2.0f;
}

// Median filter (helps ignore random spikes)
float medianOf3(float a, float b, float c) {
  if (a > b) {
    float t = a; a = b; b = t;
  }
  if (b > c) {
    float t = b; b = c; c = t;
  }
  if (a > b) {
    float t = a; a = b; b = t;
  }
  return b;
}

// -------- LED Matrix output --------
// Display a short text on the built-in matrix.
// We use the matrix text API (scrolls automatically).
void showOnMatrix(const char *text) {
  // Clear previous frame/text
  matrix.clear();

  // Start drawing text at (x=0, y=1). The last argument is brightness/color (library-specific).
  // Many examples use 0xFFFFFF; if your core expects a different type, keep this as-is.
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(text);

  // If the text is wider than 12px it will scroll left.
  // If it fits, it will just show it briefly.
  matrix.endText(SCROLL_LEFT);
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  Serial.begin(9600);
  Serial.println("HC-SR04 distance monitor started");

  // Initialize built-in LED matrix (UNO R4)
  matrix.begin();

  // Small boot message
  showOnMatrix("in");
}

void loop() {
  // Take 3 readings and use the median to reduce noise
  float d1 = readDistanceCm();
  delay(20);
  float d2 = readDistanceCm();
  delay(20);
  float d3 = readDistanceCm();

  if (d1 < 0 || d2 < 0 || d3 < 0) {
    Serial.println("No echo (out of range or wiring issue)");
    showOnMatrix("----");
  } else {
    float distanceCm = medianOf3(d1, d2, d3);
    float distanceIn = distanceCm / 2.54f;

    // Serial output (kept for debugging)
    Serial.print("Distance: ");
    Serial.print(distanceCm, 1);
    Serial.print(" cm  |  ");
    Serial.print(distanceIn, 1);
    Serial.println(" in");

    // Prepare short text for matrix (in inches)
    // Example: "12.3in"
    char buf[12];
    snprintf(buf, sizeof(buf), "%.1fin", distanceIn);

    // Show on matrix
    showOnMatrix(buf);
  }

  delay(SAMPLE_DELAY_MS);
}
