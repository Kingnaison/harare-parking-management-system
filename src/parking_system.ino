#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =========================
// PARKING BAY CONFIGURATION
// =========================

#define NUM_BAYS 4

// Bay 1
const int TRIG_PINS[NUM_BAYS] = {13, 14, 25, 26};
const int ECHO_PINS[NUM_BAYS] = {35, 34, 32, 33};

// LCD I2C
#define SDA_PIN 21
#define SCL_PIN 22
#define LCD_ADDRESS 0x27

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

// =========================
// WIFI CONFIGURATION
// =========================

const char* AP_SSID = "Sulindika_Parking";
const char* AP_PASSWORD = "Parking123";

WebServer server(80);

// =========================
// PARKING STATUS
// =========================

bool bayOccupied[NUM_BAYS] = {
  false, false, false, false
};

// 3–9 cm = occupied
const float OCCUPIED_MIN_CM = 3.0;
const float OCCUPIED_MAX_CM = 9.0;

// =========================
// SENSOR FUNCTIONS
// =========================

float readDistance(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration =
      pulseIn(echoPin, HIGH, 10000);

  // No valid echo
  if (duration == 0) {
    return -1.0;
  }

  float distance =
      (duration * 0.0343) / 2.0;

  return distance;
}

// =========================
// MEDIAN FUNCTION
// =========================

float median3(float a, float b, float c) {

  if ((a <= b && b <= c) ||
      (c <= b && b <= a)) {
    return b;
  }

  if ((b <= a && a <= c) ||
      (c <= a && a <= b)) {
    return a;
  }

  return c;
}

// =========================
// READ BAY
// =========================

bool readBay(int bay) {

  float readings[3];
  int validCount = 0;

  for (int i = 0; i < 3; i++) {

    float distance =
        readDistance(
          TRIG_PINS[bay],
          ECHO_PINS[bay]
        );

    if (distance > 0) {
      readings[validCount++] = distance;
    }

    delay(20);
  }

  // Invalid/no-echo reading:
  // retain previous occupancy state
  if (validCount == 0) {
    return bayOccupied[bay];
  }

  float distance;

  if (validCount == 1) {
    distance = readings[0];
  }
  else if (validCount == 2) {
    distance =
      (readings[0] + readings[1]) / 2.0;
  }
  else {
    distance =
      median3(
        readings[0],
        readings[1],
        readings[2]
      );
  }

  bayOccupied[bay] =
      (distance >= OCCUPIED_MIN_CM &&
       distance <= OCCUPIED_MAX_CM);

  return bayOccupied[bay];
}

// =========================
// COUNT AVAILABLE BAYS
// =========================

int getAvailableBays() {

  int available = 0;

  for (int i = 0; i < NUM_BAYS; i++) {

    if (!bayOccupied[i]) {
      available++;
    }
  }

  return available;
}

// =========================
// LCD UPDATE
// =========================

void updateLCD() {

  int available = getAvailableBays();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("PARKING BAYS");

  lcd.setCursor(0, 1);
  lcd.print("LEFT: ");
  lcd.print(available);
}

// =========================
// WEB DASHBOARD
// =========================

