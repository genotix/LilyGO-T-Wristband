// Real Time Clock functions
#define RTC_INT_PIN         34

PCF8563_Class rtc;

bool    rtcIrq = false;
uint8_t hh, mm, ss ;

void setupRTC()
{
    rtc.begin(Wire);
    //Check if the RTC clock matches, if not, use compile time
    rtc.check();

    RTC_Date datetime = rtc.getDateTime();
    hh = datetime.hour;
    mm = datetime.minute;
    ss = datetime.second;

    sleepStateRTC = false;
}

int getTensValue(int TensValue) {
  return int(TensValue/10);
}

void sleepRTC() {
    rtc.clearTimer();
    rtc.disableAlarm();
    rtc.disableCLK();
    rtc.disableTimer();
    
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);

    sleepStateRTC    = true;
}
