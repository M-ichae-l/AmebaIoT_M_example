/*

 */
#include <avr/pgmspace.h>
#include "epdif.h"
#include "epdpaint.h"
#include "image.h"
#include "qrcode.h"

#include <WiFi.h>
#include "FatFs_SD.h"
#include "IRDevice.h"

//IR pin seetings
#define IR_RX_PIN           3
#define IR_TX_PIN           6

// SD cand buffer max size
#define READ_BUF_SIZE       256

//Eink screen pin settings
#define EPD_BUSY_Pin        21
#define EPD_RES_Pin         20
#define EPD_SPI_CS_Pin      12
#define EPD_SPI_MOSI_Pin    9
#define EPD_SPI_MISO_Pin    10
#define EPD_SPI_CLK_Pin     11

#define COLORED             0
#define UNCOLORED           1
#define QRCODE_VERSION      5

EpdIf EPD(EPD_BUSY_Pin, EPD_RES_Pin, EPD_SPI_CS_Pin, EPD_SPI_MOSI_Pin, EPD_SPI_MISO_Pin, EPD_SPI_CLK_Pin);
unsigned char image[1024] = {0};
Paint paint(image, 0, 0);     // width should be the multiple of 8

// WIFI setings
char SD_ssid[READ_BUF_SIZE] = {0};                          //  your network SSID (name)
char SD_pass[READ_BUF_SIZE] = {0};                          // your network password
int keyIndex = 0;                                           // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
WiFiServer server(80);
IPAddress ip;

// FatFs setting
char filename_wifi_ssid[] = "wifi_ssid.txt";
char filename_wifi_pass[] = "wifi_pass.txt";
char filename_freq_power[] = "freq_power.txt";
char filename_vol_p[] = "vol_p.txt";
char filename_vol_m[] = "vol_m.txt";
char filename_ch_p[] = "ch_p.txt";
char filename_ch_m[] = "ch_m.txt";

unsigned int int_buf[1024] = {0};
int int_buf_count = 0;

int carrier_freq = 0;
int int_data_count_freq_power = 0;
int int_data_count_vol_p = 0;
int int_data_count_vol_m = 0;
int int_data_count_ch_p = 0;
int int_data_count_ch_m = 0;

unsigned int int_data_power[1024] = {0};
unsigned int int_data_freq_power[1024] = {0};
unsigned int int_data_vol_p[1024] = {0};
unsigned int int_data_vol_m[1024] = {0};
unsigned int int_data_ch_p[1024] = {0};
unsigned int int_data_ch_m[1024] = {0};

FatFsSD fs;

// sub functions
void read_SD_html_data (void) {}

void read_SD_wifi_data (char* filename, char* data) {
    char buf[READ_BUF_SIZE];
    char absolute_filename[128];
    sprintf(absolute_filename, "%s%s", fs.getRootPath(), filename);
    SdFatFile file = fs.open(absolute_filename);
    memset(buf, 0, sizeof(buf));
    file.read(buf, sizeof(buf));
    file.close();
//    printf("%s \r\n", filename);
//    printf("%s \r\n", buf);

    for (unsigned int i = 0; i < sizeof(buf); i++) {
//        printf("    %x  ---  ", data[i]);
        if ((buf[i] == '\r') || (buf[i] == '\n')) {
            break;
        }
        data[i] = buf[i];
//        printf("%x \r\n", data[i]);
    }
}

void read_SD_data (char* filename, int &int_data_count, unsigned int* int_data) {
    char buf[READ_BUF_SIZE];
    char absolute_filename[128];
    sprintf(absolute_filename, "%s%s", fs.getRootPath(), filename);
    SdFatFile file = fs.open(absolute_filename);
    memset(buf, 0, sizeof(buf));
    file.read(buf, sizeof(buf));
    file.close();
    buf_convert(buf, int_data_count, int_data);
}

