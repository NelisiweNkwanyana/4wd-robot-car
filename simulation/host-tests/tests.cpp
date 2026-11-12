// Host-side logic tests for the 4WD Robot Car firmware.
// Compiles the REAL firmware sources against a mock Arduino core and runs
// unit tests plus whole-sketch scenarios with simulated time and sensors.
// This checks logic only — it says nothing about motors, wiring or batteries.
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "Servo.h"
#include <functional>
#include <new>
#include <cstdio>

// Pull in the whole sketch (setup(), loop(), globals `bot`, `modes`).
#include "../../firmware/RobotCar/RobotCar.ino"

static int passed = 0, failed = 0;
#define CHECK(cond, msg) do { if (cond) passed++; else { failed++; \
  printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, msg); } } while (0)
#define SECTION(name) printf("- %s\n", name)

// ---------- helpers ----------
static int adcForVolts(float v) { return (int)(v / BATTERY_DIVIDER / 5.0f * 1023.0f + 0.5f); }

static void setLine(int l, int c, int r) {
  int on = LINE_ACTIVE_LOW ? LOW : HIGH, off = LINE_ACTIVE_LOW ? HIGH : LOW;
  sim::digitalIn[PIN_LINE_LEFT] = l ? on : off;
  sim::digitalIn[PIN_LINE_CENTER] = c ? on : off;
  sim::digitalIn[PIN_LINE_RIGHT] = r ? on : off;
}

// Run loop() for `ms` simulated milliseconds. `each` runs before every loop().
static void run(unsigned long ms, std::function<void()> each = nullptr) {
  unsigned long end = millis() + ms;
  while (millis() < end) { if (each) each(); loop(); sim::nowUs += 500; }
}

// Hold a drive key the way an RC app does: resend every 100 ms.
static void hold(char key, unsigned long ms, std::function<void()> each = nullptr) {
  unsigned long lastSend = 0; bool first = true;
  run(ms, [&]() {
    if (first || millis() - lastSend >= 100) { SoftwareSerial::instance->feed(std::string(1, key)); lastSend = millis(); first = false; }
    if (each) each();
  });
}

static void bt(const char* s) { SoftwareSerial::instance->feed(s); }

static void boot() {
  sim::reset();
  sim::analogIn[PIN_BATTERY] = adcForVolts(7.6f);
  sim::echoDistanceCm = 80;
  setLine(0, 0, 0);
  bot = Robot();                 // fresh module state
  new (&modes) ModeManager(bot);  // re-construct in place (holds a reference)
  if (SoftwareSerial::instance) { SoftwareSerial::instance->in.clear(); SoftwareSerial::instance->out.clear(); }
  sim::nowUs = 1000;
  setup();
  run(300);                       // let filters prime
}

static int L() { return bot.motors.pwmLeft(); }
static int R() { return bot.motors.pwmRight(); }

// =====================================================================
static void unitMotors() {
  SECTION("Motors: percent -> PWM mapping and ramping");
  CHECK(Motors::percentToPwm(0) == 0, "0% -> 0");
  CHECK(Motors::percentToPwm(1) == PWM_MIN_MOVING, "1% -> PWM_MIN_MOVING");
  CHECK(Motors::percentToPwm(100) == 255, "100% -> 255");
  CHECK(Motors::percentToPwm(-100) == -255, "-100% -> -255");

  sim::reset();
  Motors m; m.begin();
  m.setTarget(makeDrive(100, 100));
  unsigned long t = 100; m.update(t);
  CHECK(m.pwmLeft() == PWM_MIN_MOVING, "starts at minimum moving PWM, not full power");
  for (int i = 0; i < 30; i++) { t += RAMP_INTERVAL_MS; m.update(t); }
  CHECK(m.pwmLeft() == 255 && m.pwmRight() == 255, "ramps to full");
  CHECK(sim::pwmOut[PIN_MOTOR_ENA] == 255 && sim::digitalOut[PIN_MOTOR_IN1] == HIGH &&
        sim::digitalOut[PIN_MOTOR_IN2] == LOW, "pins: ENA=255, IN1=H, IN2=L for forward");

  // Reverse: must pass through 0, and never sit in the stall band.
  m.setTarget(makeDrive(-100, -100));
  bool sawZero = false, badJump = false, stallBand = false; int prev = m.pwmLeft();
  for (int i = 0; i < 60; i++) {
    t += RAMP_INTERVAL_MS; m.update(t);
    int cur = m.pwmLeft();
    if (cur == 0) sawZero = true;
    if (prev > 0 && cur < 0) badJump = true;
    if (cur != 0 && abs(cur) < PWM_MIN_MOVING) stallBand = true;
    prev = cur;
  }
  CHECK(sawZero && !badJump, "reversal passes through zero");
  CHECK(!stallBand, "never commands a PWM inside the stall band");
  CHECK(m.pwmLeft() == -255, "reaches full reverse");
  CHECK(sim::digitalOut[PIN_MOTOR_IN1] == LOW && sim::digitalOut[PIN_MOTOR_IN2] == HIGH, "pins flip for reverse");
  m.stopNow();
  CHECK(m.isStopped() && sim::pwmOut[PIN_MOTOR_ENA] == 0 && sim::pwmOut[PIN_MOTOR_ENB] == 0, "stopNow cuts both enables instantly");

  DriveCommand a = arcadeMix(100, 100);
  CHECK(a.left == 100 && a.right == 0, "arcadeMix scales instead of clipping");
}

