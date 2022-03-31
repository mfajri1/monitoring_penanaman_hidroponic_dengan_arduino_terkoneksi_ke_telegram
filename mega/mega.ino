

/***************************************************
 DFRobot Gravity: Analog TDS Sensor/Meter
 <https://www.dfrobot.com/wiki/index.php/Gravity:_Analog_TDS_Sensor_/_Meter_For_Arduino_SKU:_SEN0244>

 ***************************************************
 This sample code shows how to read the tds value and calibrate it with the standard buffer solution.
 707ppm(1413us/cm)@25^c standard buffer solution is recommended.

 Created 2018-1-3
 By Jason <jason.ling@dfrobot.com@dfrobot.com>

 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution.
 ****************************************************/

 /***********Notice and Trouble shooting***************
 1. This code is tested on Arduino Uno with Arduino IDE 1.0.5 r2 and 1.8.2.
 2. Calibration CMD:
     enter -> enter the calibration mode
     cal:tds value -> calibrate with the known tds value(25^c). e.g.cal:707
     exit -> save the parameters and exit the calibration mode
 ****************************************************/

#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <GravityTDS.h>
#include <EEPROM.h>
#include <OneWire.h> //Memanggil library OneWire yang diperlukan sebagai dependensi library Dallas Temperature
#include <DallasTemperature.h> // Memanggil library Dallas Temperature
#include <NewPing.h>
#include <RTClib.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>

#define TRIGGER_PIN 7 
#define ECHO_PIN 6
#define MAX_DISTANCE 200
#define ONE_WIRE_BUS 2  // Menempatkan PIN hasil pembacaan sensor DS18B20 pada PIN 2. 
//Disebut One Wire karena kita bisa menempatkan sensor DS18B20 lain pada PIN yang sama  
#define TdsSensorPin A1
#define pumpVitamin 29
#define pumpInti 27
#define pumpIsi 25
#define lampu 23