void buf_convert(char* buf, int &int_data_count, unsigned int *int_data) {
    int_buf_count = 0;
    int_data_count = 0;
    int temp_data = 0;
    int temp_ten = 1;

    for (int i = 0; i < READ_BUF_SIZE; i++) {
        if (buf[i] != 10) {
            int_buf[int_buf_count] = buf[i];
            int_buf_count++;
        } else {
            int temp_y = int_buf_count;
            for (int x = 0; x < (int_buf_count - 1); x++) {
                for (int y = 0; y < (temp_y - 2); y++) {
                    temp_ten = 10 * temp_ten;
                }
                temp_y--;
                temp_data = temp_data + ascii_to_int(int_buf[x]) * temp_ten;
                temp_ten = 1;
            }
            memset(int_buf, 0, sizeof(int_buf));
            int_data[int_data_count] = temp_data;
            int_data_count++;
            int_buf_count = 0;
            temp_data = 0;
        }
    }
}

int ascii_to_int(uint8_t ascii_number) {
    if (ascii_number == 48) {
        return 0;
    } else if (ascii_number == 49) {
        return 1;
    } else if (ascii_number == 50) {
        return 2; 
    } else if (ascii_number == 51) {
        return 3; 
    } else if (ascii_number == 52) {
        return 4; 
    } else if (ascii_number == 53) {
        return 5; 
    } else if (ascii_number == 54) {
        return 6; 
    } else if (ascii_number == 55) {
        return 7; 
    } else if (ascii_number == 56) {
        return 8; 
    } else if (ascii_number == 57) {
        return 9; 
    } else {
        //printf("  Error in ascii_to_int \r\n");
        return -1;
    }
}

void printWifiStatus() {
    // print the SSID of the network you're attached to:
    Serial.println();
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

void PrintQRCode(const char * url){
    QRCode qrcode;
    const int ps = 2;               // pixels / square : used to define the size of QR
    uint8_t qrcodeData[qrcode_getBufferSize(QRCODE_VERSION)];
    qrcode_initText(&qrcode, qrcodeData, QRCODE_VERSION, ECC_LOW, url);

    paint.SetRotate(ROTATE_0);
    paint.SetWidth (80);
    paint.SetHeight(80);
    paint.Clear(UNCOLORED);

    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                for (int xi = (x * ps + 2); xi < (x * ps + ps + 2); xi++) { // If pixel is on, we draw a "ps x ps" black square
                    for (int yi = (y * ps + 2); yi < (y * ps + ps + 2); yi++) {
                        paint.DrawPixel(xi, yi, COLORED);
                    }
                }
            }
        }
    }
    EPD.EPD_ClearScreen_White();  // Display whole screen in white
    EPD.EPD_Dis_Part(30, 200, paint.GetImage(), paint.GetWidth(), paint.GetHeight());
    EPD.EPD_UpdateDisplay();
}
// end if the sub function 

