# Assembly guide

Written before assembly, from the kit listing photos. Your kit may differ, so adapt and note the differences in this file. Take photos at every stage: they become the real robot photography for the portfolio page.

**Tools:** soldering iron + solder, small Phillips screwdriver, wire strippers, multimeter, cable ties, heat-shrink or electrical tape.
**Time:** roughly an afternoon.

## Stage 0 — Unbox and inventory
1. Lay everything out and tick it off against `docs/01-phase1-design.md §2`.
2. Identify the "battery charging module". If it's a single-cell board (TP4056 or similar), **don't** use it to charge the 2S pack. Use a standalone charger.
3. Peel the protective film off the acrylic plates (warm water helps).

## Stage 1 — Motors
1. Solder a red and a black wire to each of the 4 TT motors. Keep the polarity the same on all four: red to the same terminal (the one nearer the motor's label, say) on every motor.
2. Solder a 100 nF ceramic capacitor across each motor's two terminals.
3. Strain-relieve the wires with a dab of hot glue or a cable tie. Motor tabs snap off easily.
4. **Quick check:** touch each motor's leads to the 2S pack for one second. All four should spin the same direction when red goes to +.

## Stage 2 — Lower chassis
1. Fit the motor brackets and screw the 4 motors to the lower plate, shafts pointing outward.
2. Press the wheels on firmly.
3. Mount the 3 line sensors at the **front** underside, in a row, on the sensor bracket. Spacing: roughly the width of your tape (about 15–20 mm between sensor centres for 19 mm tape), with the IR LEDs 5–10 mm above the floor.
4. Route the motor wires up through the middle holes and label them FL, RL, FR, RR.
5. Fit the brass standoffs.

## Stage 3 — Upper chassis
1. Mount the L298N near the rear (heatsink clear of the battery holder).
2. Mount the Arduino UNO with the Sensor Shield V5 on top, USB port facing a side you can reach.
3. Mount the battery holder where you can swap cells without removing other parts.
4. Mount the servo at the front with the HC-SR04 bracket on it. **Before** screwing the horn on, power the servo with `firmware-tests/04_servo_test` and let it go to 90°, then fit the bracket pointing straight ahead.
5. Mount the HC-05 somewhere it isn't sandwiched between metal parts.
6. Screw the upper plate onto the standoffs.

## Stage 4 — Power wiring (batteries OUT)
Follow `docs/wiring.md §1`.
1. Battery + → switch → 3 A inline fuse → VBAT junction.
2. VBAT → L298N +12V terminal.
3. VBAT → Arduino VIN, using the DC barrel jack (centre-positive) or the VIN pin.
4. Battery − → L298N GND, and a separate wire from battery − → Arduino GND.
5. 470 µF capacitor across L298N +12V/GND (stripe = −).
6. 10 kΩ + 10 kΩ divider: VBAT → 10 kΩ → A5 → 10 kΩ → GND.
7. L298N 5V terminal: **leave empty**.

**Multimeter checks before inserting cells:**
- [ ] Resistance between VBAT and GND, switch ON: should **not** be near 0 Ω (it will show the L298N/Arduino input, typically hundreds of Ω or more).
- [ ] Continuity between battery −, L298N GND and Arduino GND: ~0 Ω.
- [ ] No continuity between the L298N 5V terminal and the Arduino 5V pin.

## Stage 5 — Motor driver wiring
1. **Remove the ENA and ENB jumpers** on the L298N (keep them in a bag).
2. Left motors in parallel → OUT1/OUT2; right motors → OUT3/OUT4.
3. Arduino → L298N: D5→ENA, D4→IN1, D7→IN2, D8→IN3, D10→IN4, D6→ENB.

## Stage 6 — Sensors and Bluetooth
1. HC-SR04: VCC→5V, GND, Trig→D11, Echo→D12.
2. Servo: brown→GND, red→5V, orange→D9.
3. Line sensors: DO→A0 (left), A1 (centre), A2 (right); VCC→5V; GND.
4. HC-05: VCC→5V, GND, TXD→D2, D3→1 kΩ→RXD, 2 kΩ RXD→GND.
5. Joystick (optional): long 5-wire cable, VRx→A3, VRy→A4, +5V, GND. Then set `JOYSTICK_FITTED = true`.
6. Tidy with cable ties. Keep wires clear of the wheels and the servo's sweep.

## Stage 7 — First power-up (wheels OFF the ground)
1. Put the car on a box so the wheels spin freely.
2. Connect USB only (no cells) and upload `firmware-tests/01_motor_test`. The Arduino should power from USB; the motors won't move (no motor power yet).
3. Insert charged cells, switch ON. Watch for smoke, heat or smell, and switch OFF immediately if anything is wrong.
4. Continue with `docs/testing-guide.md`, one test sketch at a time.

## Record what you changed
| Date | Change | Why |
|---|---|---|
| | | |
