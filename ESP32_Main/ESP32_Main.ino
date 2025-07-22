#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Access Point credentials
const char* ssid = "EcoCharge BMS Web Server";
const char* password = "12345678";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Arrays to store sensor data
float dhtTemps[5] = {0, 0, 0, 0, 0};
float dhtHumidities[5] = {0, 0, 0, 0, 0};
float voltages[5] = {0, 0, 0, 0, 0};
float currents[5] = {0, 0, 0, 0, 0};
float fans[5] = {0, 0, 0, 0, 0};

// Serial2 pins (TX2 and RX2)
#define RXD2 16  // GPIO 16 for RX2
#define TXD2 17  // GPIO 17 for TX2

// Serial data parsing function
void parseSerialData(String data) {
  Serial.println("Raw Data: " + data);  // Debugging the raw data received

  // Split data into fields
  String fields[25];
  int index = 0;

  while (data.length() > 0 && index < 25) {
    int delimiterIndex = data.indexOf('|');
    if (delimiterIndex == -1) {
      fields[index++] = data; // Last field
      break;
    }
    fields[index++] = data.substring(0, delimiterIndex);
    data = data.substring(delimiterIndex + 1);
  }

  // Assign values to arrays based on schema
  for (int i = 0; i < 5; i++) {
    dhtTemps[i] = fields[i * 2].toFloat();           // Temp1, Temp2, ...
    dhtHumidities[i] = fields[i * 2 + 1].toFloat();  // Humidity1, Humidity2, ...
    voltages[i] = fields[10 + i * 2].toFloat();      // Voltage1, Voltage2, ...
    currents[i] = fields[10 + i * 2 + 1].toFloat();  // Current1, Current2, ...
    fans[i] = fields[20 + i].toFloat();              // Fan1, Fan2, ...
  }

  Serial.println("Parsing Complete!");
}

// Debugging function to print parsed data
void debugParsedData() {
  Serial.println("----- Parsed Data Debug -----");
  for (int i = 0; i < 5; i++) {
    Serial.print("DHT " + String(i + 1) + ": Temp=" + String(dhtTemps[i]) + ", Humidity=" + String(dhtHumidities[i]));
    Serial.print(", Voltage=" + String(voltages[i]) + ", Current=" + String(currents[i]));
    Serial.println(", Fan=" + String(fans[i]));
  }
  Serial.println("--------------------------------");
}

// HTML + JS for the Web Page
const char* webPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Real-Time Web Dashboard</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 20px;
      background-color: #2e3b4e; /* Custom background color */
      color: black; /* Default text color */
    }
    h1 {
      text-align: center;
      color: white; /* White title text */
    }
    .container {
      display: flex;
      flex-wrap: wrap;
      gap: 20px;
      justify-content: center;
    }
    .card {
      border: 1px solid #ccc;
      padding: 15px;
      border-radius: 10px;
      background-color: white; /* White card background */
      width: 350px;
      color: black; /* Black text inside the card */
      box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1); /* Subtle shadow */
    }
    .card h2 {
      margin-bottom: 15px;
      color: black;
    }
    .updated {
      color: green !important;
      transition: color 0.5s ease;
    }
  </style>
