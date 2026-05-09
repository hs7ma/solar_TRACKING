// ============================================
//   نظام تتبع الشمس الذكي - مع Firebase
//   Solar Tracker + Firebase Dashboard
// ============================================

#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ============================================
// ⚙️  غيّر هذه الإعدادات فقط
// ============================================
const char* ssid     = "SOFY";
const char* password = "sofy.7247";

const char* FIREBASE_URL = "https://solar-tracker-14418-default-rtdb.europe-west1.firebasedatabase.app/station1.json";
// ============================================

// --- تعريف محركات السيرفو ---
Servo servoPan;
Servo servoTilt;

// --- تعريف منافذ مستشعرات الضوء (LDRs) ---
const int ldrTL = 32;
const int ldrTR = 33;
const int ldrBL = 34;
const int ldrBR = 35;

// --- تعريف منافذ السيرفو ---
const int servoPanPin  = 18;
const int servoTiltPin = 19;

// --- تعريف منافذ حساسات الفولتية والتيار ---
const int voltagePin = 36; // SVP (VP)
const int currentPin = 39; // SVN (VN)

// --- الزوايا الابتدائية ---
int panAngle  = 90;
int tiltAngle = 90;

// --- إعدادات الحساسات والسرعة ---
int tolerance = 150;
int stepDelay = 30;

// --- مؤقت لقراءة البيانات وإرسالها ---
unsigned long previousMillis = 0;
const long sendInterval = 2000; // كل ثانيتين

// --- متغيرات تخزين آخر قراءة ---
float actual_voltage = 0.0;
float actual_current = 0.0;
float power_W        = 0.0;
int   ldr_tl = 0, ldr_tr = 0, ldr_bl = 0, ldr_br = 0;

// ============================================
// 🔥 إرسال البيانات إلى Firebase
// ============================================
void sendToFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(FIREBASE_URL);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(3000);

  String json = "{";
  json += "\"voltage\":"   + String(actual_voltage, 2) + ",";
  json += "\"current\":"   + String(abs(actual_current), 2) + ",";
  json += "\"power\":"     + String(power_W, 2) + ",";
  json += "\"panAngle\":"  + String(panAngle) + ",";
  json += "\"tiltAngle\":" + String(tiltAngle) + ",";
  json += "\"ldrTL\":"     + String(ldr_tl) + ",";
  json += "\"ldrTR\":"     + String(ldr_tr) + ",";
  json += "\"ldrBL\":"     + String(ldr_bl) + ",";
  json += "\"ldrBR\":"     + String(ldr_br);
  json += "}";

  int code = http.PUT(json);

  if (code > 0) {
    Serial.println("✅ Firebase: تم الإرسال بنجاح (code: " + String(code) + ")");
  } else {
    Serial.println("❌ Firebase: فشل الإرسال (error: " + String(code) + ")");
  }

  http.end();
}

// ============================================
void setup() {
  Serial.begin(115200);

  // تهيئة محركات السيرفو
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servoPan.setPeriodHertz(50);
  servoTilt.setPeriodHertz(50);

  servoPan.attach(servoPanPin, 500, 2400);
  servoTilt.attach(servoTiltPin, 500, 2400);

  servoPan.write(panAngle);
  servoTilt.write(tiltAngle);
  delay(1000);

  // --- الاتصال بالـ WiFi ---
  Serial.println("\nجاري الاتصال بالـ WiFi...");
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ تم الاتصال بالـ WiFi!");
    Serial.print("📡 عنوان IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("🔥 سيتم الإرسال إلى Firebase كل ثانيتين");
  } else {
    Serial.println("\n❌ فشل الاتصال - الجهاز يعمل بدون WiFi");
  }

  Serial.println("تم بدء النظام...");
}

// ============================================
void loop() {

  // -----------------------------------------
  // 1. قسم تتبع الشمس (بدون تغيير)
  // -----------------------------------------
  ldr_tl = analogRead(ldrTL);
  ldr_tr = analogRead(ldrTR);
  ldr_bl = analogRead(ldrBL);
  ldr_br = analogRead(ldrBR);

  int avgTop   = (ldr_tl + ldr_tr) / 2;
  int avgBot   = (ldr_bl + ldr_br) / 2;
  int avgLeft  = (ldr_tl + ldr_bl) / 2;
  int avgRight = (ldr_tr + ldr_br) / 2;

  // الحركة العمودية
  if (abs(avgTop - avgBot) > tolerance) {
    if (avgTop > avgBot) { tiltAngle--; }
    else                 { tiltAngle++; }
  }

  // الحركة الأفقية
  if (abs(avgLeft - avgRight) > tolerance) {
    if (avgLeft > avgRight) { panAngle++; }
    else                    { panAngle--; }
  }

  tiltAngle = constrain(tiltAngle, 10, 170);
  panAngle  = constrain(panAngle,  10, 170);

  servoTilt.write(tiltAngle);
  servoPan.write(panAngle);

  // -----------------------------------------
  // 2. قراءة الفولتية والتيار + إرسال Firebase
  // -----------------------------------------
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= sendInterval) {
    previousMillis = currentMillis;

    // --- حساب الفولتية ---
    int adc_v = analogRead(voltagePin);
    float v_pin = (adc_v * 3.3) / 4095.0;
    actual_voltage = v_pin * 5.0;

    // --- حساب التيار ---
    int adc_c = analogRead(currentPin);
    float c_pin = (adc_c * 3.3) / 4095.0;
    actual_current = (c_pin - 1.65) / 0.185;

    if (actual_current < 0.05 && actual_current > -0.05) {
      actual_current = 0.0;
    }

    power_W = actual_voltage * abs(actual_current);

    // طباعة على Serial Monitor
    Serial.println("=============================");
    Serial.print("الفولتية (V): "); Serial.println(actual_voltage, 2);
    Serial.print("التيار (A):  "); Serial.println(abs(actual_current), 2);
    Serial.print("الطاقة (W):  "); Serial.println(power_W, 2);
    Serial.print("Pan Angle:   "); Serial.println(panAngle);
    Serial.print("Tilt Angle:  "); Serial.println(tiltAngle);
    Serial.println("=============================");

    // 🔥 إرسال إلى Firebase
    sendToFirebase();

    // إعادة الاتصال إذا انقطع WiFi
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠️ WiFi انقطع، جاري إعادة الاتصال...");
      WiFi.reconnect();
    }
  }

  delay(stepDelay);
}
