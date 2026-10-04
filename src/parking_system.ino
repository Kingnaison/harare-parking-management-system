#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =====================================================
// GEORGE SULINDIKA CAR PARKING BAY MANAGEMENT SYSTEM
// FINAL 4-BAY VERSION
// =====================================================

// ---------------- SENSOR PINS ----------------

#define TRIG1 13
#define ECHO1 35

#define TRIG2 14
#define ECHO2 34

#define TRIG3 25
#define ECHO3 32

#define TRIG4 26
#define ECHO4 33

#define NUM_BAYS 4


// ---------------- LCD ----------------

#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

LiquidCrystal_I2C lcd(
  LCD_ADDRESS,
  LCD_COLUMNS,
  LCD_ROWS
);


// ---------------- PARKING RANGE ----------------

// 3 cm to 9 cm = OCCUPIED

#define OCCUPIED_MIN_DISTANCE 3.0
#define OCCUPIED_MAX_DISTANCE 9.0


// ---------------- SYSTEM ----------------

#define STATUS_INTERVAL 3000

unsigned long lastSensorUpdate = 0;


// ---------------- WIFI ----------------

const char* AP_SSID = "Sulindika_Parking";
const char* AP_PASSWORD = "Parking123";

WebServer server(80);


// ---------------- PARKING DATA ----------------

float distances[NUM_BAYS] = {
  0, 0, 0, 0
};

bool occupied[NUM_BAYS] = {
  false,
  false,
  false,
  false
};


// =====================================================
// READ ONE ULTRASONIC SENSOR
// =====================================================

float readSingleDistance(
  int trigPin,
  int echoPin
) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 10 ms is more than enough for 3-9 cm
  // and prevents a failed sensor from blocking the system.
  unsigned long duration =
    pulseIn(
      echoPin,
      HIGH,
      10000
    );

  if (duration == 0) {
    return -1;
  }

  float distance =
    duration * 0.0343 / 2.0;

  if (
    distance < 2.0 ||
    distance > 400.0
  ) {
    return -1;
  }

  return distance;
}


// =====================================================
// READ SENSOR THREE TIMES
// =====================================================

float readDistance(
  int trigPin,
  int echoPin
) {

  float readings[3];

  int validCount = 0;

  for (int i = 0; i < 3; i++) {

    float distance =
      readSingleDistance(
        trigPin,
        echoPin
      );

    if (distance > 0) {

      readings[validCount] =
        distance;

      validCount++;
    }

    delay(30);
  }

  if (validCount == 0) {
    return -1;
  }

  // Sort readings

  for (
    int i = 0;
    i < validCount - 1;
    i++
  ) {

    for (
      int j = i + 1;
      j < validCount;
      j++
    ) {

      if (
        readings[j] <
        readings[i]
      ) {

        float temp =
          readings[i];

        readings[i] =
          readings[j];

        readings[j] =
          temp;
      }
    }
  }

  // Median

  if (validCount == 1) {
    return readings[0];
  }

  if (validCount == 2) {
    return (
      readings[0] +
      readings[1]
    ) / 2.0;
  }

  return readings[1];
}


// =====================================================
// COUNT OCCUPIED
// =====================================================

int getOccupiedCount() {

  int count = 0;

  for (int i = 0; i < NUM_BAYS; i++) {

    if (occupied[i]) {
      count++;
    }
  }

  return count;
}


// =====================================================
// COUNT AVAILABLE
// =====================================================

int getAvailableCount() {

  return NUM_BAYS - getOccupiedCount();
}


// =====================================================
// UPDATE LCD
// =====================================================

void updateLCD() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("PARKING BAYS");

  lcd.setCursor(0, 1);
  lcd.print("LEFT: ");
  lcd.print(getAvailableCount());
}


// =====================================================
// UPDATE FOUR SENSORS
// =====================================================

void updateSensors() {

  int trigPins[NUM_BAYS] = {
    TRIG1,
    TRIG2,
    TRIG3,
    TRIG4
  };

  int echoPins[NUM_BAYS] = {
    ECHO1,
    ECHO2,
    ECHO3,
    ECHO4
  };

  for (int i = 0; i < NUM_BAYS; i++) {

    float distance =
      readDistance(
        trigPins[i],
        echoPins[i]
      );

    if (distance >= 0) {

      distances[i] =
        distance;

      // 3 cm - 9 cm = occupied

      if (
        distance >= OCCUPIED_MIN_DISTANCE &&
        distance <= OCCUPIED_MAX_DISTANCE
      ) {

        occupied[i] = true;

      } else {

        occupied[i] = false;
      }

    }

    // Give the next ultrasonic sensor time
    // to operate independently.
    delay(50);
  }


  // Serial output for testing

  Serial.println();
  Serial.println(
    "------------------------------"
  );

  for (int i = 0; i < NUM_BAYS; i++) {

    Serial.print("Bay ");
    Serial.print(i + 1);
    Serial.print(": ");

    if (distances[i] > 0) {

      Serial.print(
        distances[i],
        2
      );

      Serial.print(" cm - ");

      if (occupied[i]) {
        Serial.println("OCCUPIED");
      } else {
        Serial.println("AVAILABLE");
      }

    } else {

      Serial.println("NO VALID READING");
    }
  }

  Serial.print("AVAILABLE: ");
  Serial.println(getAvailableCount());

  Serial.print("OCCUPIED: ");
  Serial.println(getOccupiedCount());
}


