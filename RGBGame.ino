/*Realizacija igrice uz pomoć Arduina
 * Benjamin Nadarevič
 */
#include <EEPROM.h>
#include <Tlc5940.h> // pinovi 13,11,10,9 i 3 su zauzeti za TLC5940
#include <Keypad.h> //zasad koristim ovu biblioteku, kasnije svoj kod možda
int level = 1;
int najveci=EEPROM.read(0); //maks. broj zasad je 255 
int sekvenca[15]; //maksimalna duljina sekvence je 15
int modifikator=0; //0-nema modifikatora 1 - plavi 2-žuti 3-narančasti 4-ljubičasti
int brojLedica; //koliko ih treba upalit
int pauza=1000;
// definiranje tipkala koristeći Keypad biblioteku
const byte REDOVI=3;
const byte STUPCI=3;
byte redPinovi[REDOVI]= {5,4,2};
byte stupacPinovi[STUPCI]= {8,7,6};

char poljeTipkala[REDOVI][STUPCI]= {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'}
};

Keypad tipkala = Keypad(makeKeymap(poljeTipkala),redPinovi,stupacPinovi,REDOVI,STUPCI);

void prikaziSekvencu(){
  for(int i=0;i<brojLedica;i++){
    if (level<=5){
     // Serial.println(brojLedica);
      Serial.println(sekvenca[i]);
      Tlc.set(sekvenca[i]+(sekvenca[i]+1)*2,4095); //CRVENA (broj pina crvene katode je generiran po navedenoj formuli
      //delay(1);//magija valjda
      while(Tlc.update()); /* Iz dokumentacije:If data has already been 
      shifted in this grayscale cycle, another call to update()
      will immediately return 1 without shifting in the new data.
      To ensure that a call to update() does shift in new data, use 
       while(Tlc.update()); */
      delay(1000);
      Tlc.clear();
      while(Tlc.update());
   
      
      
      Serial.println("ovdje");
     
    }
    
  }
    
  
}
void generirajSekvencu(){ 
  if  (level<=5){
      brojLedica=level+1;
    }
    
  else if (level<=15){
    brojLedica=7;
  }
  else if (brojLedica<15){
    
    brojLedica=brojLedica+((level-15)/3);//za svaki 3 level nakon 15-og povećaj duljinu sekvence za jedan
  }
  for(int i=0;i<brojLedica;i++){
    sekvenca[i]=random(0,8);
  }
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

 
  
  //delay(2550);
 
  //Serial.println(sekvenca[0]);
  
}

void loop() {
  
  
  
}
  

