#include <math.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPClient.h>
#include <Arduino.h>

// ===== WIFI =====
const char* ssid = "RIUAEMFI"; //"MEDIACOM-17073";
const char* password = ""; //"NFR384Na54";

// ===== GOOGLE SHEETS =====
String url = "https://script.google.com/macros/s/AKfycbwsDNh_BOBE_yWu6DA_dfr1j7D32vDOgg4_g24_lpruGU0W1oF4xAjcp-Hwfz_a3yA77A/exec";

// ===== RFID =====
#define SS_PIN 5
#define RST_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN);

// ===== SERVER =====
WebServer server(80);

// ===== PINES =====
const int ledRojo = 17;
const int ledAmarillo = 21;
const int ledVerde = 16;
const int buzzer = 15;

// ===== PWM =====
const int freqLED = 5000;
const int resLED = 8;
const int freqBuzz = 2000;
const int resBuzz = 8;

// ===== VARIABLES =====
int modoActivo = -1;
String uidActivo = "";
String ultimoUID = "Ninguno";

uint8_t brillo = 180;
uint8_t volumen = 180;

unsigned long tTick = 0;
unsigned long tFx = 0;
unsigned long ultimoScan = 0;

// ===== UID TARJETAS =====
byte tarjetaSueno[] = {0xA1,0xB2,0xC3,0xD4};
byte tarjetaFiesta[] = {0x49,0x0A,0xA9,0x00};
byte tarjetaNaturaleza[] = {0x55,0x66,0x77,0x88};
byte tarjetaAlarma[] = {0xE2,0xFA,0x17,0x05};

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(400);

  Serial.println("IP:");
  Serial.println(WiFi.localIP());

  SPI.begin();
  rfid.PCD_Init();

  ledcAttach(ledRojo, freqLED, resLED);
  ledcAttach(ledAmarillo, freqLED, resLED);
  ledcAttach(ledVerde, freqLED, resLED);
  ledcAttach(buzzer, freqBuzz, resBuzz);

  server.on("/", paginaWeb);

  server.on("/modo", [](){
    if (server.hasArg("m")) {
      modoActivo = server.arg("m").toInt();
      uidActivo = "WEB";
    }
    server.send(200, "text/plain", "ok");
  });

  server.on("/vol", [](){
    if (server.hasArg("v")) volumen = constrain(server.arg("v").toInt(),0,255);
    server.send(200, "text/plain", "ok");
  });

  server.on("/brillo", [](){
    if (server.hasArg("b")) brillo = constrain(server.arg("b").toInt(),0,255);
    server.send(200, "text/plain", "ok");
  });

  server.begin();
}

// ===== LOOP =====
void loop() {
  server.handleClient();
  ejecutarModo();
  leerRFID();
}

// ===== RFID =====
void leerRFID() {

  if (millis() - ultimoScan < 300) return;

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

    ultimoScan = millis();

    String uidStr = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uidStr += String(rfid.uid.uidByte[i], HEX);
    }

    ultimoUID = uidStr;

    procesarTarjeta(uidStr, rfid.uid.uidByte);

    rfid.PICC_HaltA();
  }
}

// ===== TOGGLE =====
void procesarTarjeta(String uidStr, byte *uidBytes) {

  int modoDetectado = -1;

  if (compararUID(uidBytes, tarjetaSueno)) modoDetectado = 0;
  else if (compararUID(uidBytes, tarjetaFiesta)) modoDetectado = 1;
  else if (compararUID(uidBytes, tarjetaNaturaleza)) modoDetectado = 2;
  else if (compararUID(uidBytes, tarjetaAlarma)) modoDetectado = 3;

  if (modoDetectado == -1) return;

  if (modoActivo == modoDetectado && uidActivo == uidStr) {

    enviarDatos(uidStr, String(modoDetectado), "Apagado");

    modoActivo = -1;
    uidActivo = "";

  } else {

    modoActivo = modoDetectado;
    uidActivo = uidStr;

    enviarDatos(uidStr, String(modoActivo), "Encendido");
  }
}

