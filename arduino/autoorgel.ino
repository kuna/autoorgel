#include <LiquidCrystal_I2C.h>
#include <Wire.h>
// #include <TMCStepper.h>

const int RENC_CLK = 2;
const int RENC_DT = 3;
const int RENC_SW = 4;
const int LCD_SDA = A5;
const int LCD_SDL = A6;
const int TMC_DIR = 11;
const int TMC_STEP = 12;

LiquidCrystal_I2C lcd(0x27, 16, 2);

static byte i2c_device_port = 0xFF;
static void init_scan_I2C_device() {
  if (i2c_device_port != 0xFF) {
    return;
  }
  byte addr, err;
  i2c_device_port = 0; // not found case
  for (addr = 1; addr < 128; addr++) {
    Wire.beginTransmission(addr);
    err = Wire.endTransmission(addr);
    if (err == 0) {
      i2c_device_port = addr;
      break;
    }
  }
  Serial.print("I2C scanning done: 0x");
  Serial.println(i2c_device_port, HEX);
}

void setup() {
  // Setup pins
  pinMode(RENC_CLK, INPUT);
  pinMode(RENC_DT, INPUT);
  pinMode(RENC_SW, INPUT_PULLUP);
  pinMode(TMC_DIR, OUTPUT);
  pinMode(TMC_STEP, OUTPUT);

  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  while (!Serial); // wait for serial driver

  // Wire begin for scanning I2C
  // (could be deleted after LCD parameter is hardcoded)
  Wire.begin();
  init_scan_I2C_device();

  // lcd init
  lcd.init();
  lcd.backlight();

  Serial.println("Init done.");
}

void loop() {
  static int prev_clk = LOW;
  static int current = 0;
  int dt = digitalRead(RENC_DT);
  int clk = digitalRead(RENC_CLK);
  if (clk != prev_clk && clk == HIGH) {
    // If value has been changed ...
    if (dt != clk) {
      current++;
    } else {
      current--;
    }
    Serial.println(current);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Value ");
    lcd.print(current);
  }
  prev_clk = clk;

  // Check button press
	int st_btn = digitalRead(RENC_SW);
  static int current_speed = 0;
  if (st_btn == LOW) {
    Serial.println("Button pressed!!");
    lcd.clear();
    lcd.setCursor(0, 0);
    current_speed = current;
    if (current_speed > 20) {
      current_speed = 20;
    } else if (current_speed < -20) {
      current_speed = -20;
    }
    lcd.print("Set speed to ");
    lcd.print(current_speed);
  }

  // control motor
  static int motor_clk = 0;
  static int motor_rot = 0;
  if (current_speed != 0) {
    motor_clk++;
    if (motor_clk > (21 - abs(current_speed)) * 3) {
      motor_clk = 0;
      motor_rot = (motor_rot + 1) % 2;
    }
    digitalWrite(TMC_DIR, (current_speed > 0) ? HIGH : LOW);
    digitalWrite(TMC_STEP, (motor_rot % 2) ? HIGH : LOW);
  }

  // Tentative delay to prevent log overflow
  // delay(100);
}
