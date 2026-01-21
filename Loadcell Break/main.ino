
#include <WiFi.h>
#include <Preferences.h>
#include "HX711.h"

// ---- WLAN ----
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// ---- HX711 Pins ----
#define HX711_DOUT_PIN 4
#define HX711_SCK_PIN  5

HX711 scale;

// ---- Kalibrierung ----
bool calibStep1 = false;
long calibEmpty = 0;
float scaleFactor = 1.0;

// ---- Speicher (NVS) ----
Preferences prefs;

// ---- Webserver ----
WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // ---- NVS öffnen ----
  prefs.begin("calib", false);

  // gespeicherten Skalenfaktor laden
  if (prefs.isKey("factor")) {
    scaleFactor = prefs.getFloat("factor", 1.0);
    Serial.print("Geladener Kalibrierfaktor: ");
    Serial.println(scaleFactor, 6);
  } else {
    Serial.println("Kein gespeicherter Kalibrierfaktor gefunden.");
  }

  // ---- HX711 ----
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);

  Serial.print("Warte auf HX711...");
  while (!scale.is_ready()) {
    Serial.print(".");
    delay(200);
  }
  Serial.println(" bereit.");

  scale.set_scale(scaleFactor);
  scale.tare();

  // ---- WLAN ----
  Serial.printf("Verbinde mit WLAN '%s'...\n", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nWLAN verbunden!");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  while (!client.available()) delay(1);

  String req = client.readStringUntil('\r');
  client.flush();

  // ---- Befehle auswerten ----
  if (req.indexOf("/tare") != -1) {
    scale.tare();
  }

  if (req.indexOf("/calib_start") != -1) {
    calibEmpty = scale.read_average(10);
    calibStep1 = true;
  }

  if (req.indexOf("/calib_finish") != -1) {
    if (calibStep1) {
      int pos = req.indexOf("weight=");
      if (pos > 0) {
        float knownWeight = req.substring(pos + 7).toFloat();
        long calibLoad = scale.read_average(10);
        long diff = calibLoad - calibEmpty;

        if (knownWeight != 0 && diff != 0) {
          scaleFactor = (float)diff / knownWeight;
          scale.set_scale(scaleFactor);

          // ---- Kalibrierfaktor dauerhaft speichern ----
          prefs.putFloat("factor", scaleFactor);
          Serial.println("Kalibrierfaktor gespeichert!");
        }

        calibStep1 = false;
      }
    }
  }

  // ---- Messung ----
  float weight = scale.get_units(5);

  // ---- HTML ----
 
String html =
"<!DOCTYPE html><html><head>"
"<meta charset='utf-8'/>"
"<meta http-equiv='refresh' content='1'/>"
"<title>Loadcell Messung</title>"
"<style>"

/* ---- GLOBAL ---- */
"body {"
"   margin:0;"
"   padding:0;"
"   background:#f0f2f5;"
"   font-family: Arial, sans-serif;"
"   text-align:center;"
"}"

"h1 {"
"   font-size:32px;"
"   color:#333;"
"   margin-top:40px;"
"}"

/* ---- CARD ---- */
".card {"
"   background:white;"
"   width:90%;"
"   max-width:400px;"
"   margin:30px auto;"
"   padding:25px;"
"   border-radius:14px;"
"   box-shadow:0 4px 15px rgba(0,0,0,0.1);"
"}"

".value {"
"   font-size:50px;"
"   font-weight:bold;"
"   margin:20px 0;"
"   color:#0078d4;"
"}"

/* ---- BUTTON ---- */
"button {"
"   background:#0078d4;"
"   color:white;"
"   border:none;"
"   border-radius:8px;"
"   padding:14px 28px;"
"   font-size:18px;"
"   cursor:pointer;"
"   margin:10px;"
"   transition:0.2s;"
"}"

"button:hover {"
"   background:#005a9e;"
"}"

/* ---- INPUT FELDER ---- */
"input[type='number'] {"
"   padding:10px;"
"   font-size:18px;"
"   border-radius:8px;"
"   border:1px solid #aaa;"
"   width:160px;"
"   margin:12px 0;"
"}"

/* ---- INFO ---- */
".info {"
"   font-size:16px;"
"   color:#666;"
"   margin-top:10px;"
"}"

"</style>"
"</head><body>"

"<h1>Loadcell Messung</h1>"

"<div class='card'>"
"<div class='value'>" + String(weight, 2) + " g</div>"
"</div>"

"<div class='card'>"
"<h2>Tarierung</h2>"
"/tare<button>TARA</button></a>"
"</div>"

"<div class='card'>"
"<h2>Kalibrierung</h2>";

if (!calibStep1) {

    html += "/calib_start<button>Kalibrierung starten</button></a>";

} else {

    html += "<p class='info'>Leeres Gewicht gespeichert.<br>Bitte bekanntes Gewicht auflegen.</p>"
            "/calib_finish"
            "<input type='number' name='weight' placeholder='Gewicht in g' required><br>"
            "<button type='submit'>Kalibrierung abschließen</button>"
            "</form>";
}

html += "<p class='info'>Aktueller Faktor: " + String(scaleFactor, 6) + "</p>"
"</div>"

"</body></html>";


  // ---- HTTP Antwort ----
  client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
  client.print(html);
}
