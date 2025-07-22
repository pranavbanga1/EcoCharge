#include <Wire.h>
#include <Adafruit_INA219.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <TimerOne.h> // Include the TimerOne library for timer-based interrupts

// DHT Configuration
#define DHTPIN1 2           // Pin connected to first DHT sensor (Inside)
#define DHTPIN2 8           // Pin connected to second DHT sensor (Outside)
#define DHTTYPE DHT11       // DHT11 sensor type
DHT dht1(DHTPIN1, DHTTYPE); // Inside DHT
DHT dht2(DHTPIN2, DHTTYPE); // Outside DHT

#define FANPIN 3            // PWM pin connected to the fan

// INA219 Configuration for 4 sensors
Adafruit_INA219 ina219_1(0x44);
Adafruit_INA219 ina219_2(0x45);
Adafruit_INA219 ina219_3(0x41);
Adafruit_INA219 ina219_4(0x40);

// LCD Configuration
LiquidCrystal_I2C lcd(0x27, 20, 4); // 20x4 LCD

// OLED Configuration
#define OLED_RESET -1
Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET);

// Global variables for temperature and fan duty cycle
volatile float insideTemp = 0;
volatile int fanDutyCycle = 0;

void setup() {
  // Initialize Serial Communication
  Serial.begin(115200);
  
  // Initialize DHT sensors
  dht1.begin();
  dht2.begin();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED initialization failed"));
    while (1);
  }
  display.display();
  delay(2000);  // Pause for 2 seconds
  
  // Initialize INA219 sensors
  if (!ina219_1.begin() || !ina219_2.begin()) {
    Serial.println("Couldn't find INA219 sensors");
    while (1);
  }

  // Initialize FAN pin
  pinMode(FANPIN, OUTPUT);

  // Setup Timer Interrupt for fan control (every 1 second)
  Timer1.initialize(1000000); // 1,000,000 microseconds = 1 second
  Timer1.attachInterrupt(fanControlInterrupt); // Attach the fan control interrupt

  // Initial LCD message
  lcd.setCursor(0, 0);
  lcd.print("Current & Voltage");
  lcd.setCursor(0, 1);
  lcd.print("Monitoring...");
  delay(2000); // Wait for 2 seconds before starting to display data
}

void loop() {
  // Read DHT Temperature and Humidity (Outside)
  float temp2 = dht2.readTemperature();   // Outside Temp
  float humidity2 = dht2.readHumidity();  // Outside Humidity
  float insidetemp = dht1.readTemperature();   // Inside Temp
  
  
  // Handle DHT reading errors
  if (isnan(temp2) || isnan(humidity2)) {
    Serial.println("Error reading DHT2 sensor!");
    return;
  }

  // Read INA219 Sensor Data for each row
  float busVoltage1 = ina219_1.getBusVoltage_V();
  float current1 = ina219_1.getCurrent_mA();
  float busVoltage2 = ina219_2.getBusVoltage_V();
  float current2 = ina219_2.getCurrent_mA();
  // Output to Serial Monitor
  Serial.print("Raw Data: ");
  Serial.print(insideTemp); Serial.print("|");
  Serial.print(temp2); Serial.print("|");
  Serial.print(busVoltage1); Serial.print("|");
  Serial.print(current1); Serial.println("|");
  Serial.print(busVoltage2); Serial.print("|");
  Serial.print(current2); Serial.println("|");
  // Update LCD with current and voltage data
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("V1: ");
  lcd.print(busVoltage1, 2);  // Display Bus Voltage from INA219_1
  lcd.print(" V");

  lcd.setCursor(0, 1);
  lcd.print("I1: ");
  lcd.print(current1);  // Display Current from INA219_1
  lcd.print(" mA");

  lcd.setCursor(0, 2);
  lcd.print("Inside Temp: ");
  lcd.print(insideTemp);  // Display Current from INA219_1
  lcd.print(" C");

    lcd.setCursor(0, 3);
  lcd.print("Ambient Temp: ");
  lcd.print(temp2);  // Display Current from INA219_1
  lcd.print(" C");

  // Update OLED with temperature data
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Inside Temp: ");
  display.print(insideTemp);  // Display Inside Temperature
  display.print("C");

  display.setCursor(0, 16);
  display.print("Outside Temp: ");
  display.print(temp2);  // Display Outside Temperature
  display.print("C");

  display.display(); // Refresh OLED

  delay(2000);  // Wait for 2 seconds before updating again
}

// Interrupt Service Routine (ISR) for Fan Control
void fanControlInterrupt() {
  insideTemp = dht1.readTemperature(); // Read Inside Temp

  // Check if reading is valid
  if (!isnan(insideTemp)) {
    // Adjust fan duty cycle based on temperature
    if (insideTemp > 23) {
      fanDutyCycle = map(insideTemp, 23, 33, 128, 255); // Scale from 30°C to 50°C
      fanDutyCycle = constrain(fanDutyCycle, 128, 255); // Clamp between 50% and 100%
    } else {
      fanDutyCycle = 0; // Turn off the fan
    }

    analogWrite(FANPIN, fanDutyCycle); // Set fan speed
  }
}
