#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define OLED_ADDR   0x3C

#define INIT 0
#define EXPOSING 1
#define EXPOSED 2

#define UP 9
#define ENTER 8
#define DOWN 7

#define UV A7
#define VOLTAGE A1
#define BEEPER 11

int estimate_soc();
float get_UV_index();

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);

float UV_index, target_exposure, current_exposure;
int state = 0;
int cancel, up_held, down_held, blk, SoC;

void setup() {

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Welcome");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 17);
  display.println("This is");
  display.println("the Cyanotype");
  display.println("Exposure Helper");
  display.display();
  delay(2500);

  // Default target exposure
  target_exposure = 1100;


  // put your setup code here, to run once:
  pinMode(UV, INPUT_PULLUP);
  pinMode(VOLTAGE, INPUT_PULLUP);
  pinMode(BEEPER, OUTPUT);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(ENTER, INPUT_PULLUP);

  // Make sure you are using 5V as reference voltage
  analogReference(EXTERNAL);


}
void loop() {

  // Static display
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("CEH");
  UV_index = get_UV_index();
  switch (state)
  {
  // Pre Exposure Setup
  case INIT:
    // Handling Inputs

    // Increasing exposure
    if(!digitalRead(UP))
    {
      // Checking for long press
      up_held++;
      target_exposure += constrain(up_held > 10 ? 50 : 5, 0, INTMAX_MAX);
      delay(100);
    }
    else
    {
      up_held = 0;
    }

    // Decreasing exposure
    if(!digitalRead(DOWN))
    {
      // Checking for long press
      down_held++;
      target_exposure -= constrain(down_held > 10 ? 50 : 5, 0, INTMAX_MAX);
      delay(100);
    }
    else
    {
      down_held = 0;
    }

    // Starting Exposure
    if(!digitalRead(ENTER) && cancel >= 0)
    {
      current_exposure = 0;
      cancel = 0;
      state = EXPOSING;
      delay(100);
    }
    if(digitalRead(ENTER))
    {
      cancel = 0;
    }

    //Setting up Display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 17);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 17);
    
    display.print("UV Index: ");
    display.println(UV_index > 1. ? UV_index - 1 : 0);
    display.println("Target Exp: ");
    display.print(target_exposure);
    display.println(" [UVs]");
    break;

  // Exposing the Cyanotype
  case EXPOSING:

    current_exposure += UV_index / 2.;

    // Setting up Display
    display.println("!EXPOSING!");

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 34);

    display.print("UV Index: ");
    display.println(UV_index > 1. ? UV_index - 1 : 0);
    display.print("Exposed: ");
    display.print(constrain(100.0 * current_exposure / target_exposure, 0, 100));
    display.println("%");

    // Checking for cancellation
    if(!digitalRead(ENTER))
    {
      cancel++;
    }
    else
    {
      cancel = 0;
    }

    if(cancel > 6)
    {
      cancel = -1;
      state = INIT;
    }
    // Switching to next state
    if(current_exposure >= target_exposure)
    {
      blk = 0;
      state = EXPOSED;
    }
    delay(500);
    break;

  // All done! Sounding the alarm! 
  case EXPOSED:

    blk++;
    // BLINK ON!
    if(blk % 2)
    {
    // Setting up Display
    display.setCursor(0, 17);
    display.setTextSize(3);
    display.println("!DONE!");

    // Starting to beep!
    tone(BEEPER, 50);
    delay(500);
    }
    else
    //BLINK OFF
    {
    //Stopping the tone
    noTone(BEEPER);
    delay(250);
    }
    // Returning to INIT
    if(!digitalRead(ENTER))
    {
      state = INIT;
      noTone(BEEPER);
      cancel = -1;
      delay(50);
    }
    break;
  
  default:
    state = INIT;
    break;
  }
  display.setCursor(70, 55);
  display.setTextSize(1);
  display.print("Bat: ");
  display.print(estimate_soc());
  display.print("%");
  display.display();
}


int estimate_soc() {
    return constrain(map(analogRead(VOLTAGE), 613, 858, 0, 100), 0, 100);
}

float get_UV_index(){
  // I know that this does ignore the the -1 offset specified, but this sensor
  // actually reads 0V when there is no UV, so we would get a negative index
  // For display purpose I've applied the offset, but clamped clamped it to zero
  return analogRead(UV) * (50.0 / 1024.0);
}