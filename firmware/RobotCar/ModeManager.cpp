#include "ModeManager.h"
#include "Config.h"

const __FlashStringHelper* modeName(Mode m) {
  switch (m) {
    case Mode::IDLE:        return F("IDLE");
    case Mode::LINE_FOLLOW: return F("LINE");
    case Mode::OBSTACLE:    return F("OBSTACLE");
    case Mode::BT_MANUAL:   return F("MANUAL");
    case Mode::JOYSTICK:    return F("JOYSTICK");
    default:                return F("FAULT");
  }
}

const __FlashStringHelper* faultName(Fault f) {
  switch (f) {
    case Fault::NONE:             return F("NONE");
    case Fault::LOW_BATTERY:      return F("LOW_BATTERY");
    case Fault::LINE_LOST:        return F("LINE_LOST");
    case Fault::LINE_LIFTED:      return F("LINE_ALL_BLACK");
    case Fault::ULTRASONIC:       return F("ULTRASONIC");
    case Fault::JOYSTICK_INVALID: return F("JOYSTICK");
    default:                      return F("BOXED_IN");
  }
}

void ModeManager::begin(unsigned long now) {
  current = pending = Mode::IDLE;
  bot.motors.stopNow();
  announce(now);
}

void ModeManager::announce(unsigned long) {
  Print& bt = bot.input.bluetooth();
  Serial.print(F("MODE -> ")); Serial.println(modeName(current));
  bt.print(F("MODE ")); bt.println(modeName(current));
  if (current == Mode::FAULT) {
    Serial.print(F("FAULT: ")); Serial.println(faultName(activeFault));
    bt.print(F("FAULT ")); bt.println(faultName(activeFault));
  }
}

void ModeManager::requestMode(Mode m, unsigned long now) {
  if (current == Mode::FAULT && m != Mode::IDLE) {
    bot.input.reply(F("Fault active: send S to clear first"));
    return;
  }
  if (m == Mode::JOYSTICK && !bot.joystick.valid()) {
    bot.input.reply(F("Joystick not fitted/calibrated (JOYSTICK_FITTED in Config.h)"));
    return;
  }
  if (m == current && !transitioning) return;
  pending = m;
  transitioning = true;
  transitionAt = now;
  bot.scanner.park(now);
}

void ModeManager::enterFault(Fault f, unsigned long now) {
  bot.motors.stopNow();                 // instant, no ramp
  activeFault = f;
  current = pending = Mode::FAULT;
  transitioning = false;
  lastCmd = DRIVE_STOP;
  bot.scanner.park(now);
  announce(now);
}

void ModeManager::onEnter(Mode m, unsigned long now) {
  current = m;
  linePaused = false;
  switch (m) {
    case Mode::LINE_FOLLOW: bot.follower.reset(now); break;
    case Mode::OBSTACLE:    bot.sonar.resetFault(); bot.sonar.resetFilter();
                            bot.avoider.reset(now, bot.scanner); break;
    case Mode::BT_MANUAL:   bot.input.clearDrive(); break;
    default: break;
  }
  announce(now);
}

void ModeManager::handleRequests(unsigned long now) {
  bool driveKey = bot.input.takeDriveKeyFlag();
  CommandInput::Request r = bot.input.takeRequest();

  switch (r) {
    case CommandInput::Request::ESTOP:
      bot.motors.stopNow();
      transitioning = false;
      if (bot.battery.low()) { if (current != Mode::FAULT) enterFault(Fault::LOW_BATTERY, now); }
      else { activeFault = Fault::NONE; onEnter(Mode::IDLE, now); }
      bot.input.reply(F("EMERGENCY STOP"));
      return;
    case CommandInput::Request::STOP:
      if (current == Mode::BT_MANUAL && !transitioning) break;   // S = release button
      if (current == Mode::FAULT) {
        if (bot.battery.low()) { bot.input.reply(F("Battery still low")); return; }
        activeFault = Fault::NONE;
        onEnter(Mode::IDLE, now);
        return;
      }
      requestMode(Mode::IDLE, now);
      return;
    case CommandInput::Request::MODE_LINE:     requestMode(Mode::LINE_FOLLOW, now); return;
    case CommandInput::Request::MODE_OBSTACLE: requestMode(Mode::OBSTACLE, now); return;
    case CommandInput::Request::MODE_MANUAL:   requestMode(Mode::BT_MANUAL, now); return;
    case CommandInput::Request::MODE_JOYSTICK: requestMode(Mode::JOYSTICK, now); return;
    case CommandInput::Request::STATUS:
      printStatus(Serial); printStatus(bot.input.bluetooth()); break;
    default: break;
  }
  // Pressing a drive key while idle is a shortcut into manual mode.
  if (driveKey && current == Mode::IDLE && !transitioning) requestMode(Mode::BT_MANUAL, now);
}

