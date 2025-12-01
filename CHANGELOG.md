#CHANGELOG
## [v2.1] - 2025-12-1

### Modifications:
- ***main.cpp***:
	- Changed the sensor reading variables in the loop field into the global field
	- Sensor reading variables are now set to 0 at initiation
	- Reads the sensor values first to determine if the flame is not present
	- If no flame and smoke is present, move the servo1 and read the sensor value again. It is contained in a while loop to ensure that the sensor reading only happens after the servo has been moved
	
```cpp
flameLevel = readFlameLevel();
flameDetected = isFlamePresent(flameLevel);

smokeLevel = readSmokeLevel();
smokeDetected = isSmokePresent(smokeLevel);
```

```cpp
  //detect flame & smoke
  if (!flameDetected && !smokeDetected){
    while (timedAction(servo1_Timer, 100, now)){
      sweepServo(servo1, servo1_currentAngle, servo1_reachedMax, minAngle, maxAngle, 5);
      flameLevel = readFlameLevel();
      flameDetected = isFlamePresent(flameLevel);
    }

    flameSensor_Timer = servo1_Timer;

    // Return to the middle before turning off the pump
    servo2.write(100);
    digitalWrite(PUMP_RELAY_PIN, HIGH);
    LedOff('R');
    LedOff('O');
    digitalWrite(BUZZER_PIN, LOW);
  } 
```

## [v2.0] - 2025-11-30

### Added
- ***TimedAction.h***:
	- A library used for task handling and timing by validating timer values 

### Modifications
- ***main.cpp***:
	- Previously named "sourceCode.h"
	- Refractored the sourCode.cpp to use proper timing alogrithm

## [v1.5] - 2025-11-22

### Added
- waterCode: for manually turning on water pump (tester code)
- testCode: for recording data on test period (tester code)
- sweepcode: prototype code for servo2 sweep (prototype code)

### Modifications
- sourceCode: 
	- Used flicker sensing algorithm to differentiate flames from ambient light
	- Implemented prototype version of sweepCode [requires fixing]
	
