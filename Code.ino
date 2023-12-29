#include <ESP8266WiFi.h>
#include <DHT.h>
#include <MQ135.h>

// Include Thing.CoAP
#include "Thing.CoAP.h"

//[RECOMMENDED] Alternatively, if you are NOT using Arduino IDE you can include each file you need as bellow: 
//#include "Thing.CoAP/Server.h"
//#include "Thing.CoAP/ESP/UDPPacketProvider.h"

// Declare our CoAP server and the packet handler
Thing::CoAP::Server server;
Thing::CoAP::ESP::UDPPacketProvider udpProvider;

// Change here your WiFi settings
char* ssid = "Warrir";
char* password = "Warr19";

// Define the pin for the DHT11 sensor
#define DHTPIN D6
// Initialize DHT sensor
DHT dht(DHTPIN, DHT11);
// Define the pin for the MQ135 sensor
#define PIN_MQ135 A0
MQ135 mq135_sensor(PIN_MQ135);

#define LED 2

void setup() {
  // Initializing the Serial
  Serial.begin(115200);
  Serial.println("Initializing");
    //Configure the LED as output
  pinMode(LED, OUTPUT);

  // Configure the DHT sensor
  dht.begin();

  // Try and wait until a connection to WiFi was made
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  Serial.println("My IP: ");
  Serial.println(WiFi.localIP());

  // Configure our server to use our packet handler (It will use UDP)
  server.SetPacketProvider(udpProvider);

  // Create an endpoint called "Temperature"
  server.CreateResource("Temperature", Thing::CoAP::ContentFormat::TextPlain, true) // True means that this resource is observable
    .OnGet([](Thing::CoAP::Request & request) { // We are here configuring telling our server that, when we receive a "GET" request to this endpoint, run the following code
      Serial.println("GET Request received for endpoint 'Temperature'");

      // Read the temperature from the DHT11 sensor
      float temperature = dht.readTemperature();
      if (isnan(temperature)) {
        Serial.println("Failed to read temperature from DHT sensor!");
        return Thing::CoAP::Status::InternalServerError();
      }

      // Return the current temperature
      std::string result = std::to_string(temperature);
      return Thing::CoAP::Status::Content(result);
    });

  // Create an endpoint called "Humidity"
  server.CreateResource("Humidity", Thing::CoAP::ContentFormat::TextPlain, true) // True means that this resource is observable
    .OnGet([](Thing::CoAP::Request & request) { // We are here configuring telling our server that, when we receive a "GET" request to this endpoint, run the following code
      Serial.println("GET Request received for endpoint 'Humidity'");

      // Read the humidity from the DHT11 sensor
      float humidity = dht.readHumidity();
      if (isnan(humidity)) {
        Serial.println("Failed to read humidity from DHT sensor!");
        return Thing::CoAP::Status::InternalServerError();
      }

      // Return the current humidity
      std::string result = std::to_string(humidity);
      return Thing::CoAP::Status::Content(result);
      Serial.print("Humidity: ");
      Serial.print(humidity);
    });

  server.CreateResource("AirQuality", Thing::CoAP::ContentFormat::TextPlain, true) //True means that this resource is observable
    .OnGet([](Thing::CoAP::Request & request) { //We are here configuring telling our server that, when we receive a "GET" request to this endpoint, run the following code
      Serial.println("GET Request received for endpoint 'AirQuality'");

      // Read the air quality value from the MQ135 sensor
      float airQualityValue = mq135_sensor.getPPM();

      // Convert the air quality value to a string
      std::string result = std::to_string(airQualityValue);

      //Return the current air quality value
      return Thing::CoAP::Status::Content(result);
    });
     //Create an endpoint called "LED"
  server.CreateResource("LED", Thing::CoAP::ContentFormat::TextPlain, true) //True means that this resource is observable
    .OnGet([](Thing::CoAP::Request & request) { //We are here configuring telling our server that, when we receive a "GET" request to this endpoint, run the the following code
      Serial.println("GET Request received for endpoint 'LED'");

      //Read the state of our led.
      std::string result;
      if(digitalRead(LED) == HIGH)
        result = "Off";
      else
        result = "On";

       //Return the current state of our "LED".
      return Thing::CoAP::Status::Content(result);
    }).OnPost([](Thing::CoAP::Request& request) {  //We are here configuring telling our server that, when we receive a "POST" request to this endpoint, run the the following code
      Serial.println("POST Request received for endpoint 'LED'");

      //Get the message sent fromthe client and parse it to a string      
      auto payload = request.GetPayload();
      std::string message(payload.begin(), payload.end());
      
      Serial.print("The client sent the message: ");
      Serial.println(message.c_str());

      if(message == "On") { //If the message is "On" we will turn the LED on.
        digitalWrite(LED, LOW);
      } else if (message == "Off") { //If it is "Off" we will turn the LED off.
        digitalWrite(LED, HIGH);
      } else { //In case any other message is received we will respond a "BadRequest" error.
        return Thing::CoAP::Status::BadRequest();
      }

      //In case "On" or "Off" was received, we will return "Ok" with a message saying "Command Executed".
      return Thing::CoAP::Status::Created("Command Executed");
    });
  server.Start();
}

void loop() {
  server.Process();
}
