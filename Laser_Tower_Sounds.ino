/*
  Poker Table

  Press a button on an RF keyfob to light up the next dealer's seat.
*/
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin


// constants
const int CHECK_RF_INTERVAL = 100;

const int KEYFOB_PIN_A = A3;  // keyfob A
const int KEYFOB_PIN_B = A2;  // keyfob B
const int KEYFOB_PIN_C = A1;  // keyfob C
const int KEYFOB_PIN_D = A0;  // keyfob D

const int VOLUME_HIGH = 1;
const int VOLUME_LOW = 20;

// flag to indicate the first time run
bool firstPass = true; 

unsigned long previousBlinkMillis = 0;  
unsigned long previousRFMillis = 0;  
unsigned long previousRandToneMillis = 0;  
unsigned long previousRandDealerMillis = 0;  

int keyfobStateA = LOW;   
int keyfobPrevStateA = LOW;       

int keyfobStateB = LOW;   
int keyfobPrevStateB = LOW;       

int keyfobStateC = LOW;   
int keyfobPrevStateC = LOW;       

int keyfobStateD = LOW;   
int keyfobPrevStateD = LOW;       

bool gameStarted = false;

// phrases to say when the game is taking too long.  Triggered by button C after randomization is complete.
int delayPhraseIndex = 0;
const int NUM_DELAY_PHRASES = 2;
int delayPhrases[NUM_DELAY_PHRASES];

Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);



void setup() {
  Serial.begin(9600);


  delayPhrases[0] = "/HURRY.mp3";
  delayPhrases[1] = "/TKNGLNG.mp3";
 // delayPhrases[2] = "/JEOTHEME.mp3";


  pinMode(KEYFOB_PIN_A, INPUT);
  pinMode(KEYFOB_PIN_B, INPUT);
  pinMode(KEYFOB_PIN_C, INPUT);
  pinMode(KEYFOB_PIN_D, INPUT);

  if (!musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053!"));
     while (1);
  }
  Serial.println(F("VS1053 found"));

   if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  //printDirectory(SD.open("/"), 0);

  randomSeed(millis());
}


// the loop function runs over and over again forever
void loop() {
  checkForRFSignal();
  delay(100);
}


////////////////////////////////////////////////////
// Check to see if a FOB button was pressed
////////////////////////////////////////////////////
void checkForRFSignal() {
  unsigned long currentMillis = millis();

  if (firstPass == true) {
    // get the initial states so we can check if they changed via keyfob
    keyfobPrevStateA = digitalRead(KEYFOB_PIN_A);
    keyfobPrevStateB = digitalRead(KEYFOB_PIN_B);
    keyfobPrevStateC = digitalRead(KEYFOB_PIN_C);
    keyfobPrevStateD = digitalRead(KEYFOB_PIN_D);

    firstPass = false;
    delay(100);
  } 
  else {
    // Is it time to check for a keyfob press?
    if (currentMillis - previousRFMillis >= CHECK_RF_INTERVAL) {
      //Serial.println("checking for keyfob press...");

      // save the last time we checked this pin
      previousRFMillis = currentMillis;

      /////////////////////////
      // Keyfob A
      /////////////////////////
      keyfobStateA = digitalRead(KEYFOB_PIN_A);
      //Serial.println(keyfobStateA);
      if (keyfobPrevStateA != keyfobStateA) {
        Serial.println("KEYFOB A: ");
        keyfobPrevStateA = keyfobStateA;

        if (gameStarted) {
          musicPlayer.setVolume(VOLUME_LOW,VOLUME_LOW);
          musicPlayer.startPlayingFile("/LASER.mp3");
        }
      }

      /////////////////////////
      // Keyfob B
      /////////////////////////
      keyfobStateB = digitalRead(KEYFOB_PIN_B);

      if (keyfobPrevStateB != keyfobStateB) {
        Serial.println("KEYFOB B: ");
        keyfobPrevStateB = keyfobStateB;

        musicPlayer.setVolume(VOLUME_LOW,VOLUME_LOW);
        musicPlayer.startPlayingFile("/EXPLODE.mp3");
      }

      /////////////////////////
      // Keyfob C
      /////////////////////////
      keyfobStateC = digitalRead(KEYFOB_PIN_C);

      if (keyfobPrevStateC != keyfobStateC) {
        
        keyfobPrevStateC = keyfobStateC;

        if (!gameStarted) {
          Serial.println("KEYFOB C: ");

          gameStarted = true;

          musicPlayer.setVolume(VOLUME_LOW,VOLUME_LOW);
          musicPlayer.playFullFile("/JEORND.mp3");
          musicPlayer.playFullFile("/JEORND.mp3");
          musicPlayer.setVolume(VOLUME_HIGH,VOLUME_HIGH);
          musicPlayer.startPlayingFile("/MAYCARDS.mp3");

        } else {
            delayPhraseIndex = random(0, NUM_DELAY_PHRASES);
            musicPlayer.setVolume(VOLUME_HIGH,VOLUME_HIGH);
            musicPlayer.startPlayingFile(delayPhrases[delayPhraseIndex]);
        }
      }

      /////////////////////////
      // Keyfob D
      /////////////////////////
      keyfobStateD = digitalRead(KEYFOB_PIN_D);

      if (keyfobPrevStateD != keyfobStateD) {
        Serial.println("KEYFOB D: ");
        keyfobPrevStateD = keyfobStateD;

        if (gameStarted) {
          musicPlayer.setVolume(VOLUME_HIGH,VOLUME_HIGH);
          musicPlayer.startPlayingFile("/FINROUND.mp3");
        }

      }
    }
  }
}

/// File listing helper
void printDirectory(File dir, int numTabs) {
   Serial.println("printDirectory...");

   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}


