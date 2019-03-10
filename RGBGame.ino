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
bool unazad = false;
int brojLedica; //koliko ih treba upalit
int pauza = 1000;
int duljina=500; //duljina bljeska
unsigned long finished;
int tipkaPoRedu = 0;
int nijeIzgubio = 1;
// definiranje tipkala koristeći Keypad biblioteku
const byte REDOVI = 3;
const byte STUPCI = 3;
byte redPinovi[REDOVI] = {5, 4, 2};
byte stupacPinovi[STUPCI] = {8, 7, 6};




char poljeTipkala[REDOVI][STUPCI] = {
  {'0', '1', '2'},
  {'3', '4', '5'},
  {'6', '7', '8'},




};

int predzadnjaZnamenka = 14; //ovo su znamenke displeja koje aktiviram preko tranzistora
int zadnjaZnamenka = 15; //koristim A0 i A1 kao digitalne pinove
int segment_f = 16;
int segment_g = 17; //segmente a-e kontroliram koristeći slobodne pinove drugog TLC5940-a (27-31)

Keypad tipkala = Keypad(makeKeymap(poljeTipkala), redPinovi, stupacPinovi, REDOVI, STUPCI);

void resetirajHighscore() {
  EEPROM.write(0, 0);
}

void ocistiDisplej() {
  digitalWrite(segment_f, HIGH);
  digitalWrite(segment_g, HIGH);
  for (int i = 27; i < 32; i++) {
    Tlc.set(i, 0);
    while (Tlc.update());
  }
}

void displej(int porukaID) {
  //
  int segment_brojevi[10] = {0x7E, 0x30 , 0x6D, 0x79, 0x33, 0x5B , 0x5F , 0x70 , 0x7F , 0x7B} ; //heksadecimalne vrijednosti za brojeve 0-9 u 7-segmentnom formatu abcdefg
  if (porukaID >= 0) { //manje od 0 je rezervirano za tekstualne poruke na 7-segmentnom displeju (HS(HIGHSCORE) - novi rekord , LS(LOST) izgubio )
    //Serial.println("trebao bi displejat");

    if (porukaID >= 10) {
      ocistiDisplej();
      digitalWrite(predzadnjaZnamenka, HIGH);
      digitalWrite(zadnjaZnamenka, LOW);
      displej(porukaID / 10);
      ocistiDisplej();

      digitalWrite(predzadnjaZnamenka, LOW);

      digitalWrite(zadnjaZnamenka, HIGH);
      displej(porukaID % 10);

    }

    if (bitRead(segment_brojevi[porukaID], 0) == 1) {
      digitalWrite(segment_g, LOW);
      delay(1);
      digitalWrite(segment_g, HIGH);

    }

    if (bitRead(segment_brojevi[porukaID], 1) == 1) {
      digitalWrite(segment_f, LOW);
      delay(1);
      digitalWrite(segment_f, HIGH);
    }

    for (int i = 2; i < 7; i++) {
      if (bitRead(segment_brojevi[porukaID], i) == 1) {
        Tlc.set((31 - (i - 2)), 4095); //a je spojen na TLC pin 27, b na 28, c na 29, d na 30, e na 31
        while (Tlc.update());
        Tlc.set((31 - (i - 2)), 0);
        //Serial.println("ovdje"); //trebalo je maknuti sve serial.printove zato jer su usporavali vrijeme osvježavanja 7-segmentnog displeja (titrao je)
        while (Tlc.update());
      }
    }







  }

  else if (porukaID == -1) {
    //Serial.println("TREBA PRIKAZATI H");
    //prikazuje HS (kao HIGHSCORE - srušen rekord)
    ocistiDisplej();
    digitalWrite(predzadnjaZnamenka, HIGH);
    digitalWrite(zadnjaZnamenka, LOW);


    digitalWrite(segment_f, LOW);
    digitalWrite(segment_g, LOW);
    Tlc.set(31, 4095);
    Tlc.set(29, 4095);
    Tlc.set(28, 4095);
    while (Tlc.update());

    ocistiDisplej();


    digitalWrite(predzadnjaZnamenka, LOW);
    delay(1); //bez ovog ostane upaljen segment e na zadnjoj znamenci (blijedo, ali ipak upaljen)
    digitalWrite(zadnjaZnamenka, HIGH);
    displej(5); //5 izgleda kao S



  }

  else if (porukaID == -2) { //LS - kao skraćeno za "LOST" - izgubljena igra
    ocistiDisplej();
    digitalWrite(predzadnjaZnamenka, HIGH);
    digitalWrite(zadnjaZnamenka, LOW);

    Tlc.set(31, 4095);
    Tlc.set(30, 4095);
    digitalWrite(segment_f, LOW);
    while (Tlc.update());

    ocistiDisplej();


    digitalWrite(predzadnjaZnamenka, LOW);
    delay(1);
    digitalWrite(zadnjaZnamenka, HIGH);
    displej(5);



  }
}




void prikaziSekvencu() {

  for (int i = 0; i < brojLedica; i++) {
    if (level <= 5) {

      // Serial.println(brojLedica);
      //Serial.println(sekvenca[i]);
      Tlc.set(sekvenca[i] + (sekvenca[i] + 1) * 2, 4095); //CRVENA (broj pina crvene katode je generiran po navedenoj formuli
      //delay(1);//magija valjda

    }

    else {

      if (unazad == false || (unazad == true && i != brojLedica - 1)) {

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

      else {
        Tlc.set(sekvenca[i] + (sekvenca[i] + 1) * 2, 4095); //CRVENA
        Tlc.set(sekvenca[i] + (sekvenca[i] * 2), 4095); //PLAVA
        //zajedno ljubičasta
      }

    }




    while (Tlc.update()); /* Iz dokumentacije:If data has already been
      shifted in this grayscale cycle, another call to update()
      will immediately return 1 without shifting in the new data.
      To ensure that a call to update() does shift in new data, use
       while(Tlc.update()); */
    delay(duljina);
    Tlc.clear();
    while (Tlc.update());
    delay(pauza);



    //Serial.println("ovdje");




  }

  int indeks = 0; //u petlji ispod ne mogu koristiti i za indeksiranje finalne sekvence iz razloga što nastaju "rupe" u polju za narancaste bljeskove
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

  if (unazad == true) {
    int privremenaFinalnaSekvenca[indeks]; 
    //Serial.println(indeks);
  //  Serial.println("!!!!!!!!!!!!!!!");
    for (int i = 0; i < indeks; i++) {
      privremenaFinalnaSekvenca[i] = finalnaSekvenca[i];
      //Serial.println(privremenaFinalnaSekvenca[i]);
      //delay(1000);
      
    }
    Serial.println("????????????");
    int x = 0;
    for (int i = indeks-1; i >= 0; i--) {
      finalnaSekvenca[x] = privremenaFinalnaSekvenca[i];
      //Serial.println(finalnaSekvenca[x]);
    //  delay(1000);
      x++;
    }

   // delay(3000);
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

  else {
    
    if(pauza>=200){
    pauza=pauza-(level-6)*20; //smanjuj pauzu među bljeskovima
    }
    if(duljina>250){
      duljina=duljina-(level-6)*10;
    }
    brojLedica = 3 + (level - 6);
    if (brojLedica>7){
      brojLedica=7; //maks. 7
    }
  }
  for (int i = 0; i < brojLedica; i++) {
    sekvenca[i] = random(9);

  }


  if (level > 5) {
    //uvode se boje za koja vrijede druga pravila, narancasta=>ignoriraj  , plava=>pritisni dvaput



    for (int i = 0; i < 15; i++) {
      listaNarancastih[i] = -1;
      listaPlavih[i] = -1; //čišćenje polja zbog "otpadnih vrijednosti" iz prethodnih ciklusa
    }

    if (level > 10) {
      if (random(3) == 1) {
        unazad = true; //jednom u tri nakon desetog levela
        brojLedica = random(3,6); //ako ih treba ponovit za nazad onda je 5 ledica maksimum, 3 minimum
      }
      else {
        unazad = false; //da promjeni natrag ako je u prethodnom levelu bilo true
      }
    }
    barJedanCrveni = random(brojLedica);
    brojacNarancastih = 0;
    brojacPlavih = 0;
    for (int i = 0; i < brojLedica; i++) {
      if (unazad == false || (unazad == true && i != brojLedica - 1)) {
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
     else{
      duljinaSekvence+=1;
     }
    }

  }




}
void setup() {

  pinMode(predzadnjaZnamenka, OUTPUT);
  pinMode(zadnjaZnamenka, OUTPUT);
  pinMode(segment_f, OUTPUT);
  pinMode(segment_g, OUTPUT);
  digitalWrite(segment_f, HIGH);
  digitalWrite(segment_g, HIGH);
  digitalWrite(zadnjaZnamenka, HIGH);

  //Serial.begin(9600);
  // put your setup code here, to run once:
  Tlc.init(0);
  //Serial.println("INIT");
  // delay(2000);
  randomSeed(analogRead(4));
  //broj bodova
  // Serial.print("najveci:");
  //Serial.println(najveci);
  long reset = millis();
  while (millis() - reset <= 3000) {
    char tipkaZaReset = tipkala.getKey();
    if (tipkaZaReset && tipkaZaReset - 48 == 8) {
      resetirajHighscore();
    }
  }
  generirajSekvencu();
  prikaziSekvencu();
  //Serial.println(finished);


  //delay(2550);

  //Serial.println(sekvenca[0]);

}

void loop() {


  if (nijeIzgubio) {

    
    char tipka = tipkala.getKey();
    displej(level - 1); //broj bodova




    if (tipka && (tipka - 48) == finalnaSekvenca[tipkaPoRedu]) { //(tipka-48) zbog ASCII
      //Serial.println(duljinaSekvence);

      //Serial.println("TOČNO");

      tipkaPoRedu++;

      if (tipkaPoRedu == duljinaSekvence) {

        tipkaPoRedu = 0;
        level++;

        ocistiDisplej();
        delay(100);


        /* Serial.println("TOČNO");
          Serial.println("=================");
          Serial.println(level);
          Serial.println("==============");*/

        if (level > najveci) {
          for (int i = 0; i < 10; i++) {
            long timer = millis();
            while (millis() - timer <= 300) {
              displej(-1);
            }
            ocistiDisplej();
            delay(300);


          }
          ocistiDisplej();
          EEPROM.write(0, level);
        }

        generirajSekvencu();
        delay(1000);
        prikaziSekvencu();

      }
    }
    else if (tipka) {
      //Serial.println(duljinaSekvence);
      // Serial.println(tipka-48);
      //Serial.println("IZGUBIO SI");
      nijeIzgubio = 0;
      ocistiDisplej(); // da se očisti samo jednom
    }



  }
  else {

    displej(-2);
  }

}

