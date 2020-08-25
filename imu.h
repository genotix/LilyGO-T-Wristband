// Gyroscope, Magneto, Accelerometer senser

LSM9DS1       imu; // Create an LSM9DS1 object to use from here on.

#define INT1_PIN_THS        38
#define INT2_PIN_DRDY       39
#define INTM_PIN_THS        37
#define RDYM_PIN            36

void sleepIMU() {
  #ifdef ENABLE_SERIAL    
    Serial.println("Sleeping IMU!");
    Serial.flush();
  #endif

  sleepStateIMU = true;

  imu.sleepGyro(true);          // Issue sleep request
}

String getTemperature() {
  return String(imu.temperature + 25);  // Adding 25 degrees since this is the base line temperature
}

void configureIMU() {
    imu.settings.gyro.latchInterrupt    = false;
    
    imu.settings.gyro.scale             = 245;    // Set gyroscope scale to +/-245 dps:
    imu.settings.gyro.sampleRate        = 1;      // Set gyroscope (and accel) sample rate to 14.9 Hz
    imu.settings.accel.scale            = 2;      // Set accelerometer scale to +/-2g
    imu.settings.mag.scale              = 4;      // Set magnetometer scale to +/- 4g
    imu.settings.mag.sampleRate         = 0;      // Set magnetometer sample rate to 0.625 Hz

    imu.settings.gyro.lowPowerEnable    = true;
    imu.settings.gyro.enableX           = false;
    imu.settings.gyro.enableY           = false;
    imu.settings.gyro.enableZ           = false;
    imu.settings.gyro.enabled           = false;
    
    imu.settings.mag.enabled            = false;
    imu.settings.mag.lowPowerEnable     = true;
    imu.settings.mag.operatingMode      = 11;
    
    imu.settings.accel.highResBandwidth = false;
    imu.settings.accel.enabled          = false;
    imu.settings.accel.enableX          = false;
    imu.settings.accel.enableY          = false;
    imu.settings.accel.enableZ          = false;
    
    imu.settings.temp.enabled           = false;
}


// ALWAYS run this setup otherwise the IMU will be active and DRAIN YOUR BATTERY
uint16_t initIMU()
{    
    // Set up our Arduino pins connected to interrupts.
    // We configured all of these interrupts in the LSM9DS1
    // to be active-low.
    pinMode(INT2_PIN_DRDY, INPUT);
    pinMode(INT1_PIN_THS,  INPUT);
    pinMode(INTM_PIN_THS,  INPUT);

    // The magnetometer DRDY pin (RDY) is not configurable.
    // It is active high and always turned on.
    pinMode(RDYM_PIN,      INPUT);

    // gyro.latchInterrupt controls the latching of the
    // gyro and accelerometer interrupts (INT1 and INT2).
    // false = no latching

    configureIMU();

    // Call imu.begin() to initialize the sensor and instill it with our settings.

    bool r =  imu.begin(LSM9DS1_AG_ADDR(1), LSM9DS1_M_ADDR(1), Wire); // set addresses and wire port
    if (r) {
        #ifdef ENABLE_SERIAL    
          Serial.println("IMU countacted; putting it into low-power immediately.");
        #endif
        
        /*
        You need to know using the following functions.
        These three methods are all set as protection members in the original SparkFun_LSM9DS1_Arduino_Library.
        If you need to use them, you need to put the three methods into public members.
        This is just for faster sensor testing. Low current consumption
        !!! LSM9DS1 consumes about 1 mA in low power mode but that is ONLY when you initialize it!!!
        */
        
        imu.initMag();
        imu.initAccel();
        imu.initGyro();

        sleepIMU();       // Is this working??
        return true;
    } else {
      #ifdef ENABLE_SERIAL    
          Serial.println("Error addressing IMU!");
          Serial.flush();
      #endif
      return false;
    }
}

void setupIMU() {
    // Turn on the IMU with configureIMU() (defined above)
    // check the return status of imu.begin() to make sure
    // it's connected.
    uint16_t status = initIMU();
    
    if (status == false) {
      #ifdef ENABLE_SERIAL    
        Serial.print("Failed to connect to IMU: 0x");
        Serial.println(status, HEX);
      #endif                                  
        while (1) ;
    }
}

