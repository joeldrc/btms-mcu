

#include <SPI.h>
#include <fnet.h>
#include "src\NativeEthernet\NativeEthernet.h"

EthernetServer server(80);

volatile bool plot[numTraces][samplesNumber];

// size of buffer to store HTTP requests
const uint8_t REQUEST_BUFFER = 100;
uint16_t charCnt = 0;
char httpRequest[REQUEST_BUFFER] = {0};   // HTTP request string


void ethernetConfig_thread() {
  if (Ethernet.begin(mac, 60000, 4000) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  }

  ctrlConnection();

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


void ctrlConnection() {
  while (1) {
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
    threads.delay(1000);
    threads.yield();
  }
}


// searches for a needle in the haystack
char SearchForRequest(char *haystack, char *needle) {
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


void htmlPage(auto client) {

  // Check http requests
  //if (httpRequest.indexOf("?b1") > 0)

  if (SearchForRequest(httpRequest, "?b1")) {
    status1 = !status1;
  }
  else if (SearchForRequest(httpRequest, "?b2")) {
    status2 = !status2;
  }
  else if (SearchForRequest(httpRequest, "?b3")) {
    status3 = !status3;
  }
  else if (SearchForRequest(httpRequest, "?b4")) {
    status4 = !status4;
  }

  String htmlPage2 = "";
  for (uint8_t cnt = 0; cnt < numTraces; cnt++) {

    // to be removed
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
      plot[cnt][i] = val;
    }

    htmlPage2 += "<tr><td style=\"color: blue;\">P1</td><td style=\"color: green;\"><pre>";

    for (uint32_t i = 0; i < samplesNumber; i++) {
      if (plot[cnt][i]) {
        htmlPage2 += asciiFilledSquare;
      }
      else {
        htmlPage2 += asciiSpace;
      }
    }
    htmlPage2 += "\n";

    for (uint32_t i = 0; i < samplesNumber; i++) {
      htmlPage2 += plot[cnt][i];
    }
    htmlPage2 += "\n";
    htmlPage2 += "</pre></td>";
  }

  v1 = 0;
  v2 = 0;

  // start html
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  //client.println("Refresh: 1");       // refresh the page automatically
  client.println();

  String htmlPage = html_1;
  client.println(htmlPage);

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

  client.println("<p><b>Analog read</b><br/></p>");
  client.print("V1 = ");
  client.println(v1);
  client.print("<br/>V2 = ");
  client.println(v2);

  client.println("<br><br>");
  client.println("<table><tr><th>Name</th><th> </th></tr>");
  client.println(htmlPage2);
  client.println("</tr></table></body>");
  client.println("<p><input type=\"button\" value=\"Refresh\" onclick = \"location.href='/?refresh'\"></p>");
  client.println("</html>");

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
        /*
          Serial.write(c);
          if (httpRequest.length() < REQUEST_BUFFER) {
          httpRequest += c;
          }
        */

        if (charCnt < (REQUEST_BUFFER - 1)) { //last element is 0 => null terminate string
          httpRequest[charCnt] = c;           // store each HTTP request character in HTTP_requst array
          charCnt++;
        }

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {

          htmlPage(client);

          // reset buffer
          charCnt = 0;
          for (int i = 0; i < REQUEST_BUFFER; i++) {
            httpRequest[i] = 0;
          }
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
    threads.delay(1);
    // close the connection:
    client.stop();
    Serial.println("Client disconnected.");
  }
}
