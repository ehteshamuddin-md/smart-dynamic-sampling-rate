#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// =====================================================
// PIN CONFIGURATION
// =====================================================

#define HR_PIN      34
#define SPO2_PIN    35
#define TEMP_PIN    32

#define OLED_SDA    21
#define OLED_SCL    22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// =====================================================
// WIFI CONFIGURATION
// =====================================================

#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// =====================================================
// ADAFRUIT IO CONFIGURATION
// =====================================================

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883

#define AIO_USERNAME    "Your_AIO_USERNAME"
#define AIO_KEY         "Your_AIO_KEY"

// =====================================================cd
// OLED
// =====================================================

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =====================================================
// MQTT
// =====================================================

WiFiClient client;

Adafruit_MQTT_Client mqtt(
  &client,
  AIO_SERVER,
  AIO_SERVERPORT,
  AIO_USERNAME,
  AIO_KEY
);

// =====================================================
// MQTT PUBLISH FEEDS
// =====================================================

Adafruit_MQTT_Publish heartRateFeed =
  Adafruit_MQTT_Publish(
    &mqtt,
    AIO_USERNAME "/feeds/heart-rate"
  );

Adafruit_MQTT_Publish spo2Feed =
  Adafruit_MQTT_Publish(
    &mqtt,
    AIO_USERNAME "/feeds/spo2"
  );

Adafruit_MQTT_Publish temperatureFeed =
  Adafruit_MQTT_Publish(
    &mqtt,
    AIO_USERNAME "/feeds/body-temperature"
  );

Adafruit_MQTT_Publish statusFeed =
  Adafruit_MQTT_Publish(
    &mqtt,
    AIO_USERNAME "/feeds/patient-status"
  );

Adafruit_MQTT_Publish samplingFeed =
  Adafruit_MQTT_Publish(
    &mqtt,
    AIO_USERNAME "/feeds/active-sampling"
  );

// =====================================================
// MQTT SUBSCRIBE FEEDS
// =====================================================

Adafruit_MQTT_Subscribe samplingIntervalFeed =
  Adafruit_MQTT_Subscribe(
    &mqtt,
    AIO_USERNAME "/feeds/sampling-interval"
  );

Adafruit_MQTT_Subscribe samplingModeFeed =
  Adafruit_MQTT_Subscribe(
    &mqtt,
    AIO_USERNAME "/feeds/sampling-mode"
  );

// =====================================================
// GLOBAL VARIABLES
// =====================================================

float heartRate = 0;
float spo2 = 0;
float temperature = 0;

bool patientAbnormal = false;

// AUTO mode by default
bool automaticMode = true;

// Manual sampling interval
int manualSamplingInterval = 30;

// Active sampling interval
int activeSamplingInterval = 30;

// =====================================================
// TIMING
// =====================================================

unsigned long nextSampleTime = 0;
unsigned long lastConditionCheck = 0;

const unsigned long CONDITION_CHECK_INTERVAL = 500;

// =====================================================
// FUNCTION DECLARATIONS
// =====================================================

void connectWiFi();
void MQTT_connect();

void readVitals();
void evaluatePatientCondition();
void updateSamplingRate();

void displayPatientData();
void publishPatientData();

void processMQTTMessages();