static void unitLine() {
  SECTION("LineFollower: decisions, gaps, search, lost, lifted");
  LineFollower f; unsigned long t = 1000; f.reset(t);
  DriveCommand c = f.update(0b010, t);
  CHECK(c.left == LINE_BASE_SPEED_PCT && c.right == LINE_BASE_SPEED_PCT, "centre -> straight");
  c = f.update(0b110, t);  CHECK(c.left < c.right, "line left of centre -> steer left");
  c = f.update(0b100, t);  CHECK(c.left < 0 && c.right > 0, "line far left -> inner wheel reverses");
  c = f.update(0b011, t);  CHECK(c.left > c.right, "line right -> steer right");
  c = f.update(0b001, t);  CHECK(c.right < 0 && c.left > 0, "line far right -> pivot right");
  DriveCommand before = c;
  c = f.update(0b101, t);  CHECK(c.left == before.left && c.right == before.right, "ambiguous 101 keeps last command");

  c = f.update(0b000, t + 100);
  CHECK(c.left == before.left && f.status() == LineFollower::Status::FOLLOWING, "short gap: coast on last command");
  c = f.update(0b000, t + LINE_LOST_MS + 50);
  CHECK(f.status() == LineFollower::Status::SEARCHING && c.left > 0 && c.right < 0, "search pivots toward last seen side (right)");
  c = f.update(0b000, t + LINE_SEARCH_MAX_MS + 10);
  CHECK(f.status() == LineFollower::Status::LOST && c.left == 0 && c.right == 0, "gives up -> LOST, stop");
  c = f.update(0b010, t + LINE_SEARCH_MAX_MS + 20);
  CHECK(c.left == 0 && f.status() == LineFollower::Status::LOST, "LOST is latched until reset");

  f.reset(t);
  f.update(0b111, t); c = f.update(0b111, t + LINE_ALL_BLACK_MS - 10);
  CHECK(f.status() == LineFollower::Status::FOLLOWING && c.left > 0, "short all-black (junction) -> straight");
  f.update(0b111, t + LINE_ALL_BLACK_MS + 10);
  CHECK(f.status() == LineFollower::Status::LIFTED, "long all-black -> LIFTED");
}

static void unitUltrasonic() {
  SECTION("Ultrasonic: classification and fault detection");
  sim::reset(); Ultrasonic u; u.begin();
  sim::echoDistanceCm = 30;
  int16_t r = u.measureOnce();
  CHECK(r >= 29 && r <= 31, "30 cm measured as ~30");
  sim::echoMode = sim::Echo::OUT_OF_RANGE; sim::nowUs += 60000;
  CHECK(u.measureOnce() == (int16_t)US_MAX_CM, "no return in time -> US_MAX_CM (clear)");
  sim::echoMode = sim::Echo::NO_ECHO; sim::nowUs += 60000;
  CHECK(u.measureOnce() == Ultrasonic::INVALID, "echo never rises (unplugged) -> INVALID");
  sim::echoMode = sim::Echo::STUCK_HIGH;
  CHECK(u.measureOnce() == Ultrasonic::INVALID, "echo stuck high -> INVALID");
  sim::echoMode = sim::Echo::NORMAL; sim::echoDistanceCm = 1; sim::nowUs += 60000;
  CHECK(u.measureOnce() == Ultrasonic::INVALID, "< 2 cm -> INVALID");

  sim::echoMode = sim::Echo::NO_ECHO;
  for (int i = 0; i < US_MAX_BAD_READS; i++) { sim::advanceMs(US_INTERVAL_MS); u.update(millis()); }
  CHECK(u.hasFault(), "5 invalid readings in a row -> fault");
  sim::echoMode = sim::Echo::NORMAL; sim::echoDistanceCm = 50;
  sim::advanceMs(US_INTERVAL_MS); u.update(millis());
  CHECK(!u.hasFault(), "one good reading clears the streak");

  // Median filter rejects a single spike
  sim::echoDistanceCm = 50; sim::advanceMs(US_INTERVAL_MS); u.update(millis());
  sim::echoDistanceCm = 5;  sim::advanceMs(US_INTERVAL_MS); u.update(millis());
  CHECK(u.distanceCm() >= 49, "median ignores one 5 cm spike");
}

