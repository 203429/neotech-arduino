// LCD
#include <LiquidCrystal.h>
LiquidCrystal lcd(23, 22, 17, 5, 18, 19);

// DHT11
#include "DHT.h"
#define DHTPIN 33
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// HIGRÓMETRO
#define higrometro 4

// SENSOR ULTRASONICO
#define echoPin 12
#define trigPin 14

// WIFI
#include <WiFi.h>
#include <HTTPClient.h>
const char* ssid = "Redmi Note 10S";
const char* password = "VeilHCF3202";

// BOMBA
#define relay 15

// Bytes
byte porcentChar[] = {
  B01100,
  B01101,
  B00010,
  B00100,
  B01000,
  B10110,
  B00110,
  B00000
};

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.createChar(0, porcentChar);

  dht.begin();

  pinMode(higrometro, INPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
}

void loop() {
  int r_historial = 0;
  digitalWrite(relay, HIGH);
  lcd.clear();
  digitalWrite(relay , LOW);
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  digitalWrite(relay, HIGH);
  float h_suelo = analogRead(higrometro);
  float hum_suelo = ((4095 - h_suelo) * 100) / 4095;

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long seg = pulseIn(echoPin, HIGH);
  long distancia = seg / 59;
  float n_agua = ((19 - distancia) * 100) / 16;

  Serial.println(String(t) + " " + String(h) + " " + String(hum_suelo) + " " + String(n_agua));

  lcd.setCursor(0, 0);
  lcd.print(t);
  lcd.print("C");

  lcd.setCursor(9, 0);
  lcd.print(h);
  lcd.write(byte(0));

  lcd.setCursor(0, 1);
  lcd.print(hum_suelo);
  lcd.write(byte(0));

  lcd.setCursor(9, 1);
  lcd.print(n_agua);
  lcd.write(byte(0));

  delay(2000);
  if (hum_suelo < 40) {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Regando");
    Serial.println("Regando");
    digitalWrite(relay, LOW);
    delay(2000);
    digitalWrite(relay, HIGH);
    delay(1000);
    lcd.clear();
    r_historial = 1;
  } else {
    digitalWrite(relay, HIGH);
    r_historial = 0;
  }

  lcd.setCursor(0, 0);
  lcd.print(t);
  lcd.print("C");

  lcd.setCursor(9, 0);
  lcd.print(h);
  lcd.write(byte(0));

  lcd.setCursor(0, 1);
  lcd.print(hum_suelo);
  lcd.write(byte(0));

  lcd.setCursor(9, 1);
  lcd.print(n_agua);
  lcd.write(byte(0));

    initWifi();
    Serial.println(WiFi.localIP());
  
    HTTPClient http;
    http.begin("http://192.168.198.18/neotech/valores");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Token a265b8045425c67eb7f585e2434080e65ecc582e");
    String text = "temp_ambiente=" + String(t) + "&humedad_ambiente=" + String(h) + "&humedad_suelo=" + String(hum_suelo) + "&nivel_agua=" + String(n_agua);
    
    int resp = http.POST(text);
    if (resp > 0) {
      Serial.println("Código HTTP ► " + String(resp));
      if (resp == 200) {
        String response = http.getString();
        Serial.println("El servidor respondió ▼ ");
        Serial.println(response);
      }
    } else {
      Serial.println("Error al enviar post, codigo:");
      Serial.println(resp);
    }
    http.end();
  
    if (r_historial == 1) {
      HTTPClient http;
      http.begin("http://192.168.198.18/neotech/riego");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      http.addHeader("Authorization", "Token a265b8045425c67eb7f585e2434080e65ecc582e");
      String text = "tipo=Automático";
  
      int resp = http.POST(text);
      if (resp > 0) {
        Serial.println("Código HTTP ► " + String(resp));
        if (resp == 200) {
          String response = http.getString();
          Serial.println("El servidor respondió ▼2 ");
          Serial.println(response);
        }
      } else {
        Serial.println("Error al enviar post, codigo2:");
        Serial.println(resp);
      }
      http.end();
    }
    WiFi.disconnect(true);
}

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network.");
}
