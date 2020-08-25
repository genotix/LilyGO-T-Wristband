// WARNING; Lower Serial speed to 460800
// Edit the SparkFunLSM9DS1.h file and move the declarations of 
// initMag(), initAccel() and initGyro() to the public declaration for the LOW POWER of the IMU to work properly.

#include <pcf8563.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <SparkFunLSM9DS1.h>

#include "i2c.h"

#include "config.h"
#include "sensor.h"
#include "esp_adc_cal.h"
#include "bitmap.h"

#include "functions.h"

// HINT: Doing things directly after a deep sleep https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/deep-sleep-stub.html

void setup(void)
{
  
    pinMode(TP_PIN_PIN, INPUT_PULLDOWN);      // Init the Touchsensor; set pulldown so it doesn't float
    pinMode(TP_PWR_PIN, PULLUP);              //! Must be set to pull-up output mode in order to wake up in deep sleep mode

    #ifdef ENABLE_SERIAL    
        Serial.begin(115200);
    #endif         

    initI2C();  // Needed to address all I2C devices
    
    initLCD();

    tft.pushImage(0, 0, screen_width, screen_height, logo);  // Image full screen

    lcdPopup(1000);
    
    #ifdef FACTORY_HW_TEST
        factoryTest();
    #endif

    setupRTC();
    
    // Corrected ADC reference voltage
    #ifdef ENABLE_ADC
        setupADC();
    #endif 
    
    #ifdef ENABLE_WIFI    
        setupWiFi();
    #else
        WiFi.mode(WIFI_OFF);
    #endif

    setupIMU();
        
    #ifdef ENABLE_OTA    
        setupOTA();
    #endif
    
    #ifdef ENABLE_BLE   
        setupBLE();
    #endif

    // Is this needed?
    tft.fillScreen(TFT_BLACK);
//    delay(100);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour

    //targetTime = millis() + 1000;
    targetTime = millis();
    
    digitalWrite(TP_PWR_PIN, HIGH);

    // Set the motor drive to output, if you have this module
    pinMode(MOTOR_PIN, OUTPUT);

    // Charging instructions, it is connected to IO32,
    // when it changes, you need to change the flag to know whether charging is in progress
    pinMode(CHARGE_PIN, INPUT_PULLUP);
    attachInterrupt(CHARGE_PIN, [] {
        charge_indication = true;
    }, CHANGE);

    // Check the charging instructions, if he is low, if it is true, then it is charging
    if (digitalRead(CHARGE_PIN) == LOW) {
        charge_indication = true;
    }

    // Lower MCU frequency can effectively reduce current consumption and heat
    setCpuFrequencyMhz(80);
}

uint8_t tempRegValue = 0;

void loop()
{
#ifdef ARDUINO_OTA_UPDATE
    ArduinoOTA.handle();

    //! If OTA starts, skip the following operation
    if (otaStart) return;
#endif

    if (digitalRead(TP_PIN_PIN) == HIGH) {
        if (!pressed) {
            initial = 1;
            targetTime = millis() + 1000;
            tft.fillScreen(TFT_BLACK);
            pressed = true;
            pressedTime = millis();
            
        } else {
          #if defined(ARDUINO_OTA_UPDATE)
            if (millis() - pressedTime > 3000) {
                tft.fillScreen(TFT_BLACK);
                tft.drawString("Reset WiFi Setting",  20, tft.height() / 2 );
                //delay(3000);
                wifiManager.resetSettings();
                wifiManager.erase(true);
                esp_restart();
            }
          #endif
        }
    } else {
        pressed = false;
    }
    
    RTC_Show();
    lcdPopup(4000);

#ifdef ENABLE_SERIAL    
        Serial.println("Go to Sleep");
#endif

    sleepWatch();
}
