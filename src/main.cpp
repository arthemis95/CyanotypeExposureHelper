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

int estimate_soc();

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
  delay(5000);

  // Default target exposure
  target_exposure = 3000;


  // put your setup code here, to run once:
  pinMode(UV, INPUT_PULLUP);
  pinMode(VOLTAGE, INPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(ENTER, INPUT_PULLUP);

  digitalWrite(12, LOW);

}
void loop() {

  // Static display
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("CEH");
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
      target_exposure += constrain(up_held > 20 ? 100 : 10, 0, INTMAX_MAX);
      delay(50);
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
      target_exposure -= constrain(down_held > 20 ? 100 : 10, 0, INTMAX_MAX);
      delay(50);
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
      delay(50);
    }
    else
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
    
    UV_index = analogRead(A7) * (50.0 / 1023.0);
    display.print("UV Index: ");
    display.println(UV_index - 1);
    display.print("Target Exp: ");
    display.print(target_exposure/2);
    display.println(" UVs");
    break;

  // Exposing the Cyanotype
  case EXPOSING:

    // Measuring UV
    UV_index = analogRead(A7) * (50.0 / 1023.0);
    current_exposure += UV_index;

    // Setting up Display
    display.println("!EXPOSING!");

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 34);

    display.print("UV Index: ");
    display.println(UV_index - 1);
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

    // BLINK ON!
    if(blk % 2)
    {
    // Setting up Display
    display.setCursor(0, 17);
    display.setTextSize(3);
    display.println("!DONE!");

    // Starting to beep!
    tone(11, 50);
    delay(500);
    }
    else
    //BLINK OFF
    {
    //Stopping the tone
    noTone(11);
    delay(250);
    }
    // Returning to INIT
    if(!digitalRead(ENTER))
    {
      state = INIT;
      cancel = -1;
      delay(50);
    }
    break;
  
  default:
    state = INIT;
    break;
  }
  display.setCursor(75, 55);
  display.setTextSize(1);
  display.print("Bat: ");
  display.print(estimate_soc());
  display.print("%");
  display.display();
}


int estimate_soc() {
    return constrain(map(analogRead(VOLTAGE), 613, 858, 0, 100), 0, 100);
}