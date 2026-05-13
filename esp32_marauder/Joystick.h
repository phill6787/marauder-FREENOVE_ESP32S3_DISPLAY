#ifndef Joystick_h
#define Joystick_h

#include <Arduino.h>
#include "configs.h"

#ifdef HAS_JOYSTICK

class Joystick {
  private:
    int pinX;
    int pinY;
    int pinBtn;
    
    uint16_t xRaw;
    uint16_t yRaw;
    bool btnState;
    
    // Previous state for edge detection
    bool prevUp;
    bool prevDown;
    bool prevLeft;
    bool prevRight;
    bool prevBtn;
    
    // Debounce timer
    uint32_t lastUpTime;
    uint32_t lastDownTime;
    uint32_t lastLeftTime;
    uint32_t lastRightTime;
    uint32_t lastBtnTime;
    
    uint16_t thresholdLow;
    uint16_t thresholdHigh;
    
    uint16_t debounceMs;
    
  public:
    Joystick();
    
    void begin();
    void update();  // Call each loop to read ADC
    
    bool upPressed();     // Just pressed UP
    bool downPressed();   // Just pressed DOWN
    bool leftPressed();   // Just pressed LEFT
    bool rightPressed();  // Just pressed RIGHT
    bool btnPressed();    // Just pressed joystick button
    
    bool isUp();          // Currently in UP zone
    bool isDown();        // Currently in DOWN zone
    bool isLeft();        // Currently in LEFT zone
    bool isRight();       // Currently in RIGHT zone
    bool isBtnDown();     // Joystick button is currently pressed
    
    uint16_t getXRaw();
    uint16_t getYRaw();
};

#endif // HAS_JOYSTICK
#endif