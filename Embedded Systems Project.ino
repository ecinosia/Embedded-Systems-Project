#include <EEPROM.h> //EEPROM kütüphanesi
#include <dht11.h> //DHT11 nem ve sıcaklık sensörü kütüphanesi
#include <TimerOne.h> //Timer kütüphanesi
#include <LiquidCrystal_I2C.h> //I2C LCD ekran kütüphanesi


#define DHT11PIN 7 //DHT11 analog pin tanımlaması

LiquidCrystal_I2C lcd(0x27, 16, 2); //I2C LCD ekran için gerekli adres ve piksel tanımlaması

dht11 DHT11; //DHT11 objesi oluşturma

const int time = 1000000; //Timer için zaman belirleme
bool isStarted = false; //Timer için boolean değişken oluşturma
// Değerleri saklamak için gerekli EEPROM adresleri
int addressHumidity = 0;
int addressTemperature = 2;
int addressMQ135 = 4;
//Timer interrupt fonksiyonu
void timeStarter() {
  isStarted = true;
}

void setup() {
  Serial.begin(9600);
  lcd.init(); //LCD ekran başlatıldı
  lcd.backlight(); //LCD arka ışığı yakıldı

  Timer1.initialize(time); //Timer başlatıldı.
  Timer1.attachInterrupt(timeStarter); //Timer interrupt fonksiyonu çağırıldı.
}

void loop() {
    int mq135; //MQ135 sensöründen okunan analog değeri tutan değişken
    int threshold; //MQ135 sensöründen okunan digital değeri tutan değişken
  
  //Timer başlatıldı ise değerler okunuyor ve gerekli şartlar sağlanması durumunda EEPROM'a kaydetme işlemleri yapılıyor.
  if (isStarted == true) {
    isStarted = false;
    DHT11.read(DHT11PIN);
    mq135 = analogRead(0);
    threshold = digitalRead(0);
    
    // EEPROM'da sensör verileri saklanıyor.
    if (DHT11.humidity > 0) {
      EEPROM.update(addressHumidity, DHT11.humidity);
      EEPROM.update(addressTemperature, DHT11.temperature);
    }
    if (mq135 != 0) {
      EEPROM.update(addressMQ135, mq135);
    }
  }

  float storedHumidity, storedTemperature;
  int storedMQ135;

  // EEPROM'da saklanan değerler değişkenlere atandı
  storedHumidity = EEPROM.read(addressHumidity);
  storedTemperature = EEPROM.read(addressTemperature);
  storedMQ135 = EEPROM.read(addressMQ135);

  //LCD ekrana sıcaklık ve nem değerleri yazdırıldı.
  //Nemin sıfır olması sensörün çalışmadığı durumu temsil eder.
  //Sıfır ise EEPROM'da tutulan değerler ekrana yazdırılır.
  //Değil ise güncel değerler yazdırılır
  if (DHT11.humidity == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DHT11 off,saved:");
    lcd.setCursor(0, 1);
    lcd.print("Hum: %");
    lcd.setCursor(6, 1);
    lcd.print((int)storedHumidity);
    lcd.setCursor(9, 1);
    lcd.print("Tem: ");
    lcd.setCursor(13, 1);
    lcd.print((int)storedTemperature);
    lcd.setCursor(15, 1);
    lcd.print("C");
    delay(5000);
  } else{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temperature: ");
    lcd.setCursor(13,0);
    lcd.print(DHT11.temperature);
    lcd.setCursor(15,0);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("Humidity: %");
    lcd.setCursor(11,1);
    lcd.print(DHT11.humidity);
    delay(5000);
  }

  //Hava Kalitesi için metin değişkeni
  String airQuality;

  //Hava kalitesi sensörden gelen ppm değerine göre aralıklara bölünüp inceleniyor.
  if(mq135 >= 10 && mq135 <= 250){
    airQuality = "Good";
  } else if(mq135 > 250 && mq135 <= 500){
    airQuality = "Normal";
  } else if(mq135 > 500 && mq135 <= 750){
    airQuality = "Medium";
  } else if(mq135 > 750 && mq135 <= 1000){
    airQuality = "Bad!";
  } else if(mq135 < 1000){
    airQuality = "Very Bad";
  }

  // MQ135 sensöründen gelen değerler LCD ekrana yazdırılıyor.
  //Threshold değişkeninin 1 ve 0 haricinde olduğu durumlar sensörün çalışmadığı durumlardır.
  //Bu durumda EEPROM'da saklanan değer ekrana gösterilir.
  //1 veya 0 durumunda güncel değerler verilir.
  //1 => Havada sağlığa zararlı vb. gaz tespit edilmediğinde döner.
  //0 => Havada sağlığa zararlı vb. gaz tespit edildiğinde döner.
  if(threshold != 1 && threshold != 0){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MQ135 off,saved:");
    lcd.setCursor(0,1);
    lcd.print("Air Qua: ");
    lcd.setCursor(9,1);
    lcd.print(storedMQ135);
    delay(5000);
  } else{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Air Qua: ");
    lcd.setCursor(9,0);
    lcd.print(airQuality);
    Serial.println(airQuality);
    Serial.println(mq135);
    //Threshold değeri 0 iken yani gaz tespit edildiğinde bunu LCD ekranda gösterir.
    if(threshold == 0){
      lcd.setCursor(0, 1);
      lcd.print("Gas detected!");
    }
    delay(5000);
  }
}
