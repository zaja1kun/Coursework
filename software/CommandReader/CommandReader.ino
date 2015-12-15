#define SENSOR_PIN A0
#define BATTERY_VOLTAGE_PIN A1
#define COMMAND_F_PIN 2
#define COMMAND_B_PIN 4
#define COMMAND_R_PIN 8
#define COMMAND_L_PIN 7
#define MOVE_F_PIN 5
#define MOVE_B_PIN 6
#define MOVE_R_PIN 10
#define MOVE_L_PIN 9
#define BEACON_PIN 11

#define CYCLE_TIMEOUT 10
#define MODE_CHANGE_TIMEOUT 2000

#define L_BIAS 160
#define R_BIAS 230
#define NO_BIAS 195

#define MANUAL_MODE 0
#define AUTO_MODE 0

int ControllerVAxisState = 1;
int ControllerHAxisState = 1;
bool UpdateRequested = true;
int ModeChangeTimer = 0;
byte Mode = MANUAL_MODE;
const String states[3][3] = {{"< /\\  ", "  /\\  ", "  /\\ >"},
  {"< ||  ",  "  ||  ",  "  || >"},
  {"< \\/  ", "  \\/  ", "  \\/ >"}
};

int command = 0;
int prevcommand = 0;

float getPinVoltage(int pin);
void controlModeChange();
void updateRequestedCarState();
void setCarEnginesState();

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(BATTERY_VOLTAGE_PIN, INPUT);

  pinMode(COMMAND_F_PIN, INPUT);
  pinMode(COMMAND_B_PIN, INPUT);
  pinMode(COMMAND_R_PIN, INPUT);
  pinMode(COMMAND_L_PIN, INPUT);

  pinMode(MOVE_F_PIN, OUTPUT);
  pinMode(MOVE_B_PIN, OUTPUT);
  pinMode(MOVE_R_PIN, OUTPUT);
  pinMode(MOVE_L_PIN, OUTPUT);

  setCarEnginesState(1, 1);
  Serial.begin(9600);
}

void loop() {
  updateRequestedCarState();

  if (Mode == MANUAL_MODE) {
    if (UpdateRequested) {
      Serial.println(states[ControllerVAxisState][ControllerHAxisState]);
      setCarEnginesState(ControllerVAxisState, ControllerHAxisState);
      UpdateRequested = false;
      ModeChangeTimer = 0;
    }
    else {
      if (ModeChangeTimer * CYCLE_TIMEOUT < MODE_CHANGE_TIMEOUT)
        ModeChangeTimer += 1;
      else if (ControllerVAxisState == 1 && ControllerHAxisState == 0 && ModeChangeTimer * CYCLE_TIMEOUT == MODE_CHANGE_TIMEOUT) {
        ModeChangeTimer += 1;
        controlModeChange();
      }
    }
  } else {
    if (UpdateRequested) {
      UpdateRequested = false;
      ModeChangeTimer = 0;
    } else {
      if (ModeChangeTimer * CYCLE_TIMEOUT < MODE_CHANGE_TIMEOUT)
        ModeChangeTimer += 1;
      else if (ControllerVAxisState == 1 && ControllerHAxisState == 0 && ModeChangeTimer * CYCLE_TIMEOUT == MODE_CHANGE_TIMEOUT) {
        ModeChangeTimer += 1;
        controlModeChange();
      }
    }
    int sensorValue = analogRead(SENSOR_PIN);
    if (!prevcommand) {
      if (sensorValue >= R_BIAS) {
        command = -1;
      }
      else if (sensorValue <= L_BIAS) {
        command = 1;
      }
    } else {
      if (prevcommand == 1) {
        if (sensorValue >= NO_BIAS) {
          command = 0;
        }
      } else if (prevcommand == -1) {
        if (sensorValue <= NO_BIAS) {
          command = 0;
        }
      }
    }
    if (prevcommand != command)
      switch (command) {
        case - 1:
          Serial.println("left");
          setCarEnginesState(0, 0);
          break;
        case 0:
          Serial.println("neutral");
          setCarEnginesState(0, 1);
          break;
        case 1:
          Serial.println("right");
          setCarEnginesState(0, 2);
          break;
      }
    prevcommand = command;
  }

  delay(CYCLE_TIMEOUT);
}

float getPinVoltage(int pin) {
  return analogRead(pin) * (5.0 / 1023.0);
}

void controlModeChange() {
  Serial.println("Info: Mode changed.");
  Mode = !Mode;
  digitalWrite(BEACON_PIN, Mode);
}

void updateRequestedCarState() {
  if (digitalRead(COMMAND_F_PIN) == HIGH) {
    if (ControllerVAxisState != 0)
      UpdateRequested = true;
    ControllerVAxisState = 0;
  }
  else if (digitalRead(COMMAND_B_PIN) == HIGH) {
    if (ControllerVAxisState != 2)
      UpdateRequested = true;
    ControllerVAxisState = 2;
  }
  else {
    if (ControllerVAxisState != 1)
      UpdateRequested = true;
    ControllerVAxisState = 1;
  }

  if (digitalRead(COMMAND_L_PIN) == HIGH) {
    if (ControllerHAxisState != 0)
      UpdateRequested = true;
    ControllerHAxisState = 0;
  }
  else if (digitalRead(COMMAND_R_PIN) == HIGH) {
    if (ControllerHAxisState != 2)
      UpdateRequested = true;
    ControllerHAxisState = 2;
  }
  else {
    if (ControllerHAxisState != 1)
      UpdateRequested = true;
    ControllerHAxisState = 1;
  }
}

void setCarEnginesState(int fb, int lr) {
  byte engineFState = fb == 0 ? 1 : 0;
  byte engineBState = fb == 2 ? 1 : 0;
  byte engineLState = lr == 0 ? 1 : 0;
  byte engineRState = lr == 2 ? 1 : 0;

  digitalWrite(MOVE_L_PIN, engineLState);
  digitalWrite(MOVE_R_PIN, engineRState);
  analogWrite(MOVE_F_PIN, engineFState * 230);
  analogWrite(MOVE_B_PIN, engineBState * 230);
}