// =====================================================
// WEB PAGE
// =====================================================

void handleRoot() {

  String html = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta name="viewport"
content="width=device-width, initial-scale=1.0">

<title>
George Sulindika Parking
</title>

<style>

* {
  box-sizing: border-box;
}

body {

  margin: 0;
  padding: 0;

  font-family: Arial, sans-serif;

  background: #f2f4f7;

  color: #222;
}

.header {

  background: #1f2937;

  color: white;

  text-align: center;

  padding: 25px 15px;
}

.header h1 {

  margin: 0;

  font-size: 28px;
}

.header p {

  margin: 8px 0 0;

  font-size: 16px;
}

.connection {

  margin-top: 12px;

  font-weight: bold;

  font-size: 18px;
}

.connection.online {

  color: #39ff14;
}

.connection.offline {

  color: #ff3333;
}

.container {

  max-width: 1000px;

  margin: auto;

  padding: 25px;
}

.title {

  text-align: center;

  font-size: 26px;

  font-weight: bold;

  margin-bottom: 20px;
}

.main-status {

  background: white;

  border-radius: 15px;

  padding: 25px;

  text-align: center;

  box-shadow:
    0 3px 10px
    rgba(0,0,0,0.1);

  margin-bottom: 25px;
}

.left-number {

  font-size: 60px;

  font-weight: bold;

  margin: 10px 0;
}

.left-text {

  font-size: 20px;

  font-weight: bold;
}

.summary {

  display: grid;

  grid-template-columns:
    repeat(3, 1fr);

  gap: 15px;

  margin-bottom: 25px;
}

.summary-card {

  background: white;

  padding: 20px;

  border-radius: 12px;

  text-align: center;

  box-shadow:
    0 3px 8px
    rgba(0,0,0,0.08);
}

.summary-card h3 {

  margin: 0 0 10px;

  font-size: 16px;
}

.summary-card div {

  font-size: 30px;

  font-weight: bold;
}

.bays {

  display: grid;

  grid-template-columns:
    repeat(2, 1fr);

  gap: 20px;
}

.bay {

  background: white;

  border-radius: 15px;

  padding: 25px;

  text-align: center;

  box-shadow:
    0 3px 10px
    rgba(0,0,0,0.1);
}

.bay h2 {

  margin-top: 0;
}

.status {

  display: inline-block;

  padding: 12px 25px;

  border-radius: 30px;

  font-size: 18px;

  font-weight: bold;
}

.available {

  background: #d1fae5;

  color: #065f46;
}

.occupied {

  background: #fee2e2;

  color: #991b1b;
}

.footer {

  text-align: center;

  margin-top: 30px;

  padding: 20px;

  color: #666;

  font-size: 14px;
}

@media(max-width:700px) {

  .summary {
    grid-template-columns: 1fr;
  }

  .bays {
    grid-template-columns: 1fr;
  }

  .header h1 {
    font-size: 21px;
  }
}

</style>

</head>

<body>

<div class="header">

<h1>
WELCOME TO G. SULINDIKA PARKING BAY
</h1>

<p>
Car Parking Bay Management System
</p>

<div id="connectionStatus"
     class="connection online">

ONLINE

</div>

</div>


<div class="container">

<div class="title">
PARKING BAYS
</div>


<div class="main-status">

<div class="left-number"
     id="largeAvailable">

4

</div>

<div class="left-text">
BAYS LEFT
</div>

</div>


<div class="summary">

<div class="summary-card">

<h3>
TOTAL BAYS
</h3>

<div>
4
</div>

</div>


<div class="summary-card">

<h3>
OCCUPIED
</h3>

<div id="occupied">
0
</div>

</div>


<div class="summary-card">

<h3>
AVAILABLE
</h3>

<div id="available">
4
</div>

</div>

</div>


<div class="bays">

<div class="bay">

<h2>
BAY 1
</h2>

<div id="bay1"
     class="status available">

AVAILABLE

</div>

</div>


<div class="bay">

<h2>
BAY 2
</h2>

<div id="bay2"
     class="status available">

AVAILABLE

</div>

</div>


<div class="bay">

<h2>
BAY 3
</h2>

<div id="bay3"
     class="status available">

AVAILABLE

</div>

</div>


<div class="bay">

<h2>
BAY 4
</h2>

<div id="bay4"
     class="status available">

AVAILABLE

</div>

</div>

</div>


<div class="footer">

<div id="updateTime">
Last update: Waiting...
</div>

<p>
Parking status refreshes every 3 seconds
</p>

</div>

</div>


<script>

function updateDashboard() {

  fetch(
    '/api/status',
    {
      cache: 'no-store'
    }
  )

  .then(response => {

    if (!response.ok) {
      throw new Error("Offline");
    }

    return response.json();

  })

  .then(data => {

    // ONLINE ONLY

    document
      .getElementById(
        "connectionStatus"
      )
      .innerText = "ONLINE";

    document
      .getElementById(
        "connectionStatus"
      )
      .className =
        "connection online";


    // AVAILABLE

    document
      .getElementById(
        "largeAvailable"
      )
      .innerText =
        data.available;

    document
      .getElementById(
        "available"
      )
      .innerText =
        data.available;


    // OCCUPIED

    document
      .getElementById(
        "occupied"
      )
      .innerText =
        data.occupied;


    // FOUR BAYS

    for (
      let i = 1;
      i <= 4;
      i++
    ) {

      let bay =
        document.getElementById(
          "bay" + i
        );

      if (
        data.bays[i - 1].occupied
      ) {

        bay.innerText =
          "OCCUPIED";

        bay.className =
          "status occupied";

      } else {

        bay.innerText =
          "AVAILABLE";

        bay.className =
          "status available";
      }
    }


    // TIME

    document
      .getElementById(
        "updateTime"
      )
      .innerText =
        "Last update: " +
        new Date().toLocaleTimeString();

  })

  .catch(error => {

    // OFFLINE ONLY

    document
      .getElementById(
        "connectionStatus"
      )
      .innerText =
        "OFFLINE";

    document
      .getElementById(
        "connectionStatus"
      )
      .className =
        "connection offline";
  });
}


// First check

updateDashboard();


// Every 3 seconds

setInterval(
  updateDashboard,
  3000
);

</script>

</body>

</html>

)rawliteral";


  server.send(
    200,
    "text/html",
    html
  );
}


