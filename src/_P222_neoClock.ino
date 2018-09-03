//#ifdef USES_P222
//#######################################################################################################
//#################################### Plugin 222: NEO Pixel Clock Collection ##########################################
//#######################################################################################################

// ESPEasy Plugin to control a 16x8 LED matrix or 8 7-segment displays with chip HT16K33
// written by Jochen Krapf (jk@nerd2nerd.org)

#include <NeoPixelBus.h>


#define PLUGIN_222
#define PLUGIN_ID_222         222
#define PLUGIN_NAME_222       "Display - neoClock [TESTING]"

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* Plugin_222_S = NULL;

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif

#define LED_OFFSET_S 0
#define LED_OFFSET_M 60
#define LED_OFFSET_H 120
#define LED_COUNT (60+60+24)
#define LED_COUNT_H 24
#define LED_LUM_ON 128

boolean Plugin_222(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_222;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_NONE;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_222);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte type = CONFIG(0);

        if (CONFIG(1) <= 0)
        {
          CONFIG(1) = LED_COUNT;
          CONFIG(2) = LED_OFFSET_S;
          CONFIG(3) = LED_OFFSET_M;
          CONFIG(4) = LED_OFFSET_H;
        }

        addFormSubHeader(F("Clock Settings"));

        int16_t choice = CONFIG(0);
        String options[2] = { F("none"), F("Ring") };
        addFormSelector(F("Clock Type"), F("clocktype"), 2, options, NULL, choice);

        addFormNumericBox(F("Number of LEDs"), F("clockleds"), CONFIG(1), 12, 1000);
        addFormNumericBox(F("LED Offset S"), F("clockoffsets"), CONFIG(2), 0, 1000);
        addFormNumericBox(F("LED Offset M"), F("clockoffsetm"), CONFIG(3), 0, 1000);
        addFormNumericBox(F("LED Offset H"), F("clockoffseth"), CONFIG(4), 0, 1000);
        
        addFormNumericBox(F("Seg. for Colon"), F("clocksegcol"), CONFIG(6), -1, 7);
        addHtml(F(" Value "));
        addNumericBox(F("clocksegcolval"), CONFIG(7), 0, 255);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //CONFIG(0) = getFormItemInt(F("i2c_addr"));

        CONFIG(0) = getFormItemInt(F("clocktype"));

        CONFIG(1) = getFormItemInt(F("clockleds"));
        CONFIG(2) = getFormItemInt(F("clockoffsets"));
        CONFIG(3) = getFormItemInt(F("clockoffsetm"));
        CONFIG(4) = getFormItemInt(F("clockoffseth"));

        CONFIG(6) = getFormItemInt(F("clocksegcol"));
        CONFIG(7) = getFormItemInt(F("clocksegcolval"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        byte type = CONFIG(0);

        if (!Plugin_222_S)
          Plugin_222_S = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>(LED_COUNT);

        //Plugin_222_S->();
        Plugin_222_S->Begin();
        Plugin_222_S->Show();

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        //if (!Plugin_222_M)
        //  return false;

        success = true;

        break;
      }

    case PLUGIN_CLOCK_IN:
    case PLUGIN_ONCE_A_SECOND:
      {
        //if (!Plugin_222_M || CONFIG(1) == 0)
        //  break;

        byte s = second();
        byte m = minute();
        byte h = hour();
        byte i, x;

        s = (s+59) % 60;
        m = (m+59) % 60;
        h = (h+23) % 24;

        Plugin_222_S->ClearTo(RgbColor(0, 0, 0));

        for (i=0; i<60; i+=5)
        {
          x = (i+59) % 60;
          Plugin_222_S->SetPixelColor(x+LED_OFFSET_S, RgbColor(0, 2, 2));
          Plugin_222_S->SetPixelColor(x+LED_OFFSET_M, RgbColor(0, 2, 2));
        }
        for (i=0; i<60; i+=15)
        {
          x = (i+59) % 60;
          Plugin_222_S->SetPixelColor(x+LED_OFFSET_S, RgbColor(0, 15, 15));
          Plugin_222_S->SetPixelColor(x+LED_OFFSET_M, RgbColor(0, 15, 15));
        }
        for (i=0; i<LED_COUNT_H; i+=3)
        {
          x = (i+23) % 24;
          Plugin_222_S->SetPixelColor(x+LED_OFFSET_H, RgbColor(0, 2, 2));
        }
        for (i=0; i<LED_COUNT_H; i+=12)
        {
          x = (i+23) % 24;
          Plugin_222_S->SetPixelColor(x+LED_OFFSET_H, RgbColor(0, 15, 15));
        }
        Plugin_222_S->SetPixelColor(s+LED_OFFSET_S, RgbColor(LED_LUM_ON, 0, 0));
        Plugin_222_S->SetPixelColor(m+LED_OFFSET_M, RgbColor(LED_LUM_ON, 0, 0));
        Plugin_222_S->SetPixelColor(h+LED_OFFSET_H, RgbColor(LED_LUM_ON, 0, 0));
        Plugin_222_S->Show();
        

        unsigned long t = now();  // get local time in int secs
        t %= 60*60*24;

        //uint16_t f = millis() - prevMillis;

        float ft = t + (millis() - prevMillis) * 0.001;

        float fs = ft / 60.0;
        float fm = fs / 60.0;
        float fh12 = fm / 12.0;
        float fh24 = fm / 24.0;
        fs -= int(fs);
        fm -= int(fm);
        fh12 -= int(fh12);
        fh24 -= int(fh24);

        String log = F("neoC : s=");
        log += String(fs);
        log += F(" m=");
        log += String(fm);
        log += F(" h=");
        log += String(fh12);
        addLog(LOG_LEVEL_INFO, log);

        //Plugin_222_M->ClearRowBuffer();
        //Plugin_222_M->SetDigit(CONFIG(3), hours%10);
        //Plugin_222_M->SetDigit(CONFIG(4), minutes/10);
        //Plugin_222_M->SetDigit(CONFIG(5), minutes%10);
        //if (CONFIG(6) >= 0)
        //  Plugin_222_M->SetRow(CONFIG(6), CONFIG(7));
        //Plugin_222_M->TransmitRowBuffer();

        success = true;

        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        //if (!Plugin_222_M || CONFIG(1) == 0)   //clock enabled?
        //  break;

        if (CONFIG(6) >= 0)   //colon used?
        {
          uint8_t act = ((uint16_t)millis() >> 9) & 1;   //blink with about 2 Hz
          static uint8_t last = 0;
          if (act != last)
          {
            last = act;
            //Plugin_222_M->SetRow(CONFIG(6), (act) ? CONFIG(7) : 0);
            //Plugin_222_M->TransmitRowBuffer();
          }
        }
      }

  }
  return success;
}

//#endif // USES_P222