// ===== MODOS =====
void ejecutarModo() {

  if (millis() - tTick < 30) return;
  tTick = millis();

  if (modoActivo == -1) {
    ledcWrite(ledRojo, 0);
    ledcWrite(ledAmarillo, 0);
    ledcWrite(ledVerde, 0);
    ledcWrite(buzzer, 0);
    return;
  }

  auto setLEDs = [&](uint8_t r, uint8_t a, uint8_t v){
    ledcWrite(ledRojo, (r * brillo) / 255);
    ledcWrite(ledAmarillo, (a * brillo) / 255);
    ledcWrite(ledVerde, (v * brillo) / 255);
  };

  switch (modoActivo) {

    case 0: {
      static float fase = 0;
      fase += 0.03;
      if (fase > 6.28) fase = 0;

      uint8_t wave = (sin(fase)*0.5 + 0.5)*255;
      setLEDs(0, wave, wave);

      ledcWriteTone(buzzer, 180);
      ledcWrite(buzzer, volumen/3);
      break;
    }

    case 1:
      setLEDs(random(0,255), random(0,255), random(0,255));
      ledcWriteTone(buzzer, random(900,2000));
      ledcWrite(buzzer, volumen);
      break;

    case 2:
      setLEDs(0,0,255);
      if (millis() - tFx > 40) {
        tFx = millis();
        ledcWriteTone(buzzer, random(300,800));
        ledcWrite(buzzer, volumen/2);
      }
      break;

    case 3: {
      static bool estado = false;
      if (millis() - tFx > 120) {
        tFx = millis();
        estado = !estado;
      }

      setLEDs(estado ? 255 : 0, 0, 0);

      if (estado) {
        ledcWriteTone(buzzer, 2000);
        ledcWrite(buzzer, volumen);
      } else {
        ledcWrite(buzzer, 0);
      }
      break;
    }
  }
}

// ===== WEB =====
void paginaWeb() {

  String nombreModo;
  if (modoActivo == -1) nombreModo = "Apagado";
  else if (modoActivo == 0) nombreModo = "Sueno";
  else if (modoActivo == 1) nombreModo = "Fiesta";
  else if (modoActivo == 2) nombreModo = "Naturaleza";
  else nombreModo = "Alarma";

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">

<style>
body{
  margin:0;
  font-family:Arial;
  background:#0b1220;
  color:white;
  text-align:center;
}
.container{
  max-width:420px;
  margin:auto;
  padding:20px;
}
.card{
  background:#121a2e;
  border-radius:18px;
  padding:20px;
  box-shadow:0 10px 25px rgba(0,0,0,0.4);
}
.status{
  color:#9aa4b2;
  font-size:14px;
  margin-bottom:15px;
}
.grid{
  display:grid;
  grid-template-columns:1fr 1fr;
  gap:12px;
}
button{
  border:none;
  border-radius:14px;
  padding:18px;
  color:white;
}
.sueno{background:#22c55e;}
.fiesta{background:#ec4899;}
.naturaleza{background:#16a34a;}
.alarma{background:#ef4444;}
</style>
</head>
<body>

<div class="container">
<div class="card">

<h1>Habitacion Inteligente</h1>

<div class="status">
Modo: )rawliteral" + nombreModo + R"rawliteral(<br>
UID: )rawliteral" + ultimoUID + R"rawliteral(
</div>

<div class="grid">
<button class="sueno" onclick="setModo(0)"> Sueno</button>
<button class="fiesta" onclick="setModo(1)"> Fiesta</button>
<button class="naturaleza" onclick="setModo(2)"> Naturaleza</button>
<button class="alarma" onclick="setModo(3)"> Alarma</button>
</div>

<br>
Volumen
<input type="range" min="0" max="255" value=")rawliteral" + String(volumen) + R"rawliteral(" oninput="setVol(this.value)">

<br><br>
Brillo
<input type="range" min="0" max="255" value=")rawliteral" + String(brillo) + R"rawliteral(" oninput="setBri(this.value)">

</div>
</div>

<script>
function setModo(m){ fetch('/modo?m='+m); }
function setVol(v){ fetch('/vol?v='+v); }
function setBri(b){ fetch('/brillo?b='+b); }
</script>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

// ===== GOOGLE =====
void enviarDatos(String uid, String modo, String estado) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String fullUrl = url + "?uid=" + uid + "&modo=" + modo + "&estado=" + estado;
    http.begin(fullUrl);
    http.GET();
    http.end();
  }
}

// ===== UID =====
bool compararUID(byte *uid1, byte *uid2) {
  for (byte i = 0; i < 4; i++) {
    if (uid1[i] != uid2[i]) return false;
  }
  return true;
}