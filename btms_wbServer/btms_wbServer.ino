/**
 ******************************************************************************
  @file    BTMS timing mcu
  @author  Joel Daricou  <joel.daricou@cern.ch>
  @brief   web server ctrl I/O
 ******************************************************************************
  Board:
    - Developped on teensy 4.1
    - CPU speed 600MHz
  Info programming:
    -
  References C++:
    -
  Scheme:
    -
 ******************************************************************************
*/


#include <SPI.h>
#include "src\NativeEthernet\NativeEthernet.h"
#include "src\TeensyThreads\TeensyThreads.h"


const uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };
EthernetServer server(80);

// size of buffer to store HTTP requests
#define REQUEST_BUFFER   50

// Constants
const uint8_t ledPin =  LED_BUILTIN;  // the number of the LED pin


String html5File;
uint8_t ledState = false;               // ledState used to set the LED

char HTTP_requst[50] = {0}; // HTTP request string
int char_cnt = 0;


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head><title>Digital Input Analyser</title>
<style>
span{position:absolute; left:540px; top:50px;}
</style>
</head>
<body id="test" onload="drawGraph();">
<canvas id="myCanvas" width="570" height="300"></canvas>
<menu id="menu_items" >
<div >
<span style="left:60px;"><input type="checkbox" id="c0" checked><label for="c0" style="color:#ff0000">D5</label></span>
<span style="left:110px;"><input type="checkbox" id="c1" checked ><label for="c1" style="color:#00ff00">D6</label></span>
<span style="left:160px;"><input type="checkbox" id="c2" checked ><label for="c2" style="color:#0000ff">D7</label></span>
<span style="left:210px;"><input type="checkbox" id="c3" checked ><label for="c3" style="color:#ff00ff">D8</label></span>
<span style="left:410px;">Division:<select id="dur"><option value="100">100ms</option><option value="200">200ms</option><option value="500">500ms</option><option value="1000">1sec</option></select></span>
</div>
</menu>
<span style="top:140px;">D8</span>
<span style="top:190px;">D7</span>
<span style="top:240px;">D6</span>
<span style="top:290px;">D5</span>
<script type="text/javascript">
function GetArduinoInputs(){
  var request = new XMLHttpRequest();
  request.onreadystatechange = function(){
  if(this.readyState == 4 && this.status == 200 && this.responseXML!=null){
    updateGraph(this.responseXML.getElementsByTagName('digital')[0].childNodes[0].nodeValue,
    this.responseXML.getElementsByTagName('digital')[1].childNodes[0].nodeValue,
    this.responseXML.getElementsByTagName('digital')[2].childNodes[0].nodeValue,
    this.responseXML.getElementsByTagName('digital')[3].childNodes[0].nodeValue);
    }
  }
  request.open("GET", "ajax_inputs&random="+Math.random()*1000000,true);
  request.send(null)
  setTimeout('GetArduinoInputs()',document.getElementById('dur').value);
}

var canvas = document.getElementById('myCanvas');
var ctx = canvas.getContext('2d');
myCanvas.style.border = "red 1px solid";
var aval0=[], aval1=[], aval2=[], aval3=[]; 

for(cnt=0; cnt<18; cnt++){
  aval0[cnt]=0;
  aval1[cnt]=0;
  aval2[cnt]=0;
  aval3[cnt]=0;
}

function drawGraph(){initGraph(); updateGraph(0,0,0,0); GetArduinoInputs();}

function initGraph(){
  ctx.fillStyle="#ff0000";
  ctx.font = "28px serif";
  ctx.textAlign = "center";
  ctx.fillText("DIGITAL INPUT ANALYSER",290,30);
}
function updateGraph(val0, val1, val2, val3){
  ctx.clearRect(0, 30, canvas.width, canvas.height);
  aval0.shift(); if(val0==1)aval0.push(25); else aval0.push(0);
  aval1.shift(); if(val1==1)aval1.push(25); else aval1.push(0);
  aval2.shift(); if(val2==1)aval2.push(25); else aval2.push(0);
  aval3.shift(); if(val3==1)aval3.push(25); else aval3.push(0);
  if(document.getElementById('c0').checked)drawPlot(0);
  if(document.getElementById('c1').checked)drawPlot(1);
  if(document.getElementById('c2').checked)drawPlot(2);
  if(document.getElementById('c3').checked)drawPlot(3);
}

