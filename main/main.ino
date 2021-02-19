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
#include <fnet.h>

//#include "src\Fnet\fnet.h"
#include "src\NativeEthernet\NativeEthernet.h"
#include "src\TeensyThreads\TeensyThreads.h"

#include "defines.h"
#include "webPage.h"


const uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };
EthernetServer server(80);

// size of buffer to store HTTP requests
#define REQUEST_BUFFER   50

// Global variables
char HTTP_requst[50] = {0}; // HTTP request string
int char_cnt = 0;
String password = "123456";
uint8_t ledState = false;               // ledState used to set the LED

bool status1 = false;
bool status2 = false;
bool status3 = false;
bool status4 = false;
bool status5 = false;

float v1 = 0;
float v2 = 0;

const uint32_t samplesNumber = 150; // 2400m
const uint8_t numTraces = 10;

bool values[numTraces][samplesNumber];
String row[samplesNumber];


void ethernetConfig_thread() {
  if (Ethernet.begin(mac, 60000, 4000) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  }

  auto link = Ethernet.linkStatus();
  Serial.print("Link status: ");
  switch (link) {
    case LinkON:
      Serial.println("Ethernet cable is connected.");
      break;
    case Unknown:
      Serial.println("Unknown status.");
      break;
    case LinkOFF:
      Serial.println("Ethernet cable is not connected.");
      break;
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


void ctrlLedThread() {
  while (1) {
    digitalWrite(ledPin, HIGH);
    threads.delay(100);
    digitalWrite(ledPin, LOW);
    threads.delay(100);

    threads.yield();
  }
}


void asciiConversion(bool* val, const uint32_t numVal) {
  for (uint32_t i = 0; i < numVal; i++) {
    if (val[i] == 1) {
      row[i] = "&#9608;"; //'█';
    }
    else if (val[i] == 0) {
      row[i] = "_";       //'_';
    }
  }
}


void htmlPage(auto client) {
  client.println("<html>");
  client.println("<head>");
  client.println("<title>BTMS timing</title>");
  client.println("<meta charset=\"utf-8\">");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>BTMS timing</h1>");
  client.println("<p><input type=\"button\" value=\"Log Out\" onclick = \"location.href='/?logout'\"></p>");
  client.println("<hr style=\"color: blue;\">");
  client.println("<p><b>CONTROL PANEL</b></p>");
  client.println("<table>");
  client.println("<tr><th>Name</th><th>Switch</th><th>Status</th></tr>");

  client.print("<tr><td>Switch1</td><td><input type=\"button\" value=\"ON / OFF\" onclick=\"location.href='/?b1'\"></td><td");
  if (status1) client.print (" bgcolor=\"lime\"");
  client.print(">");
  client.print(status1);
  client.println("</td></tr>");

  client.print("<tr><td>Switch2</td><td><input type=\"button\" value=\"ON / OFF\" onclick=\"location.href='/?b2'\"></td><td");
  if (status2) client.print (" bgcolor=\"lime\"");
  client.print(">");
  client.print(status2);
  client.println("</td></tr>");

  client.print("<tr><td>Switch3</td><td><input type=\"button\" value=\"ON / OFF\" onclick=\"location.href='/?b3'\"></td><td");
  if (status3) client.print (" bgcolor=\"lime\"");
  client.print(">");
  client.print(status3);
  client.println("</td></tr>");

  client.print("<tr><td>Switch4</td><td><input type=\"button\" value=\"ON / OFF\" onclick=\"location.href='/?b4'\"></td><td");
  if (status4) client.print (" bgcolor=\"lime\"");
  client.print(">");
  client.print(status4);
  client.println("</td></tr>");

  client.println("</table>");
  client.println("<br/>");

  v1 = analogRead(0) * 5.0 / 1023.0;
  v2 = analogRead(1) * 5.0 / 1023.0;

  client.println("<p><b>Analog read</b><br/></p>");
  client.print("V1 = ");
  client.println(v1);
  client.print("<br/>V2 = ");
  client.println(v2);

  client.println("<br><br>");
  client.println("<table><tr><th>Name</th><th> </th></tr>");

  for (uint8_t cnt = 0; cnt < numTraces; cnt++) {
    for (uint32_t i = 0; i < samplesNumber; i++) {
      static int cnt = 0;
      static int x = 0;
      static bool val;

      if (cnt == x) {
        val = random(2);  //random numbers from 0 to 1
        x = random(10);
        cnt = 0;
      }
      else {
        cnt++;
      }
      values[cnt][i] = val;
    }

    client.println("<tr><td style=\"color: blue;\">P1</td><td style=\"color: green;\"><pre>");
    asciiConversion(values[cnt], samplesNumber);
    for (uint32_t i = 0; i < samplesNumber; i++) {
      client.print(row[i]);
    }
    client.println("");

    for (uint32_t i = 0; i < samplesNumber; i++) {
      client.print(values[cnt][i]);
    }
    client.println("");

    client.println("</pre></td>");
  }
  client.println("</tr></table></body>");
  client.println("</html>");
}


void webServer_thread() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 1");  // refresh the page automatically every 5 sec
          client.println();

          htmlPage(client);
          client.close();
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(10);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}


void setup() {
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);

  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);

  Serial.begin(9600);
  Serial.println("BTMS mcu serial monitor");

  ethernetConfig_thread();
  //threads.addThread(ethernetConfig_thread, 1);
  threads.addThread(ctrlLedThread, 1);
}


void loop() {
  webServer_thread();
}