void setup() {
//logs
    Serial.begin(115200);

#if 0
#define EPD_BUSY_Pin        21
#define EPD_RES_Pin         20
#define EPD_SPI_CS_Pin      12
#define EPD_SPI_MOSI_Pin    9
#define EPD_SPI_MISO_Pin    10
#define EPD_SPI_CLK_Pin     11
#endif

// eink init
    pinMode(EPD_BUSY_Pin, INPUT);       // EPD_BUSY_Pin
    pinMode(EPD_RES_Pin, OUTPUT);       // EPD_RES_Pin
    pinMode(EPD_SPI_CS_Pin, OUTPUT);    // EPD_SPI_CS_Pin
    pinMode(EPD_SPI_MOSI_Pin, OUTPUT);  // EPD_SPI_MOSI_Pin
    pinMode(EPD_SPI_MISO_Pin, OUTPUT);  // EPD_SPI_MISO_Pin
    pinMode(EPD_SPI_CLK_Pin, OUTPUT);   // EPD_SPI_CLK_Pin

// SD card reading
    fs.begin();

//// read the TV data
    read_SD_data(filename_freq_power, int_data_count_freq_power, int_data_freq_power);
    carrier_freq = int_data_freq_power[0];
    for (int i = 0; i < (int_data_count_freq_power - 1); i++) {
        int_data_power[i] = int_data_freq_power[(i + 1)];
    }
    read_SD_data(filename_vol_p, int_data_count_vol_p, int_data_vol_p);
    read_SD_data(filename_vol_m, int_data_count_vol_m, int_data_vol_m);
    read_SD_data(filename_ch_p, int_data_count_ch_p, int_data_ch_p);
    read_SD_data(filename_ch_m, int_data_count_ch_m, int_data_ch_m);

//// read the wifi data
    read_SD_wifi_data(filename_wifi_ssid, SD_ssid);
    read_SD_wifi_data(filename_wifi_pass, SD_pass);
    fs.end();

// wifi connection 
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to Network named: ");
        Serial.println(SD_ssid);                   // print the network name (SSID);

        status = WiFi.begin(SD_ssid, SD_pass);
        //status = WiFi.begin(ssid, pass);
        // wait 10 seconds for connection:
        delay(10000);
    }

//server connection 
    server.begin();                           // start the web server on port 80
    //printWifiStatus();
    ip = WiFi.localIP();

// QR code setup
    EPD.EPD_SetRAMValue_BaseMap(gImage_basemapNA);
    PrintQRCode(ip.get_address());
    //EPD.EPD_Sleep();
}

