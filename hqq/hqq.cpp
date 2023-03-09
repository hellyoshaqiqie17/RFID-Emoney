#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>     //ArduinoJson digunakan untuk memparsing (membaca dan menguraikan) respons yang diterima dari server web yang dihubungi melalui koneksi HTTP setelah berhasil membaca data dari kartu RFID            
#include <ESP8266WiFi.h> 
#include <ESP8266HTTPClient.h>

#define SS_PIN 2  //D4              
#define RST_PIN 0 //D3 
//pin SS_PIN ditetapkan ke pin D4 dan pin RST_PIN ditetapkan ke pin D3.
 
MFRC522 mfrc522(SS_PIN, RST_PIN);

//menginisialisasi komunikasi SPI antara modul MFRC522 dan perangkat mikrokontroler dengan mengatur pin SS dan RST sesuai 

const char* ssid = "hellyoshaqiqie";
const char* password = "halosayangqu";

//menetapkan bahwa username dan password yang akan disambungkan ke microcontroller

String content; //untk menyimpan data UID tag yang diperoleh dari modul RFID MFRC522.
void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init(); //inisialisasi modul RFID MFRC522
  WiFi.begin(ssid, password); //inisiasi password dan username, untk menghubungkan modul ESP8266 ke jaringan WiFi dengan menggunakan SSID dan password yang telah ditentukan
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);                            // jika masih belum tersambung maka akan print
    Serial.println("Connecting..");         // ini
  }
  Serial.print("gunakan url ini untuk menyambuungkan: "); 
  Serial.print("http://");
  Serial.print(WiFi.localIP()); //alamat IP dari modul ESP8266 pada jaringan WiFi pada monitor serial
  Serial.println("");
}
 
void loop() {
  if (WiFi.status() == WL_CONNECTED) {      //jika koneksi wifi tersambung maka
    if ( ! mfrc522.PICC_IsNewCardPresent()) // dapat megecek apakah terdapat kartu yang di tap ke reader
                                            // jika ada maka :
    {
      return;
    }
    if ( ! mfrc522.PICC_ReadCardSerial())    // membaca kartu tersebut dan disimpan menggunakan variabel string content

    {
      return;
    }
    Serial.println();
    Serial.print(" UID tag :");    // print UID tag :
    content = "";                  // print UID tagnya
    byte letter;                   //menyimpan data byte yang dibaca rfid
 
    for (byte i = 0; i < mfrc522.uid.size; i++) //increment dari 0 hingga ukuran UID dikurangi satu (mfrc522.uid.size - 1). Pada setiap iterasi, nilai dari i akan digunakan untuk mengakses setiap byte dari UID melalui mfrc522.uid.uidByte[i].
    {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : " "); // menambahkan spasi atau nol di depan bilangan heksadesimal yang kurang dari 0x10 .


      Serial.print(mfrc522.uid.uidByte[i], HEX); //byte diubah ke hexadesimal contoh 0xA1, 0xB2, 0xC3, 0xD4 menjadi A1 B2 C3 D4

      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "")); /*digunakan untuk mengecek apakah nilai byte kurang dari 0x10 
                                                                         (yaitu nilainya hanya satu digit heksadesimal). Jika iya,
                                                                         maka akan menambahkan 0 di depannya agar menjadi dua digit heksadesimal*/ 
                                                                    
      content.concat(String(mfrc522.uid.uidByte[i], HEX)); //nilai heksadesimal dari byte ke-i dalam UID. Nilai tersebut kemudian diubah menjadi objek String
    }
    
    
    content.toUpperCase(); //untuk mengubah semua karakter pada string content menjadi huruf kapital

    Serial.println();

    parseJson(); //parsing/mengurai format data JSONN   
  } else {
    Serial.println("error koneksi wifi"); //jika tidakbisa maka dikarenakan koneksi wifi
  }
}
 
void parseJson() {
  HTTPClient http; // fungsi dari library untuk mengakses layanan web melalui protokol HTTP.
  http.begin("http://192.168.1.4/emoney/proses.php"); // menginisiasi HTTP dengan serverenginisiasi 

  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //mengirimkan data dalam format URL-encoded. 
                                                                      //Hal ini diperlukan agar server dapat memproses data yang kita kirimkan dengan benar.

  int httpResponseCode = http.POST("uid=" + content);//mengirimkan data UID yang didapat dari RFID tag ke server melalui HTTP POST request
                                                    // dengan cara memanggil http.POST DGN FORMAT UID  + CONTENT
 delay(100); 
  if (httpResponseCode > 0) { //digunakan untuk memeriksa apakah permintaan HTTP berhasil atau tidak. Jika kode respon HTTP yang diterima lebih besar dari 0,maka memiliki respon valid
    String response = http.getString(); /*digunakan untuk mengambil respons dari server HTTP setelah permintaan POST dilakukan menggunakan http.POST().
                                         Respons ini berupa string dan kemudian disimpan dalam variabel response*/


    //Serial.println(response);
    char json[500]; //adalah deklarasi variabel bertipe karakter yang digunakan untuk menyimpan JSON response yang diterima dari server dengan ukuran 500 char
    response.toCharArray(json, 500);
    StaticJsonDocument<200>doc; // menampung dokumen JSON dengan ukuran maksimum 200 bytes.
    deserializeJson(doc, json); //mengurai string JSON dan memasukkan nilainya ke dalam objek doc yang sudah dideklarasikan sebelumnya sebagai StaticJsonDocument
 
    const char* status = doc["Detail"]["Status"]; // mengambil value status dari objek JSON
    const char* nama = doc["Detail"]["Data User"]["nama"]; //mengambil value nama dari objek JSON
    int saldo_sebelum = doc["Detail"]["Data User"]["saldo"]; // mengambil value saldo dari objek JSON
    int nilai_transaksi = doc["Detail"]["Nilai Transaksi"]; // mengambil value Nilai Transaksi dari objek JSON
    int saldo_sekarang = doc["Detail"]["Saldo Akhir"]; // mengambil value Saldo Akhir dari objek JSON
 
    Serial.println("=============== HASIL PARSING RESPONSE ==========");
    Serial.print("Status Transaksi = "); 
    Serial.println(status);
    Serial.print("Nama Pengguna = ");
    Serial.println(nama);
    Serial.print("Saldo sebelum = ");                                   // MENAMPILKAN PARRSING RESPONSE
    Serial.println(saldo_sebelum);
    Serial.print("Nilai transaksi = ");
    Serial.println(nilai_transaksi);
    Serial.print("Saldo Sekarang = ");
    Serial.println(saldo_sekarang);
    Serial.println("==================================================");
      
  } else {
    Serial.print("Error mengirimkan POST: "); 
    Serial.println(httpResponseCode);
  }
  delay(2000);
  http.end();
}
