# Solar Tracker Dashboard

لوحة تحكم لمراقبة بيانات الجهد والتيار من ESP32 في الوقت الحقيقي.

## البنية

```
solar-tracker-vercel/
├── server.js              # Express Server (API + ملفات ثابتة)
├── index.html             # لوحة التحكم (Dashboard)
├── package.json
├── vercel.json
└── solar_tracker_server.ino   # كود ESP32
```

## كيف يعمل النظام

```
ESP32 ──(HTTP POST كل ثانيتين)──> /api/data ──(ذاكرة السيرفر)──> Dashboard (GET)
```

1. **ESP32** يقرأ المستشعرات ويرسلها عبر HTTPS POST إلى `/api/data`
2. **السيرفر** يخزن آخر قراءة في الذاكرة
3. **Dashboard** يطلب البيانات عبر GET كل 3 ثواني ويعرضها

## النشر على Railway

1. ارفع المشروع إلى GitHub
2. في [Railway](https://railway.app) اضغط **New Project → Deploy from GitHub repo**
3. اختر المشروع وانتظر النشر
4. Railway سيكتشف `npm start` تلقائياً

## النشر على Vercel

```bash
npm install -g vercel
vercel --prod
```

## إعداد ESP32

غيّر هذا السطر في كود Arduino:
```cpp
const char* SERVER_URL = "https://YOUR-APP.railway.app/api/data";
```
استبدل `YOUR-APP` برابط مشروعك.

## تنسيق البيانات

### POST من ESP32 إلى /api/data
```json
{
  "voltage": 18.50,
  "current": 2.30,
  "power": 42.55,
  "panAngle": 90,
  "tiltAngle": 45,
  "ldrTL": 512, "ldrTR": 480,
  "ldrBL": 300, "ldrBR": 350
}
```

### GET من Dashboard
```json
{
  "voltage": 18.5,
  "current": 2.3,
  "power": 42.55,
  "panAngle": 90,
  "tiltAngle": 45,
  "ldrTL": 512, "ldrTR": 480,
  "ldrBL": 300, "ldrBR": 350,
  "timestamp": 1705000000000
}
```

## استكشاف الأخطاء

| المشكلة | الحل |
|---------|------|
| Railway - خطأ "No start command" | تأكد أن `package.json` فيه `"start": "node server.js"` |
| ESP32 يظهر "فشل الاتصال" | تأكد من رابط SERVER_URL وأنه يبدأ بـ https:// |
| Dashboard يظهر "بانتظار بيانات" | ESP32 لم يرسل بيانات بعد، تأكد من اتصاله بال WiFi |
| ESP32 يظهر خطأ SSL | الكود يستخدم `setInsecure()` للسماح باتصال HTTPS |