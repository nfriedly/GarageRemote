/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 */

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);



// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

const int photoResistorAnalogPin = 0;

// constants won't change. Used here to set a pin number :
const int relayControlPin =  27;      // the number of the LED pin
const int relayGroundPin1 = 23;
const int relayGroundPin2 = 25;

// When was the last garage door button press started
unsigned long buttonPressStartTime = 0; 

// how long to "press" the garage door button
const long buttonPressLength = 500;

// this relay works the opposite of what you might expect, at least if you want the light to match the connection.
const bool BUTTON_PRESSED = LOW;
const bool BUTTON_RELEASED = HIGH;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  // setup relay control pin
  pinMode(relayControlPin, OUTPUT);
  digitalWrite(relayControlPin, BUTTON_RELEASED);
  
  // set up other relay pins (this is just because I want to plug it directly into my board instead of wiring things up properly)
  pinMode(relayGroundPin1, OUTPUT);
  pinMode(relayGroundPin2, OUTPUT);
  digitalWrite(relayGroundPin1, LOW);
  digitalWrite(relayGroundPin2, LOW);
}

void loop() {
  handleWeb();
  handleGarageDoorButton();
}

void handleWeb() {
    // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    //Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String method = "";
    boolean methodSent = false;
    String path = "";
    boolean pathSent = false;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        if(!pathSent) {
          if (!methodSent) {
            if (c == ' ') {
              methodSent = true; 
            } else {
              method += c;
            }
          } else if (c == ' ' || c == '?') {
            pathSent = true;
          } else {
            path += c;
          }
        }
        
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          if (method == "GET" && path == "/") {
              sendIndex(client);
          } else if (method == "POST" && path == "/door") {
            pressGarageDoorButton();
            sendRedirectToIndex(client);
          } else {
            send404(client);
          }
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
    //Serial.println("client disconnected");
  }
}

void sendIndex(EthernetClient client) {
    // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  //client.println("Refresh: 1");  // refresh the page automatically every 5 sec
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>Garage Control</title>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("</head>");
  client.println("<body>");
  //client.print("<p>"); client.print(method); client.print(" "); client.print(path); client.println("</p>");
  
  int brightness = analogRead(photoResistorAnalogPin);
  int brightnessPct = map(brightness, 0, 1023, 0, 100);
  client.print("<p>Brightness is "); client.print(brightnessPct); client.print("%"); client.println("</p>");
  
  client.println("<form method=\"POST\" action=\"/door\">");
  client.println("<button type=\"submit\">Open/Close Garage Door</button>");
  client.println("</form>");
  
  client.println("</body>");
  client.println("</html>");
}

void sendRedirectToIndex(EthernetClient client) {
      // send a standard http response header
  client.println("HTTP/1.1 302 FOUND");
  client.println("Location: /");
  client.println("Connection: close");  // the connection will be closed after completion of the response
}

void send404(EthernetClient client) {
    // send a standard http response header
  client.println("HTTP/1.0 404 Not Found");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println();
  client.println("Error 404: File Not Found");
}

void pressGarageDoorButton() {
  buttonPressStartTime = millis();
  digitalWrite(relayControlPin, BUTTON_PRESSED);
}

void handleGarageDoorButton() {
  
  if (buttonPressStartTime == 0) {
    return;
  }
  
  unsigned long currentMillis = millis();
    
 
  if(currentMillis - buttonPressStartTime  >= buttonPressLength) {
    buttonPressStartTime = 0;
    digitalWrite(relayControlPin, BUTTON_RELEASED);
  }
}

/*
dark: 145
indirect sun: 736
flashlight: 1000+
*/
