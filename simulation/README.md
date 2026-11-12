# Simulation

Two ways to check the firmware logic **before** any hardware is assembled. Neither one proves anything about the motors, wiring, battery or sensor placement. Only the physical tests in `docs/testing-guide.md` do that.

## 1. Host logic tests (runs on your Mac in seconds)

`host-tests/` compiles the **real** firmware sources against a small mock of the Arduino core. Time, pins, the HC-SR04 echo, the servo, the battery voltage and the serial ports are all simulated. The tests include unit checks and whole-sketch scenarios: boot, manual driving, link-loss stop, collision guard, mode transitions, line following, obstacle pausing, scan-and-turn avoidance, boxed-in fault, low battery, emergency stop, and USB-only power.

```bash
xcode-select --install          # once, provides clang++ (skip if already installed)
cd simulation/host-tests
CXX=clang++ ./run_tests.sh
```

Expected last line: `92 passed, 0 failed`.

What this does **not** cover: AVR-specific compilation (do *Verify* in the Arduino IDE for that), real timing jitter, SoftwareSerial interrupt effects, electrical behaviour.

## 2. Wokwi (browser, visual)

`wokwi/diagram.json` wires a virtual UNO to:

| Real part | Stand-in in Wokwi |
|---|---|
| L298N inputs | 6 LEDs on D4–D8, D10 (ENA/ENB brightness = PWM) |
| HC-SR04 | Wokwi HC-SR04 (click it to change distance) |
| SG90 | Wokwi servo on D9 |
| 3× line sensors | 3 slide switches on A0–A2 (towards 5 V = on line) |
| Battery divider | Potentiometer on A5 (≈780 = 7.6 V; turn down to test low battery) |
| Joystick | Wokwi analog joystick on A3/A4 |
| HC-05 | Not available; type the same commands into Wokwi's Serial Monitor |

Steps:
1. Go to wokwi.com → *New Project* → *Arduino Uno*.
2. Replace the `sketch.ino` contents with `firmware/RobotCar/RobotCar.ino`.
3. Use the tab menu → *New file* to add every `.h` and `.cpp` file from `firmware/RobotCar/` with the same names.
4. Replace `diagram.json` with `simulation/wokwi/diagram.json`.
5. To try joystick mode, set `JOYSTICK_FITTED = true` in `Config.h`.
6. Press ▶. In the Serial Monitor type `T`, `O`, `M`, `F`, `S`, `E`, `?` and watch the LEDs and servo.

`diagram.json` was written by hand and has **not** been opened in Wokwi from this repo yet. If a part's pin name differs in the current Wokwi version, re-draw that wire in the editor.
