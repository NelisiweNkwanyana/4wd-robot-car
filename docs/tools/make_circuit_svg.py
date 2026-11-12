"""Generates docs/circuit-diagram.svg from the pin plan (keep in sync with Config.h).
Run: python3 docs/tools/make_circuit_svg.py"""
import os

W, H = 1460, 1000
NAVY, TEAL, RED, ORANGE, GREY, BG = "#14213d", "#0f8b8d", "#d62828", "#e07a00", "#5c677d", "#ffffff"
out = []
def add(s): out.append(s)

def box(x, y, w, h, title, sub=None, fill="#f4f7fb"):
    add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="8" fill="{fill}" stroke="{NAVY}" stroke-width="2"/>')
    add(f'<text x="{x+w/2}" y="{y+22}" class="t">{title}</text>')
    if sub: add(f'<text x="{x+w/2}" y="{y+40}" class="s">{sub}</text>')

def pin(x, y, label, side):  # side: 'l' label inside to the right, 'r' label inside to left
    add(f'<circle cx="{x}" cy="{y}" r="3.5" fill="{NAVY}"/>')
    if side == "l": add(f'<text x="{x+11}" y="{y+4}" class="p" text-anchor="start">{label}</text>')
    elif side == "r": add(f'<text x="{x-11}" y="{y+4}" class="p" text-anchor="end">{label}</text>')
    elif side == "b": add(f'<text x="{x}" y="{y-8}" class="p">{label}</text>')
    elif side == "t": add(f'<text x="{x}" y="{y+16}" class="p">{label}</text>')

def wire(pts, color=TEAL, w=2, dash=None):
    d = " ".join(f"{x},{y}" for x, y in pts)
    da = f' stroke-dasharray="{dash}"' if dash else ""
    add(f'<polyline points="{d}" fill="none" stroke="{color}" stroke-width="{w}" stroke-linejoin="round"{da}/>')

def net(x, y, label, color, anchor="start"):
    add(f'<text x="{x}" y="{y+4}" class="n" fill="{color}" text-anchor="{anchor}">{label}</text>')

def gnd(x, y):
    add(f'<path d="M{x},{y} v8 M{x-8},{y+8} h16 M{x-5},{y+12} h10 M{x-2},{y+16} h4" stroke="{NAVY}" stroke-width="2" fill="none"/>')