bool ModeManager::obstacleAhead() const {
  return bot.sonar.hasReading() && !bot.sonar.hasFault() &&
         bot.sonar.distanceCm() <= OBSTACLE_STOP_CM;
}

DriveCommand ModeManager::behaviour(unsigned long now) {
  switch (current) {
    case Mode::LINE_FOLLOW: {
      uint8_t mask = bot.line.last();     // sampled once per loop in RobotCar.ino
      // Something on the track: wait (motors stopped) until it has been clear a while.
      if (obstacleAhead()) { linePaused = true; lineClearSince = now; return DRIVE_STOP; }
      if (linePaused) {
        if (now - lineClearSince < LINE_OBSTACLE_RESUME_MS) return DRIVE_STOP;
        linePaused = false;
        bot.follower.reset(now);        // don't count the pause as "line lost" time
      }
      DriveCommand c = bot.follower.update(mask, now);
      if (bot.follower.status() == LineFollower::Status::LOST)   enterFault(Fault::LINE_LOST, now);
      if (bot.follower.status() == LineFollower::Status::LIFTED) enterFault(Fault::LINE_LIFTED, now);
      return current == Mode::FAULT ? DRIVE_STOP : c;
    }
    case Mode::OBSTACLE: {
      if (bot.sonar.hasFault()) { enterFault(Fault::ULTRASONIC, now); return DRIVE_STOP; }
      DriveCommand c = bot.avoider.update(bot.sonar, bot.scanner, now);
      if (bot.avoider.stuck()) { enterFault(Fault::OBSTACLE_STUCK, now); return DRIVE_STOP; }
      return c;
    }
    case Mode::BT_MANUAL:
      return bot.input.manualDrive(now);          // decays to STOP on link loss
    case Mode::JOYSTICK:
      if (!bot.joystick.valid()) { enterFault(Fault::JOYSTICK_INVALID, now); return DRIVE_STOP; }
      return bot.joystick.drive();
    default:
      return DRIVE_STOP;
  }
}

DriveCommand ModeManager::safetyFilter(DriveCommand c) const {
  if (current == Mode::FAULT || bot.battery.low()) return DRIVE_STOP;
  // Manual modes: refuse to drive forward into something. Reverse and
  // spinning in place are still allowed so the driver can get out.
  bool manual = current == Mode::BT_MANUAL || current == Mode::JOYSTICK;
  if (MANUAL_COLLISION_GUARD && manual && c.left > 0 && c.right > 0 && obstacleAhead())
    return DRIVE_STOP;
  return c;
}

void ModeManager::update(unsigned long now) {
  handleRequests(now);

  if (bot.battery.low() && current != Mode::FAULT) enterFault(Fault::LOW_BATTERY, now);

  DriveCommand c = DRIVE_STOP;
  if (transitioning) {
    bot.motors.setTarget(DRIVE_STOP);
    if (bot.motors.isStopped() && now - transitionAt >= MODE_SETTLE_MS) {
      transitioning = false;
      onEnter(pending, now);
    }
  } else {
    c = behaviour(now);
  }
  c = safetyFilter(c);
  lastCmd = c;
  bot.motors.setTarget(c);
}

void ModeManager::printStatus(Print& out) const {
  out.print(F("mode="));   out.print(modeName(current));
  out.print(F(" fault=")); out.print(faultName(activeFault));
  out.print(F(" dist="));
  if (bot.sonar.hasFault()) out.print(F("ERR"));
  else { out.print(bot.sonar.distanceCm()); out.print(F("cm")); }
  out.print(F(" line="));
  uint8_t m = bot.line.last();
  out.print((m & 4) ? '1' : '0'); out.print((m & 2) ? '1' : '0'); out.print((m & 1) ? '1' : '0');
  out.print(F(" bat="));
  if (bot.battery.present()) { out.print(bot.battery.volts(), 2); out.print('V'); }
  else out.print(F("USB"));
  out.print(F(" speed=")); out.print(bot.input.speedPct());
  out.print(F("% L="));    out.print(bot.motors.pwmLeft());
  out.print(F(" R="));     out.println(bot.motors.pwmRight());
}
