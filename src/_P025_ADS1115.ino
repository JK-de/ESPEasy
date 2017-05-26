//#######################################################################################################
//#################################### Plugin 025: ADS1115 I2C 0x48)  ###############################################
//#######################################################################################################

#define PLUGIN_025
#define PLUGIN_ID_025 25
#define PLUGIN_NAME_025 "Analog input - ADS1115"
#define PLUGIN_VALUENAME1_025 "Analog"

boolean Plugin_025_init = false;

static uint16_t readRegister025(uint8_t i2cAddress, uint8_t reg) {
  Wire.beginTransmission(i2cAddress);
  Wire.write((0x00));
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, (uint8_t)2);
  return ((Wire.read() << 8) | Wire.read());
}

boolean Plugin_025(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_025;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_025);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_025));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte port = Settings.TaskDevicePort[event->TaskIndex];
        if (port > 0)   //map old port logic to new gain and mode
        {
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = Settings.TaskDevicePluginConfig[event->TaskIndex][0] / 2;
          Settings.TaskDevicePluginConfig[event->TaskIndex][0] = 0x48 + ((port-1)/4);
          Settings.TaskDevicePluginConfig[event->TaskIndex][2] = ((port-1) & 3) | 4;
          Settings.TaskDevicePort[event->TaskIndex] = 0;
        }

        #define ADS1115_I2C_OPTION 4
        byte addr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        int optionValues[ADS1115_I2C_OPTION] = { 0x48, 0x49, 0x4A, 0x4B };
        addFormSelectorI2C(string, F("plugin_025_i2c"), ADS1115_I2C_OPTION, optionValues, addr);

        #define ADS1115_PGA_OPTION 6
        byte pga = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String pgaOptions[ADS1115_PGA_OPTION] = {
          F("2/3x gain (FS=6.144V)"),
          F("1x gain (FS=4.096V)"),
          F("2x gain (FS=2.048V)"),
          F("4x gain (FS=1.024V)"),
          F("8x gain (FS=0.512V)"),
          F("16x gain (FS=0.256V)")
        };
        addFormSelector(string, F("Gain"), F("plugin_025_gain"), ADS1115_PGA_OPTION, pgaOptions, NULL, pga);

        #define ADS1115_MUX_OPTION 8
        byte mux = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String muxOptions[ADS1115_MUX_OPTION] = {
          F("AIN0 - AIN1 (Differential)"),
          F("AIN0 - AIN3"),
          F("AIN1 - AIN3"),
          F("AIN2 - AIN3"),
          F("AIN0 - GND (Single-Ended)"),
          F("AIN1 - GND"),
          F("AIN2 - GND"),
          F("AIN3 - GND"),
        };
        addFormSelector(string, F("Input Multiplexer"), F("plugin_025_mode"), ADS1115_MUX_OPTION, muxOptions, NULL, mux);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_025_i2c"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_025_gain"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_025_mode"));

        Plugin_025_init = false; // Force device setup next time
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_025_init = true;
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        //int value = 0;
        //byte unit = (Settings.TaskDevicePort[event->TaskIndex] - 1) / 4;
        //byte port = Settings.TaskDevicePort[event->TaskIndex] - (unit * 4);
        //uint8_t address = 0x48 + unit;

        uint8_t address = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        uint16_t config = (0x0003)    |  // Disable the comparator (default val)
                          (0x0000)    |  // Non-latching (default val)
                          (0x0000)    |  // Alert/Rdy active low   (default val)
                          (0x0000)    |  // Traditional comparator (default val)
                          (0x0080)    |  // 128 samples per second (default)
                          (0x0100);      // Single-shot mode (default)

        uint16_t pga = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        config |= pga << 9;

        uint16_t mux = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        config |= mux << 12;

        config |= (0x8000);   // Start a single conversion

        Wire.beginTransmission(address);
        Wire.write((uint8_t)(0x01));
        Wire.write((uint8_t)(config >> 8));
        Wire.write((uint8_t)(config & 0xFF));
        Wire.endTransmission();

        delay(8);
        UserVar[event->BaseVarIndex] = (float) readRegister025((address), (0x00)) ;
        String log = F("ADS1115  : Analog value: ");
        log += UserVar[event->BaseVarIndex];
        log += F(" @");
        log += String(config, 16);
        addLog(LOG_LEVEL_INFO,log);
        success = true;
        break;
      }
  }
  return success;
}
