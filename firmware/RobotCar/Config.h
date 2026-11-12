// Config.h — every pin and tunable value lives here.
// Values are design defaults, NOT yet tuned on hardware. Tune them during
// the tests in docs/testing-guide.md and record what you changed.
#pragma once
#include <Arduino.h>

// ---------------- Pins (see docs/wiring.md) ----------------
// D0/D1 reserved for USB upload + Serial Monitor
constexpr uint8_t PIN_BT_RX        = 2;   // <- HC-05 TXD (direct)
constexpr uint8_t PIN_BT_TX        = 3;   // -> HC-05 RXD via 1k/2k divider
constexpr uint8_t PIN_MOTOR_IN1    = 4;   // left dir A
constexpr uint8_t PIN_MOTOR_ENA    = 5;   // left PWM (Timer0)  - remove ENA jumper
constexpr uint8_t PIN_MOTOR_ENB    = 6;   // right PWM (Timer0) - remove ENB jumper
constexpr uint8_t PIN_MOTOR_IN2    = 7;   // left dir B
constexpr uint8_t PIN_MOTOR_IN3    = 8;   // right dir A
constexpr uint8_t PIN_SERVO        = 9;   // Timer1 (Servo lib) -> no PWM on D9/D10
constexpr uint8_t PIN_MOTOR_IN4    = 10;  // right dir B (digital only)
constexpr uint8_t PIN_US_TRIG      = 11;
constexpr uint8_t PIN_US_ECHO      = 12;
constexpr uint8_t PIN_STATUS_LED   = 13;  // on-board "L" LED
constexpr uint8_t PIN_LINE_LEFT    = A0;
constexpr uint8_t PIN_LINE_CENTER  = A1;
constexpr uint8_t PIN_LINE_RIGHT   = A2;
constexpr uint8_t PIN_JOY_X        = A3;  // KY-023 VRx (steering)
constexpr uint8_t PIN_JOY_Y        = A4;  // KY-023 VRy (throttle)
constexpr uint8_t PIN_BATTERY      = A5;  // 10k/10k divider from VBAT

// ---------------- Build options ----------------
constexpr bool    JOYSTICK_FITTED      = false; // set true once the KY-023 is wired
constexpr bool    SERVO_FITTED         = true;  // false = no scanning; avoider turns blind-left
constexpr bool    BATTERY_SENSE_FITTED = true;  // false = skip low-battery checks
constexpr bool    LINE_ACTIVE_LOW      = false; // flip if your modules output LOW on black
constexpr bool    INVERT_LEFT_MOTORS   = false; // flip instead of rewiring
constexpr bool    INVERT_RIGHT_MOTORS  = false;
constexpr bool    JOY_INVERT_X         = false;
constexpr bool    JOY_INVERT_Y         = false;

// ---------------- Serial / debug ----------------
constexpr bool     DEBUG_ENABLED       = true;
constexpr uint32_t DEBUG_BAUD          = 115200;
constexpr uint32_t BT_BAUD             = 9600;  // HC-05 default data-mode baud
constexpr uint16_t DEBUG_INTERVAL_MS   = 250;

// ---------------- Motors ----------------
constexpr uint8_t  PWM_MAX             = 255;
constexpr uint8_t  PWM_MIN_MOVING      = 90;    // below this TT motors usually stall - tune
constexpr uint8_t  RAMP_STEP           = 15;    // PWM change per ramp tick
constexpr uint16_t RAMP_INTERVAL_MS    = 10;    // 0 -> 255 in ~170 ms
constexpr uint8_t  DEFAULT_SPEED_PCT   = 60;    // manual-mode speed at boot
constexpr uint16_t MODE_SETTLE_MS      = 200;   // pause at STOP between modes

