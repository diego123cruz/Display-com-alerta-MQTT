// Display com alerta
// Diego Cruz 02/2026
// Esp8266 + lcd 16x2 com i2c + buzzer + botão led (diy).
// Arduino IDE 2 - NodeMCU 1.0 (ESP12E)

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

#define ledS 13
#define ledN 15
#define btnS 12
#define btnN 14
#define buzz 0

#define timeOutAlert 2

// LCD
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

// Wi-Fi
const char* ssid = "seu-wifi";
const char* password = "sua-senha";

// MQTT
const char* mqtt_server = "192.168.0.2"; // IP do Home Assistant
const char* mqtt_user   = "user";
const char* mqtt_pass   = "senha";

// Tópicos
const char* topic_set   = "casa/escritorio/lcd/texto/set";
const char* topic_state = "casa/escritorio/lcd/texto/state";
const char* topic_time = "casa/escritorio/lcd/texto/time";
const char* topic_time_config  = "homeassistant/text/escritorio/lcd_time/config";
const char* topic_disc  = "homeassistant/text/escritorio/lcd_msg/config";
const char* resp_topic_disc = "homeassistant/sensor/escritorio/lcd_resposta/config";

WiFiClient espClient;
PubSubClient client(espClient);

// Alert
unsigned long previousMillis = 0;
const long interval = 5000;
byte alertTimeOut = timeOutAlert;

void publishDiscovery() {
  // Pergunta

  delay(1000);
  const char* payload = "{\"name\":\"Mensagem:\",\"command_topic\":\"casa/escritorio/lcd/texto/set\",\"state_topic\":\"casa/escritorio/lcd/texto/state\",\"max\":32,\"unique_id\":\"esp8266_lcd_text\",\"device\":{\"identifiers\":[\"esp8266_lcd\"],\"name\":\"ESP8266 LCD\"}}";

  Serial.println(topic_disc);
  Serial.println(payload);
  client.publish(topic_disc, payload, true);


  delay(1000);
  // Resposta
  const char* resp_payload = "{\"name\":\"Resposta:\",\"unique_id\":\"lcd_resposta\",\"state_topic\":\"casa/escritorio/lcd/resposta/state\",\"device\":{\"identifiers\":[\"esp8266_lcd\"],\"name\":\"ESP8266 LCD\"}}";

  Serial.println(resp_topic_disc);
  Serial.println(resp_payload);
  client.publish(resp_topic_disc, resp_payload, true);

  delay(1000);

  const char* time_payload = "{\"name\":\"Time:\",\"command_topic\":\"casa/escritorio/lcd/texto/time\",\"state_topic\":\"casa/escritorio/lcd/texto/time\",\"max\":32,\"unique_id\":\"esp8266_lcd_time\",\"device\":{\"identifiers\":[\"esp8266_lcd\"],\"name\":\"ESP8266 LCD\"}}";
  Serial.println(topic_time_config);
  Serial.println(time_payload);
  client.publish(topic_time_config, time_payload, true);
}

void showTime(String text) {
  lcd.clear();
  lcd.noBacklight();

  // Linha 1
  lcd.setCursor(0, 0);
  lcd.print(text.substring(0, 16));

  // Linha 2
  if (text.length() > 16) {
    lcd.setCursor(0, 1);
    lcd.print(text.substring(16, 32));
  }
}

void showOnLCD(String text) {
  lcd.backlight();
  client.publish(
      "casa/escritorio/lcd/resposta/state",
      "AGUARDANDO",
      true
    );
  lcd.clear();

  // Linha 1
  lcd.setCursor(0, 0);
  lcd.print(text.substring(0, 16));

  // Linha 2
  if (text.length() > 16) {
    lcd.setCursor(0, 1);
    lcd.print(text.substring(16, 32));
  }

  digitalWrite(ledS, LOW);
  digitalWrite(ledN, LOW);

  char last = ' ';
  char alert = ' ';
  for(int l = text.length() -1; l > 0; l--) {
    if(text[l] != ' ') {
      last = text[l];
      alert = text[l-1];
      break;
    }
  }

  if (last == '?') {
    digitalWrite(ledS, HIGH);
    digitalWrite(ledN, HIGH);
    if(alert == '?') {
      loopYN(true);
    } else {
      loopYN(false);
    }
  } else if (last == '!') {
    digitalWrite(ledS, HIGH);
    if(alert == '!') {
      loopOK(true);
    } else {
      loopOK(false);
    }
  } 
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String text = String((char*)payload);

  if (String(topic) == topic_set) {
    showOnLCD(text);
    client.publish(topic_state, text.c_str(), true);
  }

  if (String(topic) == topic_time) {
    showTime(text);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("esp8266_lcd", mqtt_user, mqtt_pass)) {
      client.subscribe(topic_set);
      delay(1000);
      client.subscribe(topic_time);
      delay(1000);
      lcd.clear();
      lcd.print("reconnect...");
      enviarResposta("RST");
      publishDiscovery();
    } else {
      delay(3000);
    }
  }
}

void enviarResposta(const char* resp) {
  delay(100);
  client.publish(
    "casa/escritorio/lcd/resposta/state",
    resp,
    true
  );

  lcd.clear();
  lcd.print("Resposta: ");
  lcd.setCursor(0, 1);
  lcd.print(resp);
  delay(3000);
  lcd.noBacklight();
}

void loopYN(bool alert) {
  alertTimeOut = timeOutAlert;
  while(true) {
    yield();
    
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      if (alert) {
        callAlert();
      }
      if(alertTimeOut > 0) {
        alertTimeOut--;
      } else {
        digitalWrite(ledS, LOW);
        digitalWrite(ledN, LOW);
        enviarResposta("AUSENTE");
        break;
      }
    }

    if (digitalRead(btnN) == LOW) {
      digitalWrite(ledS, LOW);
      digitalWrite(ledN, LOW);
      enviarResposta("NAO");
      break;
    }

    if (digitalRead(btnS) == LOW) {
      digitalWrite(ledS, LOW);
      digitalWrite(ledN, LOW);
      enviarResposta("SIM");
      break;
    }
  }
}

void loopOK(bool alert) {
  alertTimeOut = timeOutAlert;
  while(true) {
    yield();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      if (alert) {
        callAlert();
      }
      if(alertTimeOut > 0) {
        alertTimeOut--;
      } else {
        digitalWrite(ledS, LOW);
        digitalWrite(ledN, LOW);
        enviarResposta("AUSENTE");
        break;
      }
    }

    if (digitalRead(btnS) == LOW) {
      digitalWrite(ledS, LOW);
      digitalWrite(ledN, LOW);
     
      enviarResposta("OK");
    }
  }
}

void callAlert() {
  for(byte x=0; x<10; x++) {  
    lcd.backlight();
    digitalWrite(buzz, HIGH);
    delay(100);
    lcd.noBacklight();
    digitalWrite(buzz, LOW);
    delay(100);
  }
  lcd.backlight();
}

void setup() {
  Serial.begin(9600);

  pinMode(ledS, OUTPUT);
  pinMode(btnS, INPUT_PULLUP);

  pinMode(ledN, OUTPUT);
  pinMode(btnN, INPUT_PULLUP);

  pinMode(buzz, OUTPUT);
  digitalWrite(buzz, LOW);

  Wire.begin(D2, D1); // SDA, SCL

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Conectando...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.clear();
  lcd.print("WiFi OK");

  client.setSocketTimeout(45);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  delay(1000);
  publishDiscovery();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  yield();
  delay(10);
}
