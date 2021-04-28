#include "encoder.h"

// ENCODER DRIVER
//****************
// enc_init()   -> Initialize on_but pins
// enc_Read()   -> 0: encoder not changed
//              -> 1: encoder clockwise
//              -> 2: encoder counter clockwise
//              -> 3: encoder clockwise fast
//              -> 4: encoder counter clockwise fast
//              -> 5: encoder push button pressed (state have change)
//              -> 6: encoder push button release (state have change)

int enc_but_curr_state = digitalRead(enc_SW);


static int last_read;
static int A_State;
static int A_previous;
static int B_previous;
static long t_deb_A;
static long t_deb_B;
static long t_fast;
static int fast_count;

static int but_previous;
static long t_deb;

void enc_init()
{
  afio_cfg_debug_ports(AFIO_DEBUG_NONE);
  pinMode(enc_A, INPUT);
  pinMode(enc_B, INPUT);
  pinMode(enc_SW, INPUT_PULLUP);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  enc_set_led_color(BLUE);

  last_read=0;
  A_State=digitalRead(enc_A);
  A_previous=digitalRead(enc_A);
  B_previous=digitalRead(enc_B);
  t_deb_A=millis();
  t_deb_B=millis();
  t_fast=millis();
  fast_count=0;

  but_previous=digitalRead(enc_SW);
  t_deb=millis();
}

int enc_read()
{
  int but=enc_but_read();
  int enc=enc_wheel_read();

  if (but==1) return 5;         // enc button pressed
  if (but==2) return 6;         // enc button released
  return enc;
}

int enc_but_read()
{
  int but=digitalRead(enc_SW);
  long t,td;

  if (but!=but_previous)       // signal stable?
  {
    t_deb=millis();
    but_previous=but;
    return 0;
  }

  t=millis();                 // signal stable on_but_debouncing_ms?
  td=t-t_deb;
  if ((td>=0)&&(td<enc_but_deboucing_ms))
    return 0;

  if (but==enc_but_curr_state)// signal NOT changed
    return 0;

  enc_but_curr_state=but;     // signal have changed
  if (enc_but_state()) return 1;
  return 2;
}


bool enc_but_state()
{
  return enc_but_curr_state==LOW;
}


int enc_wheel_read()
{
  int A=digitalRead(enc_A);
  int B=digitalRead(enc_B);
  long t,td;
  bool debouncing=false;
  bool fast;

  if (A!=A_previous)          // signal enc_A stable?
  {
    t_deb_A=millis();
    A_previous=A;
    debouncing=true;
  }

  if (B!=B_previous)          // signal enc_B stable?
  {
    t_deb_B=millis();
    B_previous=B;
    debouncing=true;
  }

  if (debouncing) return 0;

  t=millis();                 // signals stable enc_deboucing_ms?
  td=t-t_deb_A;
  if ((td>=0)&&(td<enc_but_deboucing_ms))
    return 0;
  td=t-t_deb_B;
  if ((td>=0)&&(td<enc_but_deboucing_ms))
    return 0;

  if (A==A_State)             // signal enc_A NOT changed
    return 0;
  A_State=A;                  // signal enc_A have changed

  td=t-t_fast;                // wheel change speed
  t_fast=t;
  if ((td>=0) && (td<=enc_fast_ms)) fast=true;
  else fast=false;

  if ((A==B) && (fast==false))
  {
    last_read=1;
    fast_count=0;
  }
  if ((A!=B) && (fast==false))
  {
    last_read=2;
    fast_count=0;
  }
  if ((A==B) && (fast==true) && (last_read!=2)&& (last_read!=4))
  {
    if (++fast_count>4) last_read = 1; //last_read=3;
  }
  if ((A!=B) && (fast==true) && (last_read!=1)&& (last_read!=3))
  {
    if (++fast_count>4) last_read = 2;//last_read=4;
  }
  
  return last_read;
}


void enc_set_led_color(uint8_t color)
{
  int ledOnPin;
  int ledOffPin1;
  int ledOffPin2;

  switch(color)
  {
    case RED:
      ledOnPin = LED_RED;
      ledOffPin1 = LED_GREEN;
      ledOffPin2 = LED_BLUE;
      break;
    case GREEN:
      ledOnPin = LED_GREEN;
      ledOffPin1 = LED_RED;
      ledOffPin2 = LED_BLUE;
      break;
    case BLUE:
      ledOnPin = LED_BLUE;
      ledOffPin1 = LED_GREEN;
      ledOffPin2 = LED_RED;
      break;
    default:
      ledOnPin = LED_BLUE;
      ledOffPin1 = LED_GREEN;
      ledOffPin2 = LED_RED;
      break;
  }

  digitalWrite(ledOnPin, HIGH);
  digitalWrite(ledOffPin1, LOW);
  digitalWrite(ledOffPin2, LOW);
}