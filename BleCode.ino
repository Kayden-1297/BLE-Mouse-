#include <BleMouse.h>
#include <Wire.h>

BleMouse bleMouse;

// ------- FLEX SENSOR PINS -------
const int FLEX_INDEX_PIN  = 34;   
const int FLEX_MIDDLE_PIN = 35;   

// ------- AUTO-CALIBRATED FLEX RANGE -------
int idxMin = 9999, idxMax = 0;
int midMin = 9999, midMax = 0;

bool calibDone = false;
unsigned long calibStart = 0;
const int CALIB_TIME = 3000;      // 3 seconds auto-calibration

// Flex thresholds (percentage %)
const int CLICK_ON  = 60;
const int CLICK_OFF = 40;

// ------- MOUSE GESTURE TIMINGS -------
const uint32_t DRAG_HOLD_MS = 250;
const uint32_t COMBO_WINDOW_MS = 250;

// ------- MPU6050 -------
const uint8_t MPU_ADDR = 0x68;

float gyroXoffset = 0, gyroYoffset = 0;
float pitch = 0, roll = 0;
unsigned long lastIMUms = 0;

const float sensX = 2.0;
const float sensY = 2.0;
const float deadDeg = 0.6;
const int maxStep = 15;

const float scrollPitchThresh = 4.0;
const int scrollAmount = 1;

bool idxBent = false, midBent = false;
bool wasIdxBent = false, wasMidBent = false;

bool dragging = false;
uint32_t idxBentSince = 0;
uint32_t lastBothBent = 0;


// ---------- FUNCTIONS -----------

int readFlexPct(int pin, int mn, int mx) {
  int raw = analogRead(pin);
  raw = constrain(raw, mn, mx);
  return map(raw, mn, mx, 0, 100);
}

void mpuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(true);
}

void readMPU(int16_t &ax, int16_t &ay, int16_t &az,
             int16_t &gx, int16_t &gy, int16_t &gz) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();
  gx = (Wire.read() << 8) | Wire.read();
  gy = (Wire.read() << 8) | Wire.read();
  gz = (Wire.read() << 8) | Wire.read();
}

void calibrateGyro(int samples = 500) {
  long gxSum = 0, gySum = 0;

  for (int i = 0; i < samples; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    readMPU(ax, ay, az, gx, gy, gz);
    gxSum += gx;
    gySum += gy;
    delay(2);
  }

  gyroXoffset = gxSum / (float)samples;
  gyroYoffset = gySum / (float)samples;
}


// ---------- SETUP -----------

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);

  // Start flex calibration
  calibStart = millis();
  Serial.println("Hold fingers straight for 3 sec…");

  // MPU INIT
  Wire.begin(21, 22);
  delay(50);

  mpuWrite(0x6B, 0x00);
  mpuWrite(0x1B, 0x00);
  mpuWrite(0x1C, 0x00);

  calibrateGyro();
  lastIMUms = millis();

  // BLE MOUSE
  bleMouse.begin();
  Serial.println("BLE Mouse ready.");
}


// ---------- LOOP -----------

void loop() {

  // --- AUTO CALIBRATION PHASE ---
  if (!calibDone) {
    int a = analogRead(FLEX_INDEX_PIN);
    int b = analogRead(FLEX_MIDDLE_PIN);

    idxMin = min(idxMin, a);
    idxMax = max(idxMax, a);

    midMin = min(midMin, b);
    midMax = max(midMax, b);

    if (millis() - calibStart >= CALIB_TIME) {
      calibDone = true;
      Serial.println("Flex calibration completed:");

      Serial.printf("Index range:  %d → %d\n", idxMin, idxMax);
      Serial.printf("Middle range: %d → %d\n", midMin, midMax);
      Serial.println("Start bending to control mouse.");
    }
    delay(10);
    return;
  }


  // --- NO BLE CONNECTION ---
  if (!bleMouse.isConnected()) {
    delay(50);
    return;
  }


  // ========== MPU == Cursor Move ==========
  int16_t ax, ay, az, gx, gy, gz;
  readMPU(ax, ay, az, gx, gy, gz);

  unsigned long now = millis();
  float dt = (now - lastIMUms) / 1000.0f;
  if (dt <= 0) dt = 0.01;
  lastIMUms = now;

  float axg = ax / 16384.0;
  float ayg = ay / 16384.0;
  float azg = az / 16384.0;

  float gxds = (gx - gyroXoffset) / 131.0;
  float gyds = (gy - gyroYoffset) / 131.0;

  float pitchAcc = atan2f(-axg, sqrtf(ayg * ayg + azg * azg)) * 180.0 / PI;
  float rollAcc  = atan2f(ayg, azg) * 180.0 / PI;

  const float alpha = 0.98;
  pitch = alpha * (pitch + gyds * dt) + (1 - alpha) * pitchAcc;
  roll  = alpha * (roll  + gxds * dt) + (1 - alpha) * rollAcc;

  float dxf = fabs(roll) < deadDeg ? 0 : (roll - copysignf(deadDeg, roll)) * sensX;
  float dyf = fabs(pitch) < deadDeg ? 0 : (pitch - copysignf(deadDeg, pitch)) * sensY;

  int dx = constrain((int)dxf, -maxStep, maxStep);
  int dy = constrain((int)dyf, -maxStep, maxStep);

  if (dx || dy) bleMouse.move(dx, dy);


  // ========== FLEX SENSOR PERCENTAGE ==========
  int idxPct = readFlexPct(FLEX_INDEX_PIN, idxMin, idxMax);
  int midPct = readFlexPct(FLEX_MIDDLE_PIN, midMin, midMax);

  bool idxRise = (!idxBent && idxPct >= CLICK_ON);
  bool midRise = (!midBent && midPct >= CLICK_ON);

  if (idxPct >= CLICK_ON) idxBent = true;
  if (idxPct <= CLICK_OFF) idxBent = false;

  if (midPct >= CLICK_ON) midBent = true;
  if (midPct <= CLICK_OFF) midBent = false;


  // ========== DOUBLE CLICK ==========
  if ((idxRise && midBent) || (midRise && idxBent)) {
    if (now - lastBothBent <= COMBO_WINDOW_MS) {
      bleMouse.click(MOUSE_LEFT);
      bleMouse.click(MOUSE_LEFT);
      lastBothBent = 0;
    } else {
      lastBothBent = now;
    }
  }


  // ========== DRAG ==========
  if (idxRise) { /* wait */ }

  if (!dragging && idxBent && (now - idxBentSince >= DRAG_HOLD_MS)) {
    bleMouse.press(MOUSE_LEFT);
    dragging = true;
  }

  if (dragging && !idxBent) {
    bleMouse.release(MOUSE_LEFT);
    dragging = false;
  }

  if (!dragging && !idxBent && wasIdxBent) {
    if (now - idxBentSince < DRAG_HOLD_MS)
      bleMouse.click(MOUSE_LEFT);
  }


  // ========== RIGHT CLICK ==========
  if (midRise)
    bleMouse.click(MOUSE_RIGHT);


  // ========== SCROLL ==========
  if (midBent) {
    if (pitch >  scrollPitchThresh) bleMouse.move(0, 0, -scrollAmount);
    if (pitch < -scrollPitchThresh) bleMouse.move(0, 0,  scrollAmount);
  }


  wasIdxBent = idxBent;
  wasMidBent = midBent;

  delay(10);
}