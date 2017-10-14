/*
 Vasy MIT onlab konzulens:khazy
 */
// a webserver futtatásához
#include <Ethernet.h>

// the sensor communicates using SPI, so include the library:
#include <SPI.h>
// az LM75 kezeléséhez szükséges
#include <Wire.h>
#include <string.h>

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

//tesztváltozó kijelzéshez
int count = 0;
int t = 0;
unsigned long int row = 0;

// a webservernek átadott adatok tárolásához
char tmp_buffer[500];
int last50temp[50];
long filepos=0;
String last50timestamp[50];

FILE *myFile;

// a mért adatok tárolásához txt létrehozása
const char* filename = "/media/mmcblk0p1/onlab/test29.txt";

void listenForEthernetClients();

void setup() {
  // start the SPI library:
  sleep(3);
  SPI.begin();
  Wire.begin();
  // start the Ethernet connection and the server:
  server.begin();

  Serial.begin(9600);


  // give the sensor and Ethernet shield time to set up:
  delay(1000);

  // Create the file, a tesztelés miatt szükséges a helyes szintaktika beállításához
  fclose(fopen(filename, "w"));
}



void loop()
{
  //LM 75 báziscíme a fizikális bekötés alapján
  Wire.requestFrom(0x4B, 1);

  //szenzor beolvasása
  while (Wire.available())
  {
    t = Wire.read();
  }
  delay(500);

  //save data to a txt file
  myFile = fopen(filename, "a");
  fprintf(myFile, "%d # %d °C # ", row, t);
  fclose(myFile);
  system("date >> /media/mmcblk0p1/onlab/test29.txt");


  // listen for incoming Ethernet connections:
  listenForEthernetClients();
  
  //legutolsó beolvasott érték sorszáma
  row++;

  count++;
  if (count > 100)
    count = 0;


  delay(500);
}

// a webszervert futtató függvény
void listenForEthernetClients() {
  int temperature;
  unsigned long int temp_row=0;
  char tmp_date[100];

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
          client.println("Refresh: 5");  // refresh the page automatically every 10   sec
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

          // grafikon feltöltése adatokkal, a fájlból kiolvasva
          myFile = fopen(filename, "r");
          fseek(myFile, filepos, SEEK_SET);
          do
          {
            fscanf(myFile, "%d # %d °C # %[^\n]s\n", &temp_row, &temperature, tmp_date);
            sprintf(tmp_buffer, "%d %d %s (row: %d)\n", temp_row, temperature, tmp_date, row);
            Serial.println(tmp_buffer);
            if (temp_row + 10 >= row)
            {
           	  if (temp_row + 10 == row)
           	  {
           	  	filepos = ftell(myFile);
           	  }
              sprintf(tmp_buffer, "data.addRow([(new Date(\"%s\")),%d]);", tmp_date, temperature);
              client.println(tmp_buffer);
            }
          } while (temp_row < row);
		  fclose(myFile);


          client.println(F("var chart = new google.visualization.LineChart($('#chart').get(0));"));
          client.println(F("chart.draw(data, {"));
          client.println(F("  title: 'Weather Station'"));
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