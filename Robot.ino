#include <Bluepad32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==========================================
// DEFINISI PIN MOTOR, LED, & BATERAI
// ==========================================
// KIRI
const int LEFT_IN1 = 16;
const int LEFT_IN2 = 17;

// KANAN
const int RIGHT_IN3 = 5;
const int RIGHT_IN4 = 18;

// INDIKATOR & SENSOR
const int LED_BUILTIN_PIN = 2; 
const int BATTERY_PIN = 32; 

// ==========================================
// DEFINISI LAYAR OLED (I2C)
// ==========================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==========================================
// SETTING WAKTU
// ==========================================
const unsigned long WAKTU_MUNDUR = 800;  
const unsigned long WAKTU_BATERAI = 180000; // 3 Menit = 180.000 milidetik

// Variabel Global Controller & Macro
GamepadPtr myGamepad = nullptr;
unsigned long macroTimer = 0;
int activeMacro = 0; 
bool modeBrutal = true;     
bool lastR3State = false;   

// Variabel untuk Layar & Baterai
unsigned long lastDisplayUpdate = 0;
unsigned long lastBatteryUpdate = 0; // Timer khusus baterai
bool blinkDot = false;
int currentRPM = 0; 
int currentBattery = 0; 

// ==========================================
// FUNGSI BERHENTI DARURAT
// ==========================================
void stopMotor() {
  analogWrite(LEFT_IN1, 0);
  analogWrite(LEFT_IN2, 0);
  analogWrite(RIGHT_IN3, 0);
  analogWrite(RIGHT_IN4, 0);
  currentRPM = 0;
}

// ==========================================
// FUNGSI KONEKSI BLUETOOTH
// ==========================================
void onConnectedGamepad(GamepadPtr gp) {
  if (myGamepad == nullptr) {
    myGamepad = gp;
    BP32.enableNewBluetoothConnections(false); 
  }
}

void onDisconnectedGamepad(GamepadPtr gp) {
  if (myGamepad == gp) {
    myGamepad = nullptr;
    stopMotor(); 
    BP32.enableNewBluetoothConnections(true); 
  }
}

// ==========================================
// FUNGSI BACA BATERAI (RUMUS RATA-RATA ANTI RANDOM)
// ==========================================
void updateBattery() {
  long totalADC = 0;
  
  // Baca sensor 20 kali secara beruntun dengan super cepat
  for (int i = 0; i < 20; i++) {
    totalADC += analogRead(BATTERY_PIN);
  }
  
  // Hitung nilai rata-ratanya
  int avgADC = totalADC / 20;
  
  // Masukkan angka rata-rata ke rumus kalibrasimu
  currentBattery = map(avgADC, 700, 950, 0, 100);
  currentBattery = constrain(currentBattery, 0, 100);
}

// ==========================================
// FUNGSI UPDATE LAYAR OLED
// ==========================================
void updateOLED() {
  display.clearDisplay();

  // Baris 1: Status Koneksi
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  if (myGamepad && myGamepad->isConnected()) {
    display.print("Konek ");
  } else {
    display.print("Nyari ");
  }

  if (blinkDot) display.print(".");

  // Baris 1 Kanan: Status Baterai
  display.setCursor(80, 0);
  display.print("Bat:");
  display.print(currentBattery);
  display.print("%");

  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  // Baris 2: Indikator Mode Gigi
  display.setCursor(0, 15);
  display.print("Mode: ");
  display.print(modeBrutal ? "BRUTAL (100%)" : "KALEM (50%)");

  // Baris 3: Tampilan RPM (Besar)
  display.setTextSize(3);
  display.setCursor(0, 35);
  display.print("RPM:");
  display.print(currentRPM);
  
  display.setTextSize(1);
  display.print("%");

  display.display();
}

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  pinMode(BATTERY_PIN, INPUT); 
  
  digitalWrite(LED_BUILTIN_PIN, HIGH); 
  stopMotor();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED Gagal Terhubung!"));
  } else {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.print("BOOTING..");
    display.display();
    delay(1000);
  }

  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

  // Panggil fungsi baca baterai 1x saat pertama kali mobil dinyalakan
  updateBattery();
}