function drawPlot(barSel){
  ctx.save();
  ctx.beginPath();
  ctx.lineWidth = 1;
  ctx.strokeStyle = '#888888';
  for(cnt=1; cnt<9; cnt++){
    ctx.setLineDash([1,1]);
    ctx.moveTo(0,75+cnt*25);
    ctx.lineTo(525,75+cnt*25);
    ctx.stroke();
  }
  for(cnt=1; cnt<18; cnt++){
    ctx.moveTo(0+cnt*30,90);
    ctx.lineTo(0+cnt*30,300);
    ctx.stroke();
  }
  ctx.restore();
  ctx.beginPath();
  switch(barSel){
    case 0 : ctx.moveTo(0,300-aval0[0]); break;
    case 1 : ctx.moveTo(0,250-aval1[0]); break;
    case 2 : ctx.moveTo(0,200-aval2[0]); break;
    default : ctx.moveTo(0, 150-aval3[0]); 
  }
  for(cnt=1; cnt<18; cnt++){
    switch(barSel){
      case 0 : 
        if(aval0[cnt]==aval0[cnt-1]) ctx.lineTo(cnt*30,300-aval0[cnt]);
        if(aval0[cnt]>aval0[cnt-1]) { ctx.lineTo(cnt*30,300); ctx.lineTo(cnt*30,275);}
        if(aval0[cnt]<aval0[cnt-1]) { ctx.lineTo(cnt*30,275); ctx.lineTo(cnt*30,300);}
        ctx.strokeStyle = '#ff0000'; break;
      case 1 : 
        if(aval1[cnt]==aval1[cnt-1]) ctx.lineTo(cnt*30,250-aval1[cnt]);
        if(aval1[cnt]>aval1[cnt-1]) { ctx.lineTo(cnt*30,250); ctx.lineTo(cnt*30,225);}
        if(aval1[cnt]<aval1[cnt-1]) { ctx.lineTo(cnt*30,225); ctx.lineTo(cnt*30,250);}
        ctx.strokeStyle = '#00ff00'; break;
      case 2 : 
        if(aval2[cnt]==aval2[cnt-1]) ctx.lineTo(cnt*30,200-aval2[cnt]);
        if(aval2[cnt]>aval2[cnt-1]) { ctx.lineTo(cnt*30,200); ctx.lineTo(cnt*30,175);}
        if(aval2[cnt]<aval2[cnt-1]) { ctx.lineTo(cnt*30,175); ctx.lineTo(cnt*30,200);}
        ctx.strokeStyle = '#0000ff'; break;
      default : 
        if(aval3[cnt]==aval3[cnt-1]) ctx.lineTo(cnt*30,150-aval3[cnt]);
        if(aval3[cnt]>aval3[cnt-1]) { ctx.lineTo(cnt*30,150); ctx.lineTo(cnt*30,125);}
        if(aval3[cnt]<aval3[cnt-1]) { ctx.lineTo(cnt*30,125); ctx.lineTo(cnt*30,150);}
        ctx.strokeStyle = '#ff00ff'; break;
    }
    ctx.lineWidth = 3;
    ctx.stroke();
  }
}
</script>
</body>
</html>
)rawliteral";


void ethernetConfig_thread() {
  Ethernet.begin(mac, 60000, 4000); // unsigned long timeout = 60000, unsigned long responseTimeout = 4000
  
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.");
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  server.begin();
  Serial.println("server is at ");
  Serial.println(Ethernet.localIP());
}


void statusLed_thread() {
  while (1) {
    digitalWrite(ledPin, HIGH);
    threads.delay(100);
    digitalWrite(ledPin, LOW);
    threads.delay(100);
    //threads.yield();
  }
}


void setup(){
    pinMode(ledPin, OUTPUT);
    
    pinMode(0, INPUT);
    pinMode(1, INPUT);
    pinMode(2, INPUT);
    pinMode(3, INPUT);

    Serial.begin(9600);
    Serial.println("Ethernet WebServer");

    threads.addThread(ethernetConfig_thread, 1);
    threads.addThread(statusLed_thread, 1);
}


// searches for a needle in the haystack
char SearchForRequest(char *haystack, char *needle)
{
    char needle_index = 0;
    char haystack_index = 0;
    char haystack_length;

    haystack_length = strlen(haystack);
    if (strlen(needle) > haystack_length) return 0;
    while (haystack_index < haystack_length) {
        if (haystack[haystack_index] == needle[needle_index]) {
            needle_index++;
            if (needle_index == strlen(needle)) return 1; // found needle
        }
        else needle_index = 0;
        haystack_index++;
    }
    return 0;
}


