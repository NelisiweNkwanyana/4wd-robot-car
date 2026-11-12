# Shopping list

Prices: check at the supplier; only the kit price was seen (R595, RoboFactory listing, Nov 2026).

## Required
| Item | Qty | Why | Notes |
|---|---|---|---|
| Robocentrix 4WD Smart Robot Car Kit | 1 | Chassis, UNO, L298N, motors, sensors, servo | Check contents against docs/01-phase1-design.md §2 |
| HC-05 Bluetooth module | 1 | Phone control | Listing says sold separately — confirm before buying a second. Android only; iPhone needs HM-10 |
| 18650 Li-ion cells, genuine, protected if they fit | 2 | Power (2S = 7.4 V) | Matched pair; avoid "9900 mAh" fakes; check button-top vs flat-top for your holder |
| Standalone 18650 charger | 1 | Safe charging | Do not series-charge with a single-cell module |
| Resistors 1 kΩ, 2 kΩ | 1 each | HC-05 RX level shift (5 V → 3.3 V) | |
| Resistors 10 kΩ | 2 | Battery voltage sense on A5 | |
| Inline blade fuse holder + 3 A fuse (or 3 A polyfuse) | 1 | Limits short-circuit current from the 18650s | Fit right after the switch |

## Recommended
| Item | Qty | Why |
|---|---|---|
| 470 µF 16 V electrolytic capacitor | 1 | Across L298N VS/GND, absorbs motor surges |
| 100 nF ceramic capacitors | 4 | Across each motor's terminals, reduces electrical noise |
| Black electrical tape (19 mm) + white board/paper | — | Line-following track |
| Data-capable USB cable for the UNO (USB-B, plus a USB-C adapter for a MacBook) | 1 | Uploading firmware |

## Optional
| Item | Qty | Why |
|---|---|---|
| KY-023 analog joystick + 3 m 5-wire cable | 1 | Tethered Mode D |
| Separate 5 V regulator (≥1 A) | 1 | Only if servo moves cause resets |
| HM-10 BLE module | 1 | **Instead of** the HC-05 if the controlling phone is an iPhone |
| TB6612FNG or DRV8833 driver | 1 | Later upgrade: less heat and voltage loss than the L298N |

## Tools
Multimeter, soldering iron + solder (motor leads), small screwdriver set, heat-shrink or tape, cable ties.
