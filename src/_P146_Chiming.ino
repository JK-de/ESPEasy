//#######################################################################################################
//#################################### Plugin 146: RGB-Strip ############################################
//#######################################################################################################
// written by Jochen Krapf (jk@nerd2nerd.org)

// List of commands:
// (1) RGB,<red 0-255>,<green 0-255>,<blue 0-255>

// Usage:
// (1): Set RGB Color to LED (eg. RGB,255,255,255)

//#include <*.h>   - no external lib required

static long Plugin_146_millisStateEnd = 0;

static long Plugin_146_millisChimeTime = 150;
static long Plugin_146_millisPauseTime = 750;

#define PLUGIN_146_FIFO_SIZE 64   // must be power of 2
#define PLUGIN_146_FIFO_MASK (PLUGIN_146_FIFO_SIZE-1)

static char Plugin_146_FIFO[PLUGIN_146_FIFO_SIZE];
static byte Plugin_146_FIFO_IndexR = 0;
static byte Plugin_146_FIFO_IndexW = 0;

static int Plugin_146_pin[3] = {-1,-1,-1};
static byte Plugin_146_lowActive = false;
static byte Plugin_146_chimeClock = true;

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

        //default values
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] <= 0)   //Plugin_146_millisChimeTime
          Settings.TaskDevicePluginConfig[event->TaskIndex][0] = 100;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] <= 0)   //Plugin_146_millisPauseTime
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = 500;

        sprintf_P(tmpString, PSTR("<TR><TD>Chiming Time [ms]:<TD><input type='text' name='chimetime' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += tmpString;

        sprintf_P(tmpString, PSTR("<TR><TD>Pause Time [ms]:<TD><input type='text' name='pausetime' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        string += tmpString;

        string += F("<TR><TD>Hourly Chiming Clock Strike:<TD><input type=checkbox id='chimeclock' name='chimeclock'");
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][2])
          string += F(" checked");
        string += F(">&nbsp;");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin0 = WebServer.arg(F("chimetime"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_146_millisChimeTime = plugin0.toInt();

        String plugin1 = WebServer.arg(F("pausetime"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = Plugin_146_millisPauseTime = plugin1.toInt();

        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = (WebServer.arg(F("chimeclock")) == "on");

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_146_lowActive = Settings.TaskDevicePin1Inversed[event->TaskIndex];
        Plugin_146_chimeClock = Settings.TaskDevicePluginConfig[event->TaskIndex][2];

        String log = F("Chime: GPIO ");
        for (byte i=0; i<3; i++)
        {
          int pin = Settings.TaskDevicePin[event->TaskIndex][i];
          Plugin_146_pin[i] = pin;
          if (pin >= 0)
          {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, Plugin_146_lowActive);
          }
          log += pin;
          log += F(" ");
        }
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
          int paramPos = getParamStartPos(string, 2);
          String param = string.substring(paramPos);
          Plugin_146_AddStringFIFO(param);
          success = true;
        }
        if (command == F("chimeplay"))
        {
          String name = parseString(string, 2);
          String param;
          Plugin_146_ReadChime(name, param);
          Plugin_146_AddStringFIFO(param);
          success = true;
        }
        if (command == F("chimesave"))
        {
          String name = parseString(string, 2);
          int paramPos = getParamStartPos(string, 3);
          String param = string.substring(paramPos);
          Plugin_146_WriteChime(name, param);
          //Plugin_146_AddStringFIFO(param);
          success = true;
        }

        break;
      }

      case PLUGIN_CLOCK_IN:
        {
          // static String clockChimes = "1112,1121,1122,1211,1212,1221,1222,2111,2112,2121,2122,2211";   //1..12
          // static String clockChimes = "1-1-1-11,1-1-11-1,1-1-111,1-11-1-1,1-11-11,1-111-1,1-1111,11-1-1-1,11-1-11,11-11-1,11-111,111-1-1";   //1..12
          static String clockChimes = "1111!,111!1,111!1!,11!11,11!11!,11!1!1,11!1!1!,1!111,1!111!,1!11!1,1!11!1!,1!1!11";   //1..12
          byte hours = hour();
          byte minutes = minute();

          //TODO
          if (Plugin_146_chimeClock)
          {
            Plugin_146_ReadChime("hours", clockChimes);

            // hours 0..23 -> 1..12
            hours = hours % 12;
            if (hours == 0)
              hours = 12;

            //byte index = hours;
            byte index = (minutes % 10);

            String param = parseString(clockChimes, index);
            Plugin_146_AddStringFIFO(param);
          }

          success = true;
          break;
        }


    case PLUGIN_FIFTY_PER_SECOND:
    //case PLUGIN_TEN_PER_SECOND:
      {
        long millisAct = millis();

        if (Plugin_146_millisStateEnd > 0)   // just striking?
        {
          if (Plugin_146_millisStateEnd <= millisAct)   // end reached?
          {
            for (byte i=0; i<3; i++)
            {
              if (Plugin_146_pin[i] >= 0)
                digitalWrite(Plugin_146_pin[i], Plugin_146_lowActive);
            }
            Plugin_146_millisStateEnd = 0;
          }
        }

        if (Plugin_146_millisStateEnd == 0)   // just finished?
        {
          if (! Plugin_146_IsEmptyFIFO())
          {
            char c = Plugin_146_ReadFIFO();

            String log = F("Chime: Process '");
            log += c;
            log += "'";
            addLog(LOG_LEVEL_DEBUG, log);

            Plugin_146_millisStateEnd = millisAct + Plugin_146_millisChimeTime;

            switch (c)
            {
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
              {
                byte mask = 1;
                for (byte i=0; i<3; i++)
                {
                  if (Plugin_146_pin[i] >= 0)
                    if (c & mask)
                      digitalWrite(Plugin_146_pin[i], !Plugin_146_lowActive);
                  mask <<= 1;
                }
                break;
              }
              case '=':   //long pause
                Plugin_146_millisStateEnd = millisAct + Plugin_146_millisPauseTime*3;
                break;
              case '-':
              case ' ':   //single pause
                Plugin_146_millisStateEnd = millisAct + Plugin_146_millisPauseTime;
                break;
              case '.':   //short pause
                Plugin_146_millisStateEnd = millisAct + Plugin_146_millisPauseTime/3;
                break;
              case '|':   //shortest pause
              default:
                //do nothing
                break;
            }
          }

        }
        success = true;
        break;
      }

  }
  return success;
}

// FIFO functions

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

char Plugin_146_PeekFIFO()
{
  if (Plugin_146_IsEmptyFIFO())
    return '\0';

  return Plugin_146_FIFO[Plugin_146_FIFO_IndexR];
}

boolean Plugin_146_IsEmptyFIFO()
{
  return (Plugin_146_FIFO_IndexR == Plugin_146_FIFO_IndexW);
}

void Plugin_146_AddStringFIFO(const String& param)
{
  if (param.length() == 0)
    return;

  byte i = 0;
  char c = param[i];
  char c_last = '\0';

  while (c != 0)
  {
    if (Plugin_146_IsNumeric(c) && Plugin_146_IsNumeric(c_last))   // "11" is shortcut for "1-1" -> add pause
      Plugin_146_WriteFIFO('-');
    if (c == '!')   //double strike -> add shortest pause and repeat last strike
    {
      Plugin_146_WriteFIFO('|');
      c = c_last;
    }
    Plugin_146_WriteFIFO(c);
    c_last = c;

    c = param[++i];
  }

  Plugin_146_WriteFIFO('=');
}

boolean Plugin_146_IsNumeric(char c)
{
  if (c >= '0' && c <= '9')
    return true;
  return false;
}

//File I/O functions

void Plugin_146_WriteChime(const String& name, const String& param)
{
  String fileName = F("chime_");
  fileName += name;
  fileName += F(".txt");

  String log = F("Chime: write ");
  log += fileName;
  log += F(" ");

  fs::File f = SPIFFS.open(fileName, "w");
  if (f)
  {
    f.print(param);
    f.close();
    //flashCount();
    log += param;
  }

  addLog(LOG_LEVEL_INFO, log);
}

void Plugin_146_ReadChime(const String& name, String& param)
{
  String fileName = F("chime_");
  fileName += name;
  fileName += F(".txt");

  String log = F("Chime: read ");
  log += fileName;
  log += F(" ");

  param = "";
  fs::File f = SPIFFS.open(fileName, "r+");
  if (f)
  {
    char c;
    while (f.available())
    {
      c = f.read();
      param += c;
    }
    f.close();

    log += param;
  }

  addLog(LOG_LEVEL_INFO, log);
}
