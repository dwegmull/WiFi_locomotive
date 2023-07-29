#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebSrv.h"

DNSServer dnsServer;
AsyncWebServer server(80);

const int output = D0;

int sliderValue = 0;

// setting PWM properties
const int freq = 30000;
const int ledChannel = 0;
const int resolution = 8;

const char* PARAM_INPUT = "value";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP Web Server</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    body {margin:0px auto; padding-bottom: 25px; background-color: black}
    #rcorners {position: fixed; top: 0px; left: 60px; border-radius: 25px; background: black; padding: 0px; width: 180px; height: 30px; border: 5px solid #FFDE00}
    #emergency {position: fixed; bottom: 0px; left: 60px;border-radius: 15px; background: #FF0000; padding: 5px; border: 5px solid #FF0000; color: #000000; font-size: 4rem;}
    p {position: fixed; left: 0;font-size: 1.9rem; color: #FFDE00}
    #speedSlider { -webkit-appearance: none; position: fixed; left: 0px; top: 100px; margin-top: 100px; width: 128px; height: 25px; background: #FFDE00;
      outline: none; -webkit-transition: .2s; transition: opacity .2s; transform: rotate(270deg);}
    #speedSlider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
    #speedSlider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; } 
    #directionSlider { -webkit-appearance: none; position: fixed; right: 0px; top: 100px; margin-top: 100px; width: 100px; height: 25px; background: #FFDE00;
      outline: none; -webkit-transition: .2s; transition: opacity .2s; transform: rotate(270deg);}
    #directionSlider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
    #directionSlider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; } 
    #batteryLevel {position: fixed; top: 80px; left: 100px;}
    #batteryTemperature {position: fixed; top: 160px; left: 100px;}
    #motorCurrent {position: fixed; top: 240px; left: 100px;}
  </style>
</head>
<body>
  <p id="rcorners">PLYMOUTH</p>
  <p><input type="range" id="speedSlider" min="0" max="255" value="0" step="1" class="slider"></p>
  <p><input type="range" id="directionSlider" min="0" max="1" value="0" step="1" class="slider"></p>
  <canvas id="batteryLevel" width="100" height="100"></canvas>
  <canvas id="batteryTemperature" width="100" height="100"></canvas>
  <canvas id="motorCurrent" width="100" height="100"></canvas>
  <button type="button" onclick="updateEmergency()" id="emergency">STOP</button>
<script>
var JS_speedSlider = document.getElementById("speedSlider");
var JS_directionSlider = document.getElementById("directionSlider");
var JS_speedText = document.getElementById("speedText");
JS_speedSlider.addEventListener("input", updateSliderPWM);
JS_directionSlider.addEventListener("input", updateDirection);
const batteryLevelCv = document.getElementById("batteryLevel");
drawGauge("E", "M", "F", batteryLevelCv);
const batteryTemperatureCv = document.getElementById("batteryTemperature");
drawGauge("C", "W", "H", batteryTemperatureCv);
const motorCurrentCv = document.getElementById("motorCurrent");
drawGauge("0", "1", "2", motorCurrentCv);

function updateSliderPWM()
{
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/speed?value="+ this.value, true);
  xhr.send();
}

function updateDirection()
{
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/direction?value="+ this.value, true);
  xhr.send();
}

function updateEmergency()
{
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/speed?value=0" , true);
  xhr.send();
  JS_speedSlider.value = "0";
}

function drawGauge(leftText, midText, rightText, canvas)
{
  const ctx = canvas.getContext("2d");
  ctx.strokeStyle = "#FFDE00";
  ctx.lineWidth = 4;
  ctx.beginPath();
  ctx.arc(50, 50, 47, Math.PI, 2 * Math.PI);
  ctx.moveTo(3, 50);
  ctx.lineTo(3, 60);
  ctx.lineTo(97,60);
  ctx.lineTo(97,50);
  ctx.moveTo(50, 60);
  ctx.arc(50, 60, 4, Math.PI, 2 * Math.PI);
  ctx.stroke();
  ctx.font = "24px Georgia";
  ctx.fillStyle = "#FFDE00";
  w = ctx.measureText(midText).width;
  ctx.fillText(midText, 50 - (w / 2), 25);
  ctx.save();
  ctx.translate(-20, 50);
  ctx.rotate(-Math.PI/4);
  ctx.textAlign = "center";
  ctx.font = "24px Georgia";
  ctx.fillStyle = "#FFDE00";
  w = ctx.measureText(leftText).width;
  ctx.fillText(leftText, 50 - (w / 2), 25);
  ctx.restore();
  ctx.save();
  ctx.translate(60, -10);
  ctx.rotate(Math.PI/4);
  ctx.textAlign = "center";
  ctx.font = "24px Georgia";
  ctx.fillStyle = "#FFDE00";
  w = ctx.measureText(rightText).width;
  ctx.fillText(rightText, 50 - (w / 2), 25);
  ctx.restore();
}
</script>
</body>
</html>)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if (var == "SLIDERVALUE"){
    return "0";
  }
  return String();
}

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", index_html); 
  }
};

void setupServer()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/speed", HTTP_GET, [] (AsyncWebServerRequest *request)
  {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT))
    {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderValue = inputMessage.toInt();
      ledcWrite(ledChannel, sliderValue);
    }
    else
    {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  
  server.on("/direction", HTTP_GET, [] (AsyncWebServerRequest *request)
  {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT))
    {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      // Direction pin is PIO0
      if (inputMessage.toInt() == 0)
      {
        digitalWrite(D1, LOW);
      }
      else
      {
        digitalWrite(D1, HIGH);
      }
    }
    else
    {
      inputMessage = "No message sent";
    }
    Serial.print("Direction: ");
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
}


void setup()
{
  Serial.begin(9600);
  ledcSetup(ledChannel, freq, resolution);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(output, ledChannel);
  
  ledcWrite(ledChannel, sliderValue);

  //your other setup stuff...
  WiFi.softAP("Plymouth Locomotive Works", NULL, 1, 0, 1);
  setupServer();
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  //more handlers...
  Serial.println("Starting the server");
  server.begin();
}

void loop()
{
  dnsServer.processNextRequest();
}
