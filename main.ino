/**
 * Exemple de code Arduino pour un datalogger avec horodatage et stockage sur carte SD.
 */

/* Dépendances */
#include <Wire.h> // Pour la communication I2C
#include <SPI.h>  // Pour la communication SPI
#include <SD.h>   // Pour la communication avec la carte SD
#include "DS1307.h" // Pour le module DS1307
#include "cactus_io_AM2302.h"
#include <RTClib.h>



#define AM2302_PIN 2     // what pin on the arduino is the DHT22 data line connected to

// For details on how to hookup the DHT22 sensor to the Arduino then checkout this page
// http://cactus.io/hookups/sensors/temperature-humidity/am2302/hookup-arduino-to-am2302-temp-humidity-sensor

// Initialize DHT sensor for normal 16mhz Arduino. 
AM2302 dht(AM2302_PIN);
/* Rétro-compatibilité avec Arduino 1.x et antérieur */
#if ARDUINO >= 100
#define Wire_write(x) Wire.write(x)
#define Wire_read() Wire.read()
#else
#define Wire_write(x) Wire.send(x)
#define Wire_read() Wire.receive()
#endif

RTC_DS1307 rtc;

/** Broche CS de la carte SD */
const byte SDCARD_CS_PIN = 10; // A remplacer suivant votre shield SD

/** Nom du fichier de sortie */
const char* OUTPUT_FILENAME = "data.csv";

/** Delai entre deux prises de mesures */
const unsigned long DELAY_BETWEEN_MEASURES = 5000;


/** Fonction de conversion BCD -> decimal */
byte bcd_to_decimal(byte bcd) {
  return (bcd / 16 * 10) + (bcd % 16); 
}


/** Fichier de sortie avec les mesures */
File file;


/** Fonction setup() */
void setup() {
  Serial.println("AM2302 Humidity - Temperature Sensor");
  Serial.println("RH\t\tTemp (C)\tTemp (F)\tHeat Index (C)\t Heat Index (F)");
  dht.begin();
  /* Initialise le port I2C */
  Wire.begin();

  /* Initialisation du port série (debug) */
  Serial.begin(9600);

  // SETUP RTC MODULE
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1);
  }

  // automatically sets the RTC to the date & time on PC this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  

  /* Initialisation du port SPI */
  pinMode(10, OUTPUT); // Arduino UNO
  //pinMode(53, OUTPUT); // Arduino Mega

  /* Initialisation de la carte SD */
  Serial.println(F("Initialisation de la carte SD ... "));
  if (!SD.begin(SDCARD_CS_PIN)) {
    Serial.println(F("Erreur : Impossible d'initialiser la carte SD"));
    Serial.println(F("Verifiez la carte SD et appuyez sur le bouton RESET"));
    for (;;); // Attend appui sur bouton RESET
  }

  /* Ouvre le fichier de sortie en écriture */
  Serial.println(F("Ouverture du fichier de sortie ... "));
  file = SD.open(OUTPUT_FILENAME, FILE_WRITE);
  if (!file) {
    Serial.println(F("Erreur : Impossible d'ouvrir le fichier de sortie"));
    Serial.println(F("Verifiez la carte SD et appuyez sur le bouton RESET"));
    for (;;); // Attend appui sur bouton RESET
  }
  
  /* Ajoute l'entête CSV si le fichier est vide */
  if (file.size() == 0) {
    Serial.println(F("Ecriture de l'entete CSV ..."));
    file.println(F("Date Time Humidity Temperature_C Temperature_F"));
    file.flush();
  }
}

/** Fonction loop() */
void loop() {
//   // Temps de la précédente mesure et actuel
    measure();
  delay(1000);
}


/** Fonction de mesure - à personnaliser selon ses besoins */
void measure() {
  
  /* Lit la date et heure courante */
  dht.readHumidity();
  dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(dht.humidity) || isnan(dht.temperature_C)) {
    Serial.println("DHT sensor read failure!");
    return;
  }
 
  /* Réalise la mesure */
  float humidity = dht.humidity * 5.0 / 1023;
  float temperature_C = dht.temperature_C;
  float temperature_F = dht.temperature_F;
  
  /* Affiche les données sur le port série pour debug */ 
  DateTime now = rtc.now();
  Serial.print(now.day(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(" >> ");
  Serial.print(humidity);
  Serial.print(F("; "));
  Serial.print(temperature_C);
  Serial.print(F("; "));
  Serial.print(temperature_F);
  
  Serial.println(F("; "));

  
  /* Enregistre les données sur la carte SD */
  // Horodatage
  // Données brutes
  file.print(now.day(), DEC);
  file.print('/');
  file.print(now.month(), DEC);
  file.print('/');
  file.print(now.year(), DEC);
  file.print(" ");
  file.print(now.hour(), DEC);
  file.print(':');
  file.print(now.minute(), DEC);
  file.print(F(" "));
  file.print(humidity);
  file.print(F(" "));
  file.print(temperature_C);
  file.print(F(" "));
  file.println(temperature_F);
  file.flush();
}
