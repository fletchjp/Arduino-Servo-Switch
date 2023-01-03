// SweepIOAbs
// converting to use IOAbstraction and a task manager.
// I wanted to use classes for moving up and down.
// I have not worked out how to do that.
// Instead I am using one task with this statement replacing a delay.
//        taskManager.yieldForMicros(15000);
// (c) John Fletcher (M6777)

 /* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>

#include <IoAbstraction.h>
#include <TaskManagerIO.h>
#include <ExecWithParameter.h>

#include <moustache.h>

// This returns the size of a moustache_variable_t array and also of an array of arrays.
// It can be used to control output from rendering the array of arrays
template <typename T, size_t n>
size_t moustache_size(T (&values)[n])
{
  return n;
}

// Define the formats to be used to output of position.
const char *position = "moveServo reaches {{pos}}";


Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards
const byte SERVO_PIN = 9;

int pos = 0;    // variable to store the servo position

const moustache_variable_t position_value[] = { { "pos", String(pos) } };
// This task has to coordinate the tasks and turn them on and off.
void moveServo()
{
    Serial.println("moveServo running");
    // Note that myservo and pos are global values.
    while(true) {
       for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
       // in steps of 1 degree
       myservo.write(pos);              // tell servo to go to position in variable 'pos'
       taskManager.yieldForMicros(15000);     
       //delay(15);                       // waits 15ms for the servo to reach the position
       }
        Serial.print("moveServo reaches ");
        Serial.println(pos);
       for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
          myservo.write(pos);              // tell servo to go to position in variable 'pos'
          taskManager.yieldForMicros(15000);     
          //delay(15);                       // waits 15ms for the servo to reach the position
       }
        Serial.print("moveServo reaches ");
        Serial.println(pos);
    }
}

void setup() {
  Serial.begin(115200);
  myservo.attach(SERVO_PIN);  // attaches the servo on pin 9 to the servo object
  taskManager.scheduleOnce(100,moveServo);
  Serial.println("moveServo scheduled");
}

void loop() {
  // Run IO_Abstraction tasks.
  // This replaces actions taken here in the previous version.
  taskManager.runLoop();

  /*
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  */
}
