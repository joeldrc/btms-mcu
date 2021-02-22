

const char asciiFilledSquare[] = "&#9608;"; //'█';
const char asciiSpace[] = "_";              //'_';

const uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };

const char html_1[] PROGMEM = R"rawliteral(
<html>
<head>
<title>BTMS timing</title>
<meta charset="utf-8" content="url=/">
</head>
<body>
<h1>BTMS timing</h1>
<p><input type="button" value="Log Out" onclick ="location.href='/?logout'"></p>
<hr style="color: blue;">
<p><b>CONTROL PANEL</b></p>
)rawliteral";
// <meta charset="utf-8" http-equiv="refresh" content="1; url=/">
// <body onload="window.open(location.href='/', _top);">


const uint8_t  IECY =     0;
const uint8_t ICalStp =   1;
const uint8_t ICalStrt =  3;
const uint8_t ISCY  =     4;
const uint8_t Lock  =     12;
const uint8_t ADC10 =     24;
const uint8_t ADC11 =     25;
const uint8_t ADC12 =     26;
const uint8_t ADC13 =     27;
const uint8_t _10MHzDet = 28;
const uint8_t D10MHz =    33;
const uint8_t IHCH =      34;
const uint8_t SW6 =       16;
const uint8_t SW5 =       17;
const uint8_t SW4 =       18;
const uint8_t SW3 =       19;
const uint8_t SW2 =       20;
const uint8_t SW1 =       21;
const uint8_t SECY =      2;
const uint8_t SSCY =      5;
const uint8_t SHCH =      6;
const uint8_t BFrev4 =    7;
const uint8_t TEN =       8;
const uint8_t FastGPIO1 = 9;
const uint8_t SINJ =      10;
const uint8_t BFrev3  =   11;
const uint8_t StsLed1 =   13;
const uint8_t FastGPIO3 = 14;
const uint8_t FastGPIO2 = 15;
const uint8_t StsLedOr =  22;
const uint8_t StsLedGr =  23;
const uint8_t SCalStp =   29;
const uint8_t BFrev2 =    30;
const uint8_t SCalStrt =  31;
const uint8_t BFrev1  =   32;
const uint8_t StsLed8 =   35;
const uint8_t StsLed7 =   36;
const uint8_t StsLed6 =   37;
const uint8_t StsLed5 =   38;
const uint8_t StsLed4 =   39;
const uint8_t StsLed3 =   40;
const uint8_t StsLed2 =   41;


const uint32_t samplesNumber = 150; // 2400m
const uint8_t numTraces = 6;
const char traceName[numTraces][10] = {{"SCY"}, {"CALSTRT"}, {"CALSTOP"}, {"INJ"}, {"HCH"}, {"ECY"}};
