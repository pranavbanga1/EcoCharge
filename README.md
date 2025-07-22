# 🔋 EcoCharge – Modular Vape Battery Power Supply with Real-Time BMS Control

EcoCharge is a modular, eco-friendly battery pack system developed by recycling lithium-ion vape cells and integrating real-time monitoring using an Arduino–ESP32 communication setup. It features dynamic thermal control, current safety mechanisms, and a responsive web dashboard via local hosting.

<img width="1660" height="517" alt="main" src="https://github.com/user-attachments/assets/1515b036-4de1-40f7-91b2-a6509c17483f" />

---

## 💡 Problem Statement

Billions of vape batteries are discarded every year, leading to major e-waste. EcoCharge addresses this by:
- Recycling vape lithium-ion cells.
- Creating a sustainable and modular battery pack.
- Implementing live monitoring and safety shutdown using embedded systems.

---

## ✅ Key Features

- ♻️ Recycled 4S3P vape battery pack.
- 📊 Real-time monitoring of voltage, temperature, and current.
- 🌡️ Intelligent thermal management with 5 DHT11 + PWM-controlled fans.
- 🛡️ Overcurrent protection (5A threshold with <200ms reaction time).
- 🌐 Web dashboard hosted locally using ESP32 with live JSON data.
- 🧠 Structured data communication between Arduino Mega and ESP32 via UART.

---

## 🧱 System Architecture

<p align="center">
  <img src="https://github.com/user-attachments/assets/fd385a12-7b6b-49cc-ba4e-ad4c88ca75dd" width="80%" alt="System Architecture Diagram"/>
</p>
<h2 align="center">

<table align="center">
  <thead>
    <tr>
      <th>Component</th>
      <th>Role</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>Arduino Mega</td>
      <td>Reads DHT11, INA219 data, and controls fans based on temperature.</td>
    </tr>
    <tr>
      <td>ESP32 WROOM</td>
      <td>Parses incoming data and hosts a responsive web dashboard.</td>
    </tr>
    <tr>
      <td>CF-4S30A-A BMS</td>
      <td>Manages battery cell balancing and overcurrent protection.</td>
    </tr>
    <tr>
      <td>DHT11 Sensors</td>
      <td>Tracks humidity and temperature at multiple points.</td>
    </tr>
    <tr>
      <td>INA219 Sensors</td>
      <td>Measures voltage and current from each battery row.</td>
    </tr>
    <tr>
      <td>PWM Fan Array</td>
      <td>5 fans with dynamic speed control based on temperature thresholds.</td>
    </tr>
    <tr>
      <td>OLED & LCD Screens</td>
      <td>On-device visual feedback of system metrics.</td>
    </tr>
  </tbody>
</table>


## 🧪 Testing & Validation

### ✅ Hard Real-Time:
- Overcurrent shutdown in <200ms.
- Fan speed adjusted in <5ms on threshold trigger.

<img width="1250" height="421" alt="EcoCHarge Thermister Check" src="https://github.com/user-attachments/assets/a19fb6e2-4504-4d7a-a94d-94e3964c05af" />

### ✅ Soft Real-Time:
- Dashboard updates every 5 seconds.
- Structured UART strings formatted with `|` for robust parsing.

**Test Cases:**
- 🔥 16V heater pad → Validated prolonged high-discharge safety.
- ⚡ 12V stalled motor → Validated overcurrent protection.

<img width="1660" height="517" alt="main" src="https://github.com/user-attachments/assets/36427a8d-f892-4075-838f-e52fb6312673" />


---

## 🧰 Code Breakdown

### `CodeMain.ino` (Arduino Mega)
- Reads 5x DHT11 & 5x INA219 sensors.
- Converts and packages data into pipe-separated strings:  
  `Temp1|Humidity1|...|Voltage5|Current5|Fan1|...|BatteryCap`

 <img width="1800" height="404" alt="image" src="https://github.com/user-attachments/assets/1dc070ea-4217-4259-9128-7e37a9545dcb" />

### 📟 ESP32 Code Structure (See `ESP32_Main.ino`)

<table>
<tr>
<td>

- **WiFi Access Point**  
  - Hosted via `WiFi.softAP()`, dashboard accessible at `192.168.4.1`.

- **Serial2 Interface**  
  - Communicates with Arduino Mega over RXD2/TXD2 at 9600 baud.

- **Data Parsing**  
  - Incoming pipe-separated sensor data is parsed into arrays:  
    `dhtTemps[]`, `dhtHumidities[]`, `voltages[]`, `currents[]`, `fans[]`.

- **Async Web Server**  
  - Serves HTML dashboard (`/`) and live sensor JSON (`/data`).

- **Frontend Dashboard**  
  - Auto-updates every 5s with real-time system stats via JavaScript.

</td>
<td>
  <img src="https://github.com/user-attachments/assets/d355badf-735e-49b9-8e71-8c93703543b9" alt="Dashboard" width="400" height= "450"/>
</td>
</tr>
</table>





---

#### 🌀 Fan Control Logic
- **>25°C** → Half speed  
- **≥27°C** → Full speed  
- Data sent via **Serial1** to ESP32 every few milliseconds

---

### 🖥️ ESP32 Server Interface
- **ESPAsyncWebServer** hosts a local dashboard at: `http://192.168.4.1`
- Connect to SSID: `EcoCharge BMS Web Server` (Password: `12345678`)
- Data is streamed and parsed by frontend using JavaScript `fetch()`

---

### 🧠 Key Considerations
- Match `Serial1.begin()` baud rates on both Arduino & ESP32
- Common **GND** is required between pack & INA219s
- Use `|` delimited format for consistent parsing
- DHT11s offer better rapid thermal feedback than thermistors

---

### 🧪 How to Use
1. Power the system (Arduino + ESP32)
2. Connect to Wi-Fi: `EcoCharge BMS Web Server`
3. Open browser → visit `http://192.168.4.1`
4. Monitor live stats

---

### 📸 Demo Visuals

| OLED Display | Battery Pack | Design | Interface |
|--------------|--------------|--------|-----------|
| <img width="250" alt="OLED Display" src="https://github.com/user-attachments/assets/b14f398e-935d-4aff-b8e8-f832caa2a6f6" /> | <img width="250" alt="Battery Pack" src="https://github.com/user-attachments/assets/ee3818d4-1d00-4cb8-aec5-12401e4cf304" /> | <img width="250" alt="EcoCharge Design Sketch" src="https://github.com/user-attachments/assets/0520d17a-76b2-465b-8155-8457a57b3db4" /> | <img width="250" alt="Interface" src="https://github.com/user-attachments/assets/dc20eae7-19ab-40ab-918d-0d6048deb8be" /> |



