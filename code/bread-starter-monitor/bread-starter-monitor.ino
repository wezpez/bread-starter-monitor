// Bread Starter Monitor
// By Patrick Whyte, DEC/2021
// UCL Connected Environments

// Using the CO2 Meter Amphenol T6713 i2c Example Interface
// By Marv Kausch, 12/2016 at CO2 Meter <co2meter.com>
// Revised by John Houck, 05/01/2018
// Talks via I2C to T6713 sensors and displays CO2 values
// Arduino analog input 5 - I2C SCL
// Arduino analog input 4 - I2C SDA

#include <Wire.h>  // Although Wire.h is part of the Ardunio GUI library, this statement is still necessary
#define ADDR_6713  0x15  // default I2C slave address

#include <DHT.h>
#include <DHT_U.h>
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321

#include <Adafruit_NeoPixel.h>
#define PIN 6
#define NUMPIXELS 8

#define trigPin 2
#define echoPin 3

// Set up global variables
int data [4];
int CO2Value;
int baseCO2Value;
int CO2Level1;
int CO2Level2;
int CO2Level3;
int CO2Level4;
const int CO2LED1 = 4;  //the LEDs on the NeoPixel for CO2 range
const int CO2LED2 = 5;
const int CO2LED3 = 6;
const int CO2LED4 = 7;
uint8_t DHTPin = 7;
float Temperature;
const int tempLED = 0;  // LED on the NeoPixel for temperature
float Humidity;
const int humLED = 2;  // LED on the NeoPixel for humidity
float Distance;
const int buzzerPin = 9;  //buzzer to pin 9

DHT dht(DHTPin, DHTTYPE);  // Initialize DHT sensor
Adafruit_NeoPixel pixels(NUMPIXELS, PIN);  // Initialize NeoPixel Board

// This is the default address of the CO2 sensor, 7bits shifted left.
void setup() { 
  Wire.begin ();
  
  // Start serial connection for debugging
  Serial.begin(9600);
  
  // start DHT sensor
  pinMode(DHTPin, INPUT);
  dht.begin();

  // Set up range finder
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  // Set buzzer - pin 9 as an output
  pinMode(buzzerPin, OUTPUT); 

  // start NeoPixel LEDs
  pixels.begin();

  // Get inital CO2 reading for base value and level values for range
  baseCO2Value = readCO2();
  
  // Tweak this range so it makes sense
  CO2Level1 = baseCO2Value + 500;
  CO2Level2 = baseCO2Value + 1000;
  CO2Level3 = baseCO2Value + 3000;
  CO2Level4 = baseCO2Value + 5000;
  
  Serial.println("Bread Starter Monitor");
}

///////////////////////////////////////////////////////////////////
// Function : int readCO2()
// Returns : CO2 Value upon success, 0 upon checksum failure
// Assumes : - Wire library has been imported successfully.
// - LED is connected to IO pin 13
// - CO2 sensor address is defined in co2_addr
///////////////////////////////////////////////////////////////////

int readCO2()
{
  // start I2C
  Wire.beginTransmission(ADDR_6713);
  Wire.write(0x04); Wire.write(0x13); Wire.write(0x8B); Wire.write(0x00); Wire.write(0x01);
  // end transmission
  Wire.endTransmission();
  // read report of current gas measurement in ppm
  delay(2000);
  Wire.requestFrom(ADDR_6713, 4);    // request 4 bytes from slave device
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();
    Serial.print("Func code: "); Serial.print(data[0],HEX);
    Serial.print(" byte count: "); Serial.println(data[1],HEX);
    Serial.print("MSB: 0x");  Serial.print(data[2],HEX); Serial.print("  ");
    Serial.print("LSB: 0x");  Serial.print(data[3],HEX); Serial.print("  ");
  CO2Value = ((data[2] * 0xFF ) + data[3]);
  Serial.print("CO2 Value: ");
  Serial.println(CO2Value);
  return CO2Value;
}

