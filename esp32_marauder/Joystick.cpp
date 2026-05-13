#include "Joystick.h"

#ifdef HAS_JOYSTICK

Joystick::Joystick() {
  this->pinX = JOY_X_PIN;
  this->pinY = JOY_Y_PIN;
  this->pinBtn = JOY_BTN_PIN;
  
  this->thresholdLow = JOY_THRESHOLD_LOW;
  this->thresholdHigh = JOY_THRESHOLD_HIGH;
  this->debounceMs = 200; // ms between repeat events
  
  this->xRaw = 2048; // Midpoint (12-bit ADC)
  this->yRaw = 2048;
  this->btnState = false;
  
  this->prevUp = false;
  this->prevDown = false;
  this->prevLeft = false;
  this->prevRight = false;
  this->prevBtn = false;
  
  this->lastUpTime = 0;
  this->lastDownTime = 0;
  this->lastLeftTime = 0;
  this->lastRightTime = 0;
  this->lastBtnTime = 0;
}

void Joystick::begin() {
  // ADC pins are inputs by default
  // The joystick button: INPUT_PULLUP, LOW when pressed
  pinMode(this->pinBtn, INPUT_PULLUP);
  
  // Configure ADC for joystick X and Y
  analogReadResolution(12); // 12-bit resolution (0-4095)
}

void Joystick::update() {
  // Read ADC values (12-bit, 0-4095)
  this->xRaw = analogRead(this->pinX);
  this->yRaw = analogRead(this->pinY);
  
  // Read button state (INPUT_PULLUP: LOW = pressed)
  this->btnState = (digitalRead(this->pinBtn) == LOW);
  
  // Update previous direction states
  this->prevUp = (this->prevUp && (millis() - this->lastUpTime) < this->debounceMs) ? true : false;
  this->prevDown = (this->prevDown && (millis() - this->lastDownTime) < this->debounceMs) ? true : false;
  this->prevLeft = (this->prevLeft && (millis() - this->lastLeftTime) < this->debounceMs) ? true : false;
  this->prevRight = (this->prevRight && (millis() - this->lastRightTime) < this->debounceMs) ? true : false;
  
  // Remove previous state if debounce expired AND stick is no longer in that zone
  if (millis() - this->lastUpTime >= this->debounceMs || !this->isUp()) {
    this->prevUp = false;
  }
  if (millis() - this->lastDownTime >= this->debounceMs || !this->isDown()) {
    this->prevDown = false;
  }
  if (millis() - this->lastLeftTime >= this->debounceMs || !this->isLeft()) {
    this->prevLeft = false;
  }
  if (millis() - this->lastRightTime >= this->debounceMs || !this->isRight()) {
    this->prevRight = false;
  }
  
  // Update previous button state
  if (!this->btnState) {
    this->prevBtn = false;
  }
}

bool Joystick::isUp() {
  return (this->yRaw < this->thresholdLow);
}

bool Joystick::isDown() {
  return (this->yRaw > this->thresholdHigh);
}

bool Joystick::isLeft() {
  return (this->xRaw < this->thresholdLow);
}

bool Joystick::isRight() {
  return (this->xRaw > this->thresholdHigh);
}

bool Joystick::isBtnDown() {
  return this->btnState;
}

bool Joystick::upPressed() {
  if (this->isUp() && !this->prevUp && (millis() - this->lastUpTime >= this->debounceMs)) {
    this->prevUp = true;
    this->lastUpTime = millis();
    return true;
  }
  return false;
}

bool Joystick::downPressed() {
  if (this->isDown() && !this->prevDown && (millis() - this->lastDownTime >= this->debounceMs)) {
    this->prevDown = true;
    this->lastDownTime = millis();
    return true;
  }
  return false;
}

bool Joystick::leftPressed() {
  if (this->isLeft() && !this->prevLeft && (millis() - this->lastLeftTime >= this->debounceMs)) {
    this->prevLeft = true;
    this->lastLeftTime = millis();
    return true;
  }
  return false;
}

bool Joystick::rightPressed() {
  if (this->isRight() && !this->prevRight && (millis() - this->lastRightTime >= this->debounceMs)) {
    this->prevRight = true;
    this->lastRightTime = millis();
    return true;
  }
  return false;
}

bool Joystick::btnPressed() {
  if (this->btnState && !this->prevBtn && (millis() - this->lastBtnTime >= this->debounceMs)) {
    this->prevBtn = true;
    this->lastBtnTime = millis();
    return true;
  }
  return false;
}

uint16_t Joystick::getXRaw() {
  return this->xRaw;
}

uint16_t Joystick::getYRaw() {
  return this->yRaw;
}

#endif // HAS_JOYSTICK