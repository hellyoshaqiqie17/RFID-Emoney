#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#define SS_PIN 2  //D4
#define RST_PIN 0 //D3
 
MFRC522 mfrc522(SS_PIN, RST_PIN);
const char* ssid = "Hellyoshaqiqieeee";
const char* password = "**********";
String content;
void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting..");
  }
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("");
}
 
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
      return;
    }
    if ( ! mfrc522.PICC_ReadCardSerial())
    {
      return;
    }
    Serial.println();
    Serial.print(" UID tag :");
    content = "";
    byte letter;
 
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    Serial.println();
    parseJson();
  } else {
    Serial.println("Error in WiFi connection");
  }
}
 
void parseJson() {
  HTTPClient http;
  http.begin("http://192.168.1.4/emoney/proses.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST("uid=" + content);
  delay(100);
  if (httpResponseCode > 0) {
    String response = http.getString();
    //Serial.println(response);
    char json[500];
    response.toCharArray(json, 500);
    StaticJsonDocument<200>doc;
    deserializeJson(doc, json);
 
    const char* status = doc["Detail"]["Status"];
    const char* nama = doc["Detail"]["Data User"]["nama"];
    int saldo_sebelum = doc["Detail"]["Data User"]["saldo"];
    int nilai_transaksi = doc["Detail"]["Nilai Transaksi"];
    int saldo_sekarang = doc["Detail"]["Saldo Akhir"];
 
    Serial.println("=============== HASIL PARSING RESPONSE ==========");
    Serial.print("Status Transaksi = ");
    Serial.println(status);
    Serial.print("Nama Pengguna = ");
    Serial.println(nama);
    Serial.print("Saldo sebelum = ");
    Serial.println(saldo_sebelum);
    Serial.print("Nilai transaksi = ");
    Serial.println(nilai_transaksi);
    Serial.print("Saldo Sekarang = ");
    Serial.println(saldo_sekarang);
    Serial.println("==================================================");
      
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  delay(2000);
  http.end();
}