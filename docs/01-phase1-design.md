# Phase 1 — System Design

Status: **design.** Nothing in this document has been tested on hardware yet.

> **Revision (phase 2+):** mode keys changed from `x y z j` to `T O M K` (some RC apps send `X/x` for their light buttons), `E` added as emergency stop, and module names updated to match the code (§5, §6). Pin plan unchanged.

Reference kit: *Robocentrix 4WD Obstacle Avoidance Smart Robot Car Kit* (RoboFactory, R595 at time of writing). The kit contents below are taken from the listing photos. Photos on kit listings are often generic, so **check the box against this list when it arrives**. The listing itself says batteries, charger and the Bluetooth module are sold separately, even though an HC-05 appears in the parts photo.

---

## 1. Hardware architecture

```
                       ┌──────────────────────────────┐
  2× 18650 (2S, 7.4 V) │  Power switch                │
  ─────────────────────┤                              │
                       └──────┬───────────────┬───────┘
                              │ VBAT          │ VBAT
                              ▼               ▼
                     ┌────────────────┐  ┌──────────────────────┐
                     │ L298N  (VS)    │  │ Arduino UNO (VIN)    │
                     │ 2 H-bridges    │  │ on-board 5 V reg     │
                     └──┬──────────┬──┘  └──────────┬───────────┘
            OUT1/OUT2   │          │ OUT3/OUT4      │ 5 V logic rail
                ┌───────┴──┐   ┌───┴──────┐         ├── HC-SR04
                │ Left pair│   │Right pair│         ├── 3× IR line sensors
                │ M1 ∥ M2  │   │ M3 ∥ M4  │         ├── HC-05 (VCC)
                └──────────┘   └──────────┘         ├── Joystick (optional, tethered)
                                                    └── SG90 servo (see §3.4)
            ALL GROUNDS JOINED: battery −, L298N GND, Arduino GND
```

**Drive layout: skid-steer.** The L298N has two channels, so the two left motors are wired in parallel to channel A and the two right motors to channel B. Turning is done by driving the sides at different speeds or in opposite directions.

**Control layout:** one Arduino UNO does everything. The sensor shield in the kit is just a breakout (it gives each pin a G/V/S header), so it doesn't change the pin plan.

## 2. Component list

| # | Component | In kit? | Purpose |
|---|---|---|---|
| 1 | Arduino UNO R3 (likely a CH340 clone) | Yes | Main controller: runs all firmware |
| 2 | Sensor Shield V5 | Yes | Plugs on the UNO; tidy 3-pin headers for sensors and servo |
| 3 | L298N dual H-bridge | Yes | Switches battery current to the motors; Arduino only sends logic signals |
| 4 | 4× TT geared DC motors (3–6 V) | Yes (need soldering) | Drive |
| 5 | 4× wheels, 2 chassis plates, brass standoffs, screws | Yes | Mechanical |
| 6 | 4× encoder discs | Yes | For optional speed sensors later; not used in v1 (no slot sensors listed) |
| 7 | HC-SR04 ultrasonic sensor + bracket | Yes | Distance to obstacles (2–400 cm nominal) |
| 8 | SG90 micro servo | Yes | Pans the HC-SR04 left/right to choose an escape direction |
| 9 | 3× IR line-tracking modules (TCRT5000-type, digital out) | Yes | Detect dark line under left / centre / right of the nose |
| 10 | HC-05 Bluetooth Classic module | Listing says separate | Phone control (Android only — see §3.6) |
| 11 | IR receiver + remote | Yes | Not part of the v1 requirements; kept as a later extra (no spare pin, see §4) |
| 12 | 18650 battery holder(s), switch, wires, jumpers | Yes | Power and wiring |
| 13 | "Battery charging module" | Yes | **Identify before use** — see §3.5 |
| 14 | 2× 18650 Li-ion cells | **Buy** | Power |
| 15 | Standalone 18650 charger | **Buy** | Safe charging |
| 16 | Resistors: 1 kΩ, 2 kΩ (HC-05 RX divider); 2× 10 kΩ (battery sense) | **Buy** | Level shifting / voltage measurement |
| 17 | Capacitors: 470 µF 16 V electrolytic, 100 nF ceramic ×4 | **Buy** (recommended) | Smooth motor current spikes, reduce Arduino resets |
| 18 | Analog joystick module (KY-023) | Optional | Mode D (tethered) — see §3.7 |
| 19 | Multimeter, soldering iron, heat-shrink | Tools | Motors need soldering; multimeter is essential for safe power checks |