float mapFloat(
  float x,
  float in_min,
  float in_max,
  float out_min,
  float out_max
);

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  // -----------------------------
  // I2C
  // -----------------------------

  Wire.begin(OLED_SDA, OLED_SCL);

  // -----------------------------
  // OLED
  // -----------------------------

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
      ))
  {
    Serial.println("OLED initialization failed!");

    while (true)
    {
      delay(1000);
    }
  }

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(20, 10);
  display.println("SMART MONITOR");

  display.setCursor(15, 30);
  display.println("Starting IoT...");

  display.display();

  delay(1500);

  // -----------------------------
  // Wi-Fi
  // -----------------------------

  connectWiFi();

  // -----------------------------
  // MQTT subscription
  // -----------------------------

  mqtt.subscribe(&samplingIntervalFeed);
  mqtt.subscribe(&samplingModeFeed);

  // First sampling immediately
  nextSampleTime = millis();

  Serial.println();
  Serial.println("==============================");
  Serial.println(" SMART DYNAMIC MONITORING");
  Serial.println("==============================");

  Serial.println("System started.");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
  // Maintain MQTT connection
  MQTT_connect();

  // Process dashboard commands
  processMQTTMessages();

  // ---------------------------------------------------
  // Condition monitoring
  // ---------------------------------------------------

  if (millis() - lastConditionCheck >=
      CONDITION_CHECK_INTERVAL)
  {
    lastConditionCheck = millis();

    /*
       The condition monitor checks the patient frequently.
       This allows AUTO mode to react quickly even when
       the normal sampling interval is long.
    */

    readVitals();

    evaluatePatientCondition();

    updateSamplingRate();
  }

  // ---------------------------------------------------
  // Actual data sampling
  // ---------------------------------------------------

  if (millis() >= nextSampleTime)
  {
    // Read current patient parameters
    readVitals();

    evaluatePatientCondition();

    updateSamplingRate();

    // Display locally
    displayPatientData();

    // Upload to Adafruit IO
    publishPatientData();

    // Schedule next sample
    nextSampleTime =
      millis() +
      ((unsigned long)activeSamplingInterval * 1000UL);

    // Serial output
    Serial.println();
    Serial.println("------------------------------");

    Serial.print("Heart Rate: ");
    Serial.print(heartRate);
    Serial.println(" BPM");

    Serial.print("SpO2: ");
    Serial.print(spo2);
    Serial.println(" %");

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Patient Status: ");

    if (patientAbnormal)
      Serial.println("ABNORMAL");
    else
      Serial.println("STABLE");

    Serial.print("Mode: ");

    if (automaticMode)
      Serial.println("AUTO");
    else
      Serial.println("MANUAL");

    Serial.print("Active Sampling: ");
    Serial.print(activeSamplingInterval);
    Serial.println(" seconds");
  }

  // Small non-blocking delay
  delay(10);
}

// =====================================================
// WIFI CONNECTION
// =====================================================

void connectWiFi()
{
  Serial.println();
  Serial.println("==============================");
  Serial.println("Connecting to Wokwi Wi-Fi...");
  Serial.println("==============================");

  WiFi.mode(WIFI_STA);

  WiFi.begin("Wokwi-GUEST", "");

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);

    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Wi-Fi CONNECTED!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Wi-Fi connection FAILED.");
    Serial.print("Wi-Fi status: ");
    Serial.println(WiFi.status());
  }
}
// =====================================================
// MQTT CONNECTION
// =====================================================

void MQTT_connect()
{
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to Adafruit IO MQTT... ");

  int8_t ret;

  int attempts = 0;

  while (
    (ret = mqtt.connect()) != 0 &&
    attempts < 5
  )
  {
    Serial.println(
      mqtt.connectErrorString(ret)
    );

    mqtt.disconnect();

    delay(3000);

    attempts++;
  }

  if (mqtt.connected())
  {
    Serial.println("CONNECTED");
  }
  else
  {
    Serial.println(
      "MQTT connection unavailable."
    );
  }
}

// =====================================================
// READ VITALS
// =====================================================

void readVitals()
{
  int hrADC =
    analogRead(HR_PIN);

  int spo2ADC =
    analogRead(SPO2_PIN);

  int tempADC =
    analogRead(TEMP_PIN);

  /*
     These values simulate patient sensors
     using potentiometers in Wokwi.
  */

  heartRate =
    mapFloat(
      hrADC,
      0,
      4095,
      40,
      140
    );

  spo2 =
    mapFloat(
      spo2ADC,
      0,
      4095,
      85,
      100
    );

  temperature =
    mapFloat(
      tempADC,
      0,
      4095,
      34.0,
      40.0
    );

  // Round values for cleaner display
  heartRate =
    round(heartRate);

  spo2 =
    round(spo2);

  temperature =
    round(temperature * 10) / 10.0;
}

// =====================================================
// EVALUATE PATIENT CONDITION
// =====================================================

void evaluatePatientCondition()
{
  bool hrAbnormal =
    (heartRate < 60 ||
     heartRate > 100);

  bool spo2Abnormal =
    (spo2 < 95);

  bool temperatureAbnormal =
    (temperature < 36.0 ||
     temperature > 37.5);

  patientAbnormal =
    hrAbnormal ||
    spo2Abnormal ||
    temperatureAbnormal;
}

// =====================================================
// UPDATE SAMPLING RATE
// =====================================================

void updateSamplingRate()
{
  int previousSamplingInterval =
    activeSamplingInterval;

  if (automaticMode)
  {
    if (patientAbnormal)
    {
      // Abnormal condition → high-frequency monitoring
      activeSamplingInterval = 5;
    }
    else
    {
      // Stable condition → lower monitoring frequency
      activeSamplingInterval = 30;
    }
  }
  else
  {
    // Manual mode → dashboard-selected interval
    activeSamplingInterval =
      constrain(
        manualSamplingInterval,
        5,
        60
      );
  }

  // -------------------------------------------------
  // If sampling interval changed, trigger an
  // immediate data publication.
  // -------------------------------------------------

  if (activeSamplingInterval != previousSamplingInterval)
  {
    nextSampleTime = millis();
  }
}

