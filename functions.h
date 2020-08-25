
//! Uncomment this line and a hardware test will be conducted
// #define FACTORY_HW_TEST

//! Uncomment this line and use WiFi for OTA update
// #define ARDUINO_OTA_UPDATE

//! Uncomment this line and Bluetooth will be used to transfer IMU data
// #define ENABLE_BLE_DATA_TRANSMISSION

/*
You need to know using the following functions.
These three methods are all set as protection members in the original SparkFun_LSM9DS1_Arduino_Library.
If you need to use them, you need to put the three methods into public members.
This is just for faster sensor testing. Low current consumption
!!! In actual test, LSM9DS1 consumes about 1 mA !!! If you do not turn them off, it will consume about 4~6mA!!!
**/
#define USE_PROTECTED_MEMBERS

// Assuming ALL is awake and setting this in the INIT's too
bool sleepStateIMU    = false;
bool sleepStateLCD    = false;
bool sleepStateRTC    = false;
bool sleepStateADC    = false;
bool sleepStateBT     = false;
bool sleepStateSerial = false;
bool sleepStateWifi   = false;

#include "bt.h"
#include "adc.h"
#include "rtc.h"
#include "lcd.h"
#include "imu.h"
#include "wifi.h"

#define TP_PIN_PIN          33
#define TP_PWR_PIN          25
#define LED_PIN             4
#define CHARGE_PIN          32
#define MOTOR_PIN           14

/*
uint8_t omm = 99;
uint8_t xcolon = 0;
uint32_t colour = 0;
*/
uint32_t targetTime = 0;       // for next 1 second timeout

bool pressed = false;
uint32_t pressedTime = 0;
bool charge_indication = false;
bool charge_show = false;


void factoryTest()
{
    scanI2Cdevice();
    delay(2000);

    tft.fillScreen(TFT_BLACK);
    tft.drawString("RTC Interrupt self test", 0, 0);

    int yy = 2019, mm = 5, dd = 15, h = 2, m = 10, s = 0;
    rtc.begin(Wire);
    rtc.setDateTime(yy, mm, dd, h, m, s);
    //delay(500);
    
    RTC_Date dt = rtc.getDateTime();
    if (dt.year != yy || dt.month != mm || dt.day != dd || dt.hour != h || dt.minute != m) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Write DateTime FAIL", 0, 0);
    } else {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Write DateTime PASS", 0, 0);
    }

    delay(2000);

    //! RTC Interrupt Test
    pinMode(RTC_INT_PIN, INPUT_PULLUP); //need change to rtc_pin
    attachInterrupt(RTC_INT_PIN, [] {
        rtcIrq = 1;
    }, FALLING);

    rtc.disableAlarm();

    rtc.setDateTime(2019, 4, 7, 9, 5, 57);

    rtc.setAlarmByMinutes(6);

    rtc.enableAlarm();

    for (;;) {
        snprintf(buff, sizeof(buff), "%s", rtc.formatDateTime());
        Serial.print("\t");
        Serial.println(buff);
        tft.fillScreen(TFT_BLACK);
        tft.drawString(buff, 0, 0);

        if (rtcIrq) {
            rtcIrq = 0;
            detachInterrupt(RTC_INT_PIN);
            rtc.resetAlarm();
            break;
        }
        delay(1000);
    }
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.drawString("RTC Interrupt PASS", 0, 0);
    delay(2000);

    wifi_scan();
    delay(2000);
}


