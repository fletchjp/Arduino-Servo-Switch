//////////////////////////////////////////////////////////////////////////////////
// switch2IOAbs3
// This version has a second IR detector to control the reverse move.
// Each action has an LED, red for forward and green for reverse.
// (c) John Fletcher (M6777) 2021 
//////////////////////////////////////////////////////////////////////////////////
// This uses the IR detectors to switch the servo.
// This is done such that a subsequent change of the IR input has no effect on what happens
// until the move is complete.
// In this version the reverse move is controlled by a different IR detector. 
// This is controlled by a series of lock operations.
// This could develop into a basis for an interlocking code.
//////////////////////////////////////////////////////////////////////////////////// 
// This currently starts with an initial jerk on the servo.
// It runs once when triggered as expected and resets the LED.
// The same applies to the reverse move.
//////////////////////////////////////////////////////////////////////////////////// 

// I am mounting this on a NANO with a support plate as it has plenty of ground pins.
// There are now so many wires there is a risk of the bare wire connections to the resistances
// touching.

// 3rd party libraries
#include <Streaming.h>
#include <IoAbstraction.h>
#include <TaskManagerIO.h>

const byte VER_MAJ  = 1;
const byte VER_MIN  = 0;
const byte VER_DETAIL = 0;

void heading()
{
  Serial << endl << endl << __FILE__ << endl;
  Serial << F("Ver: ") << VER_MAJ << F(".") << VER_MIN << F(".") << VER_DETAIL;
  Serial << F(" compiled on ") << __DATE__ << F(" at ") << __TIME__ << F(" using compiler ") << __cplusplus << endl;
}

IoAbstractionRef arduinoPins = ioUsingArduino();

///////////////////////////////////////////////////////////////////////////////////////////////////
// Hardware servo library
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <Servo.h>
Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards
const byte SERVO_PIN = 9;
// These are used for the class objects.
const int UP = 1;
const int DOWN = -1;

int pos = 0;    // global variable to store the servo position.
// Initial state - could become a class.
bool eventCompleted = false;
bool noTaskRunning = true;
bool isMoveupNext = true; // Not currently used

// The IR signal is pulled high and goes low when there is a detection.
const int Signal_Pin = 5;
const int IR_Pin = 6;
const int LED_Pin = 7;
// Pins for the second detector.
// I am leaving 2 and 10 to 13 clear for a CBUS connection.
const int Signal_Pin2 = 3;
const int IR_Pin2 = 4;
const int LED_Pin2 = 8;

// Task to move from start to finish with designated servo
// I am using BaseEvent to test being able to check finishing.
class MoveServoFromTo : public BaseEvent {
  private:
    int start;
    int finish;
    int delta;
    byte servo_pin;
    Servo &servo; // Reference can only be set in the constructor.
    bool completed;
  public:
    int taskId;
    MoveServoFromTo(Servo &thisservo,int star,int fin, int del, int pin)
       : start(star), finish(fin), delta(del) // pos(posn) 
       { 
          //Serial.print("MoveServo started with ");
          //Serial.println(pos);
          taskId = TASKMGR_INVALIDID;
          servo = thisservo;
          servo_pin = pin;
          completed = false;
          pos = start;
          setCompleted(false);
       }
    // Required for all BaseEvent classes.
    uint32_t timeOfNextCheck() override {
      //pos += delta;
      //Serial.println("Event Next Check");
      markTriggeredAndNotify();
      return 2000UL * 1000UL; // every 15 milliseconds we increment
    }
    // This is needed to reset the values correctly at restarted cases.
    void setStart() { 
          Serial.print("Event starting with position ");
          pos = start;
          Serial.println(pos); 
          completed = false;
          setCompleted(false);         
          //noTaskRunning = false;
    }
    // This is now called for each increment.
    void exec() override {
       //Serial.println(pos); 
       servo.write(pos);// tell servo to go to position in variable 'pos'
       pos += delta;
       delay(15);// This is needed here (or the delay to print pos).
       //taskManager.yieldForMicros(15000); This does NOT work here.
       // Test for completion depends on the sign of delta;
       if (delta > 0) {
          completed = (pos >= finish);
       } else {
          completed = (pos <= finish);
       }
       if (completed) {
          Serial.print("Event complete with position ");
          Serial.println(pos); 
          setCompleted(true);
          noTaskRunning = true;
       }
    } 
    bool isCompleted() { return completed; }
    bool isAtFinish()  { return pos == finish; }
    int getDelta()     { return delta; }
    ~MoveServoFromTo() override = default;
};

