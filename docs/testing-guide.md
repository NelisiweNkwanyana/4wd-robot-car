# Testing guide

Work top to bottom, and don't skip ahead: each test assumes the earlier ones passed. Fill in the **Result** column with what you actually observed. Until a row has a real result, it hasn't been tested.

Before any physical test, the logic tests should pass:
```bash
cd simulation/host-tests && CXX=clang++ ./run_tests.sh     # expect: 92 passed, 0 failed
```
These prove the decision logic only, not the hardware.

**Safety for every test:** wheels off the ground until test 4.3, fingers clear of the wheels, switch within reach, one hand near the switch the first time anything moves.

---

## 1. Power
| # | Test | How | Pass when | Result |
|---|---|---|---|---|
| 1.1 | Pack voltage | Multimeter across fresh cells | 8.0–8.4 V | |
| 1.2 | Common ground | Continuity battery− ↔ Arduino GND ↔ L298N GND | Beeps | |
| 1.3 | No 5 V back-feed | Continuity L298N 5V ↔ Arduino 5V | **No** beep | |
| 1.4 | Logic rail | Switch ON, measure Arduino 5V–GND | 4.8–5.1 V | |
| 1.5 | Battery sense | `07_battery_test` vs multimeter | Within 0.1 V after setting `BATTERY_CAL` | |

## 2. Motors (`01_motor_test`, wheels OFF the ground)
| # | Test | Pass when | Result |
|---|---|---|---|
| 2.1 | `1` left forward | Both left wheels spin **forward**, right side still | |
| 2.2 | `2` left backward | Both left wheels spin backward | |
| 2.3 | `3` / `4` right side | Same for the right side | |
| 2.4 | Same-side agreement | The two wheels on a side never fight each other | |
| 2.5 | Minimum PWM | Lower with `-` until a side stops. Record → `PWM_MIN_MOVING` (add ~10) | Value: |
| 2.6 | Driver temperature | 1 min of `5` at full speed: L298N heatsink warm, not too hot to touch | |
| 2.7 | Reset check | Rapid `5`/`6` switching: the "Motor test" banner must **not** reprint (a reprint means a brown-out reset) | |

Wrong direction → set `INVERT_LEFT_MOTORS` / `INVERT_RIGHT_MOTORS` in `Config.h`.

## 3. Sensor calibration
| # | Test | Sketch | Pass when | Result |
|---|---|---|---|---|
| 3.1 | Line sensors, white | `02_line_sensor_test` | All `0` over white (adjust pots) | |
| 3.2 | Line sensors, tape | same | Each prints `1` when its sensor is over the tape | |
| 3.3 | Polarity | same | If black reads `0`, set `LINE_ACTIVE_LOW = true` | |
| 3.4 | Ultrasonic accuracy | `03_ultrasonic_test` | 10/20/50 cm within ±2 cm | |
| 3.5 | Ultrasonic fault | Unplug ECHO | Prints `NO-ECHO` | |
| 3.6 | Servo centre | `04_servo_test` | Sensor faces straight ahead at 90°; no resets | |
| 3.7 | Joystick (if fitted) | `06_joystick_test` | Centre 450–575; forward increases Y; right increases X | |

## 4. Main firmware — bench (USB Serial Monitor, wheels OFF the ground)
Upload `firmware/RobotCar`. Serial Monitor 115200, *No line ending*.
| # | Test | Pass when | Result |
|---|---|---|---|
| 4.1 | Boot | Banner, `MODE -> IDLE`, LED 1 blink / 2 s | |
| 4.2 | Status | `?` prints a sensible status line | |
| 4.3 | Manual from Serial | Type `F` repeatedly (fast): wheels forward. Stop typing: wheels stop within ~0.6 s + ramp | |
| 4.4 | Emergency stop | `T` with centre sensor over tape (wheels spin), then `E`: wheels stop immediately, IDLE | |
| 4.5 | Mode transition | Manual forward, then `O`: wheels ramp down to stop before the new mode starts | |
| 4.6 | Fault + clear | Lift sensors off the tape in `T` mode: after ~1.5 s `FAULT: LINE_LOST`, LED flicker; `S` → IDLE | |

