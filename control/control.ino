#define DELAYTIME 40     // set delay time
#define BAUDRATE 115200  // setup baudrate
#define BOUNCER_DELAY 40
//------------------------- pin setup START -------------------------
#define button_1_pin 27
#define button_2_pin 26
#define button_3_pin 25
#define button_4_pin 22

#define switch_1_pin 12
#define switch_2_pin 14

#define JoyStick_1_X_pin 33
#define JoyStick_1_Y_pin 32
#define JoyStick_1_button_pin 23
#define JoyStick_2_X_pin 35
#define JoyStick_2_Y_pin 34
#define JoyStick_2_button_pin 19
//------------------------- pin setup END -------------------------

#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <Bounce2.h>

//------------------------- bouncer setup START -------------------------
Bounce button_1_bouncer = Bounce();
Bounce button_2_bouncer = Bounce();
Bounce button_3_bouncer = Bounce();
Bounce button_4_bouncer = Bounce();
Bounce switch_1_bouncer = Bounce();
Bounce switch_2_bouncer = Bounce();
//------------------------- bouncer setup z -------------------------

bool headlight_status;
bool brakelight_status;
bool foglight_status;
bool turn_left_light_status;
bool turn_right_light_status;
bool guard_light_status;

const uint64_t pipeOut = 0xE9E8F0F0E1LL;
RF24 radio(7, 8);

int switch_1_axis_1;
int switch_1_axis_2;
bool switch_1_switch;
int switch_2_axis_1;
int switch_2_axis_2;
bool switch_2_switch;

void setup() {
  radio.begin();
  radio.openWritingPipe(pipeOut);
  radio.stopListening();

  pinMode(button_1_pin, INPUT);
  pinMode(button_2_pin, INPUT);
  pinMode(button_3_pin, INPUT);
  pinMode(button_4_pin, INPUT);
  pinMode(switch_1_pin, INPUT);
  pinMode(switch_2_pin, INPUT);
  pinMode(JoyStick_1_X_pin, INPUT);
  pinMode(JoyStick_1_Y_pin, INPUT);
  pinMode(JoyStick_1_button_pin, INPUT);
  pinMode(JoyStick_2_X_pin, INPUT);
  pinMode(JoyStick_2_Y_pin, INPUT);
  pinMode(JoyStick_2_button_pin, INPUT);

  button_1_bouncer.attach(button_1_pin);
  button_1_bouncer.interval(BOUNCER_DELAY);
  button_2_bouncer.attach(button_2_pin);
  button_2_bouncer.interval(BOUNCER_DELAY);
  button_3_bouncer.attach(button_3_pin);
  button_3_bouncer.interval(BOUNCER_DELAY);
  button_4_bouncer.attach(button_4_pin);
  button_4_bouncer.interval(BOUNCER_DELAY);
  switch_1_bouncer.attach(JoyStick_1_button_pin);
  switch_1_bouncer.interval(BOUNCER_DELAY);
  switch_2_bouncer.attach(JoyStick_2_button_pin);
  switch_2_bouncer.interval(BOUNCER_DELAY);

  Serial.begin(115200);
}

int mapJoystickValues(int val, int lower, int middle, int upper) {
  val = constrain(val, lower, upper);
  if (val < middle)
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return 255 - val;
}

void loop() {
  // X, Y, light_1, light_2, light_3, light_4, light_5, light_6, light_7, brek
  int data[10];
  button_1_bouncer.update();
  button_2_bouncer.update();
  button_3_bouncer.update();
  button_4_bouncer.update();
  switch_1_bouncer.update();
  switch_2_bouncer.update();

  if (button_1_bouncer.fell()) {
    turn_left_light_status = true;
    turn_right_light_status = false;
    guard_light_status = false;
  }
  if (button_4_bouncer.fell()) {
    turn_left_light_status = false;
    turn_right_light_status = true;
    guard_light_status = false;
  }
  if (button_2_bouncer.fell()) {
    turn_left_light_status = false;
    turn_right_light_status = false;
    guard_light_status = true;
  }
  if (button_3_bouncer.fell()) {
    brakelight_status = true;
    data[9] = true;
  }
  if (button_3_bouncer.rose()) {
    brakelight_status = false;
    data[9] = false;
  }

  // 設定要傳送的資料
  data[0] = analog_deal(analogRead(JoyStick_1_X_pin));
  data[1] = analog_deal(analogRead(JoyStick_2_Y_pin));
  data[2] = turn_left_light_status;
  data[3] = turn_right_light_status;
  data[4] = guard_light_status;
  data[5] = headlight_status;
  data[6] = foglight_status;
  data[7] = brakelight_status;
  data[8] = false;// EMPTY

  Serial.println(String("") + data[0] + " " + data[1] + " " + data[2] + " " + data[3] + " " + data[4] + " " + data[5] + " " + data[6] + " " + data[7] + " " + data[8] + " " + data[9]);

  radio.write(&data, sizeof(data));
  delay(DELAYTIME);
}

int analog_deal(int value) {
  int map_value = map(analogRead(value), 0, 4095, 0, 255);
  if (map_value > 111 && map_value < 143) {
    // ignore noise
    return 127;
  } else {
    return map_value;
  }
}