// I2C databus init

#define I2C_SDA_PIN         21
#define I2C_SCL_PIN         22

void initI2C() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);     // Setup the I2C bus wire connection
  Wire.setClock(400000);
}

void scanI2Cdevice(void)
{
    uint8_t err, addr;
    int nDevices = 0;
    for (addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        err = Wire.endTransmission();
        if (err == 0) {
            #ifdef ENABLE_SERIAL            
              Serial.print("I2C device found at address 0x");
              
              if (addr < 16)
                  Serial.print("0");
              Serial.print(addr, HEX);
              Serial.println(" !");
            #endif
            nDevices++;
        } else if (err == 4) {
            #ifdef ENABLE_SERIAL            
              Serial.print("Unknow error at address 0x");
                            
              if (addr < 16) Serial.print("0");
              Serial.println(addr, HEX);
            #endif
        }
    }
    #ifdef ENABLE_SERIAL            
      if (nDevices == 0)
            Serial.println("No I2C devices found\n");
      else
          Serial.println("Done\n");
    #endif      
}
