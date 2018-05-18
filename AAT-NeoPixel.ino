/**************************************************************************************
NVE AAT003 angle sensor connected to a NeoPixel 60-LED circular array via an ATtiny85 
or Arduino Trinket (select AdaFruit Trinket (ATtiny85 @ 8 MHz). 
Sensor "SIN" output to PB3; "COS" to PB4; output to NeoPixel Array on PB2.  Rev. 4/4/18
**************************************************************************************/
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 2, NEO_GRB + NEO_KHZ800);
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

void setup() {
  clock_prescale_set(clock_div_1); //Full speed internal clock (needed for array interface)
  strip.begin();
}
void loop() {
//Calculate angle indicated by AAT003 sensor; scale for 60 divisions (0-59)
    angle = (atan2(float(analogRead(3)-512),float(analogRead(2)-512))/3.14159+1.0)*30.0; 
    if (angle != angleOld) {
      dir = (angle > angleOld)^(abs(angle - angleOld) > 30); //XOR corrects for crossing 59/0 discontinuity
      strip.clear();
      strip.setPixelColor(angleOld, 0, 0, 0); //Turn on LED; red or green indicates direction
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
