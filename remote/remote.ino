#define BAUDRATE 115200  // setup baudrate
#define receive_timeout 1000
//------------------------- pin setup START -------------------------
#define motorA_1_pin 6
#define motorA_2_pin 5

#define servo_1_pin 3
#define servo_1_min 125
#define servo_1_mid 95
#define servo_1_max 65

// mode 0: on, 1: blink
#define light_1_pin 10// turn left light
#define light_1_mode 1

#define light_2_pin 9// turn right light
#define light_2_mode 1

#define light_3_pin 4// break light
#define light_3_mode 0

#define light_4_pin 14// head light
#define light_4_mode 0

#define light_5_pin 15// fog light
#define light_5_mode 0

#define light_6_pin 16// Empty
#define light_6_mode 0

#define light_7_pin 17// Empty
#define light_7_mode 0
//------------------------- pin setup END -------------------------

//------------------------- PWM channel setup START ------------------------
#define PWM_CH_1 0
#define PWM_CH_2 1
//------------------------- PWM channel setup END ------------------------

#include <RF24.h>
#include <SPI.h>
#include <Servo.h>
#include <nRF24L01.h>

int ch_width_1;
Servo servo_1;

const uint64_t pipeIn = 0xE9E8F0F0E1LL;
RF24 radio(7, 8);

unsigned long lastRecvTime = 0;

typedef struct struct_message {
  int X;
  int Y;
  bool light_1;
  bool light_2;
  bool light_3;
  bool light_4;
  bool light_5;
  bool light_6;
  bool light_7;
  bool brek;
} struct_message;
struct_message myData;

void setup() {
  servo_1.attach(servo_1_pin);

  pinMode(motorA_1_pin, OUTPUT);
  pinMode(motorA_2_pin, OUTPUT);
  pinMode(servo_1_pin, OUTPUT);
  pinMode(light_1_pin, OUTPUT);
  pinMode(light_2_pin, OUTPUT);
  pinMode(light_3_pin, OUTPUT);
  pinMode(light_4_pin, OUTPUT);
  pinMode(light_5_pin, OUTPUT);
  pinMode(light_6_pin, OUTPUT);
  pinMode(light_7_pin, OUTPUT);

  radio.begin();
  radio.openReadingPipe(1, pipeIn);

  init_data();

  Serial.begin(115200);

  radio.startListening();
}

void recvData() {
  while (radio.available()) {
    int data[10];
    radio.read(&data, sizeof(data));

    myData.X = data[0];
    myData.Y = data[1];
    myData.light_1 = data[2];// turn left light
    myData.light_2 = data[3];// turn right light
    myData.light_3 = data[4];// guard light
    myData.light_4 = data[7];// break light
    myData.light_5 = data[5];// head light
    myData.light_6 = data[6];// fog light
    myData.light_7 = data[8];// Empty
    myData.brek = data[9];

    Serial.println(String("") + data[0] + " " + data[1] + " " + data[2] + " " + data[3] + " " + data[4] + " " + data[5] + " " + data[6] + " " + data[7] + " " + data[8] + " " + data[9]);

    lastRecvTime = millis();
    // Serial.println('Recieved Data');
  }
}

void loop() {
  recvData();
  unsigned long now = millis();
  if (now - lastRecvTime > 1000) {
    Serial.println("Lost connection");
    init_data();
  }

  if (myData.brek) {
    digitalWrite(motorA_1_pin, HIGH);
    digitalWrite(motorA_2_pin, HIGH);
  } else {
    motor_control(motorA_1_pin, motorA_2_pin, myData.X);
  }

  servo_control(servo_1, myData.Y, servo_1_min, servo_1_mid, servo_1_max);

  // turn light and guard light
  if (myData.light_3) {
    light_control(light_1_pin, light_1_mode, true);
    light_control(light_2_pin, light_2_mode, true);
  } else {
    light_control(light_1_pin, light_1_mode, myData.light_1);
    light_control(light_2_pin, light_2_mode, myData.light_2);
  }

  light_control(light_3_pin, light_3_mode, myData.light_4);
  light_control(light_6_pin, light_6_mode, myData.light_5);
  light_control(light_5_pin, light_5_mode, myData.light_6);
  light_control(light_4_pin, light_4_mode, myData.light_7);// Empty
  light_control(light_7_pin, light_7_mode, false);// Empty
}

void motor_control(int pinA, int pinB, int value) {
  if (value == 127) {
    digitalWrite(motorA_1_pin, LOW);
    digitalWrite(motorA_2_pin, LOW);
  } else if (value > 127) {
    analogWrite(motorA_1_pin, (value - 127.5) * 2);
    digitalWrite(motorA_2_pin, LOW);
  } else {
    digitalWrite(motorA_1_pin, LOW);
    analogWrite(motorA_2_pin, (127.5 - value) * 2);
  }
}

void servo_control(Servo servo, int value, int min, int mid, int max) {
  if (value == 127) {
    servo.write(mid);
  } else if (value > 127) {
    servo.write(map(value, 127, 255, mid, max));
  } else {
    servo.write(map(value, 0, 127, min, mid));
  }
}

void light_control(int pin, int mode, boolean on) {
  if (on) {
    if (mode == 0) {
      digitalWrite(pin, HIGH);
    } else if (mode == 1) {
      digitalWrite(pin, millis() % 1000 < 500);
    }
  } else {
    digitalWrite(pin, LOW);
  }
}

void init_data() {
  myData.X = 127;
  myData.Y = 127;
  myData.light_1 = true;
  myData.light_2 = true;
  myData.light_3 = true;
  myData.light_4 = true;
  myData.light_5 = true;
  myData.light_6 = true;
  myData.light_7 = true;
}