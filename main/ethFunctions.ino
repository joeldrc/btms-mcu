

#include <SPI.h>
#include <fnet.h>
#include "src\NativeEthernet\NativeEthernet.h"

EthernetServer server(80);


const char asciiFilledSquare[] = "&#9608;"; //'█';
const char asciiSpace[] = "_";              //'_';


const uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };


const char html_1[] PROGMEM = R"rawliteral(
<html>
<head>
<title>BTMS timing</title>
<meta charset="utf-8">
</head>
<body>
<h1>BTMS timing</h1>
<p><input type="button" value="Log Out" onclick ="location.href='/?logout'"></p>
<hr style="color: blue;">
<p><b>CONTROL PANEL</b></p>
)rawliteral";
// <meta charset="utf-8" http-equiv="refresh" content="1; url=/">
// <body onload="window.open(location.href='/', _top);">


bool plot[numTraces][samplesNumber] = {0};


// size of buffer to store HTTP requests
const uint8_t REQUEST_BUFFER = 100;
String httpRequest= ""; // HTTP request string


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


int8_t ctrlConnection() {
  auto link = Ethernet.linkStatus();
  int8_t stat;

  Serial.print("Link status: ");
  switch (link) {
    case LinkON:
      Serial.println("connected.");
      stat = 1;
      break;
    case Unknown:
      Serial.println("unknown.");
      stat = -1;
      break;
    case LinkOFF:
      Serial.println("not connected.");
      stat = 0;
      break;
  }
  return stat;
}


void htmlPage(auto client) {

  // Check http requests
  if (httpRequest.indexOf("?b1")  > 0) {
    status1 = !status1;
  }
  else if (httpRequest.indexOf("?b2")  > 0) {
    status2 = !status2;
  }
  else if (httpRequest.indexOf("?b3")  > 0) {
    status3 = !status3;
  }
  else if (httpRequest.indexOf("?b4")  > 0) {
    status4 = !status4;
  }
  else if (httpRequest.indexOf("?opMode")  > 0) {
    int posVal = httpRequest.indexOf("?opMode");
    String subString = httpRequest.substring(posVal + 8, posVal + 9);
    //Serial.println(subString);
    operationMode = subString.toInt();
  }
  
  String htmlPage2 = "";
  for (uint8_t cnt = 0; cnt < numTraces; cnt++) {
    
    /*
    // to be removed
    for (uint32_t i = 0; i < samplesNumber; i++) {
      plot[cnt][i] = random(2);  //random numbers from 0 to 1
    }
    */
    
    htmlPage2 += "<tr><td style=\"color: blue;\">";
    htmlPage2 += traceName[cnt];
    htmlPage2 += "</td><td style=\"color: purple;\">";
    htmlPage2 += traceTime[cnt];
    htmlPage2 += "</td><td style=\"color: black;\"><pre>";

    for (uint32_t i = 0; i < samplesNumber; i++) {
      if (plot[cnt][i]) {
        htmlPage2 += asciiFilledSquare;
      }
      else {
        htmlPage2 += asciiSpace;
      }
    }
    htmlPage2 += "\n";

    /*
      for (uint32_t i = 0; i < samplesNumber; i++) {
      htmlPage2 += plot[cnt][i];
      }
      htmlPage2 += "\n";
    */
    htmlPage2 += "</pre></td>";
  }

  v1 = 0;
  v2 = 0;


  // start html
  String htmlPage = "";
  htmlPage += "HTTP/1.1 200 OK";
  htmlPage += "Content-Type: text/html";
  htmlPage += "Connection: close";
  //htmlPage += "Refresh: 1";
  htmlPage += "Connection: close";
  htmlPage += "\n";

  htmlPage +=  html_1;
  
  htmlPage += "<form action=/>";
  htmlPage += "Operation mode: ";
  htmlPage += operationMode;
  htmlPage += " ";
  htmlPage += "<select name=\"opMode\" id=\"opMode\"><option value=\"0\">0</option><option value=\"1\">1</option><option value=\"2\">2</option><option value=\"3\">3</option></select>";
  htmlPage += "<button type=”submit”>Submit</button>";  
  htmlPage += "</button></form>";

  htmlPage += "<table>";
  htmlPage += "<tr><th>Name</th><th>Switch</th><th>Status</th></tr>";
  htmlPage += "<tr><td>Switch1</td><td><input type=\"button\" value=\"ON / OFF\" onclick=\"location.href='/?b1'\"></td><td";
  if (status1){
    htmlPage += " bgcolor=\"lime\"";
  }
  htmlPage += ">";
  htmlPage += status1;
  htmlPage += "</td></tr>";

  htmlPage += "<tr><td>Switch2</td><td><input type=\"button\" value=\"ON / OFF\" onclick=\"location.href='/?b2'\"></td><td";
  if (status2){
    htmlPage += " bgcolor=\"lime\"";
  }
  htmlPage += ">";
  htmlPage += status2;
  htmlPage += "</td></tr>";

  htmlPage += "<tr><td>Switch3</td><td><input type=\"button\" value=\"ON / OFF\" onclick=\"location.href='/?b3'\"></td><td";
  if (status3){
    htmlPage += " bgcolor=\"lime\"";
  }
  htmlPage += ">";
  htmlPage += status3;
  htmlPage += "</td></tr>";

  htmlPage += "<tr><td>Switch4</td><td><input type=\"button\" value=\"ON / OFF\" onclick=\"location.href='/?b4'\"></td><td";
  if (status4){
    htmlPage += " bgcolor=\"lime\"";
  }
  htmlPage += ">";
  htmlPage += status4;
  htmlPage += "</td></tr>";
  htmlPage += "</table><br/>";

  htmlPage += "<p><b>Analog read</b><br/></p>";
  htmlPage += "V1 = ";
  htmlPage += v1;
  htmlPage += "<br/>V2 = ";
  htmlPage += v2;

  htmlPage += "<br><br>";
  htmlPage += "<table><tr><th>Name</th><th> </th></tr>"; 
  htmlPage += htmlPage2; 
  htmlPage += "</tr></table></body>";
  htmlPage += "<p><input type=\"button\" value=\"Refresh\" onclick = \"location.href='/?refresh'\"></p>";
  htmlPage += "</html>";

  // send html page
  client.println(htmlPage);
  
  // close client connection
  client.close();
}


void webServer_thread() {
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
