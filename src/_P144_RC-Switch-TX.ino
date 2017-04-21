//#######################################################################################################
//#################################### Plugin 144: RC-Switch TX #########################################
//#######################################################################################################
// written by Jochen Krapf (jk@nerd2nerd.org)

// List of commands:
// (1) RC,<param>

// List of RC params:
// (a) SEND=<binary_code>

// Usage:
// (1a): Send 24 bit binary code ("RC,SEND=000000000001010100010001")

#include <RCSwitch.h>   //https://github.com/sui77/rc-switch.git

static RCSwitch Plugin_144_RC = RCSwitch();

#define PLUGIN_144
#define PLUGIN_ID_144         144
#define PLUGIN_NAME_144       "RC-Switch TX"


boolean Plugin_144(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_144;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
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
        string = F(PLUGIN_NAME_144);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        string += F("<TR><TD>TEST<TD>");
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        int pin = Settings.TaskDevicePin1[event->TaskIndex];
        if (pin >= 0)
        {
          String log = F("RC-Sw: Pin ");
          log += pin;
          log += F(" ");

          Plugin_144_RC.enableTransmit(pin);

          addLog(LOG_LEVEL_INFO, log);
        }


        if (Settings.TaskDevicePin2[event->TaskIndex] >= 0)
          pinMode(Settings.TaskDevicePin2[event->TaskIndex], OUTPUT);

        for (byte i=0; i<3; i++)
          if (Settings.TaskDevicePin[i][event->TaskIndex] >= 0)
            pinMode(Settings.TaskDevicePin[i][event->TaskIndex], OUTPUT);

        Settings.TaskDevicePin2[event->TaskIndex] = 42;
        if (Settings.TaskDevicePin[1][event->TaskIndex]==42)
          addLog(LOG_LEVEL_INFO, "_42_");
        Settings.TaskDevicePin3[11] = 23;
        if (Settings.TaskDevicePin[2][11]==23)
          addLog(LOG_LEVEL_INFO, "_23_");

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);

        if (command == F("rc"))
        {
          String param;
          byte paramIdx = 2;

          string.replace("  ", " ");
          string.replace(" =", "=");
          string.replace("= ", "=");

          param = parseString(string, paramIdx++);
          while (param.length())
          {
            addLog(LOG_LEVEL_INFO, param);

            int index = param.indexOf('=');
            if (index > 0)
            {
              String paramKey = param.substring(0, index);
              String paramVal = param.substring(index+1);
              paramKey.toLowerCase();

              addLog(LOG_LEVEL_INFO, paramKey);
              addLog(LOG_LEVEL_INFO, paramVal);

              if (paramKey == F("send"))
                {
                  Plugin_144_RC.send("000000000001010100010001");
                }
            }

            param = parseString(string, paramIdx++);
          }

          success = true;
        }

        break;
      }

    case PLUGIN_READ:
      {
        //no values
        success = true;
        break;
      }

  }
  return success;
}
