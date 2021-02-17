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


//#include <SPI.h>
#include "src\NativeEthernet\NativeEthernet.h"
#include "src\TeensyThreads\TeensyThreads.h"
#include "defines.h"
#include "wb_page.h"


const uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };
EthernetServer server(80);


// Global variables
String password = "123456";
uint8_t ledState = false;               // ledState used to set the LED

bool luci_ant = false;
bool luci_post = false;

bool status1 = false;
bool status2 = false;
bool status3 = false;
bool status4 = false;
bool status5 = false;

float v1 = 0;
float v2 = 0;

String readString;

const uint32_t samplesNumber = 150; // 2400ms
bool values[samplesNumber];
String row[3][samplesNumber];


void asciiConversion(bool* val, const uint32_t numVal) {
  for (uint32_t i = 0; i < numVal; i++) {
    if (i == 0) {
      if (val[i] == 1 && val[i + 1] == 0) {
        row[0][i] = "&nbsp;";  //' ';
        row[1][i] = "&#9474;"; //'│';
        row[2][i] = "&#9524;"; //'┴';
      }
      else if (val[i] == 1) {
        row[0][i] = "&#9484;"; //'┌';
        row[1][i] = "&#9474;"; //'│';
        row[2][i] = "&#9496;"; //'┘';
      }
      else if (val[i] == 0) {
        row[0][i] = "&nbsp;";  //' ';
        row[1][i] = "&nbsp;";  //' ';
        row[2][i] = "&#9472;"; //'─'
      }
    }
    else if ( i < numVal - 1) {
      if (val[i - 1] == 0 && val[i] == 1 && val[i + 1] == 0) {
        row[0][i] = "&nbsp;";  //' ';
        row[1][i] = "&#9474;"; //'│';
        row[2][i] = "&#9524;"; //'┴';
      }
      else if (val[i] == 1 && val[i + 1] == 0) {
        row[0][i] = "&#9488;"; //'┐';
        row[1][i] = "&#9474;"; //'│';
        row[2][i] = "&#9492;"; //'└';
      }
      else if (val[i - 1] == 0 && val[i] == 1) {
        row[0][i] = "&#9484;"; //'┌';
        row[1][i] = "&#9474;"; //'│';
        row[2][i] = "&#9496;"; //'┘';
      }
      else if (val[i] == 1 && val[i + 1] == 1) {
        row[0][i] = "&#9472;"; //'─'
        row[1][i] = "&nbsp;";  //' ';
        row[2][i] = "&nbsp;";  //' ';
      }
      else if (val[i] == 0) {
        row[0][i] = "&nbsp;";  //' ';
        row[1][i] = "&nbsp;";  //' ';
        row[2][i] = "&#9472;"; //'─'
      }
    }
    else {
      if (val[i - 1] == 0 && val[i] == 1) {
        row[0][i] = "&#9484;"; //'┌';
        row[1][i] = "&#9474;"; //'│';
        row[2][i] = "&#9496;"; //'┘';
      }
      else if (val[i - 1] == 1 && val[i] == 1) {
        row[0][i] = "&#9472;"; //'─'
        row[1][i] = "&nbsp;";  //' ';
        row[2][i] = "&nbsp;";  //' ';
      }
      else if (val[i] == 0) {
        row[0][i] = "&nbsp;";  //' ';
        row[1][i] = "&nbsp;";  //' ';
        row[2][i] = "&#9472;"; //'─'
      }
    }
  }
}


void ethernetConfig_thread() {
  Ethernet.begin(mac, 60000, 4000); // unsigned long timeout = 60000, unsigned long responseTimeout = 4000
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.");
  }

  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
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


void webServer_thread() {
  while (1) {
    EthernetClient client = server.available(); // listen for incoming clients
    if (client) {
      Serial.println("new client");
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          if (readString.length() < 100) {
            readString = readString + c;
          }
          if (c == '\n') {
            if (readString.indexOf("?pwd=" + password) > 0) {
              status5 = true;
            }
            if (readString.indexOf("?logout") > 0) {
              status5 = false;
            }

            // start html
            //client.print(index_html);

            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println();

            client.println("<html>");
            client.println("<head>");
            client.println("<title>BTMS timing</title>");

            // when logged In
            if (status5) {
              //Read command and change port status
              if (readString.indexOf("?b1") > 0) {
                //digitalWrite(switch1, !status1);
                //status1 = digitalRead(switch1);
                status1 = !status1;
              }
              if (readString.indexOf("?b2" ) > 0) {
                //digitalWrite(switch2, !status2);
                //status2 = digitalRead(switch2);
                status2 = !status2;
              }
              if (readString.indexOf("?b3") > 0) {
                //digitalWrite(switch3, !status3);
                //status3 = digitalRead(switch3);
                status3 = !status3;
              }
              if (readString.indexOf("?b4") > 0) {
                //digitalWrite(switch4, !status4);
                //status4 = digitalRead(switch4);
                status4 = !status4;
              }
              if (readString.indexOf("luci_anteriori") > 0) {
                luci_ant = !luci_ant;
              }
              if (readString.indexOf("luci_posteriori") > 0) {
                luci_post = !luci_post;
              }

              client.println("<meta http-equiv=\"refresh\" content=\"2;url=/\">"); //auto refresh every 2 seconds
              client.println("</head>");
              client.println("<body>");
              client.println("<h1>BTMS timing</h1>");
              client.println("<p><input type=\"button\" value=\"Log Out\" onclick = \"location.href='/?logout'\"></p>");
              client.println("<hr style=\"color: blue;\">");
              client.println("<p><b>CONTROL PANEL</b></p>");
              client.println("<table border=\"1px solid black;\" cellpadding=\"10px;\">");
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

              client.println("<pre>");

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
                values[i] = val;
              }

              asciiConversion(values, samplesNumber);

              for (uint32_t i = 0; i < samplesNumber; i++) {
                client.print(row[0][i]);
              }
              client.println("");

              for (uint32_t i = 0; i < samplesNumber; i++) {
                client.print(row[1][i]);
              }
              client.println("");

              for (uint32_t i = 0; i < samplesNumber; i++) {
                client.print(row[2][i]);
              }
              client.println("");

              for (uint32_t i = 0; i < samplesNumber; i++) {
                client.print(values[i]);
              }
              client.println("");

              client.println("</pre>");
              client.println("</body>");
            }
            else {
              client.println("<h1>BTMS timing</h1>");
              client.println("<form action=/>");
              client.println("<p><input type=\"password\" id=\"pwd\" name=\"pwd\" maxlength=\"6\">");
              client.println("<button type=”submit”>Log In</button></p>");
              client.println("</form>");
              client.println("<hr style=\"color: blue;\">");
              client.println("</head>");
            }

            client.println("</html>");


            //          client.println("<!DOCTYPE HTML>");
            //          client.println("<html>");
            //          client.println("<meta http-equiv=\"refresh\" content=\"2;url=/\">"); //auto refresh every 2 seconds
            //          client.println("<head>");
            //          client.println("<title>Admin ROVER</title>");
            //          client.println("<meta charset=\"utf-8\">");
            //          client.println("<style tipe=\"text/css\">");
            //          client.println(".bottone1 {background: gray; font-weight: bold; font-family:Verdana; box-shadow: 0px 0px 30px #000000; border-radius: 20px; width: 90px; height: 50px; border-width: 2px; border: 4px solid #000000; }");
            //          client.println(".bottone2 {background: #404040; font-weight: bold; font-family:Verdana; box-shadow: 0px 0px 30px #000000; border-radius: 20px; width: 60px; height: 50px; border-width: 2px; border: 4px solid #000000; }");
            //          client.println(".bottone3 {background: black; font-weight: bold; font-family:Verdana; border-radius: 20px; width: 25px; height: 25px; border: 0px solid #000000; box-shadow: 0px 0px 30px #000000; color: white; }");
            //          client.println(".bottone4 {background: gray;font-weight: bold; font-family:Verdana; box-shadow: 0px 0px 30px #000000; border-radius: 5 px; width: 10px; height: 15px; border-width: 2px; border: 2px solid #000000; }");
            //          client.println(".bottone5 {background: yellow;font-weight: bold; font-family:Verdana; box-shadow: 0px 0px 30px #000000; border-radius: 5 px; width: 10px; height: 15px; border-width: 2px; border: 2px solid #000000; }");
            //          client.println(".bottone6 {background: red;font-weight: bold; font-family:Verdana; box-shadow: 0px 0px 30px #000000; border-radius: 5 px; width: 10px; height: 15px; border-width: 2px; border: 2px solid #000000; }");
            //          client.println(".bottone7 {background: lime;font-weight: bold; font-family:Verdana; box-shadow: 0px 0px 30px #000000; border-radius: 5 px; width: 10px; height: 15px; border-width: 2px; border: 2px solid #000000; }");
            //          client.println("</style>");
            //          client.println("<script language=JavaScript>");
            //          client.println("function on(colore) {event.srcElement.style.backgroundColor=colore}"); // funzione per cambio colore onClick
            //          client.println("function tastiera() {location.href=(String.fromCharCode(event.keyCode) +\"x\"); }; "); // funzione per lettura tastiera onKey...
            //          client.println("</script>");
            //          client.println("</head>");
            //          client.println("<body bgcolor= \"#404040\" onkeydown=\"tastiera()\" onkeyup=\"location.href='' \" >"); // <-- inizia la funzione onKey... //onkeyup=\"location.href='stop'
            //          client.println("<table align=\"center\" bgcolor= \"silver\" border=\"0\" cellpadding=\"1\" cellspacing=\"1\" style=\"width: 1250px; border: 10px solid #000000; border-radius: 20px; background-image: linear-gradient(top, #ffffff, #000000); box-shadow: 0px 0px 30px #000000; background-clip: padding-box;\">");
            //          client.println("<tbody>");
            //          client.println("<tr>");
            //          client.println("<td width=\"25%\">");
            //          client.println("<p style=\"text-align: center;\">");
            //          client.println("<strong><span style=\"font-size:16px;\"><span style=\"font-family:verdana,geneva,sans-serif;\">PANNELLO DI CONTROLLO</span></span></strong></p>");
            //          client.println("</td>");
            //          client.println("<td style=\"text-align: center;\" width=\"50%\">");
            //          client.println("<p style=\"text-align: center;\">");
            //          client.println("<span style=\"font-family:verdana,geneva,sans-serif; font-size:20px;\">&nbsp<strong>[Rover V1.0]</strong></span></p>");
            //          client.println("</td>");
            //          client.println("<td width=\"25%\">");
            //          client.println("<p style=\"text-align: right;\">");
            //          client.println("<strong><span style=\"font-size:16px;\"><span style=\"font-family:verdana,geneva,sans-serif;\">");
            //          client.println("BATTERIA:&nbsp;");
            //          //client.println(percentuale_batteria);
            //          client.println("% &nbsp;");
            //          //client.println(tensione_batteria);
            //          client.println("V &nbsp;&nbsp;&nbsp;");
            //          client.println("<input class=\"bottone3\" title=\"Vuoi resettare la pagina?\" name=\"Reset\" onmouseover=\"on('red')\" onmouseout=\"on('black')\" onclick=\"location.href='reset'\" type=\"button\" value=\"&#8635\" /></span></strong></p>");
            //          client.println("</td>");
            //          client.println("</tr>");
            //          client.println("<tr>");
            //          client.println("<td width=\"25%\" align=\"middle\" style=\"text-align: center; \" height=\"25%\" >");
            //          client.println("<strong><span style=\"font-size:16px;\"><span style=\"font-family:verdana,geneva,sans-serif;\"> ");
            //          client.println("COMANDI:<br><br>");
            //          client.println("LUCI ANT &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span style=\"font-family:verdana,geneva,sans-serif;\"><input class=\" ");
            //          if (luci_ant)
            //            client.println("bottone5");
            //          else
            //            client.println("bottone4");
            //          client.println(" \" title=\" Luci anteriori \" name=\"Luci anteriori\" type=\"button\" onclick=\"location.href='luci_anteriori'\" value=\"\" /></span>");
            //          client.println("<br><br>");
            //          client.println("LUCI POST &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span style=\"font-family:verdana,geneva,sans-serif;\"><input class=\" ");
            //          if (luci_post)
            //            client.println("bottone5");
            //          else
            //            client.println("bottone4");
            //          client.println(" \" title=\" Luci posteriori \" name=\"Luci posteriori\" type=\"button\" onclick=\"location.href='luci_posteriori'\" value=\"\" /></span>");
            //          client.println("<br><br>");
            //          client.println("LAMP. &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span style=\"font-family:verdana,geneva,sans-serif;\"><input class=\" ");
            //
            //          client.println(" \" title=\"Lampeggiante\" name=\"Lampeggiante\" type=\"button\" onclick=\"location.href='lampeggiante'\" value=\"\" /></span>");
            //          client.println("<br><br>");
            //          client.println("Nav Assist &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span style=\"font-family:verdana,geneva,sans-serif;\"><input class=\" ");
            //
            //          client.println(" \" title=\" Controllo sensori \" name=\"Controllo sensori\" type=\"button\" onclick=\"location.href='controllo_sensori'\" value=\"\" /></span>");
            //          client.println("<br><br>");
            //          client.println("Laser &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span style=\"font-family:verdana,geneva,sans-serif;\"><input class=\" ");
            //          /*
            //            if (laser_cmd)
            //            client.println("bottone7");
            //            else
            //            client.println("bottone4");
            //          */
            //          client.println(" \" title=\" Laser \" name=\"Laser\" type=\"button\" onclick=\"location.href='laser_cmd'\" value=\"\" /></span>");
            //          client.println("<br><br><br>IMP. MOTORI:<br><br>");
            //          client.println("Precisione &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span style=\"font-family:verdana,geneva,sans-serif;\"><input class=\" ");
            //          /*
            //            if (motore_di_precisione)
            //            client.println("bottone6");
            //            else
            //            client.println("bottone4");
            //          */
            //          client.println(" \" title=\" Consigliato per manovre in zone strette \" name=\"Precisione\" type=\"button\" onclick=\"location.href='motor_analog'\" value=\"\" /></span>");
            //          client.println("<br><br>");
            //          client.println("</span></span></strong>");
            //          client.println("</td>");
            //          client.println("<td style=\"text-align: center;\" width=\"50%\">");
            //          //client.println("<iframe title=\"IpCam Rover\" align=\"middle\" bgcolor=\"gray\" frameborder=\"0\" height=\"540\" id=\"ipcam\" scrolling=\"no\" src=\"http://192.168.0.230/video/livemb.asp\" style=\" box-shadow: 0px 0px 30px #000000; border-radius: 20px; border: 10px solid \" width=\"655\"></iframe></td>");
            //          client.println("<td width=\"25%\">");
            //          client.println("<p style=\"text-align: center;\">");
            //          client.println("<strong><span style=\"font-size:18px;\"><span style=\"font-family:verdana,geneva,sans-serif;\">");
            //          /*
            //            if (allarme_sensori)
            //            client.println("&#10082; PERICOLO! &#10082; <br> BLOCCO ROVER!");
            //            else if (cm <= dist_allarme_sens)
            //            client.println("&#10082; ATTENZIONE! &#10082; <br> Rischio collisione!");
            //            else
            //            client.println("ROVER OK");
            //          */
            //          client.println("<br><br>");
            //          client.println("</strong>");
            //          client.println("<strong><span style=\"font-size:16px;\"><span style=\"font-family:verdana,geneva,sans-serif;\">");
            //          client.println("Dist: &nbsp; ");
            //          //client.println(cm);
            //          client.println("&nbsp; cm. ");
            //          client.println("<br>");
            //          client.println("Vel: &nbsp;");
            //          client.println("</strong>");
            //          client.println("<strong><span style=\"font-size:18px;\"><span style=\"font-family:verdana,geneva,sans-serif;\">");
            //          client.println("&#8597;");
            //          client.println("</strong>");
            //          client.println("<strong><span style=\"font-size:16px;\"><span style=\"font-family:verdana,geneva,sans-serif;\">");
            //          client.println("&nbsp;");
            //          //client.println(velocita_y);
            //          client.println("</strong>");
            //          client.println("<strong><span style=\"font-size:18px;\"><span style=\"font-family:verdana,geneva,sans-serif;\">");
            //          client.println("&nbsp; &#8596;");
            //          client.println("</strong>");
            //          client.println("<strong><span style=\"font-size:16px;\"><span style=\"font-family:verdana,geneva,sans-serif;\">");
            //          client.println("&nbsp;");
            //          //client.println(velocita_x);
            //          client.println("<br><br>");
            //          client.println("<p style=\"text-align: center;\">");
            //          client.println("<input class=\"bottone1\" onmouseover=\"on('lime')\" onmouseout=\"on('gray')\" onclick=\"location.href='avanti'\" name=\"AVANTI\" type=\"button\" value=\"AVANTI\" /></p>");
            //          client.println("<p style=\"text-align: center;\">");
            //          client.println("<input class=\"bottone1\" onmouseover=\"on('lime')\" onmouseout=\"on('gray')\" onclick=\"location.href='sinistra'\" name=\"SINISTRA\" type=\"button\" value=\"SINISTRA\" />");
            //          client.println("&nbsp;");
            //          client.println("<input class=\"bottone2\" onmouseover=\"on('red')\" onmouseout=\"on('#404040')\" onclick=\"location.href='stop'\" name=\"STOP\" type=\"button\" value=\"STOP\" />");
            //          client.println("&nbsp;");
            //          client.println("<input class=\"bottone1\" onmouseover=\"on('lime')\" onmouseout=\"on('gray')\" onclick=\"location.href='destra'\" name=\"DESTRA\" type=\"button\" value=\"DESTRA\" /></p>");
            //          client.println("<p style=\"text-align: center; \">");
            //          client.println("<input class=\"bottone1\" onmouseover=\"on('lime')\" onmouseout=\"on('gray')\" onclick=\"location.href='indietro'\" name=\"INDIETRO\" type=\"button\" value=\"INDIETRO\" /></p>");
            //          client.println("</td>");
            //          client.println("</tr>");
            //          client.println("<tr>");
            //          client.println("<td>");
            //          client.println("</td>");
            //          client.println("<td>");
            //          client.println("</td>");
            //          client.println("<td>");
            //          client.println("</td>");
            //          client.println("</tr>");
            //          client.println("</tbody>");
            //          client.println("</table>");
            //          client.println("<p style=\"text-align: center;\">");
            //          client.println("<span style=\"font-size:10px;\"><span style=\"font-family: verdana, geneva, sans-serif; background-color: white;\">&nbsp;&nbsp;&nbsp;</span></span></p>");
            //          client.println("</body>");
            //          client.println("</html>");


            // give the web browser time to receive the data
            threads.delay(10);
            client.stop();
            readString = "";
          }
        }
      }
    }
  }
}


void setup() {
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
  Serial.println("Ethernet WebServer Example");

  threads.addThread(ethernetConfig_thread, 1);
  threads.addThread(statusLed_thread, 1);
  //threads.addThread(webServer_thread, 1);
}


void loop() {
  webServer_thread();
}
