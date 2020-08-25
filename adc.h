// Analog to Digital Converter functions

#define BATT_ADC_PIN        35

int vref = 1100;

void setupADC()
{
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        #ifdef ENABLE_SERIAL            
          Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        #endif  
        vref = adc_chars.vref;
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        #ifdef ENABLE_SERIAL            
         Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
        #endif 
    } else {
        #ifdef ENABLE_SERIAL            
          Serial.println("Default Vref: 1100mV");
        #endif  
    }

    sleepStateADC = false;

}

String getVoltage()
{
    uint16_t v = analogRead(BATT_ADC_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    return String(battery_voltage) + "V";
}

String getBattPerc()
{
    uint16_t v = analogRead(BATT_ADC_PIN);
    float battery_voltage = (((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0)) * 100;

    int percvalue = battery_voltage;
    #ifdef ENABLE_SERIAL            
      Serial.print("Voltage ");
      Serial.println(percvalue);
    #endif
    
    int percentage = map(percvalue, MIN_VOLTAGE, MAX_VOLTAGE, 0,100);
    #ifdef ENABLE_SERIAL            
      Serial.print("Percentage ");
      Serial.println(percentage);
    #endif
    
    return String(percentage) + "%";
}


void sleepADC() {
    adc_power_off();

    sleepStateADC    = true;
}