// prepare send the XML file with arduino input values
void XML_response(EthernetClient cd)
{
    int analog_0 = 0;
    int analog_1 = 0;
    int analog_2 = 0;
    int analog_3 = 0;
    int analog_4 = 0;
    int analog_5 = 0;
    int digital_5 = 0;
    int digital_6 = 0;
    int digital_7 = 0;
    int digital_8 = 0;
       
    analog_0 = analogRead(0);
    analog_1 = analogRead(1);
    analog_2 = analogRead(2);
    analog_3 = analogRead(3);
    analog_4 = analogRead(4);
    analog_5 = analogRead(5);
     
    digital_5 = digitalRead(5);    
    //Serial.println(digital_5);
    
    digital_6 = digitalRead(6);
    digital_7 = digitalRead(7);
    digital_8 = digitalRead(8);
        
    cd.print("<?xml version = \"1.0\" ?>");
    cd.print("<inputs>");

    cd.print("<analog>");
    cd.print(analog_0);
    cd.print("</analog>");
    cd.print("<analog>");
    cd.print(analog_1);
    cd.print("</analog>");
    cd.print("<analog>");
    cd.print(analog_2);
    cd.print("</analog>");
    cd.print("<analog>");
    cd.print(analog_3);
    cd.print("</analog>");
    cd.print("<analog>");
    cd.print(analog_4);
    cd.print("</analog>");
    cd.print("<analog>");
    cd.print(analog_5);
    cd.print("</analog>");

    cd.print("<digital>");
    cd.print(digital_5);
    cd.print("</digital>");
    cd.print("<digital>");
    cd.print(digital_6);
    cd.print("</digital>");
    cd.print("<digital>");
    cd.print(digital_7);
    cd.print("</digital>");
    cd.print("<digital>");
    cd.print(digital_8);
    cd.print("</digital>");
    cd.print("</inputs>");
}


void loop(){
    EthernetClient CanvasDisplay = server.available();  // Get CanvasDisplay client

    if (CanvasDisplay) {  // got CanvasDisplay?
        boolean currentLineIsBlank = true; 
        while (CanvasDisplay.connected()) {
            if (CanvasDisplay.available()) {   // check if CanvasDisplay available to read
                char HTTP_requst_char = CanvasDisplay.read(); // read 1 byte (character) from CanvasDisplay request
                    if (char_cnt < (REQUEST_BUFFER - 1)) { //last element is 0 => null terminate string
                    HTTP_requst[char_cnt] = HTTP_requst_char;          // store each HTTP request character in HTTP_requst array
                    char_cnt++;
                }
                if (HTTP_requst_char == '\n' && currentLineIsBlank) { //give response only after last line
                    // send a standard http response header
                    CanvasDisplay.println("HTTP/1.1 200 OK");
                    if (SearchForRequest(HTTP_requst, "ajax_inputs")) {
                        CanvasDisplay.println("Content-Type: text/xml"); // data as XML
                        CanvasDisplay.println("Connection: keep-alive");
                        CanvasDisplay.println();
                        XML_response(CanvasDisplay); //send data to Canvas Display
                    }
                    else{
                        CanvasDisplay.println("Content-Type: text/html");
                        CanvasDisplay.println("Connection: keep-alive");
                        CanvasDisplay.println();
                        if (SearchForRequest(HTTP_requst, "GET /digital.htm")) {
                          Serial.println(HTTP_requst);
                        }
                        else{
                          html5File = index_html;
                        }
                        CanvasDisplay.print(html5File);
                        CanvasDisplay.print("");
                    }
                               
                    // reset buffer
                    char_cnt = 0;
                    for (int i = 0; i < REQUEST_BUFFER; i++) HTTP_requst[i] = 0;
                    break;
                }
                if (HTTP_requst_char == '\n') currentLineIsBlank = true; //new line detected
                else if (HTTP_requst_char != '\r') currentLineIsBlank = false;  //get next character
                
            } // end if (CanvasDisplay.available())
        } // end while (CanvasDisplay.connected())
        delay(1);      // short delay before closing the connection
        CanvasDisplay.stop(); // close the connection
    } // end if (CanvasDisplay)
}
