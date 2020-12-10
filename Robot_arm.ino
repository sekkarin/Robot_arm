#include "Wire.h"
#include "Adafruit_PWMServoDriver.h"

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

int pulseWidth(int angle);
int check_value(int value[]);

#define MIN_PULSE_WIDTH 600
#define MAX_PULSE_WIDTH 2600
#define FREQUENCY 100

#define JOIN_1  0 // min 0 max 180
#define JOIN_2  1 // min 56 max 170
#define JOIN_3  2 // min 20 max 105
#define GRIPPER 3 // min 0 max 98

const int variable_Resistor_1 = A0;
const int variable_Resistor_2 = A1;
const int variable_Resistor_3 = A2;
const int variable_Resistor_4 = A3;

const int sw_1 = A6;
const int sw_2 = A7;

int cout_sw_1 ;
int status_sw2 ;

int State = 0;

int vr_1_angle ;
int vr_2_angle ;
int vr_3_angle ;
int vr_4_angle ;

int servoGo[4][8] = {
  {1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1}
};

int Old_value[4] = {90, 90, 90, 0};
int Step[4] = {0, 0, 0, 0};
int dlms[8] = {0, 0, 0, 0, 0 , 0, 0, 0};
int Delay_ready[8] = {1, 1, 1, 1, 0, 0, 0, 0};

void setup() {
  pinMode(sw_1, INPUT);
  pinMode(sw_2, INPUT);
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(FREQUENCY);
  pwm.setPWM(JOIN_1, 0, pulseWidth(90));
  pwm.setPWM(JOIN_2, 0, pulseWidth(90));
  pwm.setPWM(JOIN_3, 0, pulseWidth(90));
  pwm.setPWM(GRIPPER, 0, pulseWidth(0));
}
void loop() {

  while (State == 0)
  {
    vr_1_angle = map(analogRead(variable_Resistor_1), 0, 1023, 0, 180);
    vr_2_angle = map(analogRead(variable_Resistor_2), 0, 1023, 35, 170);
    vr_3_angle = map(analogRead(variable_Resistor_3), 0, 1023, 20, 105);
    vr_4_angle = map(analogRead(variable_Resistor_4), 0, 1023, 0, 120);

    pwm.setPWM(JOIN_1, 0, pulseWidth(vr_1_angle));
    pwm.setPWM(JOIN_2, 0, pulseWidth(vr_2_angle));
    pwm.setPWM(JOIN_3, 0, pulseWidth(vr_3_angle));
    pwm.setPWM(GRIPPER, 0, pulseWidth(vr_4_angle));

    if (analogRead(sw_1) > 900) {
      delay(500);
      if (cout_sw_1 < 8) {
        servoGo[0][cout_sw_1] = vr_1_angle ;
        servoGo[1][cout_sw_1] = vr_2_angle ;
        servoGo[2][cout_sw_1] = vr_3_angle ;
        servoGo[3][cout_sw_1] = vr_4_angle ;
        cout_sw_1 ++;
      }
      else
        cout_sw_1 = 0;
      }
    if (analogRead(sw_2) > 900) {
      State = 1;
      for(int i = 0 ; i<4 ; i++)
      Step[i] = 0;
      delay(500);
    }

  }

  while(State == 1)
  {
  goDelay();

  
  pwm.setPWM(JOIN_1, 0, pulseWidth(servoDrive(0, 5, 200)));
  pwm.setPWM(JOIN_2, 0, pulseWidth(servoDrive(1, 5, 200)));
  pwm.setPWM(JOIN_3, 0, pulseWidth(servoDrive(2, 5, 200)));
  pwm.setPWM(GRIPPER, 0, pulseWidth(servoDrive(3, 1, 200)));

  }

} //end loop

int pulseWidth(int angle) {
  int pulse_wide, analog_value;
  pulse_wide = map(angle, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
  analog_value = int(float(pulse_wide) / 1000000 * FREQUENCY * 4096);
  return analog_value;
} // end

void goDelay() {  //Global Delay For servoDrive
  delay(1);
  for (int i = 0; i < 8; i++)
  {
    if (Delay_ready[i] == 1)
    {
      if (dlms[i] > 0 )
        dlms[i]--;
    }
  }
}

int servoDrive(int sch, int Delay, int stepDelay) { // Channel Servo(0,1,2,3)  |  Speed Servo  |  Delay per step{ //1 , 20 , 10
  if (dlms[sch] == 0) {
    dlms[sch] = Delay;
    if (Old_value[sch] > servoGo[sch][Step[sch]]) Old_value[sch] --;
    else if (Old_value[sch] < servoGo[sch][Step[sch]]) Old_value[sch] ++;

    if (Old_value[sch] == servoGo[sch][Step[sch]] && Delay_ready[sch + 4] == 0)
    {
      dlms[sch + 4] = stepDelay;
      Delay_ready[sch + 4] = 1;
    }
    if (dlms[sch + 4] == 0 && Delay_ready[sch + 4] == 1)
    {
      Step[sch] ++;
      Delay_ready[sch + 4] = 0;
    }
    if (Step[sch] >= 8)
      Step[sch] = 0;
  }
  return Old_value[sch];

}
