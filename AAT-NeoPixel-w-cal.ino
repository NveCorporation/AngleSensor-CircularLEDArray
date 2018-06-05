/**************************************************************************************
NVE AAT003 angle sensor connected to a NeoPixel 60-LED circular array via an ATtiny 85. 
Compatible with Arduino Trinket: select "AdaFruit Trinket (ATtiny85 @ 8 MHz)."
Sensor "SIN" output to PB3; "COS" to PB4; output to NeoPixel Array on PB2.  Rev. 4/4/18
**************************************************************************************/
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 2, NEO_GRB + NEO_KHZ800);
int AATcos; int AATsin; //AAT signals
unsigned char angle; //Current angle
unsigned char angleOld; //Previous angle
bool dir; //Direction of rotation
unsigned int stopCounter=0; //Number of program iterations where angle hasn't changed
const int repRate=100; //Number of sensor samples/second
const int stopTime=2; //Time rotation is stopped to initiate "idle" routine (sec)
const int arrowSpeed=25; //Arrow animation speed (Hz)
unsigned int arrowPos=0; //Arrow position offset from the sensor in the "idle" routine 
int arrowPixel; //Arrow pixel animation position
unsigned char arrowPixelBrightness;
unsigned char i; //Arrow pixel index
int j; //Positive or negative arrow 

//Define sensor min and max outputs (values determined in calibration routine)
int AATcosmin=1023; int AATcosmax=0; int AATsinmin=1023; int AATsinmax=0;

void setup() {
clock_prescale_set(clock_div_1); //Full speed internal clock (needed for array interface)
strip.begin();
strip.setPixelColor(0, 0, 0, 9); //Blue arrow to indicate calibration mode
strip.setPixelColor(1, 0, 0, 72);
strip.setPixelColor(2, 0, 0, 243); 
strip.show();  
//Calibrate by rotating several turns
  for(j=1; j<=5000; j++) //Sample for five seconds
  {
    AATsin = analogRead(3); //Find sensor output minimums and maximums
    AATcos = analogRead(2);
    if (AATcos<AATcosmin) AATcosmin=AATcos;
    if (AATcos>AATcosmax) AATcosmax=AATcos;
    if (AATsin<AATsinmin) AATsinmin=AATsin;
    if (AATsin>AATsinmax) AATsinmax=AATsin;
    delay(1); //Sample every millisecond
}
}
void loop() {
AATsin = 2*analogRead(3)-AATsinmax-AATsinmin; //Read sensor and correct offset
AATcos = 2*analogRead(2)-AATcosmax-AATcosmin;
//Calculate angle; normalize for amplitude; scale for 0-59
angle = (atan2(float(AATsin)/(AATsinmax-AATsinmin),float(AATcos)/(AATcosmax-AATcosmin))/3.14159+1)*30; 
if (angle != angleOld) {
   dir = (angle > angleOld)^(abs(angle - angleOld) > 30); //XOR corrects for crossing 59/0 discontinuity
   strip.clear();
   strip.setPixelColor(angle, dir*255, !dir*255, 0); //Turn on LED; red or green indicates direction
   strip.show();
   angleOld = angle;
   stopCounter=0; //Reset stop counter
   arrowPos=0; //Reset arrow position
   }
   if (stopCounter > stopTime*repRate) { //"Idle" routine
    for(j=-1; j <=1; j+=2) {
     for(i=1; i < 15; i++) {
      arrowPixel = angle+i*j;
      arrowPixel += ((arrowPixel < 0) - (arrowPixel > 60))*60; //Adjust for overrunning
      arrowPixelBrightness = 15-arrowPos/(repRate/arrowSpeed)-i; //Brighter closer to sensor position to simulate arrow
      arrowPixelBrightness *= (arrowPixelBrightness > 0) && (arrowPixelBrightness < 4); //Clip range to 1-3
      arrowPixelBrightness = arrowPixelBrightness*arrowPixelBrightness*arrowPixelBrightness*9; //Cube (for nonlinear brightness) and scale
      strip.setPixelColor(arrowPixel, 0, 0, arrowPixelBrightness); //Animate arrow LEDs
     }
    }
    strip.show();  
    arrowPos++;
    if (arrowPos == repRate/arrowSpeed*12) arrowPos = 0; //Wrap arrow position at 11
   }
  else{
  stopCounter++; //Increment stop counter
  }
delay(1000/repRate);
  }