String getDashboardHTML() {

  int available = getAvailableBays();
  int occupied = NUM_BAYS - available;

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>

<meta name="viewport"
      content="width=device-width, initial-scale=1">

<title>Sulindika Parking</title>

<style>

body {
  font-family: Arial, sans-serif;
  margin: 0;
  padding: 20px;
  background: #f2f2f2;
  text-align: center;
}

.container {
  max-width: 900px;
  margin: auto;
}

h1 {
  margin-bottom: 5px;
}

h2 {
  margin-top: 5px;
}

.summary {
  display: flex;
  justify-content: center;
  gap: 15px;
  flex-wrap: wrap;
  margin: 20px 0;
}

.summary-card {
  background: white;
  padding: 15px 25px;
  border-radius: 10px;
  min-width: 120px;
  box-shadow: 0 2px 6px rgba(0,0,0,0.15);
}

.bays {
  display: grid;
  grid-template-columns:
    repeat(auto-fit, minmax(150px, 1fr));
  gap: 15px;
}

.bay {
  background: white;
  padding: 20px;
  border-radius: 10px;
  box-shadow: 0 2px 6px rgba(0,0,0,0.15);
}

.status {
  font-size: 22px;
  font-weight: bold;
  margin-top: 10px;
}

.online {
  margin-top: 20px;
  font-weight: bold;
}

</style>

</head>

<body>

<div class="container">

<h1>WELCOME TO G. SULINDIKA PARKING BAY</h1>

<h2>PARKING BAYS</h2>

<div class="summary">

<div class="summary-card">
<strong>Total Bays</strong>
<div id="total">4</div>
</div>

<div class="summary-card">
<strong>Available</strong>
<div id="available">0</div>
</div>

<div class="summary-card">
<strong>Occupied</strong>
<div id="occupied">0</div>
</div>

</div>

<div class="bays">

<div class="bay">
<h3>Bay 1</h3>
<div id="bay1" class="status">
AVAILABLE
</div>
</div>

<div class="bay">
<h3>Bay 2</h3>
<div id="bay2" class="status">
AVAILABLE
</div>
</div>

<div class="bay">
<h3>Bay 3</h3>
<div id="bay3" class="status">
AVAILABLE
</div>
</div>

<div class="bay">
<h3>Bay 4</h3>
<div id="bay4" class="status">
AVAILABLE
</div>
</div>

</div>

<div class="online">
ESP32 STATUS:
<span id="connection">ONLINE</span>
</div>

</div>

<script>

function updateDashboard() {

  fetch('/status')
    .then(response => response.json())
    .then(data => {

      document.getElementById("total")
        .innerText = data.total;

      document.getElementById("available")
        .innerText = data.available;

      document.getElementById("occupied")
        .innerText = data.occupied;

      for (let i = 1; i <= 4; i++) {

        let element =
          document.getElementById("bay" + i);

        element.innerText =
          data["bay" + i]
            ? "OCCUPIED"
            : "AVAILABLE";
      }

      document.getElementById("connection")
        .innerText = "ONLINE";
    })
    .catch(error => {

      document.getElementById("connection")
        .innerText = "OFFLINE";
    });
}

updateDashboard();

setInterval(updateDashboard, 3000);

</script>

</body>
</html>
)rawliteral";

  return html;
}

// =========================
// WEB ROUTES
// =========================

void handleRoot() {

  server.send(
    200,
    "text/html",
    getDashboardHTML()
  );
}

void handleStatus() {

  int available = getAvailableBays();
  int occupied = NUM_BAYS - available;

  String json = "{";

  json += "\"total\":";
  json += String(NUM_BAYS);
  json += ",";

  json += "\"available\":";
  json += String(available);
  json += ",";

  json += "\"occupied\":";
  json += String(occupied);
  json += ",";

  for (int i = 0; i < NUM_BAYS; i++) {

    json += "\"bay";
    json += String(i + 1);
    json += "\":";
    json += bayOccupied[i] ? "true" : "false";

    if (i < NUM_BAYS - 1) {
      json += ",";
    }
  }

  json += "}";

  server.send(
    200,
    "application/json",
    json
  );
}

// =========================
// SETUP
// =========================

void setup() {

  Serial.begin(9600);

  // Sensor pins
  for (int i = 0; i < NUM_BAYS; i++) {

    pinMode(
      TRIG_PINS[i],
      OUTPUT
    );

    pinMode(
      ECHO_PINS[i],
      INPUT
    );

    digitalWrite(
      TRIG_PINS[i],
      LOW
    );
  }

  // LCD
  Wire.begin(
    SDA_PIN,
    SCL_PIN
  );

  lcd.init();
  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("PARKING BAYS");

  lcd.setCursor(0, 1);
  lcd.print("STARTING...");

  // Start ESP32 Access Point
  WiFi.mode(WIFI_AP);

  WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );

  Serial.println();
  Serial.println("ESP32 PARKING SYSTEM");
  Serial.println("--------------------");

  Serial.print("SSID: ");
  Serial.println(AP_SSID);

  Serial.print("IP: ");
  Serial.println(
    WiFi.softAPIP()
  );

  // Web server
  server.on(
    "/",
    handleRoot
  );

  server.on(
    "/status",
    handleStatus
  );

  server.begin();

  Serial.println(
    "Web server started."
  );

  delay(1000);

  updateLCD();
}

// =========================
// MAIN LOOP
// =========================

void loop() {

  server.handleClient();

  // Read all four parking bays
  for (int i = 0; i < NUM_BAYS; i++) {

    readBay(i);
  }

  updateLCD();

  delay(3000);
}