static void unitCommands() {
  SECTION("CommandInput: protocol, speed, timeout, ignored keys");
  CommandInput in; in.begin();
  unsigned long t = 5000;
  in.handleChar('F', t);
  DriveCommand c = in.manualDrive(t);
  CHECK(c.left == DEFAULT_SPEED_PCT && c.right == DEFAULT_SPEED_PCT, "F -> forward at default speed");
  in.handleChar('3', t); c = in.manualDrive(t);
  CHECK(c.left == 30, "digit 3 -> 30 %");
  in.handleChar('q', t); in.handleChar('L', t); c = in.manualDrive(t);
  CHECK(c.left == -100 && c.right == 100, "q + L -> spin left at 100 %");
  c = in.manualDrive(t + BT_TIMEOUT_MS + 1);
  CHECK(c.left == 0 && c.right == 0, "no repeat within BT_TIMEOUT_MS -> STOP (link-loss failsafe)");
  in.takeRequest();
  in.handleChar('x', t); in.handleChar('W', t); in.handleChar('\n', t);
  CHECK(in.takeRequest() == CommandInput::Request::NONE, "light/horn/newline chars are ignored");
  in.handleChar('E', t); in.handleChar('T', t);
  CHECK(in.takeRequest() == CommandInput::Request::ESTOP, "emergency stop can't be overwritten by a later mode key");
}

static void unitJoystick() {
  SECTION("JoystickInput: dead-zone, mapping, validation");
  JoystickInput j; j.setCenterForTest(512, 512);
  j.processSample(530, 500);
  CHECK(j.valid() && j.drive().left == 0 && j.drive().right == 0, "inside dead-zone -> stop");
  j.processSample(512, 1018);
  CHECK(j.drive().left == JOY_MAX_SPEED_PCT && j.drive().right == JOY_MAX_SPEED_PCT, "full throttle -> JOY_MAX_SPEED both sides");
  j.processSample(1018, 512);
  CHECK(j.drive().left > 0 && j.drive().right < 0, "full right -> spin right");
  for (int i = 0; i < JOY_MAX_BAD_READS; i++) j.processSample(1023, 1023);
  CHECK(!j.valid() && j.drive().left == 0, "pinned readings -> invalid, stop");
  j.processSample(512, 512);
  CHECK(j.valid(), "recovers after a good sample");
}

static void unitBattery() {
  SECTION("Battery: debounce, hysteresis, USB detection");
  Battery b; unsigned long t = 0;
  for (int i = 0; i < 20; i++) b.feedVolts(7.4f, t += 100);
  CHECK(b.present() && !b.low(), "7.4 V ok");
  for (int i = 0; i < 10; i++) b.feedVolts(6.0f, t += 100);
  CHECK(!b.low(), "1 s of sag ignored");
  for (int i = 0; i < 25; i++) b.feedVolts(6.0f, t += 100);
  CHECK(b.low(), "sustained 6.0 V -> low");
  for (int i = 0; i < 30; i++) b.feedVolts(6.6f, t += 100);
  CHECK(b.low(), "6.6 V is below resume threshold -> still low");
  for (int i = 0; i < 30; i++) b.feedVolts(7.2f, t += 100);
  CHECK(!b.low(), "7.2 V -> clears");
  Battery usb; usb.feedVolts(0.0f, 0);
  CHECK(!usb.present() && !usb.low(), "0 V = USB only, not a fault");
}

