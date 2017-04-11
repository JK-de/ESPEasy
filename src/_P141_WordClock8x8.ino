//#######################################################################################################
//#################################### Plugin 141: NeoPixel clock #######################################
//#######################################################################################################
#undef ICACHE_RAM_ATTR
#define ICACHE_RAM_ATTR   // hack to bring all isr routines in one section
#include <NeoPixelBus.h>   // use lib from https://github.com/Makuna/NeoPixelBus.git

#define PLUGIN_141_NUM_LEDS      64

byte Plugin_141_red = 0;
byte Plugin_141_green = 0;
byte Plugin_141_blue = 0;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> *Plugin_141_pixels;

#define PLUGIN_141
#define PLUGIN_ID_141         141
#define PLUGIN_NAME_141       "NeoPixel - WordClock8x8"
#define PLUGIN_VALUENAME1_141 "Clock"
boolean Plugin_141(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_141;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_141);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_141));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        char tmpString[128];
        sprintf_P(tmpString, PSTR("<TR><TD>Red:<TD><input type='text' name='plugin_141_red' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += tmpString;
        sprintf_P(tmpString, PSTR("<TR><TD>Green:<TD><input type='text' name='plugin_141_green' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        string += tmpString;
        sprintf_P(tmpString, PSTR("<TR><TD>Blue:<TD><input type='text' name='plugin_141_blue' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][2]);
        string += tmpString;
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg(F("plugin_141_red"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        String plugin2 = WebServer.arg(F("plugin_141_green"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
        String plugin3 = WebServer.arg(F("plugin_141_blue"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = plugin3.toInt();
        Plugin_141_red = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        Plugin_141_green = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        Plugin_141_blue = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        Plugin_141_update();
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!Plugin_141_pixels)
        {
          Plugin_141_pixels = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>(PLUGIN_141_NUM_LEDS, Settings.TaskDevicePin1[event->TaskIndex]);
          Plugin_141_pixels->Begin(); // This initializes the NeoPixel library.
        }
        Plugin_141_red = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        Plugin_141_green = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        Plugin_141_blue = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        success = true;
        break;
      }

    case PLUGIN_CLOCK_IN:
      {
        //Plugin_141_update();
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        //int ldrVal = map(analogRead(A0), 0, 1023, 15, 245);
        //Serial.print("LDR value: ");
        //Serial.println(ldrVal);
        //Plugin_141_pixels->setBrightness(255-ldrVal);
        //Plugin_141_pixels->show(); // This sends the updated pixel color to the hardware.
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);

        if (tmpString.equalsIgnoreCase(F("NeoClockColor")))
        {
          Plugin_141_red = event->Par1;
          Plugin_141_green = event->Par2;
          Plugin_141_blue = event->Par3;
          Plugin_141_update();
          success = true;
        }

        if (tmpString.equalsIgnoreCase(F("NeoTestAll")))
        {
          for (int i = 0; i < PLUGIN_141_NUM_LEDS; i++)
            Plugin_141_pixels->SetPixelColor(i, RgbColor(event->Par1, event->Par2, event->Par3));
          Plugin_141_pixels->Show(); // This sends the updated pixel color to the hardware.
          success = true;
        }

        if (tmpString.equalsIgnoreCase(F("NeoTestLoop")))
        {
          for (int i = 0; i < PLUGIN_141_NUM_LEDS; i++)
          {
            Plugin_141_resetAndBlack();
            Plugin_141_pixels->SetPixelColor(i, RgbColor(event->Par1, event->Par2, event->Par3));
            Plugin_141_pixels->Show(); // This sends the updated pixel color to the hardware.
            delay(200);
          }
          success = true;
        }

        break;
      }

  }
  return success;
}

void Plugin_141_update()
{
  byte Hours = hour();
  byte Minutes = minute();
  Plugin_141_resetAndBlack();
  Plugin_141_timeToStrip(Hours, Minutes);
  Plugin_141_pixels->Show(); // This sends the updated pixel color to the hardware.
}


void Plugin_141_resetAndBlack() {
  for (int i = 0; i < PLUGIN_141_NUM_LEDS; i++) {
    Plugin_141_pixels->SetPixelColor(i, RgbColor(0, 0, 0));
  }
}

void Plugin_141_pushToStrip(int ledId) {
  Plugin_141_pixels->SetPixelColor(ledId, RgbColor(Plugin_141_red, Plugin_141_green, Plugin_141_blue));
}

void Plugin_141_timeToStrip(uint8_t hours, uint8_t minutes)
{
  //show minutes
  if (minutes >= 5 && minutes < 10) {
  }

  int singleMinutes = minutes % 5;
  switch (singleMinutes) {
  }
  if (hours >= 12) {
    hours -= 12;
  }
  if (hours == 12) {
    hours = 0;
  }
  if (minutes >= 20) {
    hours++;
  }

  //show hours
  switch (hours) {
  }
  //show HOUR
  if (minutes < 5) {
  }
}
