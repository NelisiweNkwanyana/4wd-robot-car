# Demonstration checklist

For a lecturer, interview or video. Only demonstrate what passed in `docs/testing-guide.md`.

## The day before
- [ ] Cells fully charged in the standalone charger (spare pair if you have them)
- [ ] Latest firmware uploaded; `?` shows the expected values
- [ ] Line track ready: tape on white board, gentle oval plus one curve
- [ ] Obstacle props: two flat boxes (hard, flat surfaces reflect ultrasound well)
- [ ] Phone charged, HC-05 paired, app buttons mapped (`T`, `O`, `M`, `E`, `?`)
- [ ] Laptop with Arduino IDE and USB cable, in case a re-upload is needed
- [ ] Screwdriver, spare jumper wires, tape, multimeter
- [ ] Test-guide results printed or on screen (what's been tested, honestly)

## On site (5 minutes before)
- [ ] Wheels off the ground, switch ON, LED gives 1 blink (IDLE)
- [ ] `?` from the phone: battery above 7.4 V, distance sensible, line `000` over white
- [ ] Quick manual drive: forward, back, spin
- [ ] Line sensors re-checked on *this* surface and lighting

## Demo script (about 5 minutes)
1. **Intro (30 s):** what it is, the three operating modes, Arduino UNO + L298N, built from a kit plus my own firmware.
2. **Safety first (30 s):** show `E` stopping it instantly; mention the link-loss failsafe and low-battery cut-off.
3. **Manual (1 min):** drive with the phone, change speed, drive at a box to show the collision guard.
4. **Line following (1.5 min):** place it on the track, send `T`; put a box on the line to show it pausing and resuming.
5. **Obstacle avoidance (1.5 min):** send `O`, let it find its way between boxes; point out the servo scan.
6. **Under the hood (30 s):** Serial Monitor status line, `Config.h` thresholds, the 92 host-side logic tests.
7. **Close:** what I'd improve next (encoders for closed-loop speed, TB6612 driver, PID line following).

## If something goes wrong
| Problem | Quick recovery |
|---|---|
| Line following unreliable on this floor | Explain the IR/lighting dependence, show it on your board, move on to obstacle mode |
| Bluetooth won't connect | Use the Serial Monitor over USB (same commands) |
| Robot resets | Swap in fresh cells |
| Anything unsafe | `E`, then the power switch |

## Record for the portfolio
- [ ] Film each mode once in good light (landscape, steady)
- [ ] Photos: top view, side view, wiring close-up, on the track
- [ ] Save to `media/`; update the website placeholders only with these real files
