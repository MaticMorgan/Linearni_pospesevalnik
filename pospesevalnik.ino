#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int zagon = 2;

const int prozilec1 = 3;
const int prozilec2 = 4;

const int pin_volt1 = A2;
const int pin_volt2 = A3;

float analog_volt;
float volt_pin;
float volt;
float sum;
float volt_avg;

const int senzor1 = 5;
const int senzor2 = 6;
const int senzor3 = 7;

float cas0;
float cas1;
float cas2;
float cas3;
float dt1;
float dt2;
float dt3;

float timer_prozilec;

float razdalja1;
float razdalja2;
float razdalja3;

float avgv1;
float v1;
float a1;
float avgv2;
float v2;
float a2;
float avgv3;
float v3;
float a3;

int cas_prebran1;  //  Boolean, če čas ob senzorju je ali ni prebran - sprva ni prebran.
int cas_prebran2;
int cas_prebran3;

float zdej;

LiquidCrystal_I2C lcd_volt(0x3F, 16, 2);
LiquidCrystal_I2C lcd_hitrost(0x3E, 16, 2);
LiquidCrystal_I2C lcd_acc(0x3D, 16, 2);

void Voltmeter(int pin_volt) { // To je funkcija, ki 10x izmeri napetost z delilnikom napetosti, ter jo preračuna v pravo vrednost.
                               // Nato se odloči, glede na postavljeno mejo, če je kondenzator poln ali prazen, ter to izpiše na LCD.
    for (int i = 0; i < 10; i++) {
      analog_volt = analogRead(pin_volt);
      volt_pin = 5.00 * (analog_volt / 1024.00);
      volt = volt_pin * 1.10 * 10.00;
      sum += volt; 
    }

    volt_avg = sum / 10;
    sum = 0;

    if (pin_volt == pin_volt1 && volt_avg > 32) {    
      lcd_volt.setCursor(0,0);
      lcd_volt.print("Poln...");
    }
    else if (pin_volt == pin_volt1 && volt_avg < 3) {
      lcd_volt.setCursor(0,0);
      lcd_volt.print("Prazen...");
    }
    else if (pin_volt == pin_volt2 && volt_avg > 32) {
      lcd_volt.setCursor(0,1);
      lcd_volt.print("Poln...");
    }
    else if (pin_volt == pin_volt2 && volt_avg < 3) {
      lcd_volt.setCursor(0,1);
      lcd_volt.print("Prazen...");
    }
}

void setup() {

  pinMode(zagon, INPUT);

  pinMode(prozilec1, OUTPUT);
  pinMode(prozilec2, OUTPUT);

  pinMode(senzor1, INPUT);
  pinMode(senzor2, INPUT);
  pinMode(senzor3, INPUT);

  pinMode(pin_volt1, INPUT);
  pinMode(pin_volt2, INPUT);

  razdalja1 = 0.05; //razdalje v metrih
  razdalja2 = 0.11;
  razdalja3 = 0.185;

  timer_prozilec = 50000;  // čas proženja v mikrosekundah

  lcd_volt.init();
  lcd_volt.backlight();
  lcd_volt.clear();
  lcd_volt.setCursor(0,0);
  lcd_volt.print("Napetosti na");
  lcd_volt.setCursor(0,1);
  lcd_volt.print("kondezatorjih...");

  lcd_hitrost.init();
  lcd_hitrost.backlight();
  lcd_hitrost.clear();
  lcd_hitrost.setCursor(0,0);
  lcd_hitrost.print("Hitrosti...");

  lcd_acc.init();
  lcd_acc.backlight();
  lcd_acc.clear();
  lcd_acc.setCursor(0,0);
  lcd_acc.print("Pospeski...");

  delay(5000); //  počakamo, da se kondenzatorji napolnejo
  lcd_volt.clear();
  Voltmeter(pin_volt1);
  Voltmeter(pin_volt2);
}

void loop() {

   if ( digitalRead(zagon) == HIGH ) {

    delay(50);  //  Varovalo za debounce.

    if ( digitalRead(zagon) == HIGH ) {

      cas_prebran1 = false;
      cas_prebran2 = false;
      cas_prebran3 = false;
      cas0 = micros();  //  Zapomni si čas ob začetku proženja.
      digitalWrite(prozilec1, HIGH);  // Prižgi prvo tuljavo.
      delayMicroseconds(10);  //  funkcija micros() je na 4 mikrosekunde natančna
      zdej = micros(); // Zapomni si aktualen čas  
      while( (zdej - cas0) < timer_prozilec) { //  Dokler je čas, pretečen od začetka proženja manjši od timerja za prožilec naredi:
        if (  digitalRead(senzor1) == LOW && cas_prebran1 == false  ) {  //  Če čas še ni prebran in je ravno izstrelek na senzorju, potem:
          cas1 = micros();  //  Zapomni si čas, ko je izstrelek pri senzorju
          cas_prebran1 = true; //  Ker je čas bil ravnokar izmerjen spremeni vrednost
        }
        if (  digitalRead(senzor2) == LOW && cas_prebran2 == false ) {
          cas2 = micros();
          cas_prebran2 = true;
          digitalWrite(prozilec2, HIGH); //  Prižgi 2. tuljavo.
          while (digitalRead(senzor2) == LOW){
          }
          digitalWrite(prozilec2, LOW);
        }
        if (  digitalRead(senzor3) == LOW && cas_prebran3 == false ) {
          cas3 = micros();
          cas_prebran3 = true;
        }
        zdej = micros();  //  Povečaj aktualen čas
      }
      
      digitalWrite(prozilec1, LOW); //  Od začetka proženja je pretekel čas timer_prozenje, zato ugasnemo tuljavo.

      while (cas_prebran1 == false) { //  Preverimo, če je že bil izstrelek na prvem senzorju
         if ( digitalRead(senzor1) == LOW) {  //  Če še ni bil, čakamo, da bo in takrat izmerimo čas in spremenimo stanje
         cas1 = micros();
         cas_prebran1 = true;
         }
      }
      while (cas_prebran2 == false) { //  Enako kot gor, za drugi senzor.
         if ( digitalRead(senzor2) == LOW) {
         cas2 = micros();
         cas_prebran2 = true;
         digitalWrite(prozilec2, HIGH); //  Prižgi 2. tuljavo.
          while (digitalRead(senzor2) == LOW){
          }
          digitalWrite(prozilec2, LOW);
         }
      }
      digitalWrite(prozilec2, LOW);
      while (cas_prebran3 == false) { //  Enako kot gor, za tretji senzor.
         if ( digitalRead(senzor3) == LOW) {
         cas3 = micros();
         cas_prebran3 = true; 
         }
      }
      
      dt1 = ( cas1 - cas0 ) / 1000000; // Izračunaj razliko časov. Čas je v mikrosekundah, zato ga delimo z 1000000.
      dt2 = ( cas2 - cas0 ) / 1000000;
      dt3 = ( cas3 - cas0 ) / 1000000;
      
      avgv1 = razdalja1 / dt1;  //  Izračunaj povprečne hitrosti.
      avgv2 = razdalja2 / dt2;
      avgv3 = razdalja3 / dt3;
      
      v1 = 2*avgv1; //  Izračunaj hitrosti.
      v2 = 2*avgv2;
      v3 = 2*avgv3;

      a1 = v1 / dt1;  //  Izračunaj pospeške.
      a2 = v2 / dt2;
      a3 = v3 / dt3;

      lcd_hitrost.clear();  //  Izpiši hitrosti.
      lcd_hitrost.setCursor(0,0);
      lcd_hitrost.print("v_1= " + String(v1, 6) + " m/s");
      lcd_hitrost.setCursor(0,1);
      lcd_hitrost.print("v_3 = " + String(v3, 6) + " m/s");
 
      lcd_acc.clear();  //  Izpiši pospeške.
      lcd_acc.setCursor(0,0);
      lcd_acc.print("a_1= " + String(a1, 6) + " m/s^2");
      lcd_acc.setCursor(0,1);
      lcd_acc.print("a_2= " + String(a3, 6) + " m/s^2");

      lcd_volt.clear();
      lcd_volt.print("Procesiram...");
      delay(5000); //  Počakamo, da se kondenzatorji napolnijo.
      Voltmeter(pin_volt1); // izpiši napetosti na kondenzatorjih. Na tej točki, bi morali biti polni.
      Voltmeter(pin_volt2);

      }

    } else { // konec if-a       
      delay(50);
      }

}