// =====================================================================
static void sysBootAndManual() {
  SECTION("System: boot, manual driving, link-loss stop, collision guard");
  boot();
  CHECK(modes.mode() == Mode::IDLE && L() == 0 && R() == 0, "boots IDLE with motors off");
  CHECK(Serial.out.find("ready") != std::string::npos, "prints ready banner");

  hold('F', 1000);
  CHECK(modes.mode() == Mode::BT_MANUAL, "drive key from IDLE enters MANUAL");
  CHECK(L() > 0 && R() > 0, "holding F drives forward");

  run(BT_TIMEOUT_MS + 400);
  CHECK(L() == 0 && R() == 0, "app stops sending -> motors stop (communication loss)");
  CHECK(modes.mode() == Mode::BT_MANUAL, "...but stays in MANUAL, ready to resume");

  sim::echoDistanceCm = 10;
  hold('F', 800);
  CHECK(L() == 0 && R() == 0, "collision guard blocks forward into obstacle at 10 cm");
  hold('B', 800);
  CHECK(L() < 0 && R() < 0, "reverse still allowed to back away");
  bt("S"); run(300);
  CHECK(modes.mode() == Mode::BT_MANUAL && L() == 0, "S in manual = stop, stay in manual");

  bt("x"); bt("W"); run(300);
  CHECK(modes.mode() == Mode::BT_MANUAL, "RC-app light keys don't switch modes");
  bt("K"); run(300);
  CHECK(modes.mode() == Mode::BT_MANUAL, "joystick mode refused when JOYSTICK_FITTED=false");
}

static void sysTransitionRamps() {
  SECTION("System: mode change ramps to stop first");
  boot();
  sim::echoDistanceCm = 80;
  bt("9"); hold('F', 800);
  CHECK(L() > 200, "driving fast");
  bt("O");
  bool reversedDuringTransition = false, enteredBeforeStop = false;
  run(400, [&]() {
    if (modes.inTransition() && (L() < 0 || R() < 0)) reversedDuringTransition = true;
    if (modes.mode() == Mode::OBSTACLE && !modes.inTransition() && modes.lastCommand().left == 0 && L() > 200) enteredBeforeStop = true;
  });
  CHECK(!reversedDuringTransition, "no reverse command while switching");
  CHECK(!enteredBeforeStop, "new mode only starts after motors stopped");
  CHECK(modes.mode() == Mode::OBSTACLE, "arrives in OBSTACLE");
}

static void sysLineFollow() {
  SECTION("System: line following, obstacle pause, line lost fault");
  boot();
  setLine(0, 1, 0);
  bt("T"); run(600);
  CHECK(modes.mode() == Mode::LINE_FOLLOW && L() > 0 && R() > 0, "follows straight line");
  setLine(1, 0, 0); run(200);
  CHECK(L() < R(), "turns toward line on the left");

  setLine(0, 1, 0); sim::echoDistanceCm = 12; run(400);
  CHECK(modes.linePausedForObstacle() && L() == 0 && R() == 0, "obstacle on the track -> pause");
  sim::echoDistanceCm = 80; run(300);
  CHECK(L() == 0, "still waiting (resume delay)");
  run(700);
  CHECK(!modes.linePausedForObstacle() && L() > 0, "resumes once clear for 500 ms");

  setLine(0, 0, 0); run(LINE_SEARCH_MAX_MS + 500);
  CHECK(modes.mode() == Mode::FAULT && modes.fault() == Fault::LINE_LOST, "line lost -> FAULT");
  CHECK(L() == 0 && R() == 0, "motors off in FAULT");
  bt("O"); run(200);
  CHECK(modes.mode() == Mode::FAULT, "mode keys refused while faulted");
  bt("S"); run(200);
  CHECK(modes.mode() == Mode::IDLE && modes.fault() == Fault::NONE, "S clears fault -> IDLE");
}

