# Firmware upload instructions (macOS, Apple Silicon)

## 1. Install Arduino IDE 2
1. Download **Arduino IDE 2.x** for macOS (Apple Silicon) from arduino.cc/en/software.
2. Drag it to Applications and open it. Approve the security prompt if one appears.
3. *Tools → Board → Boards Manager* → make sure **Arduino AVR Boards** is installed (it normally is).

No extra libraries are needed. The firmware only uses `Servo` and `SoftwareSerial`, which ship with the IDE.

## 2. Connect the UNO
1. Plug the UNO in with a USB cable. Use a USB-C to USB-B cable or an adapter; charge-only cables won't work.
2. *Tools → Port* should show something like `/dev/cu.usbserial-XXXX` or `/dev/cu.usbmodemXXXX`.
3. **No port appears?** Kit boards are usually clones with a **CH340** USB chip. Recent macOS versions often include a driver; if not, install the official CH34x macOS driver from the chip maker (WCH), approve it under *System Settings → Privacy & Security*, then replug.

## 3. Open and upload
1. *File → Open…* → `firmware/RobotCar/RobotCar.ino`. All the `.h`/`.cpp` files open as tabs automatically. The folder name must stay `RobotCar`.
2. *Tools → Board → Arduino AVR Boards → Arduino Uno*.
3. Click **Verify (✓)** first. This is the first real AVR compile of this code (so far it has only been compiled for the host simulator), so read any errors carefully.
4. Click **Upload (→)**. The HC-05 is on D2/D3, so it does **not** need to be unplugged while uploading.
5. *Tools → Serial Monitor*, set **115200 baud** and line ending **No line ending**.

Expected output:
```
4WD Robot Car ready. Commands: T O M K S E ? | F B L R | 0-9 q
MODE -> IDLE
t=250 mode=IDLE fault=NONE dist=80cm line=000 bat=USB speed=60% L=0 R=0
```

## 4. Test sketches
Each folder in `firmware-tests/` is its own sketch. Open, upload and test them one at a time, in number order, before the main firmware. See `docs/testing-guide.md`.

## 5. Pair the phone (Android)
1. Power the robot. The HC-05 LED blinks fast (not paired).
2. Android *Settings → Bluetooth* → pair **HC-05**, PIN `1234` (or `0000`).
3. Open a Bluetooth controller app (see below) and connect. The HC-05 LED changes to a slow double-blink.
4. Send `?` and the robot replies with a status line.

### Choosing an app
The firmware speaks the common single-character "Bluetooth RC car" protocol: `F B L R G I H J S`, `0`–`9`, `q`. Any Android app that sends those characters over **Bluetooth Classic (SPP)** will work. A terminal app with custom macro buttons works too: map buttons to `T`, `O`, `M`, `K`, `E`, `?`.

**Important:** driving uses *hold-to-drive*. The app must **repeat** the key while a button is held, otherwise the robot stops after `BT_TIMEOUT_MS` (600 ms) as a safety measure. Upload `firmware-tests/05_bluetooth_test` and press a button: if you see the same character every ~50–200 ms while holding it, the app is suitable. If your app sends only once, either pick another app or raise `BT_TIMEOUT_MS`, knowing that this delays the failsafe.

**iPhone:** iOS can't connect to an HC-05 (Classic SPP). Use an **HM-10** (BLE) module instead. Wiring is identical (VCC, GND, TXD→D2, divider→RXD) and no firmware change is needed beyond checking the baud rate (HM-10 default 9600). Then use a BLE terminal or controller app.

## 6. Command reference
| Key | Action |
|---|---|
| `T` | Line-following mode |
| `O` | Obstacle-avoidance mode |
| `M` | Bluetooth manual mode (pressing any drive key while IDLE also enters it) |
| `K` | Joystick mode (only if `JOYSTICK_FITTED = true` and centre calibrated) |
| `F` `B` `L` `R` | Forward, back, spin left, spin right |
| `G` `I` `H` `J` | Forward-left, forward-right, back-left, back-right curves |
| `S` | Manual: stop. Other modes: stop and return to IDLE. FAULT: clear (if safe) |
| `E` or `!` | **Emergency stop** from any mode: motors off instantly, then IDLE |
| `0`–`9`, `q` | Manual speed 0–90 %, `q` = 100 % |
| `?` | Status line to phone and Serial Monitor |

Keys not listed (for example the `W/w`, `U/u`, `V/v`, `X/x` light and horn buttons on some apps) are ignored, so they can't change modes by accident.

## 7. Status LED (D13)
Blinks every 2 s: 1 = IDLE, 2 = LINE, 3 = OBSTACLE, 4 = MANUAL, 5 = JOYSTICK. Fast flicker = FAULT (check the Serial Monitor for the reason, then send `S`).
