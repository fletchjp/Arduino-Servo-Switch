# Arduino-Servo-Switch
 
 This has the codes for the Servo examples including switching between tasks.

## Introduction

These codes were put together to explore the best way to control servos.
Example use is illustrated on the Arduino Hardware page on the Knowledgebase.

https://www.merg.org.uk/merg_wiki/doku.php?id=arduino:hardware

These codes use the IO_Abstraction library for task scheduling.

https://www.merg.org.uk/merg_wiki/doku.php?id=arduino:codeing:scheduling

## Sweep

These examples are derived from a code on the Arduino site which shows how to make a servo sweep from one end of its range to the other and back again. This code can be obtained here:

 http://www.arduino.cc/en/Tutorial/Sweep

 This uses the hardware servo library <servo.h> which will be used throughout these examples.

 ## Sweep with scheduling

 sweepIOabs

 This example uses the simple callback as in the case TCRT5000_IOAbs_Callback in Arduino-IR-Task. The callback handles the switching between moving the servo up and down.

 ## Sweep with switching

 The examples are extended to include switching when a signal is received from an IR detector.

 switchIOAbs3

 In this case there is one detector. When a signal is received the servo moves and while it is moving no further signal is processed. When the move is finished the servo returns to the start.

 This features a more complex use of task scheduling where the movement of the servo is controlled from a task which can be switched on and off.

switch2IOAbs3

In this case there are two IR detectors and each controls the move in one of the directions.

## Simple example of switching

diceEvent3

This is a simple example of the switching. It simulates a dice being rolled and checks for a particular result (3). When 10 successes have been achieved it stops.

This is included to help with understanding the operation of the other switching examples.

## Requirements

All of these examples will run on an Arduino UNO. I have in fact used a NANO and a backplane which has more ground pins available.
There is a need for a TCRT5000 IR detector and a servo and connecting wiring.

The Task scheduling examples require the library IO_Abstraction which is available from the Arduino IDE.

Library documentation is available from the links on the Knowledgebase.

## Location

The local files are located at E:\GitHub\Arduino\Arduino-Servo-Switch

**John Fletcher (M6777)**


