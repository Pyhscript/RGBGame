/*Realizacija igrice uz pomoć Arduina
   Benjamin Nadarević
*/
#include <EEPROM.h>
#include <Tlc5940.h> // pinovi 13,11,10,9 i 3 su zauzeti za TLC5940
#include <Keypad.h> //zasad koristim ovu biblioteku, kasnije svoj kod možda

int level = 1;
int duljinaSekvence = 0;
int najveci = EEPROM.read(0); //maks. broj zasad je 255
int sekvenca[15]; //maksimalna duljina sekvence je 15
int finalnaSekvenca[30];
int listaNarancastih[15];
int listaPlavih[15];
int brojacPlavih;
int brojacNarancastih;
int brojLedica; //koliko ih treba upalit
int pauza = 1000;
unsigned long finished;
int tipkaPoRedu = 0;
int nijeIzgubio = 1;
// definiranje tipkala koristeći Keypad biblioteku
const byte REDOVI = 3;
const byte STUPCI = 3;
byte redPinovi[REDOVI] = {8, 7, 6};
byte stupacPinovi[STUPCI] = {5, 4, 2};

char poljeTipkala[REDOVI][STUPCI] = {
  {'0', '1', '2'},
  {'3', '4', '5'},
  {'6', '7', '8'},




};

Keypad tipkala = Keypad(makeKeymap(poljeTipkala), redPinovi, stupacPinovi, REDOVI, STUPCI);

void prikaziSekvencu() {

  for (int i = 0; i < brojLedica; i++) {
    if (level <= 5) {

      // Serial.println(brojLedica);
      Serial.println(sekvenca[i]);
      Tlc.set(sekvenca[i] + (sekvenca[i] + 1) * 2, 4095); //CRVENA (broj pina crvene katode je generiran po navedenoj formuli
      //delay(1);//magija valjda

    }




    else {


      int postavljen = 0;
      for (int j = 0; j < brojacNarancastih; j++) {
        if (i == listaNarancastih[j]) {
          Tlc.set(sekvenca[i] + (sekvenca[i] * 2) + 1, 295); //ZELENA u ovom omjeru s crvenom daje narancastu
          Tlc.set(sekvenca[i] + (sekvenca[i] + 1) * 2, 4095); //CRVENA
          postavljen = 1;

        }
      }
      for (int j = 0; j < brojacPlavih; j++) {
        if (i == listaPlavih[j]) {
          Tlc.set(sekvenca[i] + (sekvenca[i] * 2), 4095); // PLAVA
          postavljen = 1;

        }
      }

      if (postavljen == 0) {
        Tlc.set(sekvenca[i] + (sekvenca[i] + 1) * 2, 4095);
      }



    }


    while (Tlc.update()); /* Iz dokumentacije:If data has already been
      shifted in this grayscale cycle, another call to update()
      will immediately return 1 without shifting in the new data.
      To ensure that a call to update() does shift in new data, use
       while(Tlc.update()); */
    delay(500);
    Tlc.clear();
    while (Tlc.update());
    delay(pauza);



    Serial.println("ovdje");




  }

  int indeks=0; //u petlji ispod ne mogu koristiti i za indeksiranje finalne sekvence iz razloga što nastaju "rupe" u polju za narancaste bljeskove
  for (int i = 0; i < brojLedica; i++) {
    int kopiraj = 1;
    
    for (int j = 0; j < brojacNarancastih; j++) {
      if (listaNarancastih[j] == i) {
        
        kopiraj = 0;
      

      }

    }

    for (int j = 0; j < brojacPlavih; j++) {
      if (listaPlavih[j] == i) {
      
        finalnaSekvenca[indeks] = sekvenca[i];
        indeks++;
        finalnaSekvenca[indeks] = sekvenca[i];
        indeks++;
        kopiraj = 0;
      }

    }

    if (kopiraj == 1) {
      
      finalnaSekvenca[indeks] = sekvenca[i];
      indeks++;
    }

  }




}
void generirajSekvencu() {
  int barJedanCrveni; //kako bi se izbjegla malo vjerojatna, ali ipak moguca situacija da su svi bljeskovi narancasti
  int slucajnaBoja;

  duljinaSekvence = 0;

  if  (level <= 5) {
    brojLedica = level + 1;
    duljinaSekvence = brojLedica;
  }

  else if (level <= 15) {
    brojLedica = 4;
  }
  else if (brojLedica < 15) {

    brojLedica = 6 + ((level - 15) / 3); //za svaki 3 level nakon 15-og povećaj duljinu sekvence za jedan
  }
  for (int i = 0; i < brojLedica; i++) {
    sekvenca[i] = random(9);

  }


  if (level > 5) {
    //uvode se boje za koja vrijede druga pravila, narancasta=>ignoriraj  , plava=>pritisni dvaput

    for (int i = 0; i < 15; i++) {
      listaNarancastih[i] = -1;
      listaPlavih[i] = -1; //čišćenje polja zbog "otpadnih vrijednosti"
    }

    barJedanCrveni = random(brojLedica);
    brojacNarancastih = 0;
    brojacPlavih = 0;
    for (int i = 0; i < brojLedica; i++) {
      if (i != barJedanCrveni) {
        slucajnaBoja = random(3); //33.33% sanse za svaku boju
        if (slucajnaBoja == 0) {
          listaNarancastih[brojacNarancastih] = i;
          brojacNarancastih++;
        }

        else if (slucajnaBoja == 1) {
          listaPlavih[brojacPlavih] = i;
          brojacPlavih++;
          duljinaSekvence += 2;
        }

        else {
          duljinaSekvence += 1;
        }


      }
      else {
        duljinaSekvence += 1;
      }
    }

  }




  finished = millis();
}
void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  Tlc.init(0);
  Serial.println("INIT");
  // delay(2000);
  randomSeed(analogRead(0));

  generirajSekvencu();
  prikaziSekvencu();
  Serial.println(finished);


  //delay(2550);

  //Serial.println(sekvenca[0]);

}

void loop() {


  if (nijeIzgubio) {


    char tipka = tipkala.getKey();

    if (tipka && (tipka - 48) == finalnaSekvenca[tipkaPoRedu]) { //(tipka-48) zbog ASCII
      Serial.println(duljinaSekvence);

      Serial.println("TOČNO");
      tipkaPoRedu++;
      if (tipkaPoRedu == duljinaSekvence) {
        tipkaPoRedu = 0;
        level++;
        Serial.println("=================");
        Serial.println(level);
        Serial.println("==============");

        generirajSekvencu();
        delay(3000);
        prikaziSekvencu();
      }
    }
    else if (tipka) {
      Serial.println(duljinaSekvence);
      Serial.println("IZGUBIO SI");
      nijeIzgubio = 0;
    }



  }
  EEPROM.write(5, millis());

}


