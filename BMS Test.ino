// Thermistor Configuration
#define BETA 4000             // Beta value for NTC 5D-9 thermistor
#define R_25 4.6             // Thermistor resistance at 25°C in ohms (corrected to 4600Ω)
#define T_25 298.15           // Nominal temperature (25°C in Kelvin)
#define V_INPUT 4.9         // Measured input voltage from Arduino
#define R_FIXED 10000             // Fixed resistor value (in ohms)

#define THERMISTOR_PIN A5   // Pin for thermistor

void setup() {
    Serial.begin(115200);    // Initialize serial communication
}

float readThermistorTemperature(int pin) {
    // Step 1: Read ADC value
    int adcValue = analogRead(pin);

    // Step 2: Calculate voltage at analog pin
    float voltage = (adcValue / 1023.0) * V_INPUT;

    // Step 3: Calculate thermistor resistance
    float R_thermistor = R_FIXED * ((V_INPUT / voltage) - 1.0);

    // Step 4: Calculate temperature in Kelvin
    float temperatureK = 1.0 / (1.0 / T_25 + (1.0 / BETA) * log(R_thermistor / R_25));

    // Step 5: Convert Kelvin to Celsius
    return temperatureK - 273.15;
}

void loop() {
    // Read ADC value from thermistor
    int adcValue = analogRead(THERMISTOR_PIN);

    // Read temperature from thermistor
    float thermistorTemperature = readThermistorTemperature(THERMISTOR_PIN);

    // Print ADC value and thermistor temperature
    Serial.print("ADC Value: ");
    Serial.print(adcValue);
    Serial.print("  |  Thermistor Temperature: ");
    Serial.print(thermistorTemperature);
    Serial.println(" °C");

    delay(500);  // Delay for readability
}