// =====================================================
// OLED DISPLAY
// =====================================================

void displayPatientData()
{
  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  // Title
  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println(
    "SMART MONITOR"
  );

  // Heart rate + SpO2
  display.setCursor(0, 15);

  display.print("HR:");
  display.print((int)heartRate);
  display.print(" S:");
  display.print((int)spo2);

  // Temperature
  display.setCursor(0, 28);

  display.print("T:");
  display.print(temperature, 1);
  display.println(" C");

  // Status
  display.setCursor(0, 41);

  display.print("STATUS:");

  if (patientAbnormal)
    display.println(" ABNORMAL");
  else
    display.println(" STABLE");

  // Sampling
  display.setCursor(0, 54);

  display.print(activeSamplingInterval);
  display.print("s ");

  if (automaticMode)
    display.print("AUTO");
  else
    display.print("MANUAL");

  display.display();
}

// =====================================================
// PUBLISH DATA TO ADAFRUIT IO
// =====================================================

void publishPatientData()
{
  if (!mqtt.connected())
  {
    return;
  }

  Serial.println(
    "Publishing data to Adafruit IO..."
  );

  // Heart Rate
  if (!heartRateFeed.publish(heartRate))
  {
    Serial.println(
      "Heart Rate publish failed"
    );
  }

  // SpO2
  if (!spo2Feed.publish(spo2))
  {
    Serial.println(
      "SpO2 publish failed"
    );
  }

  // Temperature
  if (!temperatureFeed.publish(temperature))
  {
    Serial.println(
      "Temperature publish failed"
    );
  }

  // Patient status
  if (patientAbnormal)
  {
    if (!statusFeed.publish("ABNORMAL"))
    {
      Serial.println(
        "Status publish failed"
      );
    }
  }
  else
  {
    if (!statusFeed.publish("STABLE"))
    {
      Serial.println(
        "Status publish failed"
      );
    }
  }

  // Active sampling interval
  if (!samplingFeed.publish(
      (int32_t)activeSamplingInterval
    ))
  {
    Serial.println(
      "Sampling publish failed"
    );
  }

  Serial.println(
    "Data published."
  );
}

// =====================================================
// PROCESS DASHBOARD MQTT COMMANDS
// =====================================================

void processMQTTMessages()
{
  Adafruit_MQTT_Subscribe *subscription;

  while (
    (subscription = mqtt.readSubscription(10))
  )
  {
    // -----------------------------------------------
    // Sampling Interval
    // -----------------------------------------------

    if (
      subscription ==
      &samplingIntervalFeed
    )
    {
      int newInterval =
        atoi(
          (char *)samplingIntervalFeed.lastread
        );

      newInterval =
        constrain(
          newInterval,
          5,
          60
        );

      manualSamplingInterval =
        newInterval;

      Serial.print(
        "New manual sampling interval: "
      );

      Serial.print(
        manualSamplingInterval
      );

      Serial.println(" seconds");

      if (!automaticMode)
      {
        activeSamplingInterval =
          manualSamplingInterval;

        nextSampleTime =
          millis() +
          ((unsigned long)
             activeSamplingInterval *
           1000UL);
      }
    }

    // -----------------------------------------------
    // Sampling Mode
    // -----------------------------------------------

    if (
      subscription ==
      &samplingModeFeed
    )
    {
      String mode =
        String(
          (char *)samplingModeFeed.lastread
        );

      mode.trim();

      mode.toUpperCase();

      if (
        mode == "AUTO" ||
        mode == "AUTOMATIC"
      )
      {
        automaticMode = true;

        Serial.println(
          "Sampling mode: AUTO"
        );
      }
      else if (
        mode == "MANUAL"
      )
      {
        automaticMode = false;

        activeSamplingInterval =
          manualSamplingInterval;

        Serial.println(
          "Sampling mode: MANUAL"
        );

        nextSampleTime =
          millis() +
          ((unsigned long)
             activeSamplingInterval *
           1000UL);
      }

      updateSamplingRate();
    }
  }
}

// =====================================================
// FLOAT MAP FUNCTION
// =====================================================

float mapFloat(
  float x,
  float in_min,
  float in_max,
  float out_min,
  float out_max
)
{
  return (
    (x - in_min) *
    (out_max - out_min) /
    (in_max - in_min)
  ) + out_min;
}