// This has to come after the definition of MoveServoFromTo
class CheckIRpins : public Executable {
   private:
     int sig_pin;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
     int led_pin;
     // These are now local to each instance.
     int ir_signal;
     int previous_signal;
     MoveServoFromTo &task;
   public:
     CheckIRpins (MoveServoFromTo &upordown,int s, int p) : sig_pin(s), led_pin(p),  task(upordown)
     // This says that the IR to off and so the first detection will be that it has come on.
     { previous_signal = 1;}
     void exec() override {
        ir_signal = ioDeviceDigitalReadS(arduinoPins, sig_pin);
        //Serial.println(ir_signal);
        // Only act if no task is running
        if (noTaskRunning && !task.isAtFinish() ) { 
          if (ir_signal != previous_signal) {
             previous_signal = ir_signal;
             ioDeviceDigitalWrite(arduinoPins, led_pin, !ir_signal);
             taskManager.registerEvent(&task);
             noTaskRunning = false;
             Serial.print("CheckIRpins: task is started with delta ");
             Serial.println(task.getDelta());
             eventCompleted = false;
          }
        } else {
          if (task.isComplete())
          {
            noTaskRunning = true;
            Serial.print("CheckIRpins: task is completed with delta ");
            Serial.println(task.getDelta());
            // Reset LED and start state
            previous_signal = 1;
            ioDeviceDigitalWrite(arduinoPins, led_pin, !previous_signal);
            task.setCompleted(false);
          }
        }
     }
};

// This task will move the servo position up
MoveServoFromTo moveup(myservo,0,180,UP,SERVO_PIN);
// This task will move the servo position down
MoveServoFromTo movedown(myservo,180,0,DOWN,SERVO_PIN);

// Class instance - this could be used to check different sets of pins.
CheckIRpins checkThesePins(moveup,Signal_Pin, LED_Pin);
CheckIRpins checkThesePins2(movedown,Signal_Pin2, LED_Pin2);

void setup() {
  while(!Serial);
  Serial.begin(115200);
  heading();
  Serial.println("Starting the servo two detector event example");
  ioDevicePinMode(arduinoPins, Signal_Pin, INPUT_PULLUP);
  ioDevicePinMode(arduinoPins, IR_Pin, OUTPUT);
  ioDevicePinMode(arduinoPins, LED_Pin, OUTPUT);

  ioDevicePinMode(arduinoPins, Signal_Pin2, INPUT_PULLUP);
  ioDevicePinMode(arduinoPins, IR_Pin2, OUTPUT);
  ioDevicePinMode(arduinoPins, LED_Pin2, OUTPUT);
  // It does not work unless the IR pins are set high.
  ioDeviceDigitalWrite(arduinoPins, IR_Pin, HIGH);
  ioDeviceDigitalWrite(arduinoPins, IR_Pin2, HIGH);
  // Flash each pin in sequence to check operation.
  ioDeviceDigitalWrite(arduinoPins, LED_Pin, HIGH);
  ioDeviceDigitalWrite(arduinoPins, LED_Pin2, LOW);
  delay(1000);
  ioDeviceDigitalWrite(arduinoPins, LED_Pin, LOW);
  ioDeviceDigitalWrite(arduinoPins, LED_Pin2, HIGH);
  delay(1000);
  ioDeviceDigitalWrite(arduinoPins, LED_Pin2, LOW);

  myservo.attach(SERVO_PIN);  // attaches the servo on pin 9 to the servo object
  moveup.setStart();

  // Class instance is scheduled - note the & before the name to indicate "address of"
  taskManager.scheduleFixedRate(250, &checkThesePins);
  taskManager.scheduleFixedRate(250, &checkThesePins2);

  // now we register the event with the task manager.
  //taskManager.registerEvent(&moveup);
  Serial.println("IR check registered");
}

void loop() {
  // Run IO_Abstraction tasks.
  // This replaces actions taken here in the previous version.
  taskManager.runLoop();

  // This checks for the completion of moveup and if it is starts movedown.
  if (noTaskRunning && moveup.isComplete() && !eventCompleted ) {
      Serial.println("moveup event is completed");
      eventCompleted = true; // Only print this once.
      //movedown.setStart();
      //taskManager.registerEvent(&movedown);
      //Serial.println("movedown event registered");
      //eventCompleted = false;
      //noTaskRunning = false; // This makes sure there is no trigger while movedown is running.
  }

  // This checks for the completion of movedown and //if it is starts moveup.  
  if (noTaskRunning && movedown.isComplete() && !eventCompleted ) {
      Serial.println("movedown event is completed");
       eventCompleted = true; // Only print this once.
      //noTaskRunning = true; now done inside the task.
      //moveup.setStart();
      //taskManager.registerEvent(&moveup);
      //Serial.println("moveup event registered");
      //eventCompleted = false;
  }
}
