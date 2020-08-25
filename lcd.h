// LCD TFT functions

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip

#ifndef ST7735_SLPIN
  #define ST7735_SLPIN    0x10
  #define ST7735_DISPOFF  0x28
#endif

TFT_eSPI      tft = TFT_eSPI();  // Invoke library, note that the pins are defined in User_Setup.h

void initLCD() {
    tft.init();
    tft.setRotation(1);
    tft.setSwapBytes(true);
    tft.fillScreen(TFT_BLACK);                               // Empty the screen

    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, lcd_value);                                 // Start the screen in PWM OFF sstate
}

void lcdLightDim(int lcd_newbrightness=0) {
  unsigned long requestTime = millis();
  unsigned long finishTime  = requestTime + lcd_dimtime;
  int          lcd_newvalue = lcd_value;

  lcd_newbrightness = constrain(lcd_newbrightness,0,255);
  
  while ( lcd_newvalue != lcd_newbrightness and millis() < finishTime ) {
    lcd_newvalue = map(constrain(millis(), requestTime, finishTime), requestTime, finishTime, lcd_value, lcd_newbrightness);
    #ifdef ENABLE_SERIAL 
      #ifdef DEBUGMODE 
        Serial.println(lcd_newvalue);
        Serial.flush();            
      #endif    
    #endif                
    ledcWrite(0, lcd_newvalue);           // Show the screen PWM dimmed from here
    delay(20);
  }

  // Set the final value
  lcd_value = lcd_newbrightness;
  
  #ifdef ENABLE_SERIAL 
    #ifdef DEBUGMODE 
      Serial.println(lcd_value);
      Serial.flush();
    #endif    
  #endif    
  ledcWrite(0, lcd_value);             // Set to the new value
}

const uint16_t * getImage(int image=-1) {
  switch (image) {
    case 0:
      return number0;
    break;
    case 1:
      return number1;
    break;
    case 2:
      return number2;
    break;
    case 3:
      return number3_orig;
    break;  
    case 4:
      return number4;
    break;  
    case 5:
      return number5;
    break;  
    case 6:
      return number6_orig;
    break;  
    case 7:
      return number7;
    break;  
    case 8:
      return number8_orig;
    break;  
    case 9:
      return number9_orig;
    break;  
    default:
      return separator;
    break;  
  }
}

void lcdPopup(int ShowTime) {
    lcdLightDim(lcd_lit);
    delay(ShowTime);
    lcdLightDim(lcd_off);
    tft.fillScreen(TFT_BLACK);
}

void sleepLCD() {
    tft.writecommand(ST7735_SWRESET);
    delay(100);
    tft.writecommand(ST7735_SLPIN);
    delay(150);
    tft.writecommand(ST7735_DISPOFF);
    
    sleepStateLCD    = true;
}
