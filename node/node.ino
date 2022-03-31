#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

SoftwareSerial mega(4,5);

// Wifi network station credentials
#define WIFI_SSID "Ray"
#define WIFI_PASSWORD "12345678"
// Telegram BOT Token (Get from Botfather)
#define CHAT_ID "1379411172"
#define BOT_TOKEN "5191068410:AAHe_Uj236bwSj9cY7q80UTlQIJdPm1FnsU"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done
String perintah = "";
String data;
char c;
String dataMega = "";
String arrMega[4];
int indexMega = 0;
bool dataTampil = false;
int led1 = 12;
int led2 = 14;

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  Serial.begin(9600);
  mega.begin(9600);
  Serial.println();

 // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
  }
  
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
  
  bot.sendMessage(CHAT_ID, "Bot started up", "");
}
void handleNewMessages(int numNewMessages)
{
  
  
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    
    if (from_name == "")
      from_name = "Guest";

    if (text == "/motor1on"){
      perintah = "motor1on";
      mega.println(perintah);
      bot.sendMessage(chat_id, "Motor 1 ON", "");
    }else if (text == "/motor1off"){
      perintah = "motor1off";
      mega.println(perintah);
      bot.sendMessage(chat_id, "Motor 1 OFF", "");
    }else if (text == "/motor2on"){
      perintah = "motor2on";
      mega.println(perintah);
      bot.sendMessage(chat_id, "Motor 2 ON", "");
    }else if (text == "/motor2off"){
      perintah = "motor2off";
      mega.println(perintah);
      bot.sendMessage(chat_id, "Motor 2 OFF", "");
    }else if (text == "/motor3on"){
      perintah = "motor3on";
      mega.println(perintah);
      bot.sendMessage(chat_id, "Motor 3 ON", "");
    }else if (text == "/motor3off"){
      perintah = "motor3off";
      mega.println(perintah);
      bot.sendMessage(chat_id, "Motor 3 OFF", "");
    }else if (text == "/lampuon"){
      perintah = "lampuon";
      mega.println(perintah);
      bot.sendMessage(chat_id, "Lampu ON", "");
    }else if (text == "/lampuoff"){
      perintah = "lampuoff";
      mega.println(perintah);
      bot.sendMessage(chat_id, "Lampu OFF", "");
    }else if (text == "/monitor"){
      perintah = "monitor";
      mega.println(perintah);
      dataTampil = true;    
    }else if (text == "/start"){
      String welcome = "Selamat Datang.\n";
      welcome += "Ini Adalah Sistem Kami.\n\n";
      welcome += "/motor1on : hidup Motor 1\n";
      welcome += "/motor1off : mati Motor 1\n";
      welcome += "/motor2on : hidup Motor 2\n";
      welcome += "/motor2off : mati Motor 2\n";
      welcome += "/motor3on : hidup Motor 3\n";
      welcome += "/motor3off : mati Motor 3\n";
      welcome += "/lampuon : hidup Lampu\n";
      welcome += "/lampuoff : mati Lampu\n";
      welcome += "/monitor : Ambil Data\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
  
  
}
void loop() {
  digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
  
  while(mega.available()>0){
    c = char(mega.read());
    data += c;
  }
  data.trim();

  if(data != ""){
    bot.sendMessage(CHAT_ID, String(data), "Markdown");
  }

  if(dataTampil == true){
    bot.sendMessage(CHAT_ID, String(data), "Markdown");
  }
 
  
//  delay(500);
  data = "";
  dataTampil = false;

}