RTC_DS3231 rtc;
SoftwareSerial mySerial(13, 12);
SoftwareSerial node(10,11);
LiquidCrystal_I2C lcd(0x3f, 20, 4); // address lcd mungkin berbeda silahkan baca dulu addressnya lihat di www.anakkendali.com untuk program membaca alamat i2c
GravityTDS gravityTds;
OneWire oneWire(ONE_WIRE_BUS); //Membuat variabel oneWire berdasarkan PIN yang telah didefinisikan
DallasTemperature sensor(&oneWire); //Membuat variabel untuk menyimpan hasil pengukuran
NewPing us(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
float suhuDS18B20; //deklarasi variable suhu DS18B20 dengan jenis data float
float temperature = 25;
float tdsValue = 0;
const int ph_Pin  = A0;
float Po = 0;
float PH_step, dataPH, dataTDS, dataSuhu;
int nilai_analog_PH;
double TeganganPh;

//untuk kalibrasi
float ph4 = 3.226;
float ph7 = 2.691;
String kirim = "";
String data;
String kirimData = "";
int dataUs = 0;
int jarakAwal = 22;
int totalJarak = 0;
void setup(){
  mySerial.begin (9600);
  node.begin(9600);
  Serial.begin(9600);
  mp3_set_serial (mySerial);
  delay(10);
  mp3_set_volume (35);
  delay(10);
  pinMode(TRIGGER_PIN, OUTPUT); //Set pin Trigger sebagai output
  pinMode(ECHO_PIN, INPUT); //Set pin Echo sebagai input
  pinMode (ph_Pin, INPUT);
  pinMode(pumpVitamin, OUTPUT);
  pinMode(pumpInti, OUTPUT);
  pinMode(pumpIsi, OUTPUT);
  pinMode(lampu, OUTPUT);
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //initialization
  sensor.begin();      //Menginisiasikan sensor One-Wire DS18B20
  lcd.begin();
  lcd.backlight();
  mp3_play (01);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Selamat Datang");
  lcd.setCursor(0,1);
  lcd.print("Liana Putri");
  lcd.setCursor(0,2);
  lcd.print("18101152620000");
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  delay(2000);
}

void loop(){
  DateTime now = rtc.now();
  dataPH = bacaPH();
//  if(dataPH >= 7.0){
//    kirimData = "PH Air Berlebih";
//    node.println(kirimData);
//    kirim = "";
//  }
  dataTDS = bacaTDS();
  if(dataTDS <= 100.0){
    digitalWrite(pumpVitamin, HIGH);
    kirimData = "Vitamin Kurang";
    node.println(kirimData);
//    kirim = "";
  }else{
    digitalWrite(pumpVitamin, LOW);
  }
  dataSuhu = bacaSuhu();
//  if(dataSuhu >=30){
//    kirimData = "Suhu Air Terlalu Panas";
//    node.println(kirimData);
//    kirim = "";
//  }
  
  dataUs = bacaUltra();
  totalJarak = jarakAwal - dataUs;
  if(totalJarak < 5){
    digitalWrite(pumpIsi, HIGH);
  }else{
    digitalWrite(pumpIsi, LOW);
  }
  tampilLcd(String(dataPH), String(dataTDS), String(dataSuhu), String(totalJarak));

//  Serial.println(kirim);
//  Serial.print(dataPH);
//  Serial.print(" , ");
//  Serial.print(dataTDS);
//  Serial.print(" , ");
//  Serial.print(dataSuhu);
//  Serial.println();
  while(node.available()>0){
    char c = char(node.read());
    data += c;
  }
  data.trim();
  if(data.length() > 0){
    if(data == "motor1on"){
      mp3_play (04);
      digitalWrite(pumpInti, HIGH);
      Serial.println("motor1Hidup");
    }else if(data == "motor1off"){
      digitalWrite(pumpInti, LOW);
      Serial.println("motor1Mati");
    }else if(data == "motor2on"){
      mp3_play (02);
      digitalWrite(pumpVitamin, HIGH);
      Serial.println("motor2Hidup");
    }else if(data == "motor2off"){
      digitalWrite(pumpVitamin, LOW);
      Serial.println("motor2Mati");
    }else if(data == "motor3on"){
      mp3_play (03);
      digitalWrite(pumpIsi, HIGH);
      Serial.println("motor3Hidup");
    }else if(data == "motor3off"){
      digitalWrite(pumpIsi, LOW);
      Serial.println("motor3Mati");
    }else if(data == "lampuon"){
      mp3_play (05);
      digitalWrite(lampu, HIGH);
      Serial.println("lampuHidup");
    }else if(data == "lampuoff"){
      digitalWrite(lampu, LOW);
      Serial.println("lampuMati");
    }else if(data == "monitor"){
      Serial.println("monitor");
      kirim = "PH = " + String(dataPH) + ", Vitamin = " + String(dataTDS) + ", Suhu = " + String(dataSuhu) + ", Tinggi = " + String(totalJarak) ; 
      node.println(kirim);
      kirim = "";
    }
  }
  delay(1000);
  data = "";
}

int bacaUltra(){
  while(1){
    int US1 = us.ping_cm();
    return US1;
  }
}

float bacaTDS(){
  while(1){
    //temperature = readTemperature();  //add your temperature sensor and read it
    gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
    gravityTds.update();  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value
    return tdsValue;
  }
  
}

float bacaSuhu(){
  while(1){
    //sensor.setResolution(9);  // Sebelum melakukan pengukuran, atur resolusinya
    //sensor.setResolution(10);
    //sensor.setResolution(11);
    sensor.setResolution(12);
    sensor.requestTemperatures();  // Perintah konversi suhu
    suhuDS18B20 = sensor.getTempCByIndex(0);  //Membaca data suhu dari sensor #0 dan mengkonversikannya ke nilai Celsius
    // suhuDS18B20 = (suhuDS18B20*9/5) + 32;
    // suhuDS18B20 = suhuDS18B20 = 273.15;
    //Serial.println(suhuDS18B20, 1);  //Presisi 1 digit
    //Serial.println(suhuDS18B20, 2);  //Presisi 2 digit
    //Serial.println(suhuDS18B20, 3);  //Presisi 3 digit
    //Serial.println(suhuDS18B20, 4);  //Presisi 4 digit
    
    return suhuDS18B20;
  }
}

float bacaPH(){
  while(1){
    nilai_analog_PH = analogRead(ph_Pin);
    TeganganPh = 5 / 1024.0 * nilai_analog_PH;
    PH_step = (ph4 - ph7) / 3;
    Po = 7.00 + ((ph7 - TeganganPh) / PH_step);     //Po = 7.00 + ((teganganPh7 - TeganganPh) / PhStep);
    return Po;
  }
  /*
 * PH step = (tegangan PH4 – tegangan PH7) / (PH7 - PH4)
             = (3.1 – 2.5)/(7-4)
             = 0.6/3
             = 0.2
 */
}

void tampilLcd(String dat1, String dat2, String dat3, String dat4){
  DateTime now = rtc.now();
  String tgll = String(now.day())+'-'+String(now.month())+'-'+String(now.year());
  String jamm = String(now.hour())+':'+String(now.minute())+':'+String(now.second());
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(tgll);
  lcd.setCursor(11,0);
  lcd.print(jamm);
  lcd.setCursor(0,2);
  lcd.print("PH=");
  lcd.print(dat1);
  lcd.setCursor(9,2);
  lcd.print("TDS=");
  lcd.print(dat2);
  lcd.setCursor(0,3);
  lcd.print("S =");
  lcd.print(dat3);
  lcd.setCursor(9,3);
  lcd.print("T  = ");
  lcd.print(dat4);
  
}
