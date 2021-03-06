#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>

#define PIN D8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "___"; // your WiFi ssid
const char* password = "___"; // your WiFi password
const char* host = "officelight"; // the name of your fixture, and the base channel to listen to
IPAddress MQTTserver(192, 168, 1, 1); // your MQTT server IP
#define HOSTNAME "ESP8266-OTA-LIGHT-"

/* NO NEED TO CHANGE BENEATH THIS LINE */
int hue = 0;
float brightness = 0.0;
float brightnessold = 1.0;
float saturation = 0.0;

#define BUFFER_SIZE 100

WiFiClient wclient;
PubSubClient client(wclient, MQTTserver);


void callback(const MQTT::Publish& pub) {

  uint16_t i, j;
  
  currentValues();
  String myMessage = String(pub.payload_string());
  // handle message arrived
  Serial.print(pub.topic());
  Serial.print(" => ");
  String myTopic = String(pub.topic());
  
  
  if(myTopic == host)
  {

    Serial.println(pub.payload_string());

    if(pub.payload_string() == "on")
    {
      
      // use this to reset parameters if you want them to always come on bright white. 
      //hue = 0;
      //brightness = 1.0;
      brightness = brightnessold;
      //saturation = 0.0;

      
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, HSVColor(hue,saturation,brightness));
      }
      strip.show();
        
    }
    else
    {
      //hue = 0;
      brightness = 0.0;
      //saturation = 0.0;
      
      for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, HSVColor(hue,saturation,brightness));
    }
    strip.show();
    }

  }
  
  else if(myTopic == host+(String)"/brightness")
  { // Brightness up to 100
    Serial.println(pub.payload_string());
    brightnessold = (myMessage.toFloat())/100;
     brightness = brightnessold;
      for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, HSVColor(hue,saturation,brightness));
    }
    strip.show();
    
  }
  else if(myTopic == host+(String)"/hue")
  { // Hue value 0-360
    Serial.println(pub.payload_string());
    hue = myMessage.toInt();
      for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, HSVColor(hue,saturation,brightness));
    }
    strip.show();
    
  }
  else if(myTopic == host+(String)"/saturation")
  { // Saturation value at 0-100
    Serial.println(pub.payload_string());
    saturation = (myMessage.toFloat())/100;
      for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, HSVColor(hue,saturation,brightness));
    }
    strip.show();
  }
  currentValues();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);
  
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // MQTT callback
  client.set_callback(callback);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

   // Setup OTA
  Serial.println("Hostname: " + hostname);
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.begin();

}

void loop() {
 ArduinoOTA.handle();
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      if (client.connect("ESP8266: Fountain")) {
        client.publish("outTopic",(String)"hello world, I'm "+host);
        client.subscribe(host+(String)"/#");
      }
    }

    if (client.connected())
      client.loop();
   }
  
}

// Convert Hue/Saturation/Brightness values to a packed 32-bit RBG color.
// hue must be a float value between 0 and 360
// saturation must be a float value between 0 and 1
// brightness must be a float value between 0 and 1
uint32_t HSVColor(float h, float s, float v) {

 h = constrain(h, 0, 360);
  s = constrain(s, 0, 1);
  v = constrain(v, 0, 1);

  int i, b, p, q, t;
  float f;

  h /= 60.0;  // sector 0 to 5
  i = floor( h );
  f = h - i;  // factorial part of h

  b = v * 255;
  p = v * ( 1 - s ) * 255;
  q = v * ( 1 - s * f ) * 255;
  t = v * ( 1 - s * ( 1 - f ) ) * 255;

  switch( i ) {
    case 0:
      return strip.Color(b, t, p);
    case 1:
      return strip.Color(q, b, p);
    case 2:
      return strip.Color(p, b, t);
    case 3:
      return strip.Color(p, q, b);
    case 4:
      return strip.Color(t, p, b);
    default:
      return strip.Color(b, p, q);
  }
}

void currentValues(){
  Serial.println("");
  Serial.println("Current State");
  Serial.print("Hue (0-360):");
  Serial.println(hue);
  Serial.print("Saturation (0-100 in, 0-1):");
  Serial.println(saturation*100);
  Serial.print("Brightness (0-100):");
  Serial.println(brightness*100);
  Serial.println("");
}

