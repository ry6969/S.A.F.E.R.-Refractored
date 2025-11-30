# S.A.F.E.R. - Smart Automated Fire Emergency Responder

An Arduino-based fire suppression system that automatically detects and responds to fire and smoke threats using dual servo-controlled water dispensers.

## Overview

S.A.F.E.R. is an intelligent fire safety system designed to detect flames and smoke, then automatically activate water-based suppression while providing visual and audible alerts. The system uses non-blocking code architecture for responsive, real-time operation.

## Hardware Components

- **Microcontroller**: Arduino Uno (ATmega328P)
- **Sensors**:
  - IR Flame Sensor (A1) - Detects infrared radiation from flames
  - MQ Gas/Smoke Sensor (A0) - Detects smoke and combustible gases
- **Actuators**:
  - 2x Servo Motors (Pins 9, 6) - Control water dispenser positioning
  - Water Pump (Pin 2, relay-controlled)
- **User Interface**:
  - 3x LED Indicators (Pins 5, 4, 3) - Blue, Red, Orange status lights
  - Buzzer (Pin 8) - Audio alerts

## Detection Systems

### Flame Detection

The system uses a **statistical flicker detection algorithm** to distinguish real flames from false positives:

1. **Initial Threshold Check**: Flame sensor reading must be below 1000 (indicating IR detection)
2. **Flicker Analysis**:
   - Collects 15 samples at 3ms intervals
   - Calculates standard deviation of readings
   - Threshold: SD > 10 indicates flickering flame
3. **Result**: Reduces false positives from LED reflections and static IR sources

**Key Feature**: ADC settling delay (50ms) between flame and smoke readings prevents analog crosstalk.

### Smoke Detection

- **Threshold-based detection**: Gas sensor reading > 280 triggers smoke alert
- **Continuous monitoring**: Checks every loop iteration
- **Combined threat handling**: System responds differently when both flame and smoke are detected

## Response Behaviors

### 1. Normal Operation (No Threat)
- **Servo 1**: Continuously sweeps 40° - 150° at 5° increments (30ms intervals)
- **Servo 2**: Positioned at center (100°)
- **Water Pump**: OFF (relay HIGH)
- **LEDs**: All off
- **Buzzer**: Silent

### 2. Flame Detected
- **Water Pump**: Activated immediately
- **Servo 2**: Sweeps ±20° around Servo 1's current position (30ms intervals)
  - Focuses water stream near detected flame location
- **Alert Pattern**:
  - Red LED: ON
  - Buzzer: Fast beeping (100ms intervals)
- **Duration**: 5-second targeted suppression window
- **Servo Precision**: 5° sweep increments for accurate targeting

### 3. Smoke Detected (No Flame)
- **Water Pump**: Activated immediately
- **Servo 2**: Wide sweep 40° - 150° (50ms intervals)
  - Covers entire area to suppress smoke source
- **Alert Pattern**:
  - Orange LED: ON
  - Buzzer: Slow beeping (250ms intervals)

### 4. Combined Threat (Flame + Smoke)
- **Water Pump**: Activated immediately
- **Servo 2**: Wide sweep 40° - 150° (50ms intervals)
- **Alert Pattern**:
  - Red + Orange LEDs: Both ON
  - Buzzer: Slow beeping (250ms intervals)

## Non-Blocking Architecture

The system uses **TimedAction** library for non-blocking timing:

```cpp
bool timedAction(unsigned long &prev, unsigned long interval, unsigned long now)
```

**Benefits**:
- Simultaneous servo sweeping and sensor monitoring
- Responsive buzzer patterns without `delay()` blocking
- Real-time multi-threat handling

**Key Timers**:
- `servo1_Timer` - Normal patrol sweep (30ms)
- `servo2_Timer` - Fire suppression sweep (30-50ms)
- `debug_Timer` - Serial status updates (250ms)

## State Management

The system maintains persistent state using reference parameters:

- `servo1_currentAngle` / `servo2_currentAngle` - Current servo positions
- `servo1_reachedMax` / `servo2_reachedMax` - Sweep direction flags
- Static variables in `buzzerAlert()` - Independent beep timers

## Safety Features

1. **Fail-safe pump control**: Relay defaults to HIGH (pump OFF) on startup
2. **ADC crosstalk prevention**: 50ms delay between different analog pin reads
3. **IR interference mitigation**: LED indicators commented out to prevent false flame triggers
4. **Center return**: Servo 2 returns to 100° when threats clear

## Building & Uploading

This project uses **PlatformIO** for dependency management and compilation.

### Prerequisites
- Visual Studio Code with PlatformIO extension
- Arduino framework libraries (auto-installed)

### Dependencies
- `Arduino.h` - Core Arduino framework
- `Servo.h` - Servo motor control
- `TimedAction.h` - Custom non-blocking timer (in `include/`)

### Build Commands
```bash
# Compile the project
pio run

# Upload to Arduino Uno
pio run --target upload

# Open serial monitor
pio device monitor
```

### Configuration
Project settings in `platformio.ini`:
- Platform: `atmelavr`
- Board: `uno`
- Framework: `arduino`

## Serial Debug Output

The system outputs status every 250ms:
```
Flame Value: 1023 Status: No Flame Detected
Smoke Value: 150 Status: No Smoke Detected
```

When threats detected:
```
Flame Value: 850 Status: Flame Detected!!!
Smoke Value: 320 Status: Smoke Dectected!!!
```

## Known Issues & Considerations

1. **LED IR Interference**: LED indicators (especially at night) can trigger the IR flame sensor. Tape the sides of the IR sensor to prevent this
2. **Flicker Threshold Tuning**: Current threshold (SD > 10) works for typical flames. May need adjustment for different environments.

## Author

Created by **Harry Canalita** for fire safety automation using PlatformIO and Arduino framework.

## License

**Copyright © 2025 Harry Canalita. All Rights Reserved.**

This project is the intellectual property of Harry Canalita. The code, design, and documentation are intended exclusively for the **S.A.F.E.R. (Smart Automated Fire Emergency Responder)** project submitted as a Final Project for **PHYSICS and STS** coursework.

**Use Restrictions**: This work may not be used, copied, modified, or distributed without the express written permission of the owner, Harry Canalita.
