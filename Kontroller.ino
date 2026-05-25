#include <PS2X_lib.h>  
#include <esp_now.h>
#include <WiFi.h>

// Konfigurasi PIN ESP32
#define PS2_DAT        19  
#define PS2_CMD        23  
#define PS2_SEL         5  
#define PS2_CLK        18  

PS2X ps2x; 
int error = -1;

// ==========================================
// TARGET MAC ROBOT SUMO LU (WAJIB SESUAI)
// ==========================================
// Ganti angka di bawah sesuai yang muncul di Serial Monitor Robot
uint8_t robotAddress[] = {0xB0, 0xCB, 0xD8, 0xE3, 0x00, 0xF4}; 

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
esp_now_peer_info_t peerInfo;
unsigned long lastSendTime = 0; 

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Daftarkan Peer (Si Robot)
  memcpy(peerInfo.peer_addr, robotAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Setup Stik PS2
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
}

void loop() {
  // Autoreconnect PS2 jika kabel goyang
  if (error != 0) {
    delay(500);
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
    return; 
  }

  ps2x.read_gamepad(false, 0); 

  // Mapping Analog
  myGamepad.axisX = map(ps2x.Analog(PSS_LX), 0, 255, -512, 512);
  myGamepad.axisY = map(ps2x.Analog(PSS_LY), 0, 255, -512, 512);

  // Deadzone untuk stik kendor
  if (abs(myGamepad.axisX) < 80) myGamepad.axisX = 0;
  if (abs(myGamepad.axisY) < 80) myGamepad.axisY = 0;

  // Mapping D-Pad
  uint8_t dpadState = 0;
  if(ps2x.Button(PSB_PAD_UP)) dpadState |= DPAD_UP;
  if(ps2x.Button(PSB_PAD_DOWN)) dpadState |= DPAD_DOWN;
  if(ps2x.Button(PSB_PAD_RIGHT)) dpadState |= DPAD_RIGHT;
  if(ps2x.Button(PSB_PAD_LEFT)) dpadState |= DPAD_LEFT;
  myGamepad.dpad = dpadState;

  // Mapping Tombol
  myGamepad.a = ps2x.Button(PSB_CROSS);
  myGamepad.b = ps2x.Button(PSB_CIRCLE);
  myGamepad.r1 = ps2x.Button(PSB_R1);
  myGamepad.l1 = ps2x.Button(PSB_L1);
  myGamepad.r2 = ps2x.Button(PSB_R2) ? 255 : 0;
  myGamepad.l2 = ps2x.Button(PSB_L2) ? 255 : 0;
  myGamepad.thumbR = ps2x.Button(PSB_R3);
  myGamepad.misc = ps2x.Button(PSB_SELECT);

  // ==========================================
  // PENGIRIMAN DATA ANTI DELAY
  // ==========================================
  // Kirim data setiap 20 milidetik aja biar buffer ESP32 nggak meledak
  if (millis() - lastSendTime >= 20) {
    esp_now_send(robotAddress, (uint8_t *) &myGamepad, sizeof(myGamepad));
    lastSendTime = millis();
  }
}
