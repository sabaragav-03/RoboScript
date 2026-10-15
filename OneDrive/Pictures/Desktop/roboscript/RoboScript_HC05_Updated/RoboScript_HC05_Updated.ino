
#include <AFMotor.h>
#include <SoftwareSerial.h>

SoftwareSerial bt(10, 9);   // RX, TX

AF_DCMotor motor1(1);  // left front
AF_DCMotor motor2(2);  // left rear
AF_DCMotor motor3(3);  // right front
AF_DCMotor motor4(4);  // right rear

const int MOTOR_SPEED = 200;
const float LEFT_TRIM  = 1.00;
const float RIGHT_TRIM = 1.00;

const unsigned long MOVE_UNIT_MS   = 111;
const unsigned long TURN_DEGREE_MS = 6;

String input = "";

void reply(const __FlashStringHelper *msg) {
  bt.println(msg);
  Serial.println(msg);
}

void setup() {
  Serial.begin(9600);
  bt.begin(9600);
  setSpeeds();
  stopCar();
  reply(F("RoboScript BT RC Car Ready"));
}

void loop() {
  while (bt.available() > 0) handleChar(bt.read());
  while (Serial.available() > 0) handleChar(Serial.read());
}

void handleChar(char c) {
  if (c == '\n' || c == '\r' || c == ',' || c == ';' || c == ' ') {
    input.trim();
    if (input.length() > 0) executeCommand(input);
    input = "";
  } else if (input.length() < 10) {
    input += c;
  } else {
    input = "";
    reply(F("ERROR: command too long"));
  }
}

void setSpeeds() {
  motor1.setSpeed(MOTOR_SPEED * LEFT_TRIM);
  motor2.setSpeed(MOTOR_SPEED * LEFT_TRIM);
  motor3.setSpeed(MOTOR_SPEED * RIGHT_TRIM);
  motor4.setSpeed(MOTOR_SPEED * RIGHT_TRIM);
}

void executeCommand(String cmd) {
  cmd.toUpperCase();

  if (cmd == "S") {
    stopCar();
    reply(F("OK: STOP"));
    return;
  }

  if (cmd.length() < 2) {
    reply(F("ERROR: use F100, B100, L090, R090 or S"));
    return;
  }

  char action = cmd.charAt(0);
  String numberPart = cmd.substring(1);

  for (unsigned int i = 0; i < numberPart.length(); i++) {
    if (!isDigit(numberPart.charAt(i))) {
      reply(F("ERROR: invalid number"));
      return;
    }
  }

  int value = numberPart.toInt();

  if (value <= 0 || value > 999) {
    reply(F("ERROR: value must be 1-999"));
    return;
  }

  unsigned long duration;

  switch (action) {
    case 'F':
      duration = (unsigned long)value * MOVE_UNIT_MS;
      driveForward();
      reply(F("OK: FORWARD"));
      break;

    case 'B':
      duration = (unsigned long)value * MOVE_UNIT_MS;
      driveBackward();
      reply(F("OK: BACKWARD"));
      break;

    case 'L':
      duration = (unsigned long)value * TURN_DEGREE_MS;
      turnLeft();
      reply(F("OK: LEFT"));
      break;

    case 'R':
      duration = (unsigned long)value * TURN_DEGREE_MS;
      turnRight();
      reply(F("OK: RIGHT"));
      break;

    default:
      reply(F("ERROR: unknown command"));
      return;
  }

  unsigned long startTime = millis();

  while (millis() - startTime < duration) {
    if (bt.available() && (bt.peek() == 'S' || bt.peek() == 's')) {
      bt.read();
      stopCar();
      reply(F("OK: STOP (interrupted)"));
      return;
    }
  }

  stopCar();
  delay(150);
  reply(F("DONE"));
}

void driveForward() {
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void driveBackward() {
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void turnLeft() {
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void turnRight() {
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void stopCar() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}
