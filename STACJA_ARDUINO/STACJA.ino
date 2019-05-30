
//#define MY_DEBUG


// Enable and select radio type attached
#define MY_RADIO_RF24

#define MY_RF24_PA_LEVEL RF24_PA_LOW

#define MY_GATEWAY_SERIAL


#define MY_INCLUSION_MODE_FEATURE


// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 60


// Set blinking period
#define MY_DEFAULT_LED_BLINK_PERIOD 300


#include <MySensors.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RCSwitch.h>

#include <SPI.h>  //do obslugi DHT 
#include <DHT.h>  //do obslui DHT

//inicjalizacja ekranu
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// PIN do miernika temperatury
#define DHT_DATA_PIN 4

//pin nadajnika
#define NADAJNIK_PIN 7 
//offset dodawany do wyniku
#define SENSOR_TEMP_OFFSET -4
// Aktywacja nadajnika do RC
RCSwitch mySwitchTrans = RCSwitch();

//Ile ma spac pomiedzy aktualizacjami
static const uint64_t UPDATE_INTERVAL = 60000;

//wymus aktualizacje no 10 odczytow
static const uint8_t FORCE_UPDATE_N_READS = 10;

// ID poszczegolnych podmiernikow
#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1

float lastTemp;
float lastHum;
uint8_t nNoUpdatesTemp;
uint8_t nNoUpdatesHum;
bool metric = true;

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
DHT dht;

void setup()
{
    dht.setup(DHT_DATA_PIN); // set data pin of DHT sensor
  
    mySwitchTrans.enableTransmit(NADAJNIK_PIN); //ustawienie pinu TX
    mySwitchTrans.setPulseLength(170);
    mySwitchTrans.setRepeatTransmit(5);
    lcd.begin(16,2);   // Inicjalizacja LCD 2x16
    lcd.backlight(); // zalaczenie podwietlenia 
    lcd.setCursor(0,0); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna)
    lcd.print("Patryk Szelewski");
    lcd.setCursor(0,1); //Ustawienie kursora w pozycji 0,0 (drugi wiersz, pierwsza kolumna)
    lcd.print("JakubTomaszewski");
    delay(5000);
    lcd.clear();
    lcd.setCursor(0,0); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna)
    lcd.print("INTELIGENTNY DOM");
    lcd.setCursor(0,1); //Ustawienie kursora w pozycji 0,0 (drugi wiersz, pierwsza kolumna)
    lcd.print("PROJEKT 2k19");
    delay(5000);
    lcd.clear();
    lcd.print("POLACZ SIE Z:");
    lcd.setCursor(0,1); //Ustawienie kursora w pozycji 0,0 (drugi wiersz, pierwsza kolumna)
    lcd.print("192.168.0.171:8080  ");
    for(int j=0;j<3;j++)
    {
      for (int i = 0; i < 2; i++) 
      {
        delay(1000);
        lcd.scrollDisplayLeft();
      }

      for (int i = 0; i < 2; i++) 
      {
        delay(1000);
        lcd.scrollDisplayRight();
      }
    }
    delay(1000);
    
    
}

void presentation()
{ 
  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_TEMP, S_TEMP);

  metric = getControllerConfig().isMetric;

	present(1, S_BINARY); //przekaznik 1
  present(2, S_BINARY); //przekaznik 2
  present(3, S_BINARY); //przekaznik 3
  present(4, S_BINARY); //przekaznik 4
  present(5, S_BINARY); //wszystkie przekazniki
}

void loop()
{
	// Force reading sensor, so it works also after sleep()
  dht.readSensor(true);

  // Get temperature from DHT library
  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT!");
  } else if (temperature != lastTemp || nNoUpdatesTemp == FORCE_UPDATE_N_READS) {
    // Only send temperature if it changed since the last measurement or if we didn't send an update for n times
    lastTemp = temperature;
    
    // apply the offset before converting to something different than Celsius degrees
    temperature += SENSOR_TEMP_OFFSET;

    // Reset no updates counter
    nNoUpdatesTemp = 0;
    send(msgTemp.set(temperature, 1));

    //Wyswietl wszystko od nowa
    lcd.clear();
    lcd.print("Temperatura:");
    lcd.print(temperature, 0);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("Wilgotnosc:");
    lcd.print(lastHum, 0);
    lcd.print("%");

    
    #ifdef MY_DEBUG
    Serial.print("T: ");
    Serial.println(temperature);
    #endif
  } else {
    // Increase no update counter if the temperature stayed the same
    nNoUpdatesTemp++;
  }


  // Get humidity from DHT library
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum || nNoUpdatesHum == FORCE_UPDATE_N_READS) {
    // Only send humidity if it changed since the last measurement or if we didn't send an update for n times
    lastHum = humidity;
    // Reset no updates counter
    nNoUpdatesHum = 0;
    send(msgHum.set(humidity, 1));
    
    lcd.clear();
    lcd.print("Temperatura:");
    lcd.print(temperature, 0);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("Wilgotnosc:");
    lcd.print(humidity, 0);
    lcd.print("%");

    
    #ifdef MY_DEBUG
    Serial.print("H: ");
    Serial.println(humidity);
    #endif
  } else {
    // Increase no update counter if the humidity stayed the same
    nNoUpdatesHum++;
  }

  
  // Sleep for a while to save energy
  sleep(UPDATE_INTERVAL);
}

void receive(const MyMessage &message)
{
  if (message.type==V_STATUS)
  {
    switch (message.sensor)
    {
      case 1:
        mySwitchTrans.send(message.getBool()?5330227:5330236, 24);
        break;
      case 2:
        mySwitchTrans.send(message.getBool()?5330371:5330380, 24);
        break;
      case 3:
        mySwitchTrans.send(message.getBool()?5330691:5330700, 24);
        break;
      case 4:
        mySwitchTrans.send(message.getBool()?5332227:5332236, 24);
        break;
      case 5:
        mySwitchTrans.send(message.getBool()?5338371:5338380, 24);
        break;
    }
  }
}