// Gets the temperature value from the DHT22
float readTemperature()
{
  Temperature = dht.readTemperature(); 
  Serial.print("Temperature: ");
  Serial.println(Temperature);
  return Temperature;
}

// Gets the humidity value from the DHT22
float readHumidity()
{
  Humidity = dht.readHumidity();
  Serial.print("Humidity: ");
  Serial.println(Humidity);
  return Humidity;
}

// Gets the distance value from the Ultrasonic range finder
float readDistance()
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echoPin, HIGH); 
  Distance = 34400*duration/2000000;
  
  Serial.print("Distance in cm: ");
  Serial.println(Distance);
  return Distance;
}

void updateTemperatureLED()
{
  if (Temperature < 22) {
    // Too cold - Set LED to Cyan
    pixels.setPixelColor(tempLED, 0, 37, 37);
  } else if (Temperature > 28) {
    // Too hot - Set LED to Orange
    pixels.setPixelColor(tempLED, 45, 20, 0);
  } else {
    // Temperature is in optimal range - Set LED to Green
    pixels.setPixelColor(tempLED, 0, 37, 0);
  }
}

// This may need tweaking too
void updateHumidityLED()
{
  if (Humidity > 80) {
    // Too humid - Set LED to Cyan
    pixels.setPixelColor(humLED, 0, 37, 37);
  } else if (Humidity < 60) {
    // Too dry - Set LED to Orange
    pixels.setPixelColor(humLED, 45, 20, 0);
  } else {
    // Humidity is in optimal range - Set LED to Green
    pixels.setPixelColor(humLED, 0, 37, 0);
  }
}

void updateCO2LED()
{
  // Reset the range
  pixels.setPixelColor(CO2LED1, 0, 0, 0);
  pixels.setPixelColor(CO2LED2, 0, 0, 0);
  pixels.setPixelColor(CO2LED3, 0, 0, 0);
  pixels.setPixelColor(CO2LED4, 0, 0, 0);

  if (CO2Value <= baseCO2Value) {
    // Not active - Set LED to Yellow
    pixels.setPixelColor(CO2LED1, 37, 37, 0);
  } else if (CO2Value > baseCO2Value && CO2Value < CO2Level1) {
    // Mildly active - Set LED to Orange
    pixels.setPixelColor(CO2LED1, 45, 20, 0);
  }

  // If the current C02 level is greater or equal to the threshold CO2 value
  // for a level we set that LED to Red
  if (CO2Value >= CO2Level1) {
     pixels.setPixelColor(CO2LED1, 37, 0, 0);
  }
  if (CO2Value >= CO2Level2) {
     pixels.setPixelColor(CO2LED2, 37, 0, 0);
  }
  if (CO2Value >= CO2Level3) {
     pixels.setPixelColor(CO2LED3, 37, 0, 0);
  }
  if (CO2Value >= CO2Level4) {
     pixels.setPixelColor(CO2LED4, 37, 0, 0);
  }
}

void checkDistance(){
  
  if (Distance <= 6){
    //Emergency about to overflow - Sound alarm!
    for (int i = 0; i <= 10; i++) {
      tone(buzzerPin, 900);
      delay(50);
      noTone(buzzerPin); // Stop sound
      delay(50);
    }
  } else if (Distance <= 10){
    // Getting close - Warning sound; two beebs
    tone(buzzerPin, 700);
    delay(100);
    noTone(buzzerPin); // Stop sound
    delay(100);
    tone(buzzerPin, 800);
    delay(100);
    noTone(buzzerPin); // Stop sound
  }
}

void loop() {

  Serial.print("Base CO2 Value: ");
  Serial.println(baseCO2Value);
  
  // Get new readings from sensors
  readCO2();
  readTemperature();
  readHumidity();
  readDistance();

  // Update the LEDs using the new values collected from the sensors
  updateTemperatureLED();
  updateHumidityLED();
  updateCO2LED();
  pixels.show();

  // Check the starter has not risen too high
  checkDistance();
  
  delay(500);
}
