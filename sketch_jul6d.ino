#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define WIFI_SSID "Amelia" // Nama wifi
#define WIFI_PASSWORD "KosKos2dan4" // Password wifi
#define BOT_TOKEN "6030855794:AAEHcGoBBGf0cRzJZAttH7-cxHpSYzWaOvY" // Token BOT Telegram
String idtelegram = "1386652179"; // ID Telegram penerima
#define LOW_ALARM 25 // Batas suhu terendah
#define HIGH_ALARM 32 // Batas suhu tertinggi
#define BUZZER_PIN D3 // Pin buzzer di D3 atau GPIO0

OneWire oneWire(2); // Pin sensor suhu di D4 atau GPIO2
DallasTemperature sensors(&oneWire);
float currentTemperature = 0;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;

void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (text == "/suhu") // Jika pesan "berikan suhu saat ini" diterima
    {
      String message = "Lapor suhu saat ini: " + String(currentTemperature, 2) + " Celcius";
      bot.sendMessage(chat_id, message);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  sensors.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setInsecure();

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  bot_lasttime = millis();
}

void loop()
{
  if (millis() - bot_lasttime > 1000)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

  sensors.requestTemperatures();
  currentTemperature = sensors.getTempCByIndex(0);

  if (currentTemperature < LOW_ALARM || currentTemperature > HIGH_ALARM)
  {
    // Mengirim pesan peringatan ke Telegram
    String message = "Lapor Suhu saat ini sangat ";
    if (currentTemperature < LOW_ALARM)
      message += "dingin";
    else
      message += "panas";
    message += ". Suhu: " + String(currentTemperature, 2) + " Celcius";
    bot.sendMessage(idtelegram, message);

    // Mengaktifkan buzzer
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else
  {
    // Menonaktifkan buzzer
    digitalWrite(BUZZER_PIN, LOW);
  }

  delay(1000); // Delay 1 detik
}
