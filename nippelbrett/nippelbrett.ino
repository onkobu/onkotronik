/******************************************************************************

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
   der GNU General Public License, wie von der Free Software Foundation,
   Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
   veröffentlichten Version, weiterverbreiten und/oder modifizieren.

   Dieses Programm wird in der Hoffnung, dass es nützlich sein wird, aber
   OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite
   Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
   Siehe die GNU General Public License für weitere Details.

   Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
   Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.

   Nippelbrett von Onkobu Tanaake

   Arduino Uno R3 als Plattform
   http://www.arduino.cc/

   Tastatur 2 Reihen zu je max. 8 Tastern, keyscan über 3:8-Decoder (74HC138
   active low), Inspiriert von
   http://www.openmusiclabs.com/learning/digital/input-matrix-scanning/

   MP3-Shield von ELV
   http://www.elv.de/audio-shield-fuer-arduino-asa1-bausatz.html
   Artikel-Nr.: 68-10 59 22

   TimerOne von Paul Stoffregen
   https://github.com/PaulStoffregen/TimerOne

   Keyscan: alle 10ms ein Durchlauf, alle 8 Adressen durchgehen und jeweils
   doppelt nach Danegger entprellen (2x2 16bit Zähler),
   siehe http://www.mikrocontroller.net/articles/Entprellung#Timer-Verfahren_.28nach_Peter_Dannegger.29

    - alle 10ms Interruptfunktion
    - Zähler von 1-8 hochzählen und auf A0-A2 ausgeben
    - mit Zählerstand und A3-A4 die beiden Zähler befüllen

   Abpsielen: je Taster eine Geräuschdatei festlegen und bei Druck abspielen,
   während abspielen kein keyscan

 ******************************************************************************/
#include <TimerOne.h>
#include <SD.h>
#include <SPI.h>
#include <AudioShield.h>
#define NOP __asm__("nop\n\t")

char fName[8];
int cat = 0;
// Es ist einfacher, die Zeichenketten so zusammenzustellen, als im Nachhinein
// fest kodierte Werte auszulesen (Speicherverbrauch, Abbildung auf Tastennummer)
//
// Die String-Routinen aus der Arduino-Bibliothek blähen den Code um weitere 4k
// auf - etwas viel um nur 1 aus 10 bereitzustellen.
// Array für Tastenbelegungen
const char *a[10] = {
  //0   1    2    3    4
  "2", "1", "4", "3", "6",
  //5   6    7     8    9
  "5", "8", "7", "10", "9"
};
// Array für Kategorien 
const char *catChar[5] = {
  "1", "2", "3", "4", "5"
};

volatile uint16_t key_reg;
volatile uint16_t key_state;
volatile uint16_t key_press;

void setup() {
  // DDRx:
  // 0 = Input
  // 1 = Output
  // PORTx
  // if writing 1 to input, pullup is enabled
  DDRC &= ~(1 << PC3) | (1 << PC4);
  DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2);
  // enable pullups
  PORTC |= (1 << PC3) | (1 << PC4);
  // initialize serial communication at 9600 bits per second:
  //Serial.begin(9600);
  // make the pushbutton's pin an input:

  Timer1.initialize(10000); // every 10ms
  Timer1.attachInterrupt(readKeys);

  // init SD card
  SD.begin( SD_CS );

  // init MP3 shield
  VS1011.begin();
}

void readKeys(void) {
  // clear lowest 3 bits
  PORTC &= 0xF8;
  key_reg = 0;
  for (uint8_t addr = 0; addr < 8; addr++) {
    // Wait for the DMUX to settle
    NOP; NOP; NOP;
    // shift in key state, active low
    key_reg <<= 2;
    key_reg |= (((~PINC) >> 3) & 3);

    // Don't count up and overflow from the last three bits into the forth,
    // this will alter the pullup resistors.
    if (addr < 7) {
      PORTC++;
    }
  }
  PORTC &= 0xF8;

  // This is Danegger-debouncing, with 16bit counters and no hold
  // or long press option
  static uint16_t ct0, ct1, rpt;
  uint16_t i;

  i = key_state ^ key_reg;                       // key changed ?
  ct0 = ~( ct0 & i );                             // reset or count ct0
  ct1 = ct0 ^ (ct1 & i);                          // reset or count ct1
  i &= ct0 & ct1;                                 // count until roll over ?
  key_state ^= i;                                 // then toggle debounced state
  key_press |= key_state & i;                     // 0->1: key press detect
}

///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed. Each pressed key is reported
// only once
//
uint16_t get_key_press( uint16_t key_mask )
{
  cli();                                          // read and clear atomic !
  key_mask &= key_press;                          // read key(s)
  key_press ^= key_mask;                          // clear key(s)
  sei();
  return key_mask;
}

void playSample(char *name) {
  Serial.print("Play ");
  Serial.println(name);
  unsigned char buffer[32];

  //Datei öffnen und abspielen
  if ( File SoundFile = SD.open( name, FILE_READ ) ) {
    //Verstärker einschalten
    VS1011.UnsetMute();

    //Datei bis zum Ende abspielen oder Tastendruck
    while ( SoundFile.available() && key_press == 0 ) {
      //Puffer mit Daten aus der Datei füllen
      SoundFile.read( buffer, sizeof(buffer) );
      //Daten aus Puffer an MP3-Decoder senden
      VS1011.Send32( buffer );
    }
    //Internen Datenpuffer vom MP3-Decoder mit Nullen füllen
    //damit sicher alles im Puffer abgespielt wird und Puffer leer ist
    //MP3-Decoder besitzt 2048 Byte großen Datenpuffer
    VS1011.Send2048Zeros();

    //Verstärker deaktivieren
    VS1011.SetMute();

    //Datei schliessen
    SoundFile.close();
  }/* else {
    Serial.println("Aua, nix da");
  }*/
}

// the loop routine runs over and over again forever:
void loop() {
  //  Serial.println(PINC, BIN);
  //  readKeys();
  //  Serial.println(keyState, BIN);
  //  PORTC &= 0xF8;
  //  PORTC |= 1;
  //  Serial.print("PORTC ");
  //  Serial.println(PORTC,BIN);
  //  Serial.print("PINC ");
  //  Serial.println(PINC,BIN);
  //  delay(1000);
  delay(100);
  for (uint8_t keyIdx = 0; keyIdx < 16; keyIdx++) {
    if (get_key_press(1 << keyIdx) > 0) {
      // Serial.print("Category: ");
      // Serial.println(cat);
      // Serial.print("#Key: ");
      // Serial.println(keyIdx);
      // Bei Knopf 8 (unten rechts) wird die Kategorie durchgeschalten. Damit man weiß
      // in welche Kategorie man schaltet wird dazu ein Ton abgespielt.
      if (keyIdx == 8) {
        cat++;
        cat %= 5;
        strcpy(fName, catChar[cat]);
        strcat(fName, ".mp3");
        Serial.println(fName);
        playSample(fName);
      }
      // Bei allen anderen Knöpfen werden die IDs mit der Kategorie
      // zusammengesetzt, wodurch sich dann der richtige Name für die Datei ergibt.
      else {
        strcpy(fName, catChar[cat]);
        strcat(fName, "_");
        strcat(fName, a[keyIdx]);
        strcat(fName, ".mp3");
        Serial.println(fName);
        playSample(fName);
      }
    }
  }

  // A0-A5
  // 3 address pins A0-A2
  // 2 input pins A3, A4
}
