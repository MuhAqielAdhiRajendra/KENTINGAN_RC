#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> 

// ==========================================
// DEFINISI PIN & KONFIGURASI
// ==========================================
const int LEFT_IN1 = 26;   
const int LEFT_IN2 = 25;  
const int RIGHT_IN3 = 32; 
const int RIGHT_IN4 = 33; 

const int LED_BUILTIN_PIN = 2;
const int BUZZER_PIN = 26; 

const unsigned long WAKTU_MUNDUR = 800;

// ==========================================
// KALIBRASI SINKRONISASI MUNDUR
// ==========================================
float offsetMundurKiri = 1.0;  
float offsetMundurKanan = 1.0; 

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
// CALLBACK ESP-NOW (Telah disesuaikan untuk Core v3.x)
// ==========================================
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  // Cek apakah ukuran datanya sama persis
  if (len == sizeof(struct_message)) {
    memcpy(&myGamepad, incomingData, sizeof(myGamepad));
    lastRecvTime = millis();
    isConnected = true;
    
    // --- DEBUGGING: Cetak tombol yang ditekan ---
    if (myGamepad.dpad != 0) {
      Serial.print("D-Pad Ditekan! Nilai: ");
      Serial.println(myGamepad.dpad);
    }
  } else {
    // --- DEBUGGING: Tampilkan eror jika ukuran struct beda ---
    Serial.print("❌ Eror! Ukuran data beda. Diterima: ");
    Serial.print(len);
    Serial.print(" bytes, Diharapkan: ");
    Serial.print(sizeof(struct_message));
    Serial.println(" bytes.");
  }
}

void stopMotor() {
  // Wajib pakai murni analogWrite agar ESP32 tidak nge-glitch
  analogWrite(LEFT_IN1, 255);
  analogWrite(LEFT_IN2, 255);
  analogWrite(RIGHT_IN3, 255);
  analogWrite(RIGHT_IN4, 255);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- Sistem Robot Sumo Menyala ---");

  WiFi.mode(WIFI_STA);

  // Manipulasi MAC Address agar sinkron dengan Remote
  uint8_t macBaru[] = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F}; 
  esp_wifi_set_mac(WIFI_IF_STA, &macBaru[0]);

  Serial.print("MAC Address Robot (Spoofed): ");
  Serial.println(WiFi.macAddress());

  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT); 
  
  digitalWrite(LED_BUILTIN_PIN, HIGH); 
  stopMotor(); 

  tone(BUZZER_PIN, 1000, 150); delay(200);
  tone(BUZZER_PIN, 2000, 300); delay(1000); 
  noTone(BUZZER_PIN);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW Siap. Menunggu perintah dari Remote...");
}

void loop() {
  // Jika 1 detik tidak ada data masuk, stop motor (Failsafe)
  if (millis() - lastRecvTime > 1000) {
    if (isConnected) {
      Serial.println("⚠️ Sinyal Terputus! Menghentikan motor.");
    }
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
      Serial.println("Tombol Misc Ditekan -> Stop All");
    }

    bool currentR3 = myGamepad.thumbR;
    if (currentR3 && !lastR3State) {
      modeBrutal = !modeBrutal;
      digitalWrite(LED_BUILTIN_PIN, modeBrutal ? HIGH : LOW);
      if(modeBrutal) tone(BUZZER_PIN, 1800, 100); 
      else tone(BUZZER_PIN, 600, 100);  
      Serial.print("Ganti Mode: ");
      Serial.println(modeBrutal ? "BRUTAL" : "SANTAI");
    }
    lastR3State = currentR3; 

    uint8_t dpad = myGamepad.dpad;
    
    leftMotorSpeed = 0; 
    rightMotorSpeed = 0;

    if (activeMacro == 0) {
      if (myGamepad.r2 > 50 || myGamepad.r1) { activeMacro = 1; macroTimer = millis(); Serial.println("Macro 1 Aktif!"); }
      else if (myGamepad.l2 > 50 || myGamepad.l1) { activeMacro = 2; macroTimer = millis(); Serial.println("Macro 2 Aktif!"); }
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
      // KONTROL D-PAD (ARAH PIVOT SUDAH DIBENARKAN)
      // (-) Minus = Maju | (+) Plus = Mundur
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
        // PIVOT KANAN: Kiri Maju (-), Kanan Mundur (+)
        leftMotorSpeed = -512;  
        rightMotorSpeed = 512; 
      } 
      else if (dpad & DPAD_LEFT) { 
        // PIVOT KIRI: Kiri Mundur (+), Kanan Maju (-)
        leftMotorSpeed = 512; 
        rightMotorSpeed = -512; 
      }
      else if (myGamepad.a) { leftMotorSpeed = 512; rightMotorSpeed = -512; } 
      else if (myGamepad.b) { leftMotorSpeed = -512; rightMotorSpeed = 512; } 
    }

    if (!modeBrutal && activeMacro == 0) {
      leftMotorSpeed /= 2; rightMotorSpeed /= 2;
    }

    leftMotorSpeed = constrain(leftMotorSpeed, -512, 512);
    rightMotorSpeed = constrain(rightMotorSpeed, -512, 512);
    
    int pwmLeft = map(abs(leftMotorSpeed), 0, 512, 0, 255);
    int pwmRight = map(abs(rightMotorSpeed), 0, 512, 0, 255);

    if (leftMotorSpeed > 0 && rightMotorSpeed > 0) {
      pwmLeft = pwmLeft * offsetMundurKiri; 
      pwmRight = pwmRight * offsetMundurKanan;
    }

    // =======================================================
    // EKSEKUSI MOTOR (MURNI ANALOGWRITE - BEBAS GLITCH)
    // =======================================================
    
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
      analogWrite(RIGHT_IN3, 255); 
      analogWrite(RIGHT_IN4, 255); 
    }
  } 
}
