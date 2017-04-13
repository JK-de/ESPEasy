//#######################################################################################################
//#################################### Plugin 038: NeoPixel Basic #######################################
//#######################################################################################################

// List of commands:
// (1) RGB,<red 0-255>,<green 0-255>,<blue 0-255>
// (2) HSV,<hue 0-360>,<saturation 0-100>,<value;brightness 0-100>

// Usage:
// (1): Set RGB Color to specified LED number (eg. NeoPixel,5,255,255,255)
// (2): Set all LED to specified color (eg. NeoPixelAll,255,255,255)
//		If you use 'NeoPixelAll' this will off all LED (like NeoPixelAll,0,0,0)
// (3): Set color LED between <start led nr> and <stop led nr> to specified color (eg. NeoPixelLine,1,6,255,255,255)

//#include <Adafruit_NeoPixel.h>
//Adafruit_NeoPixel *Plugin_142_pixels;

static float Plugin_142_hsvPrev[3] = {0,0,0};
static float Plugin_142_hsvDest[3] = {0,0,0};
static long millisFadeBegin = 0;
static long millisFadeEnd = 0;
static long millisFadeTime = 1000;

#define PLUGIN_142
#define PLUGIN_ID_142         142
#define PLUGIN_NAME_142       "RGB-Strip"
#define PLUGIN_VALUENAME1_142 "H"
#define PLUGIN_VALUENAME2_142 "S"
#define PLUGIN_VALUENAME3_142 "V"

boolean Plugin_142(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_142;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;           // Nothing else really fit the bill ...
        Device[deviceCount].Ports = 0;
        //Device[deviceCount].Ports = 3;
        //Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        //Device[deviceCount].Custom = true;
        Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = true;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_142);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_142));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_142));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_142));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        char tmpString[128];

        string += F("<TR><TD>GPIO:<TD>");

        string += F("<TR><TD>1st GPIO (R):<TD>");
        addPinSelect(false, string, "taskdevicepin1", Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += F("<TR><TD>2nd GPIO (G):<TD>");
        addPinSelect(false, string, "taskdevicepin2", Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        string += F("<TR><TD>3rd GPIO (B):<TD>");
        addPinSelect(false, string, "taskdevicepin3", Settings.TaskDevicePluginConfig[event->TaskIndex][2]);
        string += F("<TR><TD>4th GPIO (W) optional:<TD>");
        addPinSelect(false, string, "taskdeviceport", Settings.TaskDevicePluginConfig[event->TaskIndex][3]);

        sprintf_P(tmpString, PSTR("<TR><TD>Led Count:<TD><input type='text' name='plugin_142_leds' size='3' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += tmpString;


        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin2 = WebServer.arg("taskdevicepin1");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin2.toInt();
        String plugin3 = WebServer.arg("taskdevicepin2");
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin3.toInt();
        String plugin4 = WebServer.arg("taskdevicepin3");
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = plugin4.toInt();
        String plugin5 = WebServer.arg("taskdeviceport");
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = plugin5.toInt();

        for (byte i=0; i<4; i++)
          if (Settings.TaskDevicePluginConfig[event->TaskIndex][3] >= 16)
            Settings.TaskDevicePluginConfig[event->TaskIndex][3] = -1;

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        for (byte i=0; i<4; i++)
        {
          int pin = Settings.TaskDevicePluginConfig[event->TaskIndex][i];
          if (pin >= 0)
            pinMode(pin, OUTPUT);
        }
        Output(event->TaskIndex, Plugin_142_hsvDest);
        //if (!Plugin_142_pixels)
        {
          //Plugin_142_pixels = new Adafruit_NeoPixel(Settings.TaskDevicePluginConfig[event->TaskIndex][0], Settings.TaskDevicePin1[event->TaskIndex], NEO_GRB + NEO_KHZ800);
          //Plugin_142_pixels->begin(); // This initializes the NeoPixel library.
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        bool bNewValue = false;

        String command = parseString(string, 1);

        if (command == F("rgb"))
        {
          float rgb[3];
          rgb[0] = event->Par1 / 255.0;   //R
          rgb[1] = event->Par2 / 255.0;   //G
          rgb[2] = event->Par3 / 255.0;   //B
          hsvClamp(rgb);
          rgb2hsv(rgb, Plugin_142_hsvDest);
          bNewValue = true;
        }

        if (command == F("hsv"))
        {
          Plugin_142_hsvDest[0] = event->Par1 / 360.0;   //Hue
          Plugin_142_hsvDest[1] = event->Par2 / 100.0;   //Saturation
          Plugin_142_hsvDest[2] = event->Par3 / 100.0;   //Value/Brightness
          bNewValue = true;
        }

        if (command == F("hue"))
        {
          Plugin_142_hsvDest[0] = event->Par1 / 360.0;   //Hue
          bNewValue = true;
        }

        if (command == F("dimm"))
        {
          Plugin_142_hsvDest[2] = event->Par1 / 100.0;   //Value/Brightness
          bNewValue = true;
        }

        if (command == F("off"))
        {
          Plugin_142_hsvDest[2] = 0.0;   //Value/Brightness
          bNewValue = true;
        }

        if (bNewValue)
        {
          hsvClamp(Plugin_142_hsvDest);
          hsvClamp(Plugin_142_hsvPrev);

          //get the shortest way around the color circle
          if ((Plugin_142_hsvDest[0]-Plugin_142_hsvPrev[0]) > 0.5)
            Plugin_142_hsvPrev[0] += 1.0;
          if ((Plugin_142_hsvDest[0]-Plugin_142_hsvPrev[0]) < -0.5)
            Plugin_142_hsvPrev[0] -= 1.0;

          millisFadeBegin = millis();
          millisFadeEnd = millisFadeBegin + millisFadeTime;

          String log = F("hsv:  ");
          for (byte i=0; i<3; i++)
          {
            log += toString(Plugin_142_hsvDest[i], 3);
            log += F(" ");
          }
          addLog(LOG_LEVEL_INFO, log);

          Output(event->TaskIndex, Plugin_142_hsvDest);
          success = true;
        }
        break;
      }

    case PLUGIN_READ:
      {
        UserVar[event->BaseVarIndex + 0] = Plugin_142_hsvDest[0] * 360.0;
        UserVar[event->BaseVarIndex + 1] = Plugin_142_hsvDest[1] * 100.0;
        UserVar[event->BaseVarIndex + 2] = Plugin_142_hsvDest[2] * 100.0;
        success = true;

        //String log = F("RGBW:  ");
        //log += i;
        //log += F(" bytes to attempt to fix buffer alignment");
        //log += String(mhzResp[0], HEX);
        //log += F(" ");
        //addLog(LOG_LEVEL_ERROR, log);
        //addLog(LOG_LEVEL_INFO, log);
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      break;
      {
        if (millisFadeEnd != 0)   //fading required?
        {
          float hsv[3];
          long millisAct = millis();

          if (millisAct >= millisFadeEnd)   //destination reached?
          {
            millisFadeBegin = 0;
            millisFadeEnd = 0;
            for (byte i=0; i<3; i++)
              hsv[i] = Plugin_142_hsvPrev[i] = Plugin_142_hsvDest[i];
          }
          else   //just fading
          {
            float fade = (millisAct-millisFadeBegin) / (millisFadeEnd-millisFadeBegin);

            fade = 1.0-fade;   //smooth fading out
            fade *= fade;
            fade = 1.0-fade;

            for (byte i=0; i<3; i++)
              hsv[i] = mix(Plugin_142_hsvPrev[i], Plugin_142_hsvDest[i], fade);
          }

          Output(event->TaskIndex, hsv);
        }
        success = true;
        break;
      }

/*
        #define PLUGIN_INIT_ALL                     1
        #define PLUGIN_INIT                         2
        #define PLUGIN_READ                         3
        #define PLUGIN_ONCE_A_SECOND                4
        #define PLUGIN_TEN_PER_SECOND               5
        #define PLUGIN_DEVICE_ADD                   6
        #define PLUGIN_EVENTLIST_ADD                7
        #define PLUGIN_WEBFORM_SAVE                 8
        #define PLUGIN_WEBFORM_LOAD                 9
        #define PLUGIN_WEBFORM_SHOW_VALUES         10
        #define PLUGIN_GET_DEVICENAME              11
        #define PLUGIN_GET_DEVICEVALUENAMES        12
        #define PLUGIN_WRITE                       13
        #define PLUGIN_EVENT_OUT                   14
        #define PLUGIN_WEBFORM_SHOW_CONFIG         15
        #define PLUGIN_SERIAL_IN                   16
        #define PLUGIN_UDP_IN                      17
        #define PLUGIN_CLOCK_IN                    18
        #define PLUGIN_TIMER_IN                    19
        #define PLUGIN_FIFTY_PER_SECOND            20
        #define PLUGIN_REMOTE_CONFIG               21
        */


  }
  return success;
}

void Output(int TaskIndex, float* hsvIn)
{
  float hsvw[4];
  float rgbw[4];

  for (byte i=0; i<3; i++)
    hsvw[i] = hsvIn[i];

  hsvClamp(hsvw);

  if (0)   //has white channel?
  {
    hsvw[3] = (1.0 - hsvw[1]) * hsvw[2];   // w = (1-s)*v
    hsvw[2] *= hsvw[1];   // v = s*v
  }
  else
    hsvw[3] = 0.0;


  hsv2rgb(hsvw, rgbw);
  rgbw[3] = hsvw[3];

  int actRGBW[4];
  static int lastRGBW[4] = {-1,-1,-1,-1};

  String log = F("RGBW:  ");

  for (byte i=0; i<4; i++)
  {
    int pin = Settings.TaskDevicePluginConfig[TaskIndex][i];
    if (pin >= 0)
    {
      rgbw[i] *= rgbw[i];   //simple gamma correction

      actRGBW[i] = rgbw[i] * PWMRANGE + 0.5;

      log += String(rgbw[i], DEC);
      log += F(" ");

      if (Settings.TaskDevicePin1Inversed[TaskIndex])
        actRGBW[i] = PWMRANGE - actRGBW[i];

      if (lastRGBW[i] != actRGBW[i])
      {
        lastRGBW[i] = actRGBW[i];

        analogWrite(pin, actRGBW[i]);
        setPinState(PLUGIN_ID_142, pin, PIN_MODE_PWM, actRGBW[i]);
      }
    }
  }

  addLog(LOG_LEVEL_INFO, log);

}

// HSV->RGB conversion based on GLSL version
// expects hsv channels defined in 0.0 .. 1.0 interval
float fract(float x) { return x - int(x); }

float mix(float a, float b, float t) { return a + (b - a) * t; }

float step(float e, float x) { return x < e ? 0.0 : 1.0; }

float* hsv2rgb(const float* hsv, float* rgb)
{
  rgb[0] = hsv[2] * mix(1.0, constrain(abs(fract(hsv[0] + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), hsv[1]);
  rgb[1] = hsv[2] * mix(1.0, constrain(abs(fract(hsv[0] + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), hsv[1]);
  rgb[2] = hsv[2] * mix(1.0, constrain(abs(fract(hsv[0] + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), hsv[1]);
  return rgb;
}

float* rgb2hsv(const float* rgb, float* hsv)
{
  float s = step(rgb[2], rgb[1]);
  float px = mix(rgb[2], rgb[1], s);
  float py = mix(rgb[1], rgb[2], s);
  float pz = mix(-1.0, 0.0, s);
  float pw = mix(0.6666666, -0.3333333, s);
  s = step(px, rgb[0]);
  float qx = mix(px, rgb[0], s);
  float qz = mix(pw, pz, s);
  float qw = mix(rgb[0], px, s);
  float d = qx - std::min(qw, py);
  hsv[0] = abs(qz + (qw - py) / (6.0 * d + 1e-10));
  hsv[1] = d / (qx + 1e-10);
  hsv[2] = qx;
  return hsv;
}

float* hsvClamp(float* hsv)
{
  while (hsv[0] > 1.0)
    hsv[0] -= 1.0;
  while (hsv[0] < 0.0)
    hsv[0] += 1.0;

  for (byte i=1; i<3; i++)
  {
    if (hsv[i] < 0.0)
      hsv[i] = 0.0;
    if (hsv[i] > 1.0)
      hsv[i] = 1.0;
  }
  return hsv;
}
