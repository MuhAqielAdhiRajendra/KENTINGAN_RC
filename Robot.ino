#include <esp_now.h>
#include <WiFi.h>

// ==========================================
// DEFINISI PIN & KONFIGURASI
// ==========================================
const int LEFT_IN1 = 16;
const int LEFT_IN2 = 17;
const int RIGHT_IN3 = 19;
const int RIGHT_IN4 = 18;
const int LED_BUILTIN_PIN = 2;
const int BUZZER_PIN = 26; 

const unsigned long WAKTU_MUNDUR = 800;

// ==========================================
// KALIBRASI SINKRONISASI MUNDUR
// ==========================================
float offsetMundurKiri = 1.0;  // Ubah jika roda kiri terlalu cepat saat mundur (misal 0.90)
float offsetMundurKanan = 1.0; // Ubah jika roda kanan terlalu cepat saat mundur (misal 0.90)

#define DPAD_UP    0x01
#define DPAD_DOWN  0x02
#define DPAD_RIGHT 0x04
#define DPAD_LEFT  0x08

typedef struct struct_message {
  int axisX;
  int axisY;     
  uint8_t dpad;  
  bool a;
  bool b;
  bool r1;
  bool l1;
  int r2;        
  int l2;        
  bool thumbR;   
  bool misc;
} struct_message;

struct_message myGamepad;

// Variabel Global
volatile unsigned long lastRecvTime = 0;
volatile bool isConnected = false;
volatile bool modeBrutal = true;

unsigned long macroTimer = 0;
int activeMacro = 0; 
bool lastR3State = false;
int leftMotorSpeed = 0;
int rightMotorSpeed = 0;

// ==========================================
// CALLBACK ESP-NOW
// ==========================================
void OnDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len) {
  if (len == sizeof(struct_message)) {
    memcpy(&myGamepad, incomingData, sizeof(myGamepad));
    lastRecvTime = millis();
    isConnected = true;
  }
}

void stopMotor() {
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT); 
  
  digitalWrite(LED_BUILTIN_PIN, HIGH); 
  stopMotor(); 

  // Notifikasi Suara Robot Nyala
  tone(BUZZER_PIN, 1000, 150); delay(200);
  tone(BUZZER_PIN, 2000, 300); delay(1000); 
  noTone(BUZZER_PIN);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

// ==========================================
// MAIN LOOP: GERAK MOTOR 
// ==========================================
void loop() {
  if (millis() - lastRecvTime > 1000) {
    stopMotor(); 
    isConnected = false;
    myGamepad.dpad = 0;
    activeMacro = 0;
  }

  if (isConnected) {
    if (myGamepad.misc) {
      activeMacro = 0;
      macroTimer = 0; 
      stopMotor(); 
      tone(BUZZER_PIN, 800, 200); 
      myGamepad.misc = false;
    }

    bool currentR3 = myGamepad.thumbR;
    if (currentR3 && !lastR3State) {
      modeBrutal = !modeBrutal;
      digitalWrite(LED_BUILTIN_PIN, modeBrutal ? HIGH : LOW);
      if(modeBrutal) tone(BUZZER_PIN, 1800, 100); 
      else tone(BUZZER_PIN, 600, 100);  
    }
    lastR3State = currentR3; 

    uint8_t dpad = myGamepad.dpad;
    
    leftMotorSpeed = 0; 
    rightMotorSpeed = 0;

    if (activeMacro == 0) {
      if (myGamepad.r2 > 50 || myGamepad.r1) { activeMacro = 1; macroTimer = millis(); }
      else if (myGamepad.l2 > 50 || myGamepad.l1) { activeMacro = 2; macroTimer = millis(); }
    }

    if (activeMacro == 1) {
      if (millis() - macroTimer < WAKTU_MUNDUR) { leftMotorSpeed = -512; rightMotorSpeed = 200; } 
      else activeMacro = 0;
    } 
    else if (activeMacro == 2) {
      if (millis() - macroTimer < WAKTU_MUNDUR) { leftMotorSpeed = -200; rightMotorSpeed = 512; } 
      else activeMacro = 0;
    }
    else {
      // =======================================================
      // KONTROL D-PAD
      // =======================================================
      if (dpad & DPAD_UP) { 
        // MAJU
        leftMotorSpeed = -512; 
        rightMotorSpeed = -512; 
      } 
      else if (dpad & DPAD_DOWN) { 
        // MUNDUR
        leftMotorSpeed = 512; 
        rightMotorSpeed = 512; 
      } 
      else if (dpad & DPAD_RIGHT) { 
        // PIVOT KANAN (Sudah ditukar sesuai wiring)
        // Kedua ban aktif berlawanan arah agar belok cepat
        leftMotorSpeed = 512;  
        rightMotorSpeed = -512; 
      } 
      else if (dpad & DPAD_LEFT) { 
        // PIVOT KIRI (Sudah ditukar sesuai wiring)
        // Kedua ban aktif berlawanan arah agar belok cepat
        leftMotorSpeed = -512; 
        rightMotorSpeed = 512; 
      }
      else if (myGamepad.a) { leftMotorSpeed = 512; rightMotorSpeed = -512; } // Tombol A disamakan dengan kanan
      else if (myGamepad.b) { leftMotorSpeed = -512; rightMotorSpeed = 512; } // Tombol B disamakan dengan kiri
    }

    if (!modeBrutal && activeMacro == 0) {
      leftMotorSpeed /= 2; rightMotorSpeed /= 2;
    }

    leftMotorSpeed = constrain(leftMotorSpeed, -512, 512);
    rightMotorSpeed = constrain(rightMotorSpeed, -512, 512);
    
    int pwmLeft = map(abs(leftMotorSpeed), 0, 512, 0, 255);
    int pwmRight = map(abs(rightMotorSpeed), 0, 512, 0, 255);

    // =======================================================
    // FIX: KALIBRASI HANYA AKTIF SAAT MUNDUR LURUS
    // Biar gak mengganggu kecepatan putaran saat pivot turn
    // =======================================================
    if (leftMotorSpeed > 0 && rightMotorSpeed > 0) {
      pwmLeft = pwmLeft * offsetMundurKiri; 
      pwmRight = pwmRight * offsetMundurKanan;
    }

    // Eksekusi Roda Kiri
    if (leftMotorSpeed > 0) { 
      analogWrite(LEFT_IN1, pwmLeft); 
      analogWrite(LEFT_IN2, 0); 
    } 
    else if (leftMotorSpeed < 0) { 
      analogWrite(LEFT_IN1, 0); 
      analogWrite(LEFT_IN2, pwmLeft); 
    } 
    else { 
      analogWrite(LEFT_IN1, 0); 
      analogWrite(LEFT_IN2, 0); 
    }

    // Eksekusi Roda Kanan
    if (rightMotorSpeed > 0) { 
      analogWrite(RIGHT_IN3, pwmRight); 
      analogWrite(RIGHT_IN4, 0); 
    } 
    else if (rightMotorSpeed < 0) { 
      analogWrite(RIGHT_IN3, 0); 
      analogWrite(RIGHT_IN4, pwmRight); 
    } 
    else { 
      analogWrite(RIGHT_IN3, 0); 
      analogWrite(RIGHT_IN4, 0); 
    }
  } 
}