static void sysObstacle() {
  SECTION("System: obstacle avoidance scan-and-turn");
  boot();
  bool turnedSeen = false, sawLeftLook = false, sawRightLook = false, sawReverse = false, sawLeftTurn = false;
  // Distance depends on where the servo points: wall ahead, open on the left.
  auto world = [&]() {
    if (!turnedSeen) {
      if (sim::servoAngle == SERVO_LEFT_DEG) sim::echoDistanceCm = 90;
      else if (sim::servoAngle == SERVO_RIGHT_DEG) sim::echoDistanceCm = 30;
      else sim::echoDistanceCm = (bot.avoider.state() == ObstacleAvoider::State::CRUISE) ? 15 : 15;
    } else sim::echoDistanceCm = 85;
    auto s = bot.avoider.state();
    if (s == ObstacleAvoider::State::REVERSE && L() < 0 && R() < 0) sawReverse = true;
    if (s == ObstacleAvoider::State::LOOK_LEFT) sawLeftLook = true;
    if (s == ObstacleAvoider::State::LOOK_RIGHT) sawRightLook = true;
    if (s == ObstacleAvoider::State::TURN && L() < 0 && R() > 0) { sawLeftTurn = true; }
    if (s == ObstacleAvoider::State::CHECK && sawLeftTurn) turnedSeen = true;
  };
  sim::echoDistanceCm = 80;
  bt("O"); run(800);
  CHECK(modes.mode() == Mode::OBSTACLE && L() > 0, "cruises forward when clear");
  run(4000, world);
  CHECK(sawReverse, "backs off after stopping");
  CHECK(sawLeftLook && sawRightLook, "scans left and right with the servo");
  CHECK(sawLeftTurn, "turns toward the clearer (left) side");
  CHECK(bot.avoider.state() == ObstacleAvoider::State::CRUISE && L() > 0, "resumes cruising after the turn");
  CHECK(!sim::servoAttached, "servo detached after parking (no holding current)");

  sim::echoMode = sim::Echo::NO_ECHO; run(600);
  CHECK(modes.mode() == Mode::FAULT && modes.fault() == Fault::ULTRASONIC, "unplugged sensor -> ULTRASONIC fault");
}

static void sysBoxedIn() {
  SECTION("System: boxed in -> fault instead of endless shuffling");
  boot();
  bt("O"); run(600);
  sim::echoDistanceCm = 10;
  run(20000);
  CHECK(modes.mode() == Mode::FAULT && modes.fault() == Fault::OBSTACLE_STUCK, "gives up after AVOID_MAX_RETRIES");
}

static void sysBatteryAndEstop() {
  SECTION("System: low battery fault, emergency stop");
  boot();
  setLine(0, 1, 0); bt("T"); run(800);
  CHECK(L() > 0, "moving in line mode");
  bt("E");
  loop();   // a single pass of the main loop
  CHECK(sim::pwmOut[PIN_MOTOR_ENA] == 0 && sim::pwmOut[PIN_MOTOR_ENB] == 0, "E cuts motor enables on the very next loop");
  CHECK(modes.mode() == Mode::IDLE, "E -> IDLE");

  hold('F', 600);
  sim::analogIn[PIN_BATTERY] = adcForVolts(6.0f);
  hold('F', 1000);
  CHECK(modes.mode() != Mode::FAULT, "brief sag below cut-off is tolerated");
  hold('F', 3000);
  CHECK(modes.mode() == Mode::FAULT && modes.fault() == Fault::LOW_BATTERY, "sustained low voltage -> LOW_BATTERY fault");
  CHECK(L() == 0 && R() == 0, "motors off");
  bt("S"); run(300);
  CHECK(modes.mode() == Mode::FAULT, "can't clear while still low");
  sim::analogIn[PIN_BATTERY] = adcForVolts(7.3f); run(1500);
  bt("S"); run(300);
  CHECK(modes.mode() == Mode::IDLE, "clears after recharge");
}

static void sysUsbOnly() {
  SECTION("System: USB-only bench power");
  boot();
  sim::analogIn[PIN_BATTERY] = 0; run(1500);
  bt("?"); run(100);
  CHECK(modes.mode() != Mode::FAULT, "no battery connected is not a fault");
  CHECK(SoftwareSerial::instance->out.find("bat=USB") != std::string::npos, "status reports USB power over Bluetooth");
}

int main() {
  printf("4WD Robot Car — host logic tests (simulated hardware)\n");
  unitMotors(); unitLine(); unitUltrasonic(); unitCommands(); unitJoystick(); unitBattery();
  sysBootAndManual(); sysTransitionRamps(); sysLineFollow(); sysObstacle(); sysBoxedIn();
  sysBatteryAndEstop(); sysUsbOnly();
  printf("\n%d passed, %d failed\n", passed, failed);
  return failed ? 1 : 0;
}