// ---------------- Line following ----------------
constexpr int8_t   LINE_BASE_SPEED_PCT  = 45;   // both sides on straight line
constexpr int8_t   LINE_OUTER_SPEED_PCT = 55;   // outer wheel during a correction
constexpr int8_t   LINE_SOFT_INNER_PCT  = 15;   // inner wheel, line slightly off-centre
constexpr int8_t   LINE_HARD_INNER_PCT  = -35;  // inner wheel reverses on sharp curves
constexpr int8_t   LINE_SEARCH_PCT      = 45;   // pivot speed while searching
constexpr uint16_t LINE_LOST_MS         = 400;  // keep last command through gaps
constexpr uint16_t LINE_SEARCH_MAX_MS   = 1500; // total time without line before FAULT
constexpr uint16_t LINE_ALL_BLACK_MS    = 800;  // 3x black this long = lifted/off-track
constexpr uint16_t LINE_OBSTACLE_RESUME_MS = 500; // path must stay clear this long

// ---------------- Ultrasonic / obstacle avoidance ----------------
constexpr uint16_t OBSTACLE_STOP_CM    = 20;
constexpr uint16_t OBSTACLE_SLOW_CM    = 40;
constexpr uint16_t US_MAX_CM           = 100;   // "nothing in range" is reported as this
constexpr uint16_t US_MIN_CM           = 2;     // below = invalid
constexpr uint16_t US_TIMEOUT_US       = 6000;  // echo pulse cap (~1 m); bounds blocking
constexpr uint16_t US_RISE_TIMEOUT_US  = 3000;  // echo must start within this or = fault
constexpr uint16_t US_INTERVAL_MS      = 60;    // HC-SR04 min cycle
constexpr uint8_t  US_MAX_BAD_READS    = 5;     // consecutive invalid -> sensor fault
constexpr int8_t   AVOID_CRUISE_PCT    = 55;
constexpr int8_t   AVOID_SLOW_PCT      = 35;
constexpr int8_t   AVOID_REVERSE_PCT   = 45;
constexpr int8_t   AVOID_TURN_PCT      = 55;
constexpr uint16_t AVOID_STOP_MS       = 250;
constexpr uint16_t AVOID_REVERSE_MS    = 300;
constexpr uint16_t AVOID_TURN_MS       = 450;   // ~90 deg - tune on your floor
constexpr uint8_t  AVOID_MAX_RETRIES   = 4;     // boxed in -> FAULT
constexpr uint8_t  SERVO_CENTER_DEG    = 90;
constexpr uint8_t  SERVO_LEFT_DEG      = 160;
constexpr uint8_t  SERVO_RIGHT_DEG     = 20;
constexpr uint16_t SERVO_SETTLE_MS     = 350;

// ---------------- Manual control / safety ----------------
constexpr uint16_t BT_TIMEOUT_MS       = 600;   // no drive command -> stop motors
constexpr bool     MANUAL_COLLISION_GUARD = true; // block forward if obstacle < STOP_CM
constexpr uint16_t JOY_DEADZONE        = 60;    // ADC counts around centre
constexpr uint16_t JOY_VALID_MIN       = 5;     // readings outside = disconnected/shorted
constexpr uint16_t JOY_VALID_MAX       = 1018;
constexpr uint16_t JOY_CENTER_TOL      = 120;   // centre calibration must be 512 +/- this
constexpr uint8_t  JOY_MAX_BAD_READS   = 3;
constexpr uint16_t JOY_INTERVAL_MS     = 20;
constexpr uint8_t  JOY_MAX_SPEED_PCT   = 80;    // full deflection = this speed
constexpr uint16_t JOY_EDGE_MARGIN     = 20;    // counts short of the end stop = full speed
constexpr float    BATTERY_DIVIDER     = 2.0f;  // 10k/10k
constexpr float    BATTERY_CAL         = 1.00f; // multiply to match your multimeter
constexpr float    BATTERY_PRESENT_V   = 3.0f;  // below = running on USB only
constexpr float    BATTERY_CUTOFF_V    = 6.4f;  // 3.2 V/cell
constexpr float    BATTERY_RESUME_V    = 6.8f;  // hysteresis
constexpr uint16_t BATTERY_LOW_HOLD_MS = 2000;  // must stay low this long (ignores motor sag)
constexpr uint16_t BATTERY_INTERVAL_MS = 100;