void loop() {
// create the client
    WiFiClient client = server.available();     // listen for incoming clients

// check if client available
    if (client) {                               // if you get a client,
        Serial.println("new client");           // print a message out the serial port
        String currentLine = "";                // make a String to hold incoming data from the client
        while (client.connected()) {            // loop while the client's connected
            if (client.available()) {           // if there's bytes to read from the client,
                char c = client.read();         // read a byte, then
                Serial.write(c);                // print it out the serial monitor
                if (c == '\n') {                // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
// start html
                        client.println("<!DOCTYPE HTML>");
                        client.println("<html>");
                        client.println("<head>");
                        client.println("<meta charset=\"utf-8\">");
                        client.println("<title>IR Remote Control</title>");
                        client.println("<meta content=\"width=device-width, initial-scale=1, user-scalable=no\" name=\"viewport\">");

                        client.println("<style>");
                        client.println("a {text-decoration: none;}");
                        client.println("* {margin: 0; padding: 0; box-sizing: border-box; font-family: Arial, sans-serif;}");
                        client.println("html {height: 100%; background: #ffffff; background: radial-gradient(circle, #fff 20%, #ccc); background-size: cover;}");
                        client.println(".wrapper {width: calc(100% - 24px); max-width: 200px; padding: 12px 12px; margin: 0 auto;}");

                        client.println(".buttons button {display: block; border: none; clear: both; background: #00A6DC; box-shadow: 0px 4px #00749a; border-radius: 6px; margin-bottom: 12px; width: 60px; height: 60px; position: relative; top: 0; cursor: pointer; outline: none; color: #fff; line-height: 60px; font-size: 14px; text-align: center; user-select: none; -webkit-transition: all 0.2s ease; -moz-transition: all 0.2s ease; -ms-transition: all 0.2s ease; -o-transition: all 0.2s ease; transition: all 0.2s ease;}");
                        client.println(".buttons button.full {width: 50%;}");
                        client.println(".buttons button.power {background: #FF3A19; box-shadow: 0px 4px #cc2e14; margin-bottom: 24px;}");
                        client.println(".buttons button:hover {background: #0095c6; box-shadow: 0px 4px #006484;}");
                        client.println(".buttons button.power:hover {background: #e63417; box-shadow: 0px 4px #99230f;}");
                        client.println(".buttons button:active {top: 4px; box-shadow: 0px 0px #006484;}");
                        client.println(".buttons button.power:active {top: 4px; box-shadow: 0px 0px #99230f;}");
                        client.println(".pull-left {display: block; float: left; width: 50%; clear: left;}");
                        client.println(".pull-right {display: block; float: right; width: 50%; clear: right;}");
                        client.println(".pull-right button {margin: 0 0 12px auto;}");
                        client.println(".buttons:before,");
                        client.println(".buttons:after,");
                        client.println(".pull-left:before,");
                        client.println(".pull-left:after,");
                        client.println(".pull-right:before,");
                        client.println(".pull-right:after {content: ""; display: table;}");
                        client.println(".buttons:after,");
                        client.println(".pull-left:after,");
                        client.println(".pull-right:after {clear: both;}");
                        client.println("</style>");

                        client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script>");
                        client.println("<script>");
                        client.println("$(document).ready(function(){$(\"button\").click(function(){var url = \"/?playback=\" + $(this).attr('data-id'); $.ajax({url: url}); }); });");
                        client.println("</script>");
                        client.println("</head>");

                        client.println("<body>");
                        client.println("<div class=\"wrapper\">");
                        client.println("<!-- Buttons -->");
                        client.println("<div class=\"buttons\">");
                        client.println("<a href=\"/P\"><button data-id=\"101\" class=\"power full\">ON/OFF</button></a>");
                        client.println("<div class=\"pull-left\">");
                        client.println("<a href=\"/Vp\"><button data-id=\"102\">VOL +</button></a>");
                        client.println("<a href=\"/Vm\"><button data-id=\"103\">VOL -</button></a>");
                        client.println("</div>");
                        client.println("<div class=\"pull-right\">");
                        client.println("<a href=\"/Cp\"><button data-id=\"104\">CH +</button></a>");
                        client.println("<a href=\"/Cm\"><button data-id=\"105\">CH -</button></a>");
                        client.println("</div>");
                        client.println("</div>");
                        client.println("</div>");
                        client.println("</body>");
                        client.println("</html>");
// end html


                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    } else {    // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                } else if (c != '\r') {  // if you got anything else but a carriage return character,
                    currentLine += c;      // add it to the end of the currentLine
                }

// check user input and give IR signal to control TV
                // Check to see if the client request was "GET /H" or "GET /L":
                if (currentLine.endsWith("GET /P")) {
                    IR.begin(IR_RX_PIN, IR_TX_PIN, IR_MODE_TX, carrier_freq);
                    IR.send(int_data_power, (int_data_count_freq_power - 1));
                    Serial.println("Finish Send power on/off");
                }
                if (currentLine.endsWith("GET /Vp")) {
                    IR.begin(IR_RX_PIN, IR_TX_PIN, IR_MODE_TX, carrier_freq);
                    IR.send(int_data_vol_p, int_data_count_vol_p);
                    Serial.println("Finish Send vol +");
                }
                if (currentLine.endsWith("GET /Vm")) {
                    IR.begin(IR_RX_PIN, IR_TX_PIN, IR_MODE_TX, carrier_freq);
                    IR.send(int_data_vol_m, int_data_count_vol_m);
                    Serial.println("Finish Send vol -");
                }
                if (currentLine.endsWith("GET /Cp")) {
                    IR.begin(IR_RX_PIN, IR_TX_PIN, IR_MODE_TX, carrier_freq);
                    IR.send(int_data_ch_p, int_data_count_ch_p);
                    Serial.println("Finish Send ch +");
                }
                if (currentLine.endsWith("GET /Cm")) {
                    IR.begin(IR_RX_PIN, IR_TX_PIN, IR_MODE_TX, carrier_freq);
                    IR.send(int_data_ch_m, int_data_count_ch_m);
                    Serial.println("Finish Send ch -");
                }
            }
        }
        // close the connection:
        client.stop();
        Serial.println("client disonnected");
    } else {
        Serial.println("server not available");
    }
}
