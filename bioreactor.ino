#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/*Temperature*********************************************************************/
// Connect signal pin of the temperature sensor to to D18
#define ONE_WIRE_BUS 18

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

/*Website**************************************************************************/
// Replace with your network credentials
const char* ssid = ""; //Enter your WIFI SSID
const char* password = "";   //Enter your WIFI password

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

/*Pump*****************************************************************************/
// Auxiliar variables to store the current output state
String output26State = "off";

// Assign output variables to GPIO pins
int enA = 25;
int in1 = 26;
int in2 = 27;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 10000;

/*CO2******************************************************************************/
#define         MG_PIN                       (34)     //define which analog input channel you are going to use
#define         BOOL_PIN                     (2)
#define         DC_GAIN                      (8.5)   //define the DC gain of amplifier
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interval(in milisecond) between each samples in
                                                     //normal operation
 
//These two values differ from sensor to sensor. user should derermine this value.
#define         ZERO_POINT_VOLTAGE           (0.761) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define         REACTION_VOLTGAE             (0.02) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2
 
/*****************************Globals***********************************************/
float           CO2Curve[3]  =  {2.602,ZERO_POINT_VOLTAGE,(REACTION_VOLTGAE/(2.602-3))};
                                                     //two points are taken from the curve.
                                                     //with these two points, a line is formed which is
                                                     //"approximately equivalent" to the original curve.
                                                     //data format:{ x, y, slope}; point1: (lg400, 0.324), point2: (lg4000, 0.280)
                                                     //slope = ( reaction voltage ) / (log400 –log1000)
 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void setup() {
  Serial.begin(115200);

  // Set all the motor control pins to outputs
	pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in1, OUTPUT);

  // Turn off motors - Initial state
	digitalWrite(in1, LOW);
	digitalWrite(in2, LOW);

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

  //Temperature
  sensors.begin();

  //CO2
  pinMode(BOOL_PIN, INPUT);                        //set pin to input
  digitalWrite(BOOL_PIN, HIGH);                    //turn on pullup resistors
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  sensors.requestTemperatures(); // Send the command to get temperatures

  //sets pump speed (can be 0-255)
  analogWrite(enA, 150);

  //CO2 
  int percentage;
  float volts;  

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
            
            // turns the pump on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("Pump On");
              output26State = "on";
              digitalWrite(in1, HIGH);
              digitalWrite(in2, LOW);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("Pump Off");
              output26State = "off";
              digitalWrite(in1, LOW);
              digitalWrite(in2, LOW);
            } 
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: left;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Temporary Immersion Bioreactor</h1>");
            client.println("<hr>");

            ///////////////////////////////////////////////////////////////////////////////
            //Pump
            // Display current state, and ON/OFF buttons for the pump
            client.println("<p>Pump State: " + output26State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 

            ///////////////////////////////////////////////////////////////////////////////
            //Temperature 
            float temp = sensors.getTempCByIndex(0);
            String temperatureStr = String(temp, 2);

            client.println("<hr>");
            client.println("<p>Temperature: <br>");
            client.println(temp);
            client.println("&deg;C</span></p>");

            ///////////////////////////////////////////////////////////////////////////////
            //CO2 sensor

            //prints volts received from the CO2 sensor in the serial monitor
            volts = MGRead(MG_PIN);
            Serial.print( "SEN0159:" );
            Serial.print(volts);
            Serial.print( "V           " );

            //prints ppm in the serial monitor
            percentage = MGGetPercentage(volts,CO2Curve);
            Serial.print("CO2:");
            if (percentage == -1) {
                Serial.print( "<400" );
            } else {
                Serial.print(percentage);
            }
            Serial.print( "ppm" );
            Serial.print("\n");

            //prints ppm in the website
            client.println("<hr>");
            client.println("<p>CO<sub>2</sub> Level: <br>");
            if (percentage == -1) {
                client.println("<400");
            } else {
                client.println(percentage);
            }
            client.println("ppm</p>");


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

/*****************************  MGRead *********************************************
Input:   mg_pin - analog channel
Output:  output of SEN-000007
Remarks: This function reads the output of SEN-000007
************************************************************************************/
float MGRead(int mg_pin)
{
    int i;
    float v=0;
 
    for (i=0;i<READ_SAMPLE_TIMES;i++) {
        v += analogRead(mg_pin);
        delay(READ_SAMPLE_INTERVAL);
    }
    v = (v/READ_SAMPLE_TIMES) *5/1024 ;
    return v;
}
 
/*****************************  MQGetPercentage **********************************
Input:   volts   - SEN-000007 output measured in volts
         pcurve  - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm)
         of the line could be derived if y(MG-811 output) is provided. As it is a
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic
         value.
************************************************************************************/
int  MGGetPercentage(float volts, float *pcurve)
{
   if ((volts/DC_GAIN )>=ZERO_POINT_VOLTAGE) {
      return -1;
   } else {
      return pow(10, ((volts/DC_GAIN)-pcurve[1])/pcurve[2]+pcurve[0]);
   }
}


