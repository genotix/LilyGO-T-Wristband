// Wifi functions

char buff[256];
bool initial = 1;
bool otaStart = false;


/*
 *        WiFi
 */
 
void setupWiFi()
{
#ifdef ARDUINO_OTA_UPDATE
    WiFiManager wifiManager;
    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setBreakAfterConfig(true);          // Without this saveConfigCallback does not get fired
    wifiManager.autoConnect("T-Wristband");
#endif
}

void wifi_scan()
{
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);

    tft.drawString("Scan Network", tft.width() / 2, tft.height() / 2);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int16_t n = WiFi.scanNetworks();
    tft.fillScreen(TFT_BLACK);
    if (n == 0) {
        tft.drawString("no networks found", tft.width() / 2, tft.height() / 2);
    } else {
        tft.setTextDatum(TL_DATUM);
        tft.setCursor(0, 0);
        for (int i = 0; i < n; ++i) {
            sprintf(buff,
                    "[%d]:%s(%d)",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i));
            Serial.println(buff);
            tft.println(buff);
        }
    }
    WiFi.mode(WIFI_OFF);

    sleepStateWifi = true;
}




/*
 *        OTA update
 */

#ifdef ARDUINO_OTA_UPDATE
  #include <ESPmDNS.h>
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
  //  git clone -b development https://github.com/tzapu/WiFiManager.git
  #include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
  WiFiManager wifiManager;
  
  void configModeCallback (WiFiManager *myWiFiManager)
  {
      #ifdef ENABLE_SERIAL            
        Serial.println("Entered config mode");
        Serial.println(WiFi.softAPIP());
        //if you used auto generated SSID, print it
        Serial.println(myWiFiManager->getConfigPortalSSID());
      #endif
      
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      tft.drawString("Connect hotspot name ",  20, tft.height() / 2 - 20);
      tft.drawString("configure wrist",  35, tft.height() / 2  + 20);
      tft.setTextColor(TFT_GREEN);
      tft.drawString("\"T-Wristband\"",  40, tft.height() / 2 );
  
  }
  
  void drawProgressBar(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint8_t percentage, uint16_t frameColor, uint16_t barColor)
  {
      if (percentage == 0) {
          tft.fillRoundRect(x0, y0, w, h, 3, TFT_BLACK);
      }
      uint8_t margin = 2;
      uint16_t barHeight = h - 2 * margin;
      uint16_t barWidth = w - 2 * margin;
      tft.drawRoundRect(x0, y0, w, h, 3, frameColor);
      tft.fillRect(x0 + margin, y0 + margin, barWidth * percentage / 100.0, barHeight, barColor);
  }
#endif

void setupOTA()
{
#ifdef ARDUINO_OTA_UPDATE
    // Port defaults to 3232
    // ArduinoOTA.setPort(3232);

    // Hostname defaults to esp3232-[MAC]
    ArduinoOTA.setHostname("T-Wristband");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        #ifdef ENABLE_SERIAL            
          Serial.println("Start updating " + type);
        #endif  
        otaStart = true;
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Updating...", tft.width() / 2 - 20, 55 );
    })
    .onEnd([]() {
        #ifdef ENABLE_SERIAL            
          Serial.println("\nEnd");
          delay(500);
        #endif  
    })
    .onProgress([](unsigned int progress, unsigned int total) {
        // Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        int percentage = (progress / (total / 100));
        tft.setTextDatum(TC_DATUM);
        tft.setTextPadding(tft.textWidth(" 888% "));
        tft.drawString(String(percentage) + "%", 145, 35);
        drawProgressBar(10, 30, 120, 15, percentage, TFT_WHITE, TFT_BLUE);
    })
    .onError([](ota_error_t error) {

        #ifdef ENABLE_SERIAL            
          Serial.printf("Error[%u]: ", error);
          if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
          else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
          else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
          else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
          else if (error == OTA_END_ERROR) Serial.println("End Failed");
        #endif
        
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Update Failed", tft.width() / 2 - 20, 55 );
        delay(3000);
        otaStart = false;
        initial = 1;
        targetTime = millis() + 1000;
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        omm = 99;
    });

    ArduinoOTA.begin();
#endif
}
