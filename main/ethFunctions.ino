

#include <SPI.h>
#include <fnet.h>
#include "src\NativeEthernet\NativeEthernet.h"


EthernetServer server(80);
uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };


const char asciiFilledSquare[] = "&#9608;"; //'█';
const char asciiSpace[] = "_";              //'_';


// size of buffer to store HTTP requests
const uint8_t REQUEST_BUFFER = 100;
String httpRequest = ""; // HTTP request string


void ethernetConfig_thread() {
  if (Ethernet.begin(mac, 60000, 4000) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  }

  // start the server
  server.begin();
  Serial.print("MAC: ");
  for (byte octet = 0; octet < 6; octet++) {
    Serial.print(mac[octet], HEX);
    if (octet < 5) {
      Serial.print('-');
    }
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
}


int ctrlConnection() {
  auto link = Ethernet.linkStatus();

  Serial.print("Link status: ");
  switch (link) {
    case LinkON:
      Serial.println("connected.");
      return 1;
      break;
    case Unknown:
      Serial.println("unknown.");
      return -1;
      break;
    case LinkOFF:
      Serial.println("not connected.");
      return 0;
      break;
    default: {
        return -2;
      }
  }
}


const char html_1[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>BTMS MCU</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
* {
  box-sizing: border-box;
}
footer {
  text-align: center;
  padding: 3px;
  background-color: LightBlue;
}
</style>
<script>
function addOptionsFunction() {
  var x = document.getElementById("timingPanel");
  if (x.style.display === "none") {
    x.style.display = "block";
  } else {
    x.style.display = "none";
  }
}
</script>
</head>
<body style="font-family:Verdana;color: DarkSlateGray;">
)rawliteral";


String setupTiming(){
  String htm = "<br><input type=\"checkbox\" onclick=\"addOptionsFunction()\" name=\"advancedSettings\">Advanced settings</p>";
  htm += "<div style=\"text-align:left;display: none;\" id=\"timingPanel\"><h3>SETUP TIMING</h3>";
  htm += "<form action=\"/\"><table>";

  htm += "<tr><td>calstartTime (Value in uS):</td><td><input type=\"number\" id=\"quantity\" name=\"val1\" min=\"1\" max=\"1200000\" value=\"";
  htm += calstartTime;
  htm += "\"></td></tr>";
  htm += "<tr><td>calstopTime (Value in uS):</td><td><input type=\"number\" id=\"quantity\" name=\"val2\" min=\"1\" max=\"1200000\" value=\"";
  htm += calstopTime;
  htm += "\"></td></tr>";
  htm += "<tr><td>injTime (Value in uS):</td><td><input type=\"number\" id=\"quantity\" name=\"val3\" min=\"1\" max=\"1200000\" value=\"";
  htm += injTime;
  htm += "\"></td></tr>";
  htm += "<tr><td>hchTime (Value in uS):</td><td><input type=\"number\" id=\"quantity\" name=\"val4\" min=\"1\" max=\"1200000\" value=\"";
  htm += hchTime;
  htm += "\"></td></tr>";
  htm += "<tr><td>ecyTime (Value in uS):</td><td><input type=\"number\" id=\"quantity\" name=\"val5\" min=\"1\" max=\"1200000\" value=\"";
  htm += ecyTime;
  htm += "\"></td></tr>";
  
  htm += "</table><br><input type=\"submit\" value=\"Save\"></form>";
  htm += "<input type=\"submit\" value=\"Reset\" onclick=\"location.href='/?reset='\"></div>";
  return htm;
}


String opModeOption(int mode){
  String htm = "<form action=/><label for=\"opMode\"><b>Operation mode: ";
  htm += mode;
  htm += "</b><br><br></label>";
  htm += "<select name=\"opMode\" id=\"opMode\"><optgroup label=\"Read only\">";
  
  if (mode==0) htm += "<option value=\"0\" selected>0. READ TIMING</option>";
  else htm += "<option value=\"0\">0. READ TIMING</option>";
  
  htm += "</optgroup><optgroup label=\"Simulated\">";

  if (mode==1) htm += "<option value=\"1\" selected>1. Simulate SCY and ECY</option>";
  else htm += "<option value=\"1\">1. Simulate SCY and ECY</option>";

  if (mode==2) htm += "<option value=\"2\" selected>2. Simulate SCY, INJ and ECY</option>";
  else htm += "<option value=\"2\">2. Simulate SCY, INJ and ECY</option>";

  if (mode==3) htm += "<option value=\"3\" selected>3. Simulate SCY, CALSTART, CALSTOP, INJ, HCH and ECY</option>";
  else htm += "<option value=\"3\">3. Simulate SCY, CALSTART, CALSTOP, INJ, HCH and ECY</option>";

  htm += "</optgroup></select><button type=”submit”>Change</button></form>"; 
  return htm;
}


const char html_10[] PROGMEM = R"rawliteral(
<p><input type="button" value="Refresh" onclick = "location.href='/?refresh'"></p>
<br>
<footer>
  <p><b>Version: 2021.01.V.1.0</b><br><br><a style="color: DarkSlateGray;" href="mailto:joel.daricou@cern.ch">joel.daricou@cern.ch</a></p>
</footer>
</body>
</html> 
)rawliteral";


String h1_title(int val){
  String htm = "<div style=\"background-color:LightBlue;padding:15px;text-align:center;\"><h1>BTMS MCU #";
  htm += val;
  htm += "</h1></div>";
  return htm;
}