## 5. Line following (on the floor)
Track: 19 mm black electrical tape on white board or paper. Start with a large oval, curves of at least 30 cm radius.
| # | Test | Pass when | Result |
|---|---|---|---|
| 5.1 | Straight | Follows 1 m straight line without leaving it | |
| 5.2 | Gentle curve | Completes the oval 3 laps | |
| 5.3 | Tight curve | Note the smallest radius it handles | Radius: |
| 5.4 | Gap | 3 cm gap in the tape: continues | |
| 5.5 | Line end | Tape ends: searches, then stops with `LINE_LOST` | |
| 5.6 | Obstacle on line | Box on the track: stops before it; resumes ~0.5 s after removal | |
| 5.7 | Lifted | Pick the car up: `LINE_ALL_BLACK` fault, wheels stop | |

Tuning: oscillating → lower `LINE_BASE_SPEED_PCT` / `LINE_OUTER_SPEED_PCT`; runs wide on curves → make `LINE_HARD_INNER_PCT` more negative.

## 6. Obstacle avoidance (on the floor)
| # | Test | Pass when | Result |
|---|---|---|---|
| 6.1 | Stop distance | Drives at a wall, stops ≈`OBSTACLE_STOP_CM` (20 cm) away. Measure: | |
| 6.2 | Slow zone | Slows between 40 and 20 cm | |
| 6.3 | Scan | Backs off, looks left, looks right | |
| 6.4 | Choice | With a wall on one side only, turns to the open side (5 tries) | /5 |
| 6.5 | Turn angle | Measure the turn; adjust `AVOID_TURN_MS` for ~90° | ms: |
| 6.6 | Boxed in | In a box open only behind: `BOXED_IN` fault after retries | |
| 6.7 | Soft obstacles | Try a cushion or curtain (ultrasound absorbers); note the result honestly | |

## 7. Bluetooth
| # | Test | Pass when | Result |
|---|---|---|---|
| 7.1 | Pair | HC-05 pairs, LED double-blinks when connected | |
| 7.2 | App behaviour | `05_bluetooth_test`: key repeats while held (record interval) | ms: |
| 7.3 | Drive | All 8 directions in manual mode | |
| 7.4 | Speed | `3` vs `9` clearly different | |
| 7.5 | Mode select | `T`, `O`, `M` from the phone switch modes, and the phone receives `MODE …` | |
| 7.6 | **Link loss** | Drive forward, then switch the phone's Bluetooth OFF: car stops within ~0.6 s | |
| 7.7 | Range | Distance at which control becomes unreliable | m: |
| 7.8 | Collision guard | In manual, drive at a wall: stops at ~20 cm; reverse still works | |

## 8. Joystick (if fitted)
| # | Test | Pass when | Result |
|---|---|---|---|
| 8.1 | Calibration | Power on with stick centred: no calibration warning | |
| 8.2 | Neutral stop | Release the stick: car stops | |
| 8.3 | Proportional | Half deflection is visibly slower than full | |
| 8.4 | Directions | Forward/back/left/right correct (else `JOY_INVERT_X/Y`) | |
| 8.5 | Invalid input | Unplug the VRy wire while driving: `FAULT: JOYSTICK`, stop | |

## 9. Emergency stopping (repeat after any firmware change)
| # | Test | Pass when | Result |
|---|---|---|---|
| 9.1 | `E` from each mode | Immediate stop in LINE, OBSTACLE, MANUAL, JOYSTICK | |
| 9.2 | `S` from autonomous modes | Ramped stop, back to IDLE | |
| 9.3 | Power switch | Switching off stops everything instantly | |
| 9.4 | Reset button | Pressing RESET stops motors; boots into IDLE (never straight into a moving mode) | |

## 10. Battery performance
| # | Test | Result |
|---|---|---|
| 10.1 | Run time, line-following loop, from full charge to `LOW_BATTERY` fault | min |
| 10.2 | Voltage at fault (multimeter, resting after 1 min) | V |
| 10.3 | Cell temperature after the run (should be no more than warm) | |
| 10.4 | Behaviour as voltage drops: does line following get worse? (Lower voltage = slower motors, same PWM) | |
