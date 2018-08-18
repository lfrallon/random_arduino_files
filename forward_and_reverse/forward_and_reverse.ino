#include <address.h>
#include <confdescparser.h>
#include <hid.h>
#include <hidboot.h>
#include <hidusagestr.h>
#include <KeyboardController.h>
#include <MouseController.h>
#include <parsetools.h>
#include <Usb.h>
#include <usb_ch9.h>

#include <TimerOne.h>

// Arduino RBD Motor Library v2.1.1 Example - Spin a motor forward and reverse with events.
// https://github.com/alextaujenis/RBD_Motor
// Copyright 2016 Alex Taujenis
// MIT License

#include <RBD_Timer.h> // https://github.com/alextaujenis/RBD_Timer
#include <RBD_Motor.h> // https://github.com/alextaujenis/RBD_Motor

// motor shield
RBD::Motor motor(2, 11, 8); // pwm pin, forward pin, reverse pin

void setup() {
  motor.rampUp(5000);
}

void loop() {
  motor.update();

  if(motor.onTargetSpeed()) {
    if(motor.isOn()) {
      motor.rampDown(3000);
    }
    else {
      motor.toggleDirection();
      motor.rampUp(5000);
    }
  }
}
