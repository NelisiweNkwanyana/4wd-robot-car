# Troubleshooting

Start with the Serial Monitor (115200) and `?`. The status line shows the mode, fault, distance, line sensors, battery voltage and the actual motor PWM.

## Power and resets
| Symptom | Likely cause | Fix |
|---|---|---|
| Arduino resets (banner reprints) when motors start or reverse | Voltage dip / motor noise | 470 µF across L298N VS–GND, 100 nF on each motor, star grounds, fresh cells. Increase ramp time (lower `RAMP_STEP`) |
| Resets when the servo moves | Servo current on the 5 V rail | Separate 5 V regulator for the servo (shared GND). Or `SERVO_FITTED = false` |
| Nothing powers on | Fuse, switch, cell orientation, flat cells | Measure voltage after each part in the chain |
| L298N very hot | Stalled motors, or running near 2 A | Don't push against objects. Check wheels turn freely. Consider a TB6612FNG/DRV8833 upgrade |
| `bat=USB` while cells are in | Divider not connected to A5 or VBAT | Check the 10 k/10 k divider wiring |
| `LOW_BATTERY` with full cells | Wrong `BATTERY_CAL` or divider values | Run `07_battery_test`, set `BATTERY_CAL` |

## Motors
| Symptom | Likely cause | Fix |
|---|---|---|
| No motor moves, Serial shows non-zero `L=`/`R=` | ENA/ENB jumpers still on, motor power off, no common ground | Remove jumpers (D5/D6 drive EN), check VS voltage, join grounds |
| Motors hum but don't turn | PWM below the stall threshold, weak cells | Raise `PWM_MIN_MOVING`, charge cells |
| One side backwards | Wiring polarity | `INVERT_LEFT_MOTORS`/`INVERT_RIGHT_MOTORS` |
| Wheels on one side fight | One motor reversed within the pair | Swap that motor's two leads |
| Car drifts on "straight" | Motor mismatch (normal for TT motors) | Line mode corrects itself; for manual, accept it or add per-side trim later |
| Motors jerk on mode change | Expected | Every transition ramps to stop for `MODE_SETTLE_MS` |

## Line following
| Symptom | Likely cause | Fix |
|---|---|---|
| Sensors always `000` or `111` | Height wrong, pots mis-set, wrong polarity | 5–10 mm height; tune pots with `02_line_sensor_test`; `LINE_ACTIVE_LOW` |
| Overshoots curves and loses the line | Too fast | Lower `LINE_BASE_SPEED_PCT` and `LINE_OUTER_SPEED_PCT` |
| Wobbles left/right on straights | Corrections too strong | Raise `LINE_SOFT_INNER_PCT`, lower `LINE_OUTER_SPEED_PCT` |
| Turns the wrong way | Left/right sensor wires swapped | Swap A0 and A2 wires |
| `LINE_ALL_BLACK` on a dark floor | Floor reads as "line" | Use a white board/paper track |
| Works indoors, fails near windows | Sunlight saturates IR sensors | Shade the sensors or test away from direct sun |

## Ultrasonic / obstacle avoidance
| Symptom | Likely cause | Fix |
|---|---|---|
| `dist=ERR`, `ULTRASONIC` fault | Echo/Trig swapped, no 5 V, loose wire | Check D11 = Trig, D12 = Echo; run `03_ultrasonic_test` |
| Stops for nothing | Sensor sees the floor or the chassis | Tilt the sensor up slightly; make sure the bracket isn't in the beam |
| Hits angled or soft objects | Ultrasound reflects away or is absorbed | Known HC-SR04 limitation. Document it; slow down (`AVOID_CRUISE_PCT`) |
| Always turns the same way | Servo not moving, or left/right angles reversed | `04_servo_test`; swap `SERVO_LEFT_DEG`/`SERVO_RIGHT_DEG` |
| Turns too little or too much | Floor grip, battery level | Tune `AVOID_TURN_MS` |
| `BOXED_IN` too easily | Stop distance too big for the space | Lower `OBSTACLE_STOP_CM` (keep ≥ 15) |

## Bluetooth
| Symptom | Likely cause | Fix |
|---|---|---|
| Phone can't find HC-05 | iPhone (not supported), module not powered | Android, or switch to an HM-10; check VCC/GND |
| Connects but nothing happens | TXD/RXD swapped, wrong baud | HC-05 TXD → D2. Default data baud 9600 |
| Garbled characters | Baud mismatch, missing divider | `BT_BAUD` matches module; check the divider |
| Car moves in short bursts, then stops | App sends each key once (no repeat) | Different app, or raise `BT_TIMEOUT_MS` (slower failsafe) |
| Pressing a button switches modes | App sends mode letters | Check with `05_bluetooth_test` which characters it sends |
| Manual forward won't go | Collision guard: obstacle within 20 cm | Back away or turn; disable `MANUAL_COLLISION_GUARD` only for testing |

## Joystick
| Symptom | Likely cause | Fix |
|---|---|---|
| `K` refused | `JOYSTICK_FITTED = false`, or the stick was moved at power-on | Set it true; power on with the stick centred |
| Creeps when released | Centre outside dead-zone | Raise `JOY_DEADZONE` |
| Reversed axes | Module orientation | `JOY_INVERT_X` / `JOY_INVERT_Y` |
| `FAULT: JOYSTICK` | Loose wire (reads 0 or 1023) | Check the cable; that's the protection working |

## Upload
| Symptom | Fix |
|---|---|
| No serial port on the Mac | Data-capable cable; CH34x driver for clone boards |
| `avrdude: stk500_recv(): programmer is not responding` | Wrong port/board; something on D0/D1; try another cable |
| Compile error in the IDE | All files must sit in `firmware/RobotCar/`, folder name unchanged. Paste the error into an issue/notes |