</head>
<body>
  <h1>Real-Time Sensor Dashboard</h1>
  <div class="container">
    <!-- Card 1 -->
    <div class="card">
      <h2>Temperature Data</h2>
      <p>Ambient Temperature: <span id="ambientTemp">##.##</span> °C | <span id="ambientHumidity">##.##</span> %</p>
      <p>Cell Row 1: <span id="cell1Temp">##.##</span> °C | <span id="cell1Humidity">##.##</span> %</p>
      <p>Cell Row 2: <span id="cell2Temp">##.##</span> °C | <span id="cell2Humidity">##.##</span> %</p>
      <p>Cell Row 3: <span id="cell3Temp">##.##</span> °C | <span id="cell3Humidity">##.##</span> %</p>
      <p>Cell Row 4: <span id="cell4Temp">##.##</span> °C | <span id="cell4Humidity">##.##</span> %</p>
    </div>
    <!-- Card 2 -->
    <div class="card">
      <h2>Voltage and Current Info</h2>
      <p>Output Load: <span id="outputVoltage">##.##</span> V | <span id="outputCurrent">##.##</span> A</p>
      <p>Cell Row 1: <span id="cell1Voltage">##.##</span> V | <span id="cell1Current">##.##</span> A</p>
      <p>Cell Row 2: <span id="cell2Voltage">##.##</span> V | <span id="cell2Current">##.##</span> A</p>
      <p>Cell Row 3: <span id="cell3Voltage">##.##</span> V | <span id="cell3Current">##.##</span> A</p>
      <p>Cell Row 4: <span id="cell4Voltage">##.##</span> V | <span id="cell4Current">##.##</span> A</p>
    </div>
    <!-- Card 3 -->
    <div class="card">
      <h2>Fan Speed Info</h2>
      <p>Intake Fan 1: <span id="fan1">###</span></p>
      <p>Exhaust Fan 2: <span id="fan2">###</span></p>
      <p>Exhaust Fan 3: <span id="fan3">###</span></p>
      <p>Exhaust Fan 4: <span id="fan4">###</span></p>
      <p>Exhaust Fan 5: <span id="fan5">###</span></p>
    </div>
    <!-- Card 4 -->
    <div class="card">
      <h2>Battery Pack Info</h2>
      <p>Battery Capacity: TBD</p>
      <p>Nominal Voltage: TBD</p>
    </div>
  </div>
  <script>
    function updateValue(id, value) {
      const element = document.getElementById(id);
      if (element.innerText !== value) {
        element.classList.add("updated");
        setTimeout(() => element.classList.remove("updated"), 500);
      }
      element.innerText = value;
    }

    setInterval(() => {
      fetch("/data")
        .then((response) => response.json())
        .then((data) => {
          console.log("Received JSON:", data);

          // Update temperature and humidity
          updateValue("ambientTemp", data.dhtTemps[0].toFixed(2));
          updateValue("ambientHumidity", data.dhtHumidities[0].toFixed(2));
          updateValue("cell1Temp", data.dhtTemps[1].toFixed(2));
          updateValue("cell1Humidity", data.dhtHumidities[1].toFixed(2));
          updateValue("cell2Temp", data.dhtTemps[2].toFixed(2));
          updateValue("cell2Humidity", data.dhtHumidities[2].toFixed(2));
          updateValue("cell3Temp", data.dhtTemps[3].toFixed(2));
          updateValue("cell3Humidity", data.dhtHumidities[3].toFixed(2));
          updateValue("cell4Temp", data.dhtTemps[4].toFixed(2));
          updateValue("cell4Humidity", data.dhtHumidities[4].toFixed(2));

          // Update voltage and current
          updateValue("outputVoltage", data.voltages[0].toFixed(2));
          updateValue("outputCurrent", data.currents[0].toFixed(2));
          updateValue("cell1Voltage", data.voltages[1].toFixed(2));
          updateValue("cell1Current", data.currents[1].toFixed(2));
          updateValue("cell2Voltage", data.voltages[2].toFixed(2));
          updateValue("cell2Current", data.currents[2].toFixed(2));
          updateValue("cell3Voltage", data.voltages[3].toFixed(2));
          updateValue("cell3Current", data.currents[3].toFixed(2));
          updateValue("cell4Voltage", data.voltages[4].toFixed(2));
          updateValue("cell4Current", data.currents[4].toFixed(2));

          // Update fan speeds
          updateValue("fan1", data.fans[0].toFixed(0));
          updateValue("fan2", data.fans[1].toFixed(0));
          updateValue("fan3", data.fans[2].toFixed(0));
          updateValue("fan4", data.fans[3].toFixed(0));
          updateValue("fan5", data.fans[4].toFixed(0));
        })
        .catch((error) => console.error("Error fetching data:", error));
    }, 5000);
  </script>
</body>
</html>
)rawliteral";

// Handle HTTP requests
void setupServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", webPage);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{";
    json += "\"dhtTemps\":[" + String(dhtTemps[0]) + "," + String(dhtTemps[1]) + "," + String(dhtTemps[2]) + "," + String(dhtTemps[3]) + "," + String(dhtTemps[4]) + "],";
    json += "\"dhtHumidities\":[" + String(dhtHumidities[0]) + "," + String(dhtHumidities[1]) + "," + String(dhtHumidities[2]) + "," + String(dhtHumidities[3]) + "," + String(dhtHumidities[4]) + "],";
    json += "\"voltages\":[" + String(voltages[0]) + "," + String(voltages[1]) + "," + String(voltages[2]) + "," + String(voltages[3]) + "," + String(voltages[4]) + "],";
    json += "\"currents\":[" + String(currents[0]) + "," + String(currents[1]) + "," + String(currents[2]) + "," + String(currents[3]) + "," + String(currents[4]) + "],";
    json += "\"fans\":[" + String(fans[0]) + "," + String(fans[1]) + "," + String(fans[2]) + "," + String(fans[3]) + "," + String(fans[4]) + "]";
    json += "}";

    Serial.println("Full JSON: " + json);
    request->send(200, "application/json", json);
  });

  server.begin();
}

void setup() {
  Serial.begin(115200);//had to switch back to 9600 because of compatibility issues
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP IP address: " + IP.toString());

  setupServer();
}

void loop() {
  if (Serial2.available()) {
    String incomingData = Serial2.readStringUntil('\n');
    parseSerialData(incomingData);
    debugParsedData();
  }
}
