#define MIN_VOLTAGE               327
#define MAX_VOLTAGE               420

//#define DEBUGMODE
#define ENABLE_ADC
//#define ENABLE_SENSOR         // Do not enable the sensor; will this work in lowering the power usage??

//#define ENABLE_SERIAL

const byte lcd_lit      =  220;
const byte lcd_off      =  0;
const byte lcd_dimtime  =  1000;     // Lit time in milliseconds
byte       lcd_value    =  lcd_off;

/*
#define ENABLE_SERIAL
#define ENABLE_WIFI
#define ENABLE_SENSOR
#define ENABLE_OTA
#define ENABLE_BLE
*/
