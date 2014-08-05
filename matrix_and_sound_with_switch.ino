#include <Tone.h>

Tone tone1;

#define OCTAVE_OFFSET 0
unsigned char i;
unsigned char j;
/*Port Definitions*/
int Max7219_pinCLK = 11;
int Max7219_pinCS = 10;
int Max7219_pinDIN = 9;
int notes[] = { 0,
NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7
};
unsigned char disp1[38][8] = {
 {0x3C,0x22,0x22,0x22,0x3C,0x24,0x22,0x21},
 {0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x7C},
 {0x0,0x1E,0x20,0x20,0x3E,0x2,0x2,0x3C},
 {0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x7C},
 {0x0,0x1E,0x20,0x20,0x3E,0x2,0x2,0x3C},
 {0x0,0x3E,0x8,0x8,0x8,0x8,0x8,0x8},
 {0x8,0x14,0x22,0x3E,0x22,0x22,0x22,0x22},
 {0x0,0x42,0x62,0x52,0x4A,0x46,0x42,0x0},
 {0x3C,0x40,0x40,0x40,0x40,0x40,0x3C,0x0},
 {0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x7C},
 {0,0,0,0,0,0,0,0},
 {0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x7C},
 {0x0,0x1E,0x20,0x20,0x3E,0x2,0x2,0x3C},
 {0,0,0,0,0,0,0,0},
 {0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x40},
 {0x42,0x42,0x42,0x42,0x42,0x42,0x22,0x1C},
 {0x0,0x3E,0x8,0x8,0x8,0x8,0x8,0x8},
 {0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x7C},
 {0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7C},
 {0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x7C},
 };
//Song variables
char *StarWars = "StarWars:d=4,o=5,b=45:32p,32f#";//,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#.6,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#6";
char *StarTrek = "Star Trek:d=16,o=5,b=63:8f.";//,a#,4d#.6,8d6,a#.,g.,c.6,4f6";

const int buttonPin = 2;

int buttonState = 0;
int lastButtonState = 0;
int songPlayed = 0;

void setup(void)
{
  pinMode(Max7219_pinCLK,OUTPUT);
  pinMode(Max7219_pinCS,OUTPUT);
  pinMode(Max7219_pinDIN,OUTPUT);
  pinMode(buttonPin, INPUT);
  delay(50);
  Init_MAX7219();
  Serial.begin(9600);
  tone1.begin(8);
}

#define isdigit(n) (n >= '0' && n <= '9')

//functions for tone library
void play_rtttl(char *p)
{
  // Absolutely no error checking in here

  byte default_dur = 4;
  byte default_oct = 6;
  int bpm = 63;
  int num;
  long wholenote;
  long duration;
  byte note;
  byte scale;

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while(*p != ':') p++;    // ignore name
  p++;                     // skip ':'

  // get default duration
  if(*p == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if(num > 0) default_dur = num;
    p++;                   // skip comma
  }

  // get default octave
  if(*p == 'o')
  {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if(num >= 3 && num <=7) default_oct = num;
    p++;                   // skip comma
  }

  // get BPM
  if(*p == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)

  // now begin note loop
  while(*p)
  {
    // first, get note duration, if available
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }

    if(num) duration = wholenote / num;
    else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

    // now get the note
    note = 0;

    switch(*p)
    {
      case 'c':
        note = 1;
        break;
      case 'd':
        note = 3;
        break;
      case 'e':
        note = 5;
        break;
      case 'f':
        note = 6;
        break;
      case 'g':
        note = 8;
        break;
      case 'a':
        note = 10;
        break;
      case 'b':
        note = 12;
        break;
      case 'p':
      default:
        note = 0;
    }
    p++;

    // now, get optional '#' sharp
    if(*p == '#')
    {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if(*p == '.')
    {
      duration += duration/2;
      p++;
    }

    // now, get scale
    if(isdigit(*p))
    {
      scale = *p - '0';
      p++;
    }
    else
    {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if(*p == ',')
      p++;       // skip comma for next note (or we may be at the end)

    // now play the note

    if(note)
    {
      tone1.play(notes[(scale - 4) * 12 + note]);
      delay(duration);
      tone1.stop();
    }
    else
    {
      Serial.print("Pausing: ");
      Serial.println(duration, 10);
      delay(duration);
    }
  }
}

//functions for LED Matrix
void Write_Max7219_byte(unsigned char DATA)
{
            unsigned char i;
      digitalWrite(Max7219_pinCS,LOW);
      for(i=8;i>=1;i--)
          {
             digitalWrite(Max7219_pinCLK,LOW);
             digitalWrite(Max7219_pinDIN,DATA&0x80);// Extracting a bit data
             DATA = DATA<<1;
             digitalWrite(Max7219_pinCLK,HIGH);
           }
}


void Write_Max7219(unsigned char address,unsigned char dat)
{
        digitalWrite(Max7219_pinCS,LOW);
        Write_Max7219_byte(address);           //address，code of LED
        Write_Max7219_byte(dat);               //data，figure on LED
        digitalWrite(Max7219_pinCS,HIGH);
}

void Init_MAX7219(void)
{
 Write_Max7219(0x09, 0x00);       //decoding ：BCD
 Write_Max7219(0x0a, 0x03);       //brightness
 Write_Max7219(0x0b, 0x07);       //scanlimit；8 LEDs
 Write_Max7219(0x0c, 0x01);       //power-down mode：0，normal mode：1
 Write_Max7219(0x0f, 0x00);       //test display：1；EOT，display：0
}

void loop(void){
  buttonState = digitalRead(buttonPin);
  if (buttonState != lastButtonState) {
    songPlayed = 0;
    if (buttonState == HIGH){
      play_rtttl(StarTrek);
      for(j=0;j<38;j++){
        for(i=1;i<9;i++)
          Write_Max7219(i,disp1[j][i-1]);
        }
      Serial.println("Assimilate");
    }
  else{
    play_rtttl(StarWars);
    Serial.println("Trollololol");
  }
  songPlayed++;
  }
  lastButtonState = buttonState;
  delay(1000);
}