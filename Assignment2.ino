// Blynk template and authentication settings
#define BLYNK_TEMPLATE_ID "TMPL6pZyHMS-8" // Blynk template identifier for your project.
#define BLYNK_TEMPLATE_NAME "Temperature and Humidity Monitoring" // Name of the Blynk template.
#define BLYNK_AUTH_TOKEN "sz7kTVq8KWne0dnbV71ut77pplqogioP" // Authentication token to connect your device to the Blynk app.

// Enable printing to the serial monitor
#define BLYNK_PRINT Serial

// Include necessary libraries
#include <ESP8266WiFi.h>        // Library for ESP8266 WiFi functionality
#include <BlynkSimpleEsp8266.h> // Library for Blynk functionality on ESP8266
#include <DHT.h>                // Library for DHT sensor
#include <EEPROM.h>             // Library for EEPROM functionality

// Define the pin where the DHT sensor is connected
#define DHTPIN D6 

// Define the type of DHT sensor used (DHT22)
#define DHTTYPE DHT22

// Create an instance of the DHT sensor, allowing interaction with it through the dht object.
DHT dht(DHTPIN, DHTTYPE);

// Blynk authentication token
char auth[] = BLYNK_AUTH_TOKEN;

// WiFi credentials
char ssid[] = "Your_SSID"; // WiFi SSID
char pass[] = "Your_PASSWORD";   // WiFi password

// Previous humidity and temperature values
float previousHumidity = 0.0; // Stores the last known humidity value read from the sensor.
float previousTemperature = 0.0; //  Stores the last known temperature value read from the sensor.

// Function to write a float to EEPROM with commit
void writeFloatToEEPROM(int address, float value) {
  byte *p = (byte *)(void *)&value; // Cast float to byte pointer
  for (int i = 0; i < sizeof(value); i++) {
    EEPROM.write(address + i, *p); // Write each byte
    p++;
  }
  EEPROM.commit(); // Ensure data is saved
}

// Function to read a float from EEPROM
float readFloatFromEEPROM(int address) {
  float value;
  byte *p = (byte *)(void *)&value; // Cast byte pointer to float pointer
  for (int i = 0; i < sizeof(value); i++) {
    *p = EEPROM.read(address + i); // Read each byte
    p++;
  }
  return value;
}

// The function that runs once when the microcontroller starts.
void setup() {
  // Start the serial communication at a baud rate of 9600
  Serial.begin(9600);
  
  // Initialize the DHT sensor
  dht.begin();
  
  // Connect to Blynk using the authentication token, WiFi SSID, and password
  Blynk.begin(auth, ssid, pass);

  // Initialize EEPROM
  EEPROM.begin(512); // Initialize EEPROM with size (usually 512 for ESP8266)
  
  // Read previous humidity and temperature from EEPROM
  previousHumidity = readFloatFromEEPROM(0); // Read humidity from address 0
  previousTemperature = readFloatFromEEPROM(4); // Read temperature from address 4
  
  // Check if EEPROM contains valid data by verifying that the initial float values make sense
  if (isnan(previousHumidity) || isnan(previousTemperature)) {
    Serial.println("Invalid EEPROM data, setting default values.");
    previousHumidity = 0.0;
    previousTemperature = 0.0;
  }
}

// The function that runs repeatedly after the setup completes
void loop() {
  // Run Blynk in the main loop to keep the connection alive
  Blynk.run();
  
  // Read temperature from the DHT sensor
  float temp = dht.readTemperature();
  
  // Read humidity from the DHT sensor
  float hum = dht.readHumidity();
  
  // Check if the readings from the DHT sensor are valid
  if (isnan(temp) || isnan(hum)) {
    // Print an error message to the serial monitor if the readings are not valid
    Serial.println("Failed to read from DHT sensor!");
    return; // Exit the loop if the readings are not valid
  }

  // Print the temperature and humidity values to the serial monitor
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("°C  Humidity: ");
  Serial.print(hum);
  Serial.println("%");

  // Send the humidity value to Blynk app, where V0 is the virtual pin assigned to humidity
  Blynk.virtualWrite(V0, hum);
  
  // Send the temperature value to Blynk app, where V1 is the virtual pin assigned to temperature
  Blynk.virtualWrite(V1, temp);

  // Check temperature level and print message
  if (temp > 30.0) {
    Serial.println("Heat: Temperature is greater than 30°C");
    // Optionally, send a heat message to the Blynk app
    Blynk.virtualWrite(V2, "Heat: Temperature increased!");
  } else {
    Serial.println("Normal: Temperature is 30°C or lower.");
    // Optionally, send a normal message to the Blynk app
    Blynk.virtualWrite(V2, "Heat : Normal Temperature!");
  }

  // Checks if there's a significant change in temperature or humidity before updating EEPROM.
  if (abs(hum - previousHumidity) > 0.5 || abs(temp - previousTemperature) > 0.5) {
    writeFloatToEEPROM(0, hum);
    writeFloatToEEPROM(4, temp);
  }

  // Update the previous humidity and temperature values
  previousHumidity = hum;
  previousTemperature = temp;

  // Wait for 2 seconds before taking another reading
  delay(2000);
}