// ==========================================
// LOOP UTAMA
// ==========================================
void loop() {
  BP32.update();

  int leftMotorSpeed = 0;
  int rightMotorSpeed = 0;

  if (myGamepad && myGamepad->isConnected()) {
    
    // 0. PANIC BUTTON
    if (myGamepad->miscButtons() & 0x02) {
      activeMacro = 0; macroTimer = 0; stopMotor(); delay(500); return;                
    }

    // 0.5 TOGGLE GIGI
    bool currentR3 = myGamepad->thumbR();
    if (currentR3 && !lastR3State) {
      modeBrutal = !modeBrutal; 
      digitalWrite(LED_BUILTIN_PIN, modeBrutal ? HIGH : LOW);
    }
    lastR3State = currentR3; 

    // 1. BACA INPUT ANALOG & DPAD
    int axisX = myGamepad->axisX();
    int axisY = myGamepad->axisY();
    uint8_t dpad = myGamepad->dpad();
    
    // DEADZONE DINAIKKAN JADI 80 BIAR KEBAL STICK DRIFT
    if (abs(axisX) < 80) axisX = 0;
    if (abs(axisY) < 80) axisY = 0;

    // 2. TRIGGER MACRO MENGHINDAR (DIPERBAIKI)
    if (activeMacro == 0) {
      if (myGamepad->r2() > 50 || myGamepad->r1()) {
        activeMacro = 1; 
        macroTimer = millis();
      }
      else if (myGamepad->l2() > 50 || myGamepad->l1()) {
        activeMacro = 2; 
        macroTimer = millis();
      }
    }

    // 3. EKSEKUSI PERGERAKAN
    if (activeMacro == 1) {
      unsigned long elapsed = millis() - macroTimer;
      if (elapsed < WAKTU_MUNDUR) {
        leftMotorSpeed = -512; rightMotorSpeed = 200;  
      } else { activeMacro = 0; }
    } 
    else if (activeMacro == 2) {
      unsigned long elapsed = millis() - macroTimer;
      if (elapsed < WAKTU_MUNDUR) {
        leftMotorSpeed = -200; rightMotorSpeed = 512;  
      } else { activeMacro = 0; }
    }
    else {
      // KONTROL MANUAL
      if (dpad & DPAD_UP) { leftMotorSpeed = 512; rightMotorSpeed = -512; } 
      else if (dpad & DPAD_DOWN) { leftMotorSpeed = -512; rightMotorSpeed = 512; } 
      else if (dpad & DPAD_RIGHT) { leftMotorSpeed = 512; rightMotorSpeed = 512; } 
      else if (dpad & DPAD_LEFT) { leftMotorSpeed = -512; rightMotorSpeed = -512; }
      else if (myGamepad->a()) { leftMotorSpeed = 512; rightMotorSpeed = -512; } 
      else if (myGamepad->b()) { leftMotorSpeed = -512; rightMotorSpeed = 512; } 
      else {
        int throttle = -axisY; 
        int steering = axisX;  
        leftMotorSpeed = throttle + steering;
        rightMotorSpeed = -throttle + steering;
      }
    }

    // 4. LIMITER RPM & OUTPUT
    if (!modeBrutal && activeMacro == 0) {
      leftMotorSpeed = leftMotorSpeed / 2;
      rightMotorSpeed = rightMotorSpeed / 2;
    }

    leftMotorSpeed = constrain(leftMotorSpeed, -512, 512);
    rightMotorSpeed = constrain(rightMotorSpeed, -512, 512);

    int pwmLeft = map(abs(leftMotorSpeed), 0, 512, 0, 255);
    int pwmRight = map(abs(rightMotorSpeed), 0, 512, 0, 255);

    if (leftMotorSpeed > 0) { analogWrite(LEFT_IN1, pwmLeft); analogWrite(LEFT_IN2, 0); } 
    else if (leftMotorSpeed < 0) { analogWrite(LEFT_IN1, 0); analogWrite(LEFT_IN2, pwmLeft); } 
    else { analogWrite(LEFT_IN1, 0); analogWrite(LEFT_IN2, 0); }

    if (rightMotorSpeed > 0) { analogWrite(RIGHT_IN3, pwmRight); analogWrite(RIGHT_IN4, 0); } 
    else if (rightMotorSpeed < 0) { analogWrite(RIGHT_IN3, 0); analogWrite(RIGHT_IN4, pwmRight); } 
    else { analogWrite(RIGHT_IN3, 0); analogWrite(RIGHT_IN4, 0); }

  } else {
    stopMotor();
  }
  
  // ====================================================
  // 5. UPDATE BATERAI (Hanya Jalan Tiap 3 Menit)
  // ====================================================
  if (millis() - lastBatteryUpdate >= WAKTU_BATERAI) {
    lastBatteryUpdate = millis();
    updateBattery();
  }

  // ====================================================
  // 6. UPDATE LAYAR OLED (Jalan Tiap 250ms)
  // ====================================================
  if (millis() - lastDisplayUpdate >= 250) {
    lastDisplayUpdate = millis();
    blinkDot = !blinkDot; 
    
    // Update persentase RPM
    int avgSpeed = (abs(leftMotorSpeed) + abs(rightMotorSpeed)) / 2;
    currentRPM = map(avgSpeed, 0, 512, 0, 100);

    updateOLED();
  }

  delay(15); 
}
