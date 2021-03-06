
#include <SoftwareSerial.h>
const byte rxPin = 16;
const byte txPin = 17;
SoftwareSerial myremote(16, 17); //Tx & Rx is connected to Arduino #12 & #11

#include <WiFi.h>
// Replace with your network credentials
const char* ssid = "Better than Roi's";
const char* password = "0509221149";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);
  pinMode(rxPin,INPUT);
  pinMode(txPin,OUTPUT);
  myremote.begin(115200);
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              if(output26State=="off")
              {
                Serial.println("Ac-On");
                output26State = "on";
                }

                else
                {
                  Serial.println("Ac-OFF");
                  output26State = "off";
                  }
              
                myremote.println("o");
               delay(1500);
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /27/tempdown") >= 0) {
              Serial.println("Temp decreased");
                myremote.println("d");
                delay(1500);
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /27/tempUp") >= 0) {
              Serial.println("Temp Raised");
                myremote.println("u");
               delay(1500);
            }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #FF0000;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Ac Controller Interface</h1>");
            
            // Display buttons 
            client.println("<p>AC - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/on\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display raise temp  
            client.println("<p>raise temp</p>");
            // If the output27State is off, it displays the ON button       
            
            client.println("<p><a href=\"/27/tempUp\"><button class=\"button\">Temp Up+</button></a></p>");

            client.println("<p>Temp Down</p>");
            // If the output27State is off, it displays the ON button       
            
            client.println("<p><a href=\"/27/tempdown\"><button class=\"button\">Temp Down - </button></a></p>");
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
