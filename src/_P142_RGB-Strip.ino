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

float Plugin_142_hsvAct[3] = {0,0,0};
float Plugin_142_hsvDest[3] = {0,0,0};

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

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
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
          clamp(rgb);
          rgb2hsv(rgb, Plugin_142_hsvDest)
          bNewValue = true;
          success = true;
        }

        if (command == F("hsv"))
        {
          Plugin_142_hsvDest[0] = event->Par1 / 360.0;   //Hue
          Plugin_142_hsvDest[1] = event->Par2 / 100.0;   //Saturation
          Plugin_142_hsvDest[2] = event->Par3 / 100.0;   //Value/Brightness
          clamp(Plugin_142_hsvDest);
          bNewValue = true;
          success = true;
        }

        if (bNewValue)
        {
          //analogWrite(event->Par1, event->Par2);
          //setPinState(PLUGIN_ID_142, event->Par1, PIN_MODE_PWM, event->Par2);
        }
        break;
      }
  }
  return success;
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

float* clamp(float* hsv)
{
  for (byte i=0; i<3; i++)
  {
    if (hsv[i] < 0.0)
      hsv[i] = 0.0;
    if (hsv[i] > 1.0)
      hsv[i] = 1.0;
  }
  return hsv;
}