// =====================================================
// API
// =====================================================

void handleStatus() {

  String json = "{";

  json += "\"total\":";
  json += NUM_BAYS;

  json += ",\"occupied\":";
  json += getOccupiedCount();

  json += ",\"available\":";
  json += getAvailableCount();

  json += ",\"bays\":[";


  for (int i = 0; i < NUM_BAYS; i++) {

    if (i > 0) {
      json += ",";
    }

    json += "{";

    json += "\"bay\":";
    json += i + 1;

    json += ",\"occupied\":";

    if (occupied[i]) {
      json += "true";
    } else {
      json += "false";
    }

    json += "}";

  }


  json += "]";

  json += "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  delay(500);


  // ===================================================
  // SENSOR PINS
  // ===================================================

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(TRIG3, OUTPUT);
  pinMode(ECHO3, INPUT);

  pinMode(TRIG4, OUTPUT);
  pinMode(ECHO4, INPUT);


  digitalWrite(TRIG1, LOW);
  digitalWrite(TRIG2, LOW);
  digitalWrite(TRIG3, LOW);
  digitalWrite(TRIG4, LOW);


  // ===================================================
  // LCD STARTS FIRST
  // ===================================================

  Wire.begin(21, 22);

  lcd.init();

  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("PARKING BAYS");

  lcd.setCursor(0, 1);
  lcd.print("LEFT: 4");


  Serial.println();
  Serial.println("==============================");
  Serial.println("LCD INITIALIZED");
  Serial.println("==============================");


  // ===================================================
  // START WIFI
  // ===================================================

  WiFi.mode(WIFI_AP);

  bool wifiStarted =
    WiFi.softAP(
      AP_SSID,
      AP_PASSWORD
    );


  Serial.println();

  if (wifiStarted) {

    Serial.println(
      "WIFI ACCESS POINT: STARTED"
    );

    Serial.print(
      "SSID: "
    );

    Serial.println(
      AP_SSID
    );

    Serial.print(
      "IP ADDRESS: "
    );

    Serial.println(
      WiFi.softAPIP()
    );

  } else {

    Serial.println(
      "WIFI ACCESS POINT: FAILED"
    );
  }


  // ===================================================
  // START WEB SERVER
  // ===================================================

  server.on(
    "/",
    handleRoot
  );

  server.on(
    "/api/status",
    handleStatus
  );


  server.begin();


  Serial.println(
    "WEB SERVER: STARTED"
  );

  Serial.println(
    "SYSTEM READY"
  );

  Serial.println(
    "=============================="
  );


  // IMPORTANT:
  // Do NOT read sensors during setup.
  // The system starts first.
  lastSensorUpdate =
    millis();
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // Web server
  server.handleClient();


  // Sensor update every 3 seconds

  if (
    millis() -
    lastSensorUpdate
    >= STATUS_INTERVAL
  ) {

    lastSensorUpdate =
      millis();


    updateSensors();

    updateLCD();
  }
}
