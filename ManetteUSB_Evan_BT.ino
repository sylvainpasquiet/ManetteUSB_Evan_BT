
// Satisfy the IDE, which needs to see the include statment in the ino too.
//#ifdef dobogusinclude
//  #include <spi4teensy3.h>
//#endif

//#define PWM_FLT 0x0
//#define PWM_FWD1 0x1
//#define PWM_FWD2 0x2
//#define PWM_FWD3 0x3
//#define PWM_FWD4 0x4
//#define PWM_FWD5 0x5
//#define PWM_FWD6 0x6
//#define PWM_FWD7 0x7
//#define PWM_BRK 0x8
//#define PWM_REV7 0x9
//#define PWM_REV6 0xA
//#define PWM_REV5 0xB
//#define PWM_REV4 0xC
//#define PWM_REV3 0xD
//#define PWM_REV2 0xE
//#define PWM_REV1 0xf

#include <SPI.h>
#include <PS3BT.h>
#include <legopowerfunctions.h>
#include <EEPROM.h>

#define PinLed1 2
#define PinLed2 3
#define PinLed3 4
#define PinLed4 5
#define PinClignoD 6
#define PinClignoG 7
#define PinIR 8

LEGOPowerFunctions lego(PinIR);

USB Usb;
BTD Btd(&Usb); 
//PS3BT PS3(&Btd); 
PS3BT PS3(&Btd, 0x00, 0x15, 0x83, 0x3D, 0x0A, 0x58); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch



bool Boot;
bool Marche;
char MiseAJour=0;
char Rouge=0;
char Bleu=0;
char NewRouge=0;
char NewBleu=0;
char Canal;
char Led[4];
char Vibration=0;
unsigned long VibrationTime=0;
unsigned long oldtime=0;
unsigned long oldtimeCligno=0;

void SetVibration()
{
  PS3.setRumbleOn(RumbleHigh);
  Vibration=1;        
  VibrationTime=millis(); 
}


void setup() 
{
  pinMode(PinLed1, OUTPUT);
  pinMode(PinLed2, OUTPUT);
  pinMode(PinLed3, OUTPUT);
  pinMode(PinLed4, OUTPUT);
  pinMode(PinClignoD, OUTPUT);
  pinMode(PinClignoG, OUTPUT);
   
  Serial.begin(115200);
  
  if (Usb.Init() == -1) 
  {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.print(F("\r\nPS3 Bluetooth Library Started"));
  
  Canal = EEPROM.read(0);
  if ((Canal!=CH1)&&(Canal!=CH2)&&(Canal!=CH3)&&(Canal!=CH4))
  {
    Canal = CH1;
    EEPROM.write(0, Canal);
  }
}

void loop() {
  Usb.Task();
  if (PS3.PS3Connected || PS3.PS3NavigationConnected) 
  {
    if (PS3.getButtonClick(SELECT)) 
    {
      Canal = (Canal+1)%4;
      EEPROM.write(0, Canal);
      Boot=0;
      SetVibration();
    }

    // Si on appui sur un des symboles, on change les LED 
    if (PS3.getButtonClick(TRIANGLE)) Led[0]=!Led[0];  
    if (PS3.getButtonClick(CIRCLE))  Led[1]=!Led[1];
    if (PS3.getButtonClick(CROSS)) {Led[2]=!Led[2]; SetVibration();}  
    if (PS3.getButtonClick(SQUARE)) {Led[3]=!Led[3]; SetVibration();}  

    if (Vibration==1)
    {
      if (millis()-VibrationTime>300)
      {
        Vibration=0;
        PS3.setRumbleOff();
      }
    }

    // Pour faire le marche ou l'arret
    if (PS3.getButtonClick(START)) 
    {
      Marche=!Marche;
      Boot=0;
    }
    
    // Au demarrage, on eteint toutes les LED et on allume la LED suivant le canal choisi plus haut
    if (!Boot)
    {
      Boot=1; 
      PS3.setLedOff(); 
      if (Marche)
      {
        switch(Canal)
        {
          case CH1: PS3.setLedOn(LED1); break;
          case CH2: PS3.setLedOn(LED2); break;
          case CH3: PS3.setLedOn(LED3); break;
          case CH4: PS3.setLedOn(LED4); break;
        }
      }
    }

    NewRouge=(PS3.getAnalogHat(LeftHatY))/18;
    NewBleu=(255-PS3.getAnalogHat(RightHatY))/18;

    switch(NewRouge)
    {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:NewRouge=9+NewRouge;break;
      case 7:NewRouge=PWM_FLT;break;
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:NewRouge=NewRouge-7;break;
    }
    switch(NewBleu)
    {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:NewBleu=9+NewBleu;break;
      case 7:NewBleu=PWM_FLT;break;
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:NewBleu=NewBleu-7;break;
    }

    if (Marche) 
    {     
      // On gère le bleu
      if (NewBleu!=Bleu) { Bleu=NewBleu; MiseAJour=1;}
      // On gère le rouge
      if (NewRouge!=Rouge) { Rouge=NewRouge; MiseAJour=1;}
    }
    else
     {     
      if (Rouge!=PWM_FLT) {Rouge=PWM_FLT; MiseAJour=1;}
      if (Bleu!=PWM_FLT) {Bleu=PWM_FLT; MiseAJour=1;}
    }   
  }

  if (Marche)
  {
    if (Led[0]) digitalWrite(PinLed1, HIGH);
    else digitalWrite(PinLed1, LOW);
    if (Led[1]) digitalWrite(PinLed2, HIGH);
    else digitalWrite(PinLed2, LOW);
    if (Led[2]) digitalWrite(PinLed3, HIGH);
    else digitalWrite(PinLed3, LOW);
    if (Led[3]) digitalWrite(PinLed4, HIGH);
    else digitalWrite(PinLed4, LOW);
  }

  // on force une nouvelle commande si cela fait trop longtemps sans commande
  if (millis()-oldtime>1000)
  {
    MiseAJour=1;
  }
  // on a demande au dessus de changer quelque chose, alors on envoie la nouvelle commande Power function
  if (MiseAJour==1)
  {  
    oldtime = millis();
    MiseAJour=0;
    lego.ComboPWM(Bleu, Rouge, Canal);
  }   
  if (Led[3])
  {
    if (millis()-oldtimeCligno>500)
    {
      digitalWrite(PinClignoD, HIGH);digitalWrite(PinClignoG, LOW);
    }  
    else
    {
      digitalWrite(PinClignoD, LOW);digitalWrite(PinClignoG, HIGH); 
    }
  }
  else
  {
    digitalWrite(PinClignoD, LOW);digitalWrite(PinClignoG, LOW); 
  }
  if (millis()-oldtimeCligno>1000)
  {
    oldtimeCligno = millis();
  }
}