// After turning the IMU on, configure the interrupts:
// configureLSM9DS1Interrupts();    
void configureLSM9DS1Interrupts()
{
    /////////////////////////////////////////////
    // Configure INT1 - Gyro & Accel Threshold //
    /////////////////////////////////////////////
    // For more information on setting gyro interrupt, threshold,
    // and configuring the intterup, see the datasheet.
    // We'll configure INT_GEN_CFG_G, INT_GEN_THS_??_G,
    // INT_GEN_DUR_G, and INT1_CTRL.
    // 1. Configure the gyro interrupt generator:
    //  - ZHIE_G: Z-axis high event (more can be or'd together)
    //  - false: and/or (false = OR) (not applicable)
    //  - false: latch interrupt (false = not latched)
    imu.configGyroInt(ZHIE_G, false, false);
    // 2. Configure the gyro threshold
    //   - 500: Threshold (raw value from gyro)
    //   - Z_AXIS: Z-axis threshold
    //   - 10: duration (based on ODR)
    //   - true: wait (wait duration before interrupt goes low)
    imu.configGyroThs(500, Z_AXIS, 10, true);
    // 3. Configure accelerometer interrupt generator:
    //   - XHIE_XL: x-axis high event
    //     More axis events can be or'd together
    //   - false: OR interrupts (N/A, since we only have 1)
    imu.configAccelInt(XHIE_XL, false);
    // 4. Configure accelerometer threshold:
    //   - 20: Threshold (raw value from accel)
    //     Multiply this value by 128 to get threshold value.
    //     (20 = 2600 raw accel value)
    //   - X_AXIS: Write to X-axis threshold
    //   - 10: duration (based on ODR)
    //   - false: wait (wait [duration] before interrupt goes low)
    imu.configAccelThs(20, X_AXIS, 1, false);
    // 5. Configure INT1 - assign it to gyro interrupt
    //   - XG_INT1: Says we're configuring INT1
    //   - INT1_IG_G | INT1_IG_XL: Sets interrupt source to
    //     both gyro interrupt and accel
    //   - INT_ACTIVE_LOW: Sets interrupt to active low.
    //         (Can otherwise be set to INT_ACTIVE_HIGH.)
    //   - INT_PUSH_PULL: Sets interrupt to a push-pull.
    //         (Can otherwise be set to INT_OPEN_DRAIN.)
    imu.configInt(XG_INT1, INT1_IG_G | INT_IG_XL, INT_ACTIVE_LOW, INT_PUSH_PULL);

    ////////////////////////////////////////////////
    // Configure INT2 - Gyro and Accel Data Ready //
    ////////////////////////////////////////////////
    // Configure interrupt 2 to fire whenever new accelerometer
    // or gyroscope data is available.
    // Note XG_INT2 means configuring interrupt 2.
    // INT_DRDY_XL is OR'd with INT_DRDY_G
    imu.configInt(XG_INT2, INT_DRDY_XL | INT_DRDY_G, INT_ACTIVE_LOW, INT_PUSH_PULL);

    //////////////////////////////////////
    // Configure Magnetometer Interrupt //
    //////////////////////////////////////
    // 1. Configure magnetometer interrupt:
    //   - XIEN: axis to be monitored. Can be an or'd combination
    //     of XIEN, YIEN, or ZIEN.
    //   - INT_ACTIVE_LOW: Interrupt goes low when active.
    //   - true: Latch interrupt
    imu.configMagInt(XIEN, INT_ACTIVE_LOW, true);
    // 2. Configure magnetometer threshold.
    //   There's only one threshold value for all 3 mag axes.
    //   This is the raw mag value that must be exceeded to
    //   generate an interrupt.
    imu.configMagThs(10000);

}

void enableIMU() {
  #ifdef ENABLE_SERIAL    
    Serial.println("Waking IMU!");
  #endif
  
  sleepStateIMU = false;

  imu.sleepGyro(false);   // Need to test this
}

void getIMU()
{
#ifdef ENABLE_SENSOR
    // Update the sensor values whenever new data is available
    if ( imu.gyroAvailable() ) {
        #ifdef ENABLE_SERIAL              
          Serial.println("Gyroscope is available");
          Serial.flush();
        #endif
        // To read from the gyroscope,  first call the
        // readGyro() function. When it exits, it'll update the
        // gx, gy, and gz variables with the most current data.
        imu.readGyro();
    } else {
        #ifdef ENABLE_SERIAL              
          Serial.println("Invalid gyroscope");
          Serial.flush();
        #endif
    }
    if ( imu.accelAvailable() ) {
        #ifdef ENABLE_SERIAL              
          Serial.println("Accelerometer is available");
          Serial.flush();
        #endif
        // To read from the accelerometer, first call the
        // readAccel() function. When it exits, it'll update the
        // ax, ay, and az variables with the most current data.
        imu.readAccel();
    } else {
        #ifdef ENABLE_SERIAL            
          Serial.println("Invalid accelerometer");
          Serial.flush();
        #endif  
    }
    if ( imu.magAvailable() ) {
        #ifdef ENABLE_SERIAL              
          Serial.println("Magnetometer is available");
          Serial.flush();
        #endif
        // To read from the magnetometer, first call the
        // readMag() function. When it exits, it'll update the
        // mx, my, and mz variables with the most current data.
        imu.readMag();
    } else {
        #ifdef ENABLE_SERIAL            
          Serial.println("Invalid magnetometer");
          Serial.flush();
        #endif  
    }
#endif
}