add(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" font-family="Inter, Helvetica, Arial, sans-serif">')
add(f'''<style>
.t{{font-size:15px;font-weight:700;fill:{NAVY}}}
.s{{font-size:11.5px;fill:{GREY}}}
.p{{font-size:11.5px;fill:{NAVY};font-family:"JetBrains Mono",Menlo,monospace}}
.n{{font-size:11.5px;font-weight:700;paint-order:stroke;stroke:#fff;stroke-width:4px;font-family:"JetBrains Mono",Menlo,monospace}}
.h{{font-size:22px;font-weight:800;fill:{NAVY}}}
.c{{font-size:12px;fill:{GREY}}}
</style>''')
add(f'<rect width="{W}" height="{H}" fill="{BG}"/>')
add(f'<text x="40" y="48" class="h" text-anchor="start">4WD Robot Car — wiring schematic</text>')
add(f'<text x="40" y="70" class="c" text-anchor="start">Arduino UNO · L298N · HC-SR04 on SG90 · 3× IR line sensors · HC-05 · KY-023 (optional) · 2S 18650. Pins match firmware/RobotCar/Config.h. Design drawing — not yet verified on hardware.</text>')

# ---------------- Arduino UNO ----------------
ux, uy, uw, uh = 540, 150, 230, 640
box(ux, uy, uw, uh, "Arduino UNO R3", "ATmega328P · 5 V logic")
dpins = [("D2", 2), ("D3", 3), ("D4", 4), ("D5 ~", 5), ("D6 ~", 6), ("D7", 7), ("D8", 8),
         ("D9 ~", 9), ("D10", 10), ("D11", 11), ("D12", 12), ("D13", 13)]
dy = {}
for i, (lab, n) in enumerate(dpins):
    y = uy + 80 + i * 40
    pin(ux + uw, y, lab, "r"); dy[n] = y
apins = ["A0", "A1", "A2", "A3", "A4", "A5"]
ay = {}
for i, lab in enumerate(apins):
    y = uy + 300 + i * 40
    pin(ux, y, lab, "l"); ay[lab] = y
pin(ux, uy + 120, "D0/D1 USB", "l")
add(f'<text x="{ux+12}" y="{uy+142}" class="s" text-anchor="start">reserved: upload + debug</text>')
bpins = {"VIN": ux + 50, "5V": ux + 115, "GND": ux + 180}
for lab, x in bpins.items(): pin(x, uy + uh, lab, "b")
add(f'<text x="{ux+uw-14}" y="{dy[13]+22}" class="s" text-anchor="end">on-board LED "L"</text>')

# ---------------- L298N ----------------
lx, ly, lw, lh = 1000, 120, 190, 320
box(lx, ly, lw, lh, "L298N", "dual H-bridge · ~2 A/ch", fill="#fff4f4")
lp = {"ENA": 0, "IN1": 1, "IN2": 2, "IN3": 3, "IN4": 4, "ENB": 5}
lpy = {}
for k, i in lp.items():
    y = ly + 80 + i * 36; pin(lx, y, k, "l"); lpy[k] = y
add(f'<text x="{lx+lw/2}" y="{ly+280}" class="s">remove ENA/ENB jumpers</text>')
for lab, y in [("OUT1", ly + 90), ("OUT2", ly + 130), ("OUT3", ly + 200), ("OUT4", ly + 240)]:
    pin(lx + lw, y, lab, "r")
lb = {"+12V (VS)": lx + 40, "GND": lx + 105, "5V": lx + 160}
for lab, x in lb.items(): pin(x, ly + lh, lab, "b")
add(f'<text x="{lx+lw/2}" y="{ly+296}" class="s">5V pin: leave unconnected</text>')

# motors (two terminals each: + top-left, - bottom-left)
my = {"M1|front-left": 160, "M2|rear-left": 250, "M3|front-right": 345, "M4|rear-right": 435}
for name, y in my.items():
    mid, pos = name.split("|")
    add(f'<circle cx="1300" cy="{y}" r="26" fill="#fff" stroke="{NAVY}" stroke-width="2"/><text x="1300" y="{y+5}" class="t">{mid}</text>')
    add(f'<text x="1336" y="{y+4}" class="s" text-anchor="start">{pos}</text>')
def dot(x, y): add(f'<circle cx="{x}" cy="{y}" r="4" fill="{NAVY}"/>')
def pair(outA, outB, y1, y2):
    wire([(lx + lw, outA), (1236, outA), (1236, y1 - 9), (1276, y1 - 9)], RED)
    wire([(1236, y2 - 9), (1276, y2 - 9)], RED); wire([(1236, outA), (1236, y2 - 9)], RED); dot(1236, outA)
    wire([(lx + lw, outB), (1252, outB), (1252, y1 + 9), (1276, y1 + 9)], NAVY)
    wire([(1252, outB), (1252, y2 + 9), (1276, y2 + 9)], NAVY); dot(1252, outB)
pair(ly + 90, ly + 130, 160, 250)
pair(ly + 200, ly + 240, 345, 435)
add(f'<text x="1450" y="{ly-12}" class="s" text-anchor="end">left pair ∥ OUT1/2 · right pair ∥ OUT3/4</text>')
add(f'<text x="1450" y="{ly+352}" class="s" text-anchor="end">100 nF across each motor</text>')

# Arduino -> L298N signals, each in its own vertical channel
sig = [(5, "ENA"), (4, "IN1"), (7, "IN2"), (8, "IN3"), (10, "IN4"), (6, "ENB")]
for i, (d, k) in enumerate(sig):
    cx = 830 + i * 22
    wire([(ux + uw, dy[d]), (cx, dy[d]), (cx, lpy[k]), (lx, lpy[k])], TEAL)

# ---------------- HC-SR04 ----------------
sx, sy = 1000, 500
box(sx, sy, 190, 100, "HC-SR04", "ultrasonic, front")
pin(sx, sy + 60, "TRIG", "l"); pin(sx, sy + 84, "ECHO", "l")
net(sx + 182, sy + 84, "VCC +5V", ORANGE, "end"); gnd(sx + 175, sy + 100)
wire([(ux + uw, dy[11]), (965, dy[11]), (965, sy + 60), (sx, sy + 60)], TEAL)
wire([(ux + uw, dy[12]), (980, dy[12]), (980, sy + 84), (sx, sy + 84)], TEAL)

# ---------------- Servo ----------------
vx, vy = 1000, 640
box(vx, vy, 190, 70, "SG90 servo", "pans the HC-SR04")
pin(vx, vy + 50, "SIG", "l")
net(vx + 160, vy + 50, "V+ +5V*", ORANGE, "end"); gnd(vx + 175, vy + 70)
wire([(ux + uw, dy[9]), (950, dy[9]), (950, vy + 50), (vx, vy + 50)], TEAL)

# ---------------- HC-05 ----------------
bx, by = 1000, 740
box(bx, by, 190, 110, "HC-05", "Bluetooth Classic (Android)")
pin(bx, by + 60, "TXD", "l"); pin(bx, by + 92, "RXD 3.3V", "l")
net(bx + 182, by + 92, "VCC +5V", ORANGE, "end"); gnd(bx + 175, by + 110)
wire([(ux + uw, dy[2]), (800, dy[2]), (800, by + 60), (bx, by + 60)], TEAL)
wire([(ux + uw, dy[3]), (815, dy[3]), (815, by + 92), (880, by + 92)], TEAL)
add(f'<rect x="880" y="{by+84}" width="40" height="16" fill="#fff" stroke="{NAVY}" stroke-width="1.6"/><text x="900" y="{by+78}" class="p">1 kΩ</text>')
wire([(920, by + 92), (bx, by + 92)], TEAL); dot(950, by + 92)
wire([(950, by + 92), (950, by + 102)], TEAL)
add(f'<rect x="942" y="{by+102}" width="16" height="30" fill="#fff" stroke="{NAVY}" stroke-width="1.6"/><text x="964" y="{by+122}" class="p" text-anchor="start">2 kΩ</text>')
gnd(950, by + 132)

# ---------------- Line sensors ----------------
for i, (name, a) in enumerate([("IR line LEFT", "A0"), ("IR line CENTRE", "A1"), ("IR line RIGHT", "A2")]):
    x, y = 150, 120 + i * 92
    box(x, y, 190, 70, name, "TCRT5000 module · DO")
    pin(x + 190, y + 50, "DO", "r")
    net(x - 8, y + 30, "+5V", ORANGE, "end"); gnd(x + 15, y + 70)
    cx = 380 + i * 22
    wire([(x + 190, y + 50), (cx, y + 50), (cx, ay[a]), (ux, ay[a])], TEAL)

# ---------------- Joystick ----------------
jx, jy = 150, 420
box(jx, jy, 190, 100, "KY-023 joystick", "optional · tethered", fill="#f2fbfb")
pin(jx + 190, jy + 55, "VRx", "r"); pin(jx + 190, jy + 80, "VRy", "r")
net(jx - 8, jy + 40, "+5V", ORANGE, "end"); gnd(jx + 15, jy + 100)
wire([(jx + 190, jy + 55), (450, jy + 55), (450, ay["A3"]), (ux, ay["A3"])], TEAL, dash="6 4")
wire([(jx + 190, jy + 80), (470, jy + 80), (470, ay["A4"]), (ux, ay["A4"])], TEAL, dash="6 4")

# ---------------- Battery sense divider ----------------
add(f'<rect x="488" y="{ay["A5"]+18}" width="16" height="34" fill="#fff" stroke="{NAVY}" stroke-width="1.6"/>')
add(f'<text x="480" y="{ay["A5"]+40}" class="p" text-anchor="end">10 kΩ</text>')
add(f'<rect x="488" y="{ay["A5"]+74}" width="16" height="34" fill="#fff" stroke="{NAVY}" stroke-width="1.6"/>')
add(f'<text x="480" y="{ay["A5"]+96}" class="p" text-anchor="end">10 kΩ</text>')
wire([(ux, ay["A5"]), (496, ay["A5"]), (496, ay["A5"] + 18)], TEAL)
wire([(496, ay["A5"] + 52), (496, ay["A5"] + 74)], TEAL)
gnd(496, ay["A5"] + 108)
# top of divider from VBAT: drawn as a net label
wire([(496, ay["A5"] + 18), (496, ay["A5"] + 10)], TEAL)
net(470, ay["A5"] + 2, "VBAT →", RED, "end")

# ---------------- Power ----------------
px, py = 150, 820
box(px, py, 200, 100, "2S 18650 pack", "7.4 V nom · 8.4 V full", fill="#fff4f4")
pin(px + 200, py + 40, "+", "r"); pin(px + 200, py + 75, "−", "r")
wire([(px + 200, py + 40), (390, py + 40)], RED, 3)
add(f'<circle cx="394" cy="{py+40}" r="4" fill="none" stroke="{RED}" stroke-width="2"/><line x1="398" y1="{py+40}" x2="428" y2="{py+26}" stroke="{RED}" stroke-width="3"/><circle cx="432" cy="{py+40}" r="4" fill="none" stroke="{RED}" stroke-width="2"/>')
add(f'<text x="412" y="{py+16}" class="p">SWITCH</text>')
wire([(436, py + 40), (455, py + 40)], RED, 3)
add(f'<rect x="455" y="{py+32}" width="40" height="16" rx="8" fill="#fff" stroke="{RED}" stroke-width="2"/><text x="475" y="{py+66}" class="p">3 A fuse</text>')
vin = ux + 50
wire([(495, py + 40), (520, py + 40), (vin, py + 40), (vin, uy + uh)], RED, 3)
wire([(520, py + 40), (520, 965), (1205, 965), (1205, 470), (lb["+12V (VS)"], 470), (lb["+12V (VS)"], ly + lh)], RED, 3)
dot(520, py + 40)
add(f'<text x="1199" y="960" class="n" fill="{RED}" text-anchor="end">VBAT 7–8.4 V</text>')
add(f'<text x="{lx+8}" y="490" class="s" text-anchor="start">470 µF across VS–GND</text>')
gx = ux + 180
wire([(px + 200, py + 75), (gx, py + 75), (gx, uy + uh)], NAVY, 3)
wire([(gx, py + 75), (gx, 945), (1222, 945), (1222, 455), (lb["GND"], 455), (lb["GND"], ly + lh)], NAVY, 3)
dot(gx, py + 75)
add(f'<text x="{gx+10}" y="938" class="n" fill="{NAVY}" text-anchor="start">COMMON GROUND: battery −, UNO, L298N, every module</text>')
wire([(ux + 115, uy + uh), (ux + 115, uy + uh + 22)], ORANGE, 3)
add(f'<text x="{ux+108}" y="{uy+uh+20}" class="n" fill="{ORANGE}" text-anchor="end">+5V → all modules</text>')

# legend
lgx, lgy = 160, 580
add(f'<text x="{lgx}" y="{lgy}" class="t" text-anchor="start">Legend</text>')
for i, (c, t, d) in enumerate([(RED, "battery / motor power", None), (NAVY, "ground", None), (ORANGE, "+5 V logic", None), (TEAL, "signal", None), (TEAL, "optional (joystick)", "6 4")]):
    y = lgy + 22 + i * 20
    wire([(lgx, y), (lgx + 34, y)], c, 3, d)
    add(f'<text x="{lgx+44}" y="{y+4}" class="s" text-anchor="start">{t}</text>')
add(f'<text x="{lgx}" y="{lgy+132}" class="s" text-anchor="start">* servo: separate 5 V regulator if resets</text>')
add("</svg>")

import re as _re
out = [_re.sub(r'<text (?![^>]*text-anchor)', '<text text-anchor="middle" ', l) for l in out]
path = os.path.join(os.path.dirname(__file__), "..", "circuit-diagram.svg")
open(path, "w").write("\n".join(out))
print("wrote", os.path.abspath(path))
