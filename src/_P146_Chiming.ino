//#######################################################################################################
//#################################### Plugin 146: RGB-Strip ############################################
//#######################################################################################################
// written by Jochen Krapf (jk@nerd2nerd.org)

// List of commands:
// (1) RGB,<red 0-255>,<green 0-255>,<blue 0-255>

// Usage:
// (1): Set RGB Color to LED (eg. RGB,255,255,255)

//#include <*.h>   - no external lib required

static long Plugin_146_millisChimeEnd = 0;
static long Plugin_146_millisPauseEnd = 0;

static long Plugin_146_millisChimeTime = 200;
static long Plugin_146_millisPauseTime = 1000;

#define PLUGIN_146_FIFO_SIZE 32   // must be power of 2
#define PLUGIN_146_FIFO_MASK (PLUGIN_146_FIFO_SIZE-1)

static char Plugin_146_FIFO[PLUGIN_146_FIFO_SIZE];
static byte Plugin_146_FIFO_IndexR = 0;
static byte Plugin_146_FIFO_IndexW = 0;

static int Plugin_146_pin[3] = {-1,-1,-1};
static int Plugin_146_lowActive = false;

#define PLUGIN_146
#define PLUGIN_ID_146         146
#define PLUGIN_NAME_146       "Chiming Mechanism"
#define PLUGIN_VALUENAME1_146 "H"
#define PLUGIN_VALUENAME2_146 "S"
#define PLUGIN_VALUENAME3_146 "V"



boolean Plugin_146(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_146;
        Device[deviceCount].Type = DEVICE_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = true;
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
        string = F(PLUGIN_NAME_146);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        char tmpString[128];

        string += F("<TR><TD>GPIO:<TD>");

        string += F("<TR><TD>Chiming Time [ms]:<TD>");
        addPinSelect(false, string, "chimetime", Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += F("<TR><TD>Pause Time [ms]:<TD>");
        addPinSelect(false, string, "pausetime", Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        string += F("<TR><TD>xxx:<TD>");
        addPinSelect(false, string, "xxx", Settings.TaskDevicePluginConfig[event->TaskIndex][2]);
        string += F("<TR><TD>yyy:<TD>");
        addPinSelect(false, string, "yyy", Settings.TaskDevicePluginConfig[event->TaskIndex][3]);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin2 = WebServer.arg("chimetime");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_146_millisChimeTime = plugin2.toInt();
        String plugin3 = WebServer.arg("pausetime");
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = Plugin_146_millisPauseTime = plugin3.toInt();
        String plugin4 = WebServer.arg("xxx");
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = plugin4.toInt();
        String plugin5 = WebServer.arg("yyy");
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = plugin5.toInt();

        for (byte i=0; i<3; i++)
          Plugin_146_pin[i] = Settings.TaskDevicePin[event->TaskIndex][i];

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        String log = F("Chime: Pin ");
        for (byte i=0; i<3; i++)
        {
          int pin = Settings.TaskDevicePin[event->TaskIndex][i];
          Plugin_146_pin[i] = pin;
          if (pin >= 0)
            pinMode(pin, OUTPUT);
          log += pin;
          log += F(" ");
        }
        Plugin_146_lowActive = Settings.TaskDevicePin1Inversed[event->TaskIndex];
        addLog(LOG_LEVEL_INFO, log);


        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        bool bNewValue = false;

        String command = parseString(string, 1);

        if (command == F("chime"))
        {
          String param = parseString(string, 2);
          byte i = 0;
          while (param[i] != 0)
          {
            Plugin_146_WriteFIFO(param[i++]);
          }
          success = true;
        }

        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
    //case PLUGIN_TEN_PER_SECOND:
      {
        long millisAct = millis();

        if (Plugin_146_millisChimeEnd > 0)   // just striking?
        {
          if (Plugin_146_millisChimeEnd < millisAct)   // end reached?
          {
            for (byte i=0; i<3; i++)
            {
              if (Plugin_146_pin[i] >= 0)
                digitalWrite(Plugin_146_pin[i], 0);
            }
            Plugin_146_millisChimeEnd = 0;
          }
        }
        else if (Plugin_146_millisPauseEnd > 0)   // just waiting?
        {
          if (Plugin_146_millisPauseEnd < millisAct)   // end reached?
          {
            Plugin_146_millisPauseEnd = 0;
          }
        }
        else
        {
          if (! Plugin_146_IsEmptyFIFO())
          {
            char c = Plugin_146_ReadFIFO();
            Plugin_146_millisChimeEnd = millisAct + 100;
            Plugin_146_millisPauseEnd = millisAct + 1000;

            if (c >= '1' && c <= '7')
            {
              byte mask = 1;
              for (byte i=0; i<3; i++)
              {
                if (Plugin_146_pin[i] >= 0)
                  if (c & mask)
                    digitalWrite(Plugin_146_pin[i], 1);
                mask <<= 1;
              }
            }
          }

        }
        success = true;
        break;
      }

  }
  return success;
}

void Plugin_146_WriteFIFO(char c)
{
  if (Plugin_146_FIFO_IndexR == ((Plugin_146_FIFO_IndexW+1) & PLUGIN_146_FIFO_MASK))
    return;

  Plugin_146_FIFO[Plugin_146_FIFO_IndexW] = c;
  Plugin_146_FIFO_IndexW++;
  Plugin_146_FIFO_IndexW &= PLUGIN_146_FIFO_MASK;
}

char Plugin_146_ReadFIFO()
{
  if (Plugin_146_IsEmptyFIFO())
    return '\0';

  char c = Plugin_146_FIFO[Plugin_146_FIFO_IndexR];
  Plugin_146_FIFO_IndexR++;
  Plugin_146_FIFO_IndexR &= PLUGIN_146_FIFO_MASK;

  return c;
}

boolean Plugin_146_IsEmptyFIFO()
{
  return (Plugin_146_FIFO_IndexR == Plugin_146_FIFO_IndexW);
}
