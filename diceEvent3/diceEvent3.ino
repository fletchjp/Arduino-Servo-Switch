// diceEvent3
// Can I detect that the event has shut down?
// Yes I have done that in loop()
// (c) John Fletcher (M6777) 2021 

// This is the diceEvent part from the TaskManagerIO example eventHandling.

#include <TaskManagerIO.h>

int successes = 0;
const int SUCCESSMAX = 10;
bool eventCompleted = false;

/**
 * An event that extends BaseEvent allows for event driven programming, either notified by polling, interrupt or
 * another thread. There are two important methods that you need to implement, timeOfNextCheck that allows for polling
 * events, where you do the check in that method and trigger the event calling setTriggered(). Alternatively, like
 * this event, another thread or interrupt can trigger the event, in which case you call markTriggeredAndNotify() which
 * wakes up task manager. When the event is triggered, is exec() method will be called.
 */
class DiceEvent : public BaseEvent {
private:
    volatile int diceValue;
    const int desiredValue;
    static const uint32_t NEXT_CHECK_INTERVAL = 60UL * 1000000UL; // 60 seconds away, maximum is about 1 hour.
public:
    DiceEvent(int desired) : desiredValue(desired) {
        diceValue = 0;
    }

    /**
     * Here we tell task manager when we wish to be checked upon again, to see if we should execute. In polling events
     * we'd do our check here, and mark it as triggered if our condition was met, here instead we just tell task manager
     * not to call us for 60 seconds at a go
     * @return the time to the next check
     */
    uint32_t timeOfNextCheck() override {
        // simulate rolling the dice
        diceValue = (rand() % 7);

        if(diceValue == desiredValue) {
            markTriggeredAndNotify();
        }

        return 250UL * 1000UL; // every 100 milliseconds we roll the dice
    }

    /**
     * This is called when the event is triggered. We just log something here
     */
    void exec() override {
        Serial.print("Dice face matched with ");
        Serial.print(diceValue);
        Serial.print(" for this number of times ");
        successes++;
        Serial.println(successes);
        if (successes >= SUCCESSMAX) {
          Serial.println("That is enough of that");
          setCompleted(true);
        }
    }

    /**
     * We should always provide a destructor.
     */
    ~DiceEvent() override = default;
} diceEvent(3);


void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("Starting the event example");

    // now we register the event with the task manager.
    taskManager.registerEvent(&diceEvent);
}

void loop() {
  // put your main code here, to run repeatedly:
    taskManager.runLoop();

    if (!eventCompleted && diceEvent.isComplete() ) {
      Serial.println("Event is completed");
      eventCompleted = true; // Only print this once.
    }
}