String showInfo(String color, String title){
  String htm = "<p><svg width=\"14\" height=\"14\"><circle cx=\"7\" cy=\"7\" r=\"8\" fill=\"";
  htm += color;
  htm += "\"/> Your browser does not support inline SVG. </svg><b> ";
  htm += title;
  htm += "</b></p>";
  return htm;
}


String h2_title(String title){
  String htm = "<br><div style=\"text-align:left;\"><h2>";
  htm += title;
  htm += "</h2></div><hr>";
  return htm;
}


/*
  void buildPlot(){
  for(uint8_t i; i < numTraces; i++){
    if (traceTime[i] < psTimeCycle){

    }
  }
  }
*/


uint32_t httpFilterString(String httpRqst, String request){
  int posVal = httpRqst.indexOf('=', httpRqst.indexOf(request));
  int endNumber = httpRqst.indexOf(' ', posVal + 1);
  //Serial.println(httpRqst);
  String subString = httpRequest.substring(posVal + 1, endNumber);
  Serial.println(subString.toInt());
  return subString.toInt();  
}


void htmlPage(auto client) {

  uint32_t tempVal = 0;
  
  if (httpRequest.indexOf("opMode=")  > 0) {
    operationMode = httpFilterString(httpRequest, "opMode=");
  }

  if (httpRequest.indexOf("val1=")  > 0) {
    tempVal = httpFilterString(httpRequest, "val1=");
    noInterrupts();
    calstartTime = tempVal;
    interrupts();
  }
  if (httpRequest.indexOf("val2=")  > 0) {    
    tempVal = httpFilterString(httpRequest, "val2=");
    noInterrupts();
    calstopTime =  tempVal;
    interrupts();
  }
  if (httpRequest.indexOf("val3=")  > 0) {
    tempVal = httpFilterString(httpRequest, "val3=");
    noInterrupts();
    injTime = tempVal;
    interrupts();
  }
  if (httpRequest.indexOf("val4=")  > 0) {
    tempVal = httpFilterString(httpRequest, "val4=");
    noInterrupts();
    hchTime = tempVal;
    interrupts();
  }
  if (httpRequest.indexOf("val5=")  > 0) {
    tempVal = httpFilterString(httpRequest, "val5=");
    noInterrupts();
    ecyTime = tempVal;
    interrupts();
  }

  if (httpRequest.indexOf("reset=")  > 0) {
    noInterrupts();
    scyTime = SCY_T;            // time in uS
    calstartTime = CALSTART_T;  // time in uS
    calstopTime = CALSTOP_T;    // time in uS
    injTime = INJ_T;            // time in uS
    hchTime = HCH_T;            // time in uS
    ecyTime = ECY_T;            // time in uS
    psTimeCycle = PSCYCLE_T;    // time in uS
    interrupts();
  }
  

  // start html
  String htmlPage = "";
  htmlPage += "HTTP/1.1 200 OK";
  htmlPage += "Content-Type: text/html";
  htmlPage += "Connection: close";
  //htmlPage += "Refresh: 1";
  htmlPage += "Connection: close";
  htmlPage += "\n";

  htmlPage += html_1;

  htmlPage += h1_title(BoardSN);

  htmlPage += h2_title("CONTROL PANEL");
  if (boardStatus == 0) htmlPage += showInfo("LawnGreen", "Board Status");
  else htmlPage += showInfo("Red", "Board Status");
  
  if (det10Mhz == 1) htmlPage += showInfo("LawnGreen", "DET10MHz");
  else htmlPage += showInfo("LightGray", "DET10MHz");
  
  if (lock == 0) htmlPage += showInfo("LawnGreen", "LOCK");
  else htmlPage += showInfo("LightGray", "LOCK");

  htmlPage += showInfo("LightGray", "MCU T: " + String(cpuTemp) + " &#176;C");
  htmlPage += showInfo("LightGray", "BOX T: " + String(v1) + " &#176;C");
  htmlPage += showInfo("LightGray", "V2: " + String(v2) + " V");
  htmlPage += showInfo("LightGray", "V3: " + String(v3) + " V");
  htmlPage += showInfo("LightGray", "V4: " + String(v4) + " V");

  
  htmlPage += h2_title("SETTINGS");
  htmlPage += opModeOption(operationMode);
  htmlPage += setupTiming();


  htmlPage += h2_title("PLOTS");
  String html_2 = "<table>";
  html_2 += "<tr><th> </th><th>t [uS]</th><th>Value</th></tr>"; 
  for (uint8_t cnt = 0; cnt < numTraces; cnt++) {
    html_2 += "<tr><th>";
    html_2 += traceName[cnt];
    html_2 += "</th><td>";
    html_2 += traceTime[cnt];
    html_2 += "</td><td><pre>";  
    for (uint32_t i = 0; i < samplesNumber; i++) {
      if (plot[cnt][i]) {
        html_2 += asciiFilledSquare;
      }
      else {
        html_2 += asciiSpace;
      }
    }
    html_2 += "\n";  
    /*
      for (uint32_t i = 0; i < samplesNumber; i++) {
      htmlPage2 += plot[cnt][i];
      }
      htmlPage2 += "\n";
    */
    html_2 += "</pre></td></tr>";
  }
  html_2 += "</table>";


  htmlPage += html_2; 
  htmlPage += html_10;

// send html page
client.println(htmlPage);

// close client connection
client.close();
}


void handleWebServer() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("New client.");

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        if (httpRequest.length() < REQUEST_BUFFER) {
          httpRequest += c;
        }

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {

          htmlPage(client);

          // reset buffer
          httpRequest = "";
          break;
        }
        if (c == '\n') {
          // start new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // character received on the current line
          currentLineIsBlank = false;
        }
      }
    }

    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("Client disconnected.");
  }
}
