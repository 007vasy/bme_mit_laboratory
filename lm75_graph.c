/*
 Vasy MIT onlab konzulens:khazy
 */

#include <Ethernet.h>
// the sensor communicates using SPI, so include the library:
#include <SPI.h>

#include <Wire.h>
#include <string.h>






// Galileo 2 board mac address
byte mac[] = {
  0x98, 0x4F, 0xEE, 0x05, 0x35, 0xC5
};
// assign an IP address for the controller:
IPAddress ip(152, 66, 210, 66);
IPAddress gateway(152, 66, 210, 127);
IPAddress subnet(255, 255, 255, 128);


// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

//tesztváltozó kijelzéshez
int count = 0;
int t = 0;
unsigned long int row = 0;

int last50temp[50];
String last50timestamp[50];

FILE *myFile;

const char* filename = "/media/mmcblk0p1/onlab/test28.txt";

void setup() {
  // start the SPI library:
  sleep(3);
  SPI.begin();
  Wire.begin();
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();

  Serial.begin(9600);


  // give the sensor and Ethernet shield time to set up:
  delay(1000);

// Create the file

  system("touch" + *filename);

}


void loop()
{
  Wire.requestFrom(0x4B, 1);

  while (Wire.available())
  {
    t = Wire.read();
  }
  delay(500);

  //save data to a txt file
  system("touch" + *filename);
  myFile = fopen(filename, "r");
  fprintf(myFile, "%d # %d °C # ", row, t);
  row++;
  system("date >> /media/mmcblk0p1/onlab/test28.txt");

  fclose(myFile);

  // listen for incoming Ethernet connections:
  listenForEthernetClients();
  count++;
  if (count > 100)
    count = 0;


  delay(500);
}

void listenForEthernetClients() {
  int temperature;
  unsigned long int temp_row;
  char day[3], month[3], dayint[2], temp_time[8], utc[3], year[4];

  // listen for incoming clients


  EthernetClient client = server.available();


  if (client) {
    Serial.println("Got a client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 10");  // refresh the page automatically every 10   sec
          client.println();
          client.println("<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>");
          client.println("<html>");
          client.println("<head>");

          client.println("<title>T: " + String(t) + " °C</title>");

          client.println("<meta http-equiv='content-type' content='text/html; charset=utf-8' />");

          client.println("<meta name='author' content='Vasy' />");

          client.println("<meta name='copyright' content='Vasy' />");


          client.println(F("<script src='https://ajax.googleapis.com/ajax/libs/jquery/1.11.1/jquery.min.js'></script>"));

          client.println(F("<script src='https://www.google.com/jsapi'></script>"));

          client.println(F("<script>"));
          client.println(F("function drawChart() {"));
          client.println(F("var public_key = 'dZ4EVmE8yGCRGx5XRX1W';"));
          

          client.println(F("var data = new google.visualization.DataTable();"));

          client.println(F(" data.addColumn('datetime', 'Time');"));

          client.println(F("data.addColumn('number', 'Temp');"));




          system("touch" + *filename);
          myFile = fopen(filename, "w+");
          do
          {
            fscanf(myFile, "%d # %d °C # %s %s %s %s %s %s\n", &temp_row, &temperature, day, month, dayint, temp_time, utc, year);
            if (temp_row >= row - 50)
            {
              client.println(F("data.addRow(["));
              client.println(F('(new Date(" + year + "." + month + "." + dayint + " " + temp_time + ")),parseInt(" + temperature + ")'));
              client.println(F("]);"));
            }
          } while (temp_row <= row);


          client.println(F("var chart = new google.visualization.LineChart($('#chart').get(0));"));
          client.println(F("chart.draw(data, {"));
          client.println(F("title: 'Weather Station'"));
          client.println(F("});"));
          client.println(F("});"));
          client.println(F("}"));//drawChart

          client.println(F("google.load('visualization', '1', {"));
          client.println(F("packages: ['corechart']"));
          client.println(F("});"));

          client.println(F("google.setOnLoadCallback(drawChart);"));
          client.println(F("</script>"));


          client.println("</head>");

          client.println("<body>");
          client.println("<?php>");
          client.println("<?>");

          // print the current readings, in HTML format:
          client.print("<p align='center'>Temperature: " + String(t));


          client.print(" °C</p>");
          client.println("<br />");

          client.println("<p align='center'>Counter: " + String(count) + "</p>");
          client.println("<br />");

          client.println(" <div id='chart' style='width: 100%%;'></div>");

          client.println("</body>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}