void show_time(int hours, int minutes){
    tft.setRotation(0);

    int showvalue;

    // Hours
    showvalue = getTensValue(hours);
    tft.pushImage(spacer, spacer                        ,  number_width, number_height, getImage(showvalue));
    showvalue = hours - 10 * getTensValue(hours);
    tft.pushImage(spacer, 2 * spacer + number_height    ,  number_width, number_height, getImage(showvalue));
    
    // Minutes
    showvalue = getTensValue(minutes);
    tft.pushImage(left_margin + spacer, 3 * spacer + 2 * number_height,  number_width, number_height, getImage(showvalue));
    
    showvalue = minutes - 10 * getTensValue(minutes);
    tft.pushImage(left_margin + spacer, 4 * spacer + 3 * number_height,  number_width, number_height, getImage(showvalue));

    // Separator
    tft.pushImage(left_separator_margin, 8 * spacer + 2 * number_height,  separator_width, separator_height, separator);  // Image full screen
}

void RTC_Show()
{
    Serial.println("Showing time");
    Serial.flush();
    
    if (charge_indication) {
        charge_indication = false;
        if (digitalRead(CHARGE_PIN) == LOW) {
            #ifdef ENABLE_SERIAL            
              Serial.println("Showing charge indicator");
              Serial.flush();
            #endif  
            tft.pushImage(140, 55, 16, 16, charge);
        } else {
            #ifdef ENABLE_SERIAL            
              Serial.println("Not showing charge indicator");
              Serial.flush();
            #endif  
            tft.fillRect(140, 55, 16, 16, TFT_BLACK);
        }
    }

    RTC_Date datetime = rtc.getDateTime();
    show_time(datetime.hour, datetime.minute);

    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawRightString( getBattPerc(), screen_height, 0, 1); // Next size up font 2
    #ifdef DEBUGMODE
      tft.drawRightString( getVoltage(),  screen_height, 12, 1); // Next size up font 2
      tft.drawRightString( getTemperature(),  screen_height, 24, 1); // Next size up font 2
    #endif  
}

void IMU_Show()
{
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    
#ifdef ENABLE_SENSOR
    getIMU();

    snprintf(buff, sizeof(buff), "--  ACC  GYR   MAG");
    tft.drawString(buff, 0, 0);
    snprintf(buff, sizeof(buff), "x %.2f  %.2f  %.2f", imu.calcAccel(imu.ax), imu.calcGyro(imu.gx), imu.calcMag(imu.mx));
    tft.drawString(buff, 0, 16);
    snprintf(buff, sizeof(buff), "y %.2f  %.2f  %.2f", imu.calcAccel(imu.ay), imu.calcGyro(imu.gy), imu.calcMag(imu.my));
    tft.drawString(buff, 0, 32);
    snprintf(buff, sizeof(buff), "z %.2f  %.2f  %.2f", imu.calcAccel(imu.az), imu.calcGyro(imu.gz), imu.calcMag(imu.mz));
    tft.drawString(buff, 0, 48);
    delay(200);
#else
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("Sensor is not enabled");
    delay(200);
#endif

}

void buzz() 
{
     digitalWrite(MOTOR_PIN, HIGH);
     digitalWrite(LED_PIN, HIGH);
     delay(100);
     digitalWrite(LED_PIN, LOW);
     digitalWrite(MOTOR_PIN, LOW);
     delay(500);
     digitalWrite(MOTOR_PIN, HIGH);
     digitalWrite(LED_PIN, HIGH);
     delay(100);
     digitalWrite(LED_PIN, LOW);
     digitalWrite(MOTOR_PIN, LOW); 
}


void sleepSerial() {
    #ifdef ENABLE_SERIAL          
        Serial.end(); // Stop serial communication
    #endif

    sleepStateSerial = true;
}

void setWakeInterrupt() {
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_33, ESP_EXT1_WAKEUP_ANY_HIGH);
}

void sleepHoldingDisplay() {
    sleepIMU();
    sleepRTC();
    sleepADC();
    sleepBT();
    sleepSerial();
}

void sleepWatch() {
    sleepIMU();
    sleepLCD();
    sleepRTC();
    sleepADC();
    sleepBT();
    sleepSerial();

    setWakeInterrupt();
    
    delay(200);
    esp_deep_sleep_disable_rom_logging();
    esp_deep_sleep_start();
}