## 3. Electrical compatibility assessment

### 3.1 Motor current vs L298N limits
- TT motors draw roughly 0.1–0.25 A running and around 1 A or more when stalled (exact figures vary by batch — measure yours with the multimeter).
- Two motors share each L298N channel, so a channel can see ~2 A when both stall (e.g. robot pushing against a wall).
- The L298N is rated about 2 A per channel. **Verdict: works, with little margin at stall.** Firmware mitigations: speed ramping, and stopping when an obstacle is close so the motors don't sit in stall.

### 3.2 Voltage drop
- The L298N is an old bipolar driver and loses roughly 2 V (sometimes more) internally.
- 2S 18650 gives 8.4 V full → ~6.4 V near empty, so the motors see roughly 4–6 V. That suits 3–6 V TT motors.
- **Verdict: 2S is the right pack.** 3S (11.1 V) would overdrive the motors unless PWM was capped.

**Upgrade option (not required):** TB6612FNG (MOSFET, ~0.5 V loss, but ~1.2 A continuous per channel — marginal for two motors) or DRV8833. Keep the L298N for v1: it's in the kit and the firmware abstracts it behind one `Motors` module, so swapping later only changes that file.

### 3.3 Logic power
- Arduino is powered from the battery through **VIN** (not the 5 V pin). The UNO regulator needs roughly 6+ V on VIN for a clean 5 V, which a 2S pack provides down to the firmware cut-off.
- **Do not also connect the L298N's 5 V terminal to the Arduino's 5 V.** Two 5 V sources fighting each other is a classic way to damage a board. Leave the L298N 5 V pin unconnected (its jumper can stay on; it powers the L298N's own logic).
- 5 V load estimate: UNO ~50 mA, HC-SR04 ~15 mA, 3 line sensors ~60 mA, HC-05 ~40 mA (pairing peak) → ~165 mA. Comfortable.

### 3.4 Servo
- An SG90 can pull several hundred mA when it moves or stalls. On the UNO 5 V rail that can cause resets.
- v1 plan: power from the 5 V rail, **move it only while the robot is stopped**, and `detach()` it when idle. If you see resets during scans, move servo power to a separate 5 V regulator (shared ground).

### 3.5 Batteries — safety
- Use 2 **genuine, matched** 18650 cells (same brand, same charge level). Fake high-capacity cells are common locally; a claimed 9900 mAh 18650 is fake.
- Prefer **protected** cells. Check whether your holder needs button-top cells.
- **Never charge two cells in series with a single-cell module.** If the kit's "charging module" is a TP4056-style board, it is a single-cell charger. Simplest safe route: remove the cells and charge them in a standalone 18650 charger.
- Firmware low-battery cut-off at 6.4 V (3.2 V/cell) protects the cells and prevents brown-out behaviour.
- Never leave cells charging unattended; store them out of the holder when the robot isn't in use for a while.

### 3.6 Bluetooth
- HC-05 is Bluetooth **Classic (SPP)**. Android supports it; **iPhones do not**. For iPhone you'd need an HM-10 (BLE) module instead — same UART wiring, different app.
- HC-05 **RX pin is 3.3 V logic**: the Arduino TX line goes through a 1 kΩ / 2 kΩ divider. HC-05 TX → Arduino RX can connect directly.
- Wired to software serial pins (D2/D3), not D0/D1, so uploading and the Serial Monitor keep working with the module attached.

### 3.7 Joystick
- A joystick plugged into the car's own UNO means a cable tether. That's fine for a demo but awkward.
- Wireless alternatives: (a) the phone app's joystick screen over Bluetooth — no extra hardware; (b) a separate handheld controller (second Arduino + joystick + second HC-05 as master) — a later phase.
- **The controls share one architecture:** Bluetooth buttons and a physical joystick both produce the same `DriveCommand {left, right}` (−100…100 %), using the same `arcadeMix()` helper, and pass through the same safety filter into one `Motors` module. Only the input layer differs, so a wireless handheld controller later just needs to send the existing Bluetooth characters.

### 3.8 Common ground
Battery −, L298N GND and Arduino GND must be one node. Without it the L298N can't read the Arduino's logic levels and behaviour is erratic.

## 4. Pin assignment plan

Timer conflicts checked first:
- `Servo` library uses **Timer1** → PWM on D9/D10 is lost. So motor speed PWM goes on **D5/D6 (Timer0)**.
- `SoftwareSerial` works on any pin; only the RX pin needs pin-change interrupts (all UNO pins have them).
- D0/D1 reserved for USB upload and Serial Monitor debug.
- No I²C devices, so A4/A5 are free as analog inputs.

| Pin | Function | Connects to | Notes |
|---|---|---|---|
| D0 / D1 | USB serial | — | Reserved: upload + debug |
| D2 | BT RX (SoftwareSerial) | HC-05 TXD | Direct |
| D3 | BT TX (SoftwareSerial) | HC-05 RXD | **Through 1 k / 2 k divider** |
| D4 | IN1 | L298N IN1 | Left direction A |
| D5 | ENA (PWM, Timer0) | L298N ENA | Left speed — **remove ENA jumper** |
| D6 | ENB (PWM, Timer0) | L298N ENB | Right speed — **remove ENB jumper** |
| D7 | IN2 | L298N IN2 | Left direction B |
| D8 | IN3 | L298N IN3 | Right direction A |
| D9 | Servo signal | SG90 | Timer1 |
| D10 | IN4 | L298N IN4 | Right direction B (digital only) |
| D11 | Ultrasonic TRIG | HC-SR04 Trig | |
| D12 | Ultrasonic ECHO | HC-SR04 Echo | 5 V output, safe for UNO |
| D13 | Status LED | On-board LED | Blink pattern = mode / fault |
| A0 | Line sensor LEFT | IR module DO | Read as digital |
| A1 | Line sensor CENTRE | IR module DO | |
| A2 | Line sensor RIGHT | IR module DO | |
| A3 | Joystick X | KY-023 VRx | Optional |
| A4 | Joystick Y | KY-023 VRy | Optional |
| A5 | Battery sense | 10 k / 10 k divider from VBAT | 8.4 V → 4.2 V at pin |

All 18 usable pins are allocated. Consequences:
- Joystick push-button: not wired (stopping is by returning the stick to centre).
- IR remote receiver: not in v1. If wanted later, it can take A3 when no joystick is fitted (the firmware would make this a build option).

## 5. Firmware architecture

Arduino IDE 2 sketch (runs natively on Apple Silicon). Only built-in libraries (`Servo`, `SoftwareSerial`) — nothing to install.

```
                  ┌───────────── inputs ─────────────┐
   BluetoothInput ─┐                                 │
   JoystickInput  ─┼─► DriveCommand ─┐               │
                   │                 │               │
   LineFollower ◄──── LineSensors    ├─► ModeManager ─► Safety ─► Motors ─► L298N
   ObstacleAvoider ◄─ Ultrasonic +   │   (one owner     (cut-offs,  (ramping,
                      Scanner(servo) ┘    of motors)     timeouts)   mixer)
                                          │
                                          └─► StatusLed, Serial debug
```

Rules that prevent conflicting motor commands:
1. **Only `ModeManager` calls `Motors`.** Behaviour modules return a `DriveCommand`; they never touch pins.
2. **`Safety` sits between `ModeManager` and `Motors`** and can override anything (low battery, Bluetooth timeout, obstacle too close in manual forward).
3. **Every mode change passes through STOP:** ramp to 0, wait `MODE_SETTLE_MS`, then enter the new mode.
4. **Non-blocking loop:** every module has an `update(now)` that uses `millis()`. The one bounded blocking call is the ultrasonic `pulseIn`, capped by a timeout (~6 ms ≈ 1 m range).

Modes:

| Mode | Entered by | Exit / fault behaviour |
|---|---|---|
| `IDLE` (boot default) | Power-on, `S` command, any fault cleared | Motors off |
| `LINE_FOLLOW` | `T` | Line lost > `LINE_LOST_MS` → search, then `FAULT` |
| `OBSTACLE` | `O` | Stop → scan → turn to clearer side → resume |
| `BT_MANUAL` | `M` (or any drive key from IDLE) | No packet for `BT_TIMEOUT_MS` → stop |
| `JOYSTICK` | `K` | Stick reads out of range → stop |
| `FAULT` | Low battery, line lost, sensor error | Motors off, LED error blink; `S` clears (unless battery still low) |

Bluetooth protocol (single characters, compatible with common Android "Bluetooth RC Car" apps):
- Drive: `F` `B` `L` `R` `S`, diagonals `G` `I` `H` `J`
- Speed: `0`–`9`, `q` = 100 %
- Mode: `T` line, `O` obstacle, `M` manual, `K` joystick
- `S`: in manual = stop (apps send it when a button is released); in other modes = stop → `IDLE`; in `FAULT` = clear
- `E` / `!` in **any** mode = emergency stop (instant, no ramp) → `IDLE`
- `?` = status. Commands are also accepted from the USB Serial Monitor.
- Full reference: `docs/upload-instructions.md §6`

## 6. Folder structure

```
4WD-Robot-Car/
├── README.md
├── firmware/
│   └── RobotCar/                 ← open this folder in Arduino IDE
│       ├── RobotCar.ino          setup()/loop() only
│       ├── Config.h              pins + every tunable threshold
│       ├── Types.h               DriveCommand, Mode, Fault, arcadeMix()
│       ├── Motors.h/.cpp         L298N driver, ramping
│       ├── LineSensors.h/.cpp
│       ├── Ultrasonic.h/.cpp     HC-SR04, validation, median filter
│       ├── Scanner.h/.cpp        servo pan
│       ├── LineFollower.h/.cpp
│       ├── ObstacleAvoider.h/.cpp
│       ├── CommandInput.h/.cpp   Bluetooth + USB command protocol, link timeout
│       ├── JoystickInput.h/.cpp
│       ├── Battery.h/.cpp        voltage sense, debounced cut-off
│       ├── ModeManager.h/.cpp    mode state machine + safety filter (sole motor owner)
│       └── StatusLed.h/.cpp
├── firmware-tests/               small single-purpose sketches (motor test, sensor test…)
├── simulation/                   host logic tests (g++) + Wokwi diagram
├── docs/
│   ├── 01-phase1-design.md       ← this file
│   ├── shopping-list.md
│   ├── wiring.md
│   ├── circuit-diagram.svg       (generated by docs/tools/make_circuit_svg.py)
│   ├── assembly-guide.md
│   ├── upload-instructions.md
│   ├── testing-guide.md
│   ├── troubleshooting.md
│   └── demo-checklist.md
├── media/                        real photos/videos only — placeholders until you shoot them
└── website/                      portfolio presentation page (final phase)
```

## 7. Phase plan

| Phase | Deliverable | Needs hardware? | Status |
|---|---|---|---|
| 1 | This design, folder skeleton, `Config.h` | No | Done |
| 2 | Wiring doc, circuit + power diagram, assembly guide | No | Done (unverified) |
| 3 | `Motors` + test sketches (motor direction, ramp) | Yes, to verify | Code done · hardware test pending |
| 4 | Sensors: line, ultrasonic, servo, battery | Yes | Code done · hardware test pending |
| 5 | Line following + obstacle avoidance behaviours | Yes | Code done · hardware test pending |
| 6 | Bluetooth + joystick + mode manager + safety | Yes | Code done · hardware test pending |
| 7 | Wokwi simulation of logic parts | No | Done: 92 host tests pass; Wokwi file untested |
| 8 | Testing guide, troubleshooting, demo checklist | Results come from you | Guides done · results pending |
| 9 | Portfolio page (placeholders until real photos/video exist) | No | Done (placeholders) |

Note for the portfolio page: the kit listing photos belong to the vendor and show *their* build, so they shouldn't appear as "robot photography". Use labelled placeholders until you photograph your own robot.
