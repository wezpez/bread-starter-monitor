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
#define ADDR_6713  0x15 // default I2C slave address

#include <DHT.h>
#include <DHT_U.h>

#include <Adafruit_NeoPixel.h>
#define PIN 6
#define NUMPIXELS 8

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define trigPin 2
#define echoPin 3

int data [4];
int CO2ppmValue;
uint8_t DHTPin = 7;
float Temperature;
float Humidity;
const int buzzerPin = 9; //buzzer to arduino pin 9

DHT dht(DHTPin, DHTTYPE);   // Initialize DHT sensor.

Adafruit_NeoPixel pixels(NUMPIXELS, PIN); 

// This is the default address of the CO2 sensor, 7bits shifted left.
void setup() { 
  Wire.begin ();
  Serial.begin(9600);
  
  // start DHT sensor
  pinMode(DHTPin, INPUT);
  dht.begin();

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  pinMode(buzzerPin, OUTPUT); // Set buzzer - pin 9 as an output

  pixels.begin();
  
  Serial.println("Application Note AN161_ardunio_T6713_I2C");
}

///////////////////////////////////////////////////////////////////
// Function : int readCO2()
// Returns : CO2 Value upon success, 0 upon checksum failure
// Assumes : - Wire library has been imported successfully.
// - LED is connected to IO pin 13
// - CO2 sensor address is defined in co2_addr
///////////////////////////////////////////////////////////////////

int readC02()
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
  CO2ppmValue = ((data[2] * 0xFF ) + data[3]);
}

float readTemperature()
{
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Serial.print("Temperature: ");
  Serial.println(Temperature);
  return Temperature;
}

float readHumidity()
{
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  Serial.print("Humidity: ");
  Serial.println(Humidity);
  return Humidity;
}

int readDistance()
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
  float distance = 34400*duration/2000000;
  
  Serial.print("Distance in cm: ");
  Serial.println(distance);
  return distance;
}

void loop() {

  int co2Value = readC02();
  {
    Serial.print("CO2 Value: ");
    Serial.println(CO2ppmValue);
  }

  readTemperature();
  readHumidity();
  int distance = readDistance();

  pixels.setPixelColor(0, 0, 0, 0);
  pixels.setPixelColor(1, 37, 0, 0);
  pixels.setPixelColor(2, 0, 37, 0);
  pixels.setPixelColor(3, 0, 0, 37);
  pixels.setPixelColor(4, 37, 37, 0);
  pixels.setPixelColor(5, 37, 0, 37);
  pixels.setPixelColor(6, 0, 37, 37);
  pixels.setPixelColor(7, 37, 37, 37);


  // this could be needed just once maybe in setup
  pixels.show();

  int buzzerFrequency = map(distance, 0,50, 300, 1000);
  tone(buzzerPin, buzzerFrequency);
  delay(100);
  noTone(buzzerPin);     // Stop sound...

  
/*
  tone(buzzerPin, 500); // Send 1KHz sound signal...
  delay(100);        // ...for 1 sec
  tone(buzzerPin, 600); // Send 1KHz sound signal...
  delay(100);        // ...for 1 sec
  tone(buzzerPin, 700); // Send 1KHz sound signal...
  delay(100);        // ...for 1 sec
  tone(buzzerPin, 800); // Send 1KHz sound signal...
  delay(100);        // ...for 1 sec
  tone(buzzerPin, 700); // Send 1KHz sound signal...
  delay(100);        // ...for 1 sec
  tone(buzzerPin, 600); // Send 1KHz sound signal...
  delay(100);        // ...for 1 sec
  tone(buzzerPin, 500); // Send 1KHz sound signal...
  delay(100);        // ...for 1 sec
  tone(buzzerPin, 400); // Send 1KHz sound signal...
  delay(100);        // ...for 1 sec
  noTone(buzzerPin);     // Stop sound...
  */
  

 
  
  delay(200);
}
