#pragma once

#include "wled.h"
#include "digits.h"

#define PHOTORESISTOR_PIN A0
#define LEDS_NO 127

// the frequency to check photoresistor, 1 seconds
#ifndef USERMOD_SN_PHOTORESISTOR_MEASUREMENT_INTERVAL
#define USERMOD_SN_PHOTORESISTOR_MEASUREMENT_INTERVAL 1000
#endif

// how many seconds after boot to take first measurement, 1 seconds
#ifndef USERMOD_SN_PHOTORESISTOR_FIRST_MEASUREMENT_AT
#define USERMOD_SN_PHOTORESISTOR_FIRST_MEASUREMENT_AT 1000
#endif

// supplied voltage
#ifndef USERMOD_SN_PHOTORESISTOR_REFERENCE_VOLTAGE
#define USERMOD_SN_PHOTORESISTOR_REFERENCE_VOLTAGE 1
#endif

// 10 bits
#ifndef USERMOD_SN_PHOTORESISTOR_ADC_PRECISION
#define USERMOD_SN_PHOTORESISTOR_ADC_PRECISION 1024.0f
#endif

// resistor size 10K hms
#ifndef USERMOD_SN_PHOTORESISTOR_RESISTOR_VALUE
#define USERMOD_SN_PHOTORESISTOR_RESISTOR_VALUE 6800.0f
#endif

// only report if differance grater than offset value
#ifndef USERMOD_SN_PHOTORESISTOR_OFFSET_VALUE
#define USERMOD_SN_PHOTORESISTOR_OFFSET_VALUE 5
#endif
/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This is an example for a v2 usermod.
 * v2 usermods are class inheritance based and can (but don't have to) implement more functions, each of them is shown in this example.
 * Multiple v2 usermods can be added to one compilation easily.
 * 
 * Creating a usermod:
 * This file serves as an example. If you want to create a usermod, it is recommended to use usermod_v2_empty.h from the usermods folder as a template.
 * Please remember to rename the class and file to a descriptive name.
 * You may also use multiple .h and .cpp files.
 * 
 * Using a usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding #include "usermod_filename.h" in the top and registerUsermod(new MyUsermodClass()) in the bottom of usermods_list.cpp
 */

//class name. Use something descriptive and leave the ": public Usermod" part :)
class HexaClock : public Usermod {
  private:

  float referenceVoltage = USERMOD_SN_PHOTORESISTOR_REFERENCE_VOLTAGE;
  float resistorValue = USERMOD_SN_PHOTORESISTOR_RESISTOR_VALUE;
  float adcPrecision = USERMOD_SN_PHOTORESISTOR_ADC_PRECISION;
  int8_t offset = USERMOD_SN_PHOTORESISTOR_OFFSET_VALUE;

  unsigned long readingInterval = USERMOD_SN_PHOTORESISTOR_MEASUREMENT_INTERVAL;
  // set last reading as "40 sec before boot", so first reading is taken after 20 sec
  unsigned long lastMeasurement = UINT32_MAX - (USERMOD_SN_PHOTORESISTOR_MEASUREMENT_INTERVAL - USERMOD_SN_PHOTORESISTOR_FIRST_MEASUREMENT_AT);
  // flag to indicate we have finished the first getTemperature call
  // allows this library to report to the user how long until the first
  // measurement
  bool getLuminanceComplete = false;
  uint16_t lastLuminance = 0;

    //Private class members. You can declare variables and functions only accessible to your usermod here
    unsigned long lastTime = 0;

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    bool displayClock = true;
    bool ledmapEnabled = true;
    uint8_t reverseRoundMap[LEDS_NO];
    uint8_t reverseVerticalMap[LEDS_NO];
    bool autoBrightnessEnabled = true;
    float autoBrightnessACoeff = 0.5;
    float autoBrightnessBCoeff = 0.0;
    byte autoBrightnessMinBri = 10;
    bool powerOn = true;
    bool nightModeEnabled = true;
    bool nightModeOn = false;
    uint16_t nightModeThreshold = 10;
    byte nightModeBri = 1;
    
    bool digitWhite = true;
    bool reverseDigits = false;

    byte prevPreset;
    int16_t prevPlaylist;

    // These config variables have defaults set inside readFromConfig()

    int hours = 0, minutes = 0;

    uint16_t getLuminance()
    {
      // http://forum.arduino.cc/index.php?topic=37555.0
      // https://forum.arduino.cc/index.php?topic=185158.0
      float volts = analogRead(PHOTORESISTOR_PIN) * (referenceVoltage / adcPrecision);
      float amps = volts / resistorValue;
      float lux = amps * 1000000 * 2.0;
      lastLuminance = uint16_t(lux);

      lastMeasurement = millis();
      getLuminanceComplete = true;
      return uint16_t(lux);
    }

  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {
      Serial.println("Hello from hexa_clock usermod!");

      for(int i=0; i<LEDS_NO; i++)
      {
        reverseRoundMap[roundMap[i]] = i;
        reverseVerticalMap[verticalMap[i]] = i;
      }
      // set pinmode
      pinMode(PHOTORESISTOR_PIN, INPUT);
    }


    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
      Serial.println("Connected to WiFi!");
    }


    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() {
      if (millis() - lastTime > 1000) {
        if(autoBrightnessEnabled && powerOn)
        {
          uint16_t lux = getLuminance();
          
          if(lux < nightModeThreshold && nightModeEnabled)
          {
            if(!nightModeOn)
            {
              nightModeOn = true;
              prevPreset = currentPreset;
              
              prevPlaylist = currentPlaylist;
              currentPlaylist = -1;
              //apply black background
              if(!reverseDigits) applyPreset(3);
            }
            
            bri = nightModeBri;
          }
          else {
            if(nightModeOn)
            {
              nightModeOn = false;
              //if there was a playlist playing on play it again
              if(prevPlaylist != -1)
              {
                currentPlaylist = prevPlaylist;
              }
              else 
              {
                applyPreset(prevPreset);
              }
            }
            bri = constrain(lux * autoBrightnessACoeff + autoBrightnessBCoeff,autoBrightnessMinBri,255);

          }
          colorUpdated(CALL_MODE_BUTTON);
          updateInterfaces(CALL_MODE_BUTTON);
        }

        
        hours = hour(localTime);
        minutes = minute(localTime);
        lastTime = millis();
      }
    }



    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    
    void addToJsonInfo(JsonObject& root)
    {
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray lightArr = user.createNestedArray("Light"); //name
      lightArr.add(lastLuminance); //value
      lightArr.add(" lux"); //unit
    }
    


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
      //root["user0"] = userVar0;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
      if(root.containsKey("on"))
      {
        if(root["on"]==true){
          powerOn = true;
        }else
        {
          powerOn = false;
        } 
      }
    }


    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     * 
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.  
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("HexClockUsermod");
      top["displayClock"] = displayClock;
      top["ledmapEnabled"] = ledmapEnabled;
      top["autoBrightnessEnabled"] = autoBrightnessEnabled;
      top["autoBrightnessACoeff"] = autoBrightnessACoeff;
      top["autoBrightnessBCoeff"] = autoBrightnessBCoeff;
      top["autoBrightnessMinBri"] = autoBrightnessMinBri;
      top["nightModeEnabled"] = nightModeEnabled;
      top["nightModeThreshold"] = nightModeThreshold;
      top["nightModeBri"] = nightModeBri;
      top["digitWhite"] = digitWhite;
      top["reverseDigits"] = reverseDigits;
    }


    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     * 
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     * 
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root)
    {
      // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
      // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

      JsonObject top = root["HexClockUsermod"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["displayClock"], displayClock, true);
      configComplete &= getJsonValue(top["ledmapEnabled"], ledmapEnabled, true);
      configComplete &= getJsonValue(top["autoBrightnessEnabled"], autoBrightnessEnabled, true);
      configComplete &= getJsonValue(top["autoBrightnessACoeff"], autoBrightnessACoeff, 2.0);
      configComplete &= getJsonValue(top["autoBrightnessBCoeff"], autoBrightnessBCoeff, 0.0);
      configComplete &= getJsonValue(top["autoBrightnessMinBri"], autoBrightnessMinBri, 10);
      configComplete &= getJsonValue(top["nightModeEnabled"], nightModeEnabled, true);
      configComplete &= getJsonValue(top["nightModeThreshold"], nightModeThreshold, 10);
      configComplete &= getJsonValue(top["nightModeBri"], nightModeBri, 1);
      configComplete &= getJsonValue(top["digitWhite"], digitWhite, true);
      configComplete &= getJsonValue(top["reverseDigits"], reverseDigits, false);

      return configComplete;
    }


    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw()
    {
      if(!displayClock)return;
      int orientation = 0;
      int digit;
      //for flat-top (1,3,5) orientations there are 12 pixels
      int pixelsNo = 13 - (orientation%2);
      //used when reversing digits
      bool reverseMask[LEDS_NO] = {false};
      uint8_t local_hours = !useAMPM ? hours : (hours>12 ? hours-12 : (hours==0 ? 12 : hours));

      for(int p = 0; p < 4; p++)
      {
        switch(p){
          case 0:
            digit = local_hours/10;
            break;
          case 1:
            digit = local_hours%10;
            break;
          case 2:
            digit = minutes/10;
            break;
          case 3:
            digit = minutes%10;
            break;                                  
        }

        // iterate through every pixel of the digit mask
        for(int i=0; i<pixelsNo; i++){
          //check if this pixel is involved in displaying the time (active)
          bool active = digitMask[orientation%2][digit][i];
          if(active)
          {
            int ledId;
            //ledmap 1 => vertical pattern (as if strips were spreaded vertically)
            if(ledmapEnabled && currentLedmap == 1)
            {
              ledId = reverseVerticalMap[digitSegment[orientation][p][i]];
            }
            //ledmap 0 => round pattern (as if strips were spreaded in circles)
            else if(ledmapEnabled)
            {
              ledId = reverseRoundMap[digitSegment[orientation][p][i]];
            }
            //no ledmap
            else
            {
              ledId = digitSegment[orientation][p][i];
            }

            //normal => white digits
            if(!reverseDigits)
            {
              strip.setPixelColor(ledId, RGBW32(255*digitWhite,255*digitWhite,255*digitWhite,0));
            }
            // black digits instead of white
            else 
            {
              //mark this digit as 'should be dark' to later set color
              reverseMask[ledId]=true;
            }
          }
        }
      }
      
      //apply dark digits instead of white
      if(reverseDigits)
      {
        for(int i=0; i<LEDS_NO; i++)
        {
          if(!reverseMask[i])
          {
            strip.setPixelColor(i, RGBW32(255*!digitWhite,255*!digitWhite,255*!digitWhite,0));
          }
        }
      }

    }

   
    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_HEXA_CLOCK;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};
