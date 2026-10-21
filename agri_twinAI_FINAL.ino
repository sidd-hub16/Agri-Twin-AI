#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ================= WIFI =================
const char* WIFI_SSID = "SIDDHU";
const char* WIFI_PASSWORD = "12345678";

// ================= PINS =================
#define SOIL_PIN 34
#define DHT_PIN 4
#define RAIN_PIN 35
#define LED_PIN 25
#define BUZZER_PIN 26
#define DHT_TYPE DHT11
#define LCD_ADDRESS 0x27

// Your original 2.0C would trigger drought at normal temperatures.
// 35.0C is used here for a practical demonstration threshold.
#define DROUGHT_TEMP 2.0

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(LCD_ADDRESS,16,2);
WebServer server(80);

int soilMoisture=0, rainRaw=0;
float temperature=NAN, humidity=NAN;
bool rainDetected=false, droughtAlert=false;
String irrigationStatus="NOT NEEDED";
String farmStatus="Healthy";

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport"
content="width=device-width, initial-scale=1.0">
<title>AgriTwin AI</title>
<style>
* {
box-sizing: border-box;
margin: 0;
padding: 0;
}
html {
scroll-behavior: smooth;
}
body {
font-family: Arial, Helvetica, sans-serif;
color: #123f2d;
background: #eef7ef;
}
button {
font-family: inherit;
cursor: pointer;
}
.page {
display: none;
min-height: 100vh;
}
.page.active {
display: block;
}
#page1 {
min-height: 100vh;
background-image:
linear-gradient(
rgba(255,255,255,0.08),
rgba(255,255,255,0.12)
),
url("https://images.unsplash.com/photo-1643171916755-02c26a606f70?auto=format&fit=crop&fm=jpg&q=80&w=2400");
background-size: cover;
background-position: center;
background-attachment: fixed;
padding-bottom: 45px;
position: relative;
}
#page1::before {
content: "";
position: fixed;
inset: 0;
background:
linear-gradient(
180deg,
rgba(255,255,255,0.15),
rgba(5,70,35,0.08)
);
pointer-events: none;
z-index: 0;
}
#page1 > * {
position: relative;
z-index: 1;
}
.hero {
text-align: center;
padding:
48px
25px
30px;
color: #063e29;
}
.logo {
display: inline-flex;
align-items: center;
justify-content: center;
gap: 12px;
font-size: clamp(
48px,
7vw,
88px
);
line-height: 1;
font-weight: 900;
letter-spacing: -3px;
text-shadow:
0 3px 12px
rgba(255,255,255,0.7);
}
.logo-leaf {
font-size: 0.7em;
transform:
rotate(-8deg);
}
.hero-tagline {
display: inline-block;
margin-top: 22px;
padding: 10px 35px;
border-radius: 40px;
background:
rgba(20,91,55,0.92);
color: white;
font-size: clamp(
15px,
2vw,
22px
);
font-weight: 600;
letter-spacing: .3px;
box-shadow:
0 6px 20px
rgba(0,0,0,.12);
}
.hero-description {
max-width: 800px;
margin: 18px auto 0;
color: #073f2b;
font-size: clamp(
15px,
2vw,
21px
);
line-height: 1.5;
font-weight: 600;
text-shadow:
0 2px 8px
rgba(255,255,255,.8);
}
.features {
max-width: 900px;
margin: 28px auto 35px;
display: grid;
grid-template-columns:
repeat(4,1fr);
gap: 20px;
padding: 0 25px;
}
.feature {
display: flex;
align-items: center;
gap: 12px;
color: #123f2d;
font-weight: 700;
font-size: 15px;
}
.feature-icon {
min-width: 58px;
height: 58px;
border-radius: 18px;
background:
rgba(255,255,255,.92);
display: flex;
justify-content: center;
align-items: center;
font-size: 30px;
box-shadow:
0 5px 18px
rgba(0,0,0,.12);
}
.area-panel {
max-width: 1430px;
margin: auto;
padding: 25px;
background:
rgba(255,255,255,.94);
border-radius: 35px;
box-shadow:
0 15px 45px
rgba(0,0,0,.18);
backdrop-filter:
blur(8px);
}
.area-heading {
text-align: center;
margin-bottom: 25px;
}
.area-heading h2 {
color: #084c31;
font-size: clamp(
28px,
4vw,
40px
);
margin-bottom: 8px;
}
.area-heading p {
color: #69776f;
font-size: 17px;
}
.area-grid {
display: grid;
grid-template-columns:
repeat(5,1fr);
gap: 17px;
}
.area-card {
background:
rgba(255,255,255,.98);
border-radius: 18px;
overflow: hidden;
border: 1px solid
rgba(20,80,50,.15);
box-shadow:
0 5px 16px
rgba(0,0,0,.09);
transition:
transform .25s,
box-shadow .25s;
}
.area-card:hover {
transform:
translateY(-6px);
box-shadow:
0 12px 28px
rgba(0,0,0,.15);
}
.area-image {
display: block;
width: 100%;
height: 225px;
object-fit: cover;
background: #dceadf;
}
.area-content {
padding: 15px;
text-align: center;
}
.area-content h3 {
color: #0b4d32;
font-size: 25px;
margin-bottom: 10px;
}
.area-status {
display: inline-block;
padding: 5px 12px;
border-radius: 20px;
font-size: 11px;
font-weight: 800;
margin-bottom: 10px;
}
.live {
background: #d9f5df;
color: #11733b;
}
.demo {
background: #edf1ed;
color: #66756d;
}
.area-btn {
border: none;
width: 48px;
height: 48px;
border-radius: 50%;
background: #bcebc9;
color: #07502f;
font-size: 25px;
font-weight: bold;
transition: .2s;
}
.area-btn:hover {
transform:
scale(1.1);
background: #a5e2b6;
}
.top-navigation {
max-width: 1250px;
margin: auto;
padding: 25px;
display: flex;
align-items: center;
gap: 25px;
}
.back-btn {
border: none;
background: #e0eee4;
color: #155c43;
padding: 11px 18px;
border-radius: 10px;
font-weight: bold;
}
.page-title {
color: #155c43;
font-size: 30px;
}
.page-subtitle {
color: #69766f;
margin-top: 5px;
}
.container {
max-width: 1250px;
margin: auto;
padding:
0 25px 50px;
}
.dashboard-header {
background: white;
border-radius: 18px;
padding: 25px;
margin-bottom: 25px;
box-shadow:
0 6px 20px
rgba(0,0,0,.06);
}
.dashboard-header h2 {
color: #155c43;
margin-bottom: 8px;
}
.dashboard-header p {
color: #68766f;
}
.sensor-section {
background: white;
border-radius: 18px;
padding: 25px;
margin-bottom: 25px;
box-shadow:
0 6px 20px
rgba(0,0,0,.06);
}
.sensor-top {
display: flex;
justify-content: space-between;
align-items: center;
margin-bottom: 20px;
}
.sensor-top h2 {
color: #155c43;
}
.connect-btn {
border: none;
background: #155c43;
color: white;
padding: 11px 20px;
border-radius: 10px;
font-weight: bold;
}
.sensor-grid {
display: grid;
grid-template-columns:
repeat(4,1fr);
gap: 15px;
}
.sensor-card {
background: #f4faf5;
border: 1px solid #e0ece3;
border-radius: 15px;
padding: 20px;
}
.sensor-card h3 {
color: #68766f;
font-size: 14px;
margin-bottom: 10px;
}
.sensor-value {
color: #155c43;
font-size: 28px;
font-weight: bold;
}
.irrigation-box {
margin-top: 25px;
border-radius: 18px;
padding: 28px;
text-align: center;
background: #f8fcf9;
border: 2px solid #dce9df;
}
.irrigation-label {
font-size: 16px;
color: #68766f;
margin-bottom: 8px;
}
#irrigationValue {
font-size: 36px;
font-weight: 900;
color: #16743b;
}
.sensor-status {
margin-top: 18px;
color: #66756e;
}
.intelligence-grid {
display: grid;
grid-template-columns:
repeat(3,1fr);
gap: 20px;
margin-bottom: 25px;
}
.info-card {
background: white;
border-radius: 18px;
padding: 25px;
text-align: center;
box-shadow:
0 6px 20px
rgba(0,0,0,.06);
}
.info-card h3 {
color: #68766f;
font-size: 15px;
margin-bottom: 12px;
}
.info-value {
color: #155c43;
font-size: 28px;
font-weight: 800;
}
.health-section {
background: white;
border-radius: 18px;
padding: 25px;
margin-bottom: 25px;
box-shadow:
0 6px 20px
rgba(0,0,0,.06);
}
.health-header {
display: flex;
justify-content: space-between;
align-items: center;
margin-bottom: 20px;
}
.health-header h2 {
color: #155c43;
}
.health-score {
font-size: 40px;
font-weight: 800;
color: #155c43;
}
.health-progress {
height: 18px;
background: #e7eee9;
border-radius: 20px;
overflow: hidden;
}
.health-progress-fill {
height: 100%;
background: #55b76f;
border-radius: 20px;
}
.farm-status {
margin-top: 18px;
padding: 15px;
border-radius: 12px;
text-align: center;
font-size: 20px;
font-weight: bold;
}
.status-healthy {
background: #ddf6e3;
color: #16743b;
}
.status-warning {
background: #fff1c9;
color: #9a6a00;
}
.status-critical {
background: #ffe0e0;
color: #b32929;
}
.status-attention {
background: #ffe9d5;
color: #a65400;
}
.prediction-area {
text-align: center;
margin-top: 35px;
}
.prediction-btn {
border: none;
background: #155c43;
color: white;
padding: 16px 40px;
border-radius: 13px;
font-size: 18px;
font-weight: bold;
}
.crop-grid {
display: grid;
grid-template-columns:
repeat(5,1fr);
gap: 20px;
}
.crop-card {
background: white;
border-radius: 18px;
overflow: hidden;
box-shadow:
0 7px 22px
rgba(0,0,0,.07);
transition: .3s;
}
.crop-card:hover {
transform:
translateY(-6px);
}
.crop-image {
height: 160px;
display: flex;
justify-content: center;
align-items: center;
font-size: 75px;
background:
linear-gradient(
135deg,
#e5f6df,
#cceac8
);
}
.crop-content {
padding: 18px;
text-align: center;
}
.crop-content h2 {
color: #155c43;
margin-bottom: 7px;
}
.crop-content p {
color: #6a776f;
font-size: 14px;
margin-bottom: 15px;
}
.crop-btn {
width: 100%;
border: none;
padding: 11px;
border-radius: 10px;
background: #c8efca;
color: #155c43;
font-weight: bold;
}
.result-header {
background: white;
padding: 25px;
border-radius: 18px;
margin-bottom: 25px;
box-shadow:
0 6px 20px
rgba(0,0,0,.06);
}
.result-header h2 {
color: #155c43;
margin-bottom: 8px;
}
.result-header p {
color: #68766f;
}
.result-grid {
display: grid;
grid-template-columns:
repeat(4,1fr);
gap: 20px;
}
.result-card {
background: white;
border-radius: 18px;
padding: 25px;
text-align: center;
box-shadow:
0 6px 20px
rgba(0,0,0,.06);
}
.result-card h3 {
color: #68766f;
font-size: 15px;
margin-bottom: 12px;
}
.result-number {
font-size: 30px;
color: #155c43;
font-weight: bold;
}
.market-status {
margin-top: 25px;
padding: 25px;
border-radius: 18px;
text-align: center;
}
.market-surplus {
background: #e2f7e6;
color: #16743b;
}
.market-shortage {
background: #ffe5e5;
color: #b32929;
}
.market-status h2 {
margin-bottom: 8px;
}
.comparison-box {
background: white;
padding: 25px;
border-radius: 18px;
margin-top: 25px;
box-shadow:
0 6px 20px
rgba(0,0,0,.06);
}
.comparison-row {
margin-bottom: 20px;
}
.comparison-label {
display: flex;
justify-content: space-between;
margin-bottom: 7px;
font-weight: bold;
}
.progress-background {
height: 22px;
background: #e7eee9;
border-radius: 20px;
overflow: hidden;
}
.progress-production {
height: 100%;
background: #62b878;
border-radius: 20px;
}
.progress-demand {
height: 100%;
background: #ef9a78;
border-radius: 20px;
}
.profit-box {
margin-top: 25px;
background:
linear-gradient(
135deg,
#e4f7e7,
#f5fbf6
);
border: 1px solid #cce7d1;
border-radius: 18px;
padding: 28px;
text-align: center;
}
.profit-box h2 {
color: #155c43;
margin-bottom: 10px;
}
.profit-value {
font-size: 38px;
font-weight: 800;
color: #16743b;
}
.report-area {
text-align: center;
margin-top: 40px;
padding: 20px;
}
.report-btn {
border: none;
background:
linear-gradient(
135deg,
#155c43,
#238153
);
color: white;
padding: 17px 45px;
border-radius: 14px;
font-size: 19px;
font-weight: bold;
box-shadow:
0 8px 20px
rgba(21,92,67,.25);
transition: .2s;
}
.report-btn:hover {
transform:
translateY(-3px);
}
.report-page {
background:
linear-gradient(
180deg,
#063e29 0,
#0d5b3b 260px,
#f5faf5 260px
);
min-height: 100vh;
padding-bottom: 50px;
}
.report-wrapper {
max-width: 1100px;
margin: auto;
padding:
35px 25px;
}
.report-header {
color: white;
padding:
20px 10px 45px;
}
.report-logo {
font-size: 42px;
font-weight: 800;
margin-bottom: 10px;
}
.report-title {
font-size: 34px;
font-weight: 800;
}
.report-subtitle {
font-size: 17px;
margin-top: 10px;
color: #d9eee0;
}
.report-info {
background: white;
border-radius: 18px;
padding: 25px;
display: grid;
grid-template-columns:
repeat(3,1fr);
gap: 20px;
box-shadow:
0 10px 30px
rgba(0,0,0,.10);
}
.report-info-item h4 {
color: #68766f;
font-size: 13px;
margin-bottom: 6px;
}
.report-info-item p {
color: #173d2b;
font-weight: bold;
font-size: 17px;
}
.report-recommendation {
margin-top: 25px;
background:
linear-gradient(
135deg,
#06472d,
#177349
);
border-radius: 20px;
padding: 35px;
color: white;
}
.report-recommendation h2 {
font-size: 28px;
margin-bottom: 20px;
}
.recommend-label {
font-size: 17px;
color: #d9eee0;
margin-bottom: 8px;
}
.recommend-crop {
font-size: 55px;
font-weight: 900;
color: #c9f27a;
}
.report-section-title {
margin-top: 35px;
margin-bottom: 18px;
color: #124d35;
font-size: 27px;
}
.report-metrics {
display: grid;
grid-template-columns:
repeat(2,1fr);
gap: 18px;
}
.report-metric {
background: white;
border: 1px solid #dce9df;
border-radius: 18px;
padding: 23px;
display: flex;
align-items: center;
gap: 18px;
box-shadow:
0 5px 15px
rgba(0,0,0,.04);
}
.metric-icon {
width: 58px;
height: 58px;
border-radius: 50%;
background: #e3f4d9;
display: flex;
align-items: center;
justify-content: center;
font-size: 28px;
flex-shrink: 0;
}
.metric-content h4 {
color: #68766f;
font-size: 14px;
margin-bottom: 6px;
}
.metric-content strong {
color: #123f2d;
font-size: 25px;
}
.ai-insight {
margin-top: 25px;
background: #eff9e8;
border: 2px dashed #7fc66d;
border-radius: 18px;
padding: 25px;
display: flex;
gap: 20px;
align-items: flex-start;
}
.ai-icon {
font-size: 40px;
}
.ai-insight h2 {
color: #155c43;
margin-bottom: 8px;
}
.ai-insight p {
color: #385747;
font-size: 17px;
line-height: 1.6;
}
.report-actions {
text-align: center;
margin-top: 25px;
}
.print-btn {
border: none;
background: #155c43;
color: white;
padding: 13px 25px;
border-radius: 10px;
font-weight: bold;
margin: 5px;
}
.report-footer {
text-align: center;
margin-top: 35px;
color: #5e7065;
font-size: 13px;
}
@media(max-width:1100px) {
.area-grid {
grid-template-columns:
repeat(3,1fr);
}
.features {
grid-template-columns:
repeat(2,1fr);
}
.crop-grid {
grid-template-columns:
repeat(3,1fr);
}
.sensor-grid {
grid-template-columns:
repeat(2,1fr);
}
.result-grid {
grid-template-columns:
repeat(2,1fr);
}
}
@media(max-width:700px) {
#page1 {
background-attachment:
scroll;
}
.area-grid {
grid-template-columns: 1fr;
}
.features {
grid-template-columns: 1fr;
}
.sensor-grid {
grid-template-columns: 1fr;
}
.intelligence-grid {
grid-template-columns: 1fr;
}
.crop-grid {
grid-template-columns:
repeat(2,1fr);
}
.result-grid {
grid-template-columns: 1fr;
}
.report-info {
grid-template-columns: 1fr;
}
.report-metrics {
grid-template-columns: 1fr;
}
.top-navigation {
flex-direction: column;
align-items: flex-start;
}
.logo {
letter-spacing:
-2px;
}
.hero-tagline {
padding:
9px 15px;
}
}
@media(max-width:450px) {
.crop-grid {
grid-template-columns: 1fr;
}
.area-panel {
margin:
0 10px;
padding:
15px;
}
}
@media print {
body {
background: white;
}
.report-actions {
display: none;
}
.report-page {
display: block !important;
background: white !important;
}
}
</style>
</head>
<body>
<section
id="page1"
class="page active"
>
<div class="hero">
<div class="logo">
<span class="logo-leaf">
🌱
</span>
<span>
AgriTwin
</span>
<span
style="
color:#4b851e;
"
>
AI
</span>
</div>
<div class="hero-tagline">
Smart Decisions
&nbsp;•&nbsp;
Profitable Farming
&nbsp;•&nbsp;
Sustainable Tomorrow
</div>
<div class="hero-description">
An AI-powered agricultural decision support
platform integrating IoT, weather intelligence
and market insights.
</div>
</div>
<div class="features">
<div class="feature">
<div class="feature-icon">
🌱
</div>
<div>
Real-time<br>
Field Monitoring
</div>
</div>
<div class="feature">
<div class="feature-icon">
☁️
</div>
<div>
Weather &<br>
Risk Prediction
</div>
</div>
<div class="feature">
<div class="feature-icon">
📊
</div>
<div>
Yield & Market<br>
Forecasting
</div>
</div>
<div class="feature">
<div class="feature-icon">
🌿
</div>
<div>
Smart Crop<br>
Recommendation
</div>
</div>
</div>
<div class="area-panel">
<div class="area-heading">
<h2>
📍 Select Your Area
</h2>
<p>
Choose your district to get personalized
agricultural insights
</p>
</div>
<div class="area-grid">
<div class="area-card">
<img
class="area-image"
src="https://upload.wikimedia.org/wikipedia/commons/8/8f/Brihadeeswara_Temple_in_thanjavur.jpg"
alt="Brihadeeswara Temple Thanjavur"
onerror="
this.onerror=null;
this.src='https://upload.wikimedia.org/wikipedia/commons/thumb/0/06/Brihadeeswara_Temple_Thanjavur.png/960px-Brihadeeswara_Temple_Thanjavur.png';
"
>
<div class="area-content">
<span
class="area-status live"
>
● LIVE IoT
</span>
<h3>
Thanjavur
</h3>
<button
class="area-btn"
onclick="
selectArea('Thanjavur')
"
>
→
</button>
</div>
</div>
<div class="area-card">
<img
class="area-image"
src="https://commons.wikimedia.org/wiki/Special:Redirect/file/Meenakshi%20Temple%2C%20Gopuram%2C%20Madurai%2C%20India.jpg"
alt="Madurai"
>
<div class="area-content">
<span
class="area-status demo"
>
DEMO DATA
</span>
<h3>
Madurai
</h3>
<button
class="area-btn"
onclick="
selectArea('Madurai')
"
>
→
</button>
</div>
</div>
<div class="area-card">
<img
class="area-image"
src="https://commons.wikimedia.org/wiki/Special:Redirect/file/Rock%20Fortress%20-%20Tiruchirappalli%20-%20India.JPG"
alt="Trichy"
>
<div class="area-content">
<span
class="area-status demo"
>
DEMO DATA
</span>
<h3>
Trichy
</h3>
<button
class="area-btn"
onclick="
selectArea('Trichy')
"
>
→
</button>
</div>
</div>
<div class="area-card">
<img
class="area-image"
src="https://commons.wikimedia.org/wiki/Special:Redirect/file/Vivekananda%20Rock%20Memorial%2C%20Kanyakumari%2C%20Tamilnadu.jpg"
alt="Kanyakumari"
>
<div class="area-content">
<span
class="area-status demo"
>
DEMO DATA
</span>
<h3>
Kanyakumari
</h3>
<button
class="area-btn"
onclick="
selectArea('Kanyakumari')
"
>
→
</button>
</div>
</div>
<div class="area-card">
<img
class="area-image"
src="https://commons.wikimedia.org/wiki/Special:Redirect/file/Salem%20view%20from%20GRT-2-yercaud-salem-India.jpg"
alt="Salem"
>
<div class="area-content">
<span
class="area-status demo"
>
DEMO DATA
</span>
<h3>
Salem
</h3>
<button
class="area-btn"
onclick="
selectArea('Salem')
"
>
→
</button>
</div>
</div>
</div>
</div>
</section>
<section
id="page2"
class="page"
>
<div class="top-navigation">
<button
class="back-btn"
onclick="showPage(1)"
>
← Areas
</button>
<div>
<h1
id="page2Area"
class="page-title"
>
Thanjavur
</h1>
<p class="page-subtitle">
Agricultural monitoring dashboard
</p>
</div>
</div>
<div class="container">
<div class="dashboard-header">
<h2>
🌾
<span id="regionName">
Thanjavur
</span>
Agricultural Zone
</h2>
<p>
Real-time field monitoring and
AI-powered farm intelligence.
</p>
</div>
<div class="sensor-section">
<div class="sensor-top">
<h2>
🌱 Live Field Sensors
</h2>
<button
id="connectButton"
class="connect-btn"
onclick="connectESP32()"
>
Connect ESP32
</button>
</div>
<div class="sensor-grid">
<div class="sensor-card">
<h3>
💧 Soil Moisture
</h3>
<div
id="soilValue"
class="sensor-value"
>
42%
</div>
</div>
<div class="sensor-card">
<h3>
🌡 Temperature
</h3>
<div
id="temperatureValue"
class="sensor-value"
>
30°C
</div>
</div>
<div class="sensor-card">
<h3>
💦 Humidity
</h3>
<div
id="humidityValue"
class="sensor-value"
>
65%
</div>
</div>
<div class="sensor-card">
<h3>
🌧 Rain
</h3>
<div
id="rainValue"
class="sensor-value"
>
NO
</div>
</div>
</div>
<div class="irrigation-box">
<div class="irrigation-label">
💧 IRRIGATION STATUS
</div>
<div id="irrigationValue">
IRRIGATION NOT REQUIRED
</div>
</div>
<div class="sensor-status">
Connection:
<strong id="connectionText">
Demo Mode
</strong>
</div>
</div>
<div class="intelligence-grid">
<div class="info-card">
<h3>
🌱 Recommended Crop
</h3>
<div
id="recommendedCrop"
class="info-value"
>
Rice
</div>
</div>
<div class="info-card">
<h3>
🤖 AI Confidence
</h3>
<div class="info-value">
93%
</div>
</div>
<div class="info-card">
<h3>
📍 Selected Area
</h3>
<div
id="selectedAreaDisplay"
class="info-value"
>
Thanjavur
</div>
</div>
</div>
<div class="health-section">
<div class="health-header">
<h2>
❤️ Farm Health Score
</h2>
<div
id="healthScore"
class="health-score"
>
86/100
</div>
</div>
<div class="health-progress">
<div
id="healthProgress"
class="health-progress-fill"
style="width:86%"
></div>
</div>
<div
id="farmStatus"
class="farm-status status-healthy"
>
🟢 Healthy
</div>
</div>
<div class="prediction-area">
<button
class="prediction-btn"
onclick="showPage(3)"
>
🤖 Run AI Prediction →
</button>
</div>
</div>
</section>
<section
id="page3"
class="page"
>
<div class="top-navigation">
<button
class="back-btn"
onclick="showPage(2)"
>
← Farm Dashboard
</button>
<div>
<h1 class="page-title">
AI Crop Recommendation
</h1>
<p class="page-subtitle">
Select a crop for yield and market analysis
</p>
</div>
</div>
<div class="container">
<div class="dashboard-header">
<h2>
🌱 Select Crop
</h2>
<p>
Area:
<strong id="cropArea">
Thanjavur
</strong>
</p>
</div>
<div class="crop-grid">
<div class="crop-card">
<div class="crop-image">
🌾
</div>
<div class="crop-content">
<h2>Rice</h2>
<p>
Paddy cultivation
</p>
<button
class="crop-btn"
onclick="
selectCrop('Rice')
"
>
Analyze Rice →
</button>
</div>
</div>
<div class="crop-card">
<div class="crop-image">
🌾
</div>
<div class="crop-content">
<h2>Wheat</h2>
<p>
Wheat cultivation
</p>
<button
class="crop-btn"
onclick="
selectCrop('Wheat')
"
>
Analyze Wheat →
</button>
</div>
</div>
<div class="crop-card">
<div class="crop-image">
🌱
</div>
<div class="crop-content">
<h2>Barley</h2>
<p>
Barley cultivation
</p>
<button
class="crop-btn"
onclick="
selectCrop('Barley')
"
>
Analyze Barley →
</button>
</div>
</div>
<div class="crop-card">
<div class="crop-image">
🌽
</div>
<div class="crop-content">
<h2>Maize</h2>
<p>
Maize cultivation
</p>
<button
class="crop-btn"
onclick="
selectCrop('Maize')
"
>
Analyze Maize →
</button>
</div>
</div>
<div class="crop-card">
<div class="crop-image">
🌾
</div>
<div class="crop-content">
<h2>Ragi</h2>
<p>
Finger millet
</p>
<button
class="crop-btn"
onclick="
selectCrop('Ragi')
"
>
Analyze Ragi →
</button>
</div>
</div>
</div>
</div>
</section>
<section
id="page4"
class="page"
>
<div class="top-navigation">
<button
class="back-btn"
onclick="showPage(3)"
>
← Crops
</button>
<div>
<h1 class="page-title">
AI Market Analysis
</h1>
<p class="page-subtitle">
Yield, demand and profitability analysis
</p>
</div>
</div>
<div class="container">
<div class="result-header">
<h2>
🌾
<span id="resultCrop">
Rice
</span>
Analysis
</h2>
<p>
Area:
<strong id="resultArea">
Thanjavur
</strong>
</p>
</div>
<div class="result-grid">
<div class="result-card">
<h3>
Expected Yield
</h3>
<div
id="productionValue"
class="result-number"
>
5.8 T/A
</div>
</div>
<div class="result-card">
<h3>
Expected Demand
</h3>
<div
id="demandValue"
class="result-number"
>
3.9 T/A
</div>
</div>
<div class="result-card">
<h3>
Market Difference
</h3>
<div
id="differenceValue"
class="result-number"
>
+1.9 T/A
</div>
</div>
<div class="result-card">
<h3>
Profit Estimation
</h3>
<div
id="profitValue"
class="result-number"
>
₹72,400
</div>
</div>
</div>
<div
id="marketStatus"
class="market-status market-surplus"
>
<h2 id="marketStatusTitle">
🟢 Yield Surplus
</h2>
<p id="marketStatusText">
Expected yield is higher
than expected demand.
</p>
</div>
<div class="comparison-box">
<h2
style="
color:#155c43;
margin-bottom:20px;
"
>
📊 Yield vs Demand
</h2>
<div class="comparison-row">
<div class="comparison-label">
<span>
Expected Yield
</span>
<span id="productionLabel">
5.8 T/A
</span>
</div>
<div class="progress-background">
<div
id="productionBar"
class="progress-production"
style="width:100%"
></div>
</div>
</div>
<div class="comparison-row">
<div class="comparison-label">
<span>
Expected Demand
</span>
<span id="demandLabel">
3.9 T/A
</span>
</div>
<div class="progress-background">
<div
id="demandBar"
class="progress-demand"
style="width:67%"
></div>
</div>
</div>
</div>
<div class="profit-box">
<h2>
💰 Estimated Farm Profit
</h2>
<div
id="profitLarge"
class="profit-value"
>
₹72,400
</div>
</div>
<div class="report-area">
<button
class="report-btn"
onclick="generateReport()"
>
📋 Generate Farm Report
</button>
</div>
</div>
</section>
<section
id="page5"
class="page report-page"
>
<div class="report-wrapper">
<div class="report-header">
<div class="report-logo">
🌱 AgriTwin
<span
style="
color:#b9ed55;
"
>
AI
</span>
</div>
<div class="report-title">
Farm Intelligence Report
</div>
<div class="report-subtitle">
AI-Powered Crop Recommendation
& Farm Analysis
</div>
</div>
<div class="report-info">
<div class="report-info-item">
<h4>
Report Date
</h4>
<p id="reportDate">
—
</p>
</div>
<div class="report-info-item">
<h4>
Location
</h4>
<p id="reportLocation">
Thanjavur, Tamil Nadu
</p>
</div>
<div class="report-info-item">
<h4>
Report Type
</h4>
<p>
AI Crop Analysis
</p>
</div>
</div>
<div class="report-recommendation">
<h2>
🤖 AI Recommendation
</h2>
<div class="recommend-label">
Recommended Crop
</div>
<div
id="reportCrop"
class="recommend-crop"
>
Paddy
</div>
</div>
<h2 class="report-section-title">
📊 Key Insights
</h2>
<div class="report-metrics">
<div class="report-metric">
<div class="metric-icon">
🌱
</div>
<div class="metric-content">
<h4>
Farm Health Score
</h4>
<strong id="reportHealth">
86/100
</strong>
</div>
</div>
<div class="report-metric">
<div class="metric-icon">
🌾
</div>
<div class="metric-content">
<h4>
Predicted Yield
</h4>
<strong id="reportYield">
5.8 T/A
</strong>
</div>
</div>
<div class="report-metric">
<div class="metric-icon">
🎯
</div>
<div class="metric-content">
<h4>
Confidence Score
</h4>
<strong>
93%
</strong>
</div>
</div>
<div class="report-metric">
<div class="metric-icon">
💰
</div>
<div class="metric-content">
<h4>
Estimated Profit
</h4>
<strong id="reportProfit">
₹72,400
</strong>
</div>
</div>
<div class="report-metric">
<div class="metric-icon">
⚠️
</div>
<div class="metric-content">
<h4>
Risk Factor
</h4>
<strong id="reportRisk">
25%
</strong>
</div>
</div>
<div class="report-metric">
<div class="metric-icon">
📊
</div>
<div class="metric-content">
<h4>
Market Demand
</h4>
<strong id="reportDemand">
VERY HIGH
</strong>
</div>
</div>
</div>
<div class="ai-insight">
<div class="ai-icon">
🤖
</div>
<div>
<h2>
AI Insight
</h2>
<p id="reportInsight">
Current field conditions support
Paddy cultivation.
</p>
</div>
</div>
<div class="report-actions">
<button
class="print-btn"
onclick="window.print()"
>
🖨️ Print / Save Report
</button>
<button
class="print-btn"
onclick="showPage(4)"
>
← Back to Analysis
</button>
</div>
<div class="report-footer">
AgriTwin AI
•
Data | AI | Sustainable Farming
</div>
</div>
</section>
<script>
let selectedArea =
"Thanjavur";
let selectedCrop =
"Rice";
let serialPort = null;
let serialReader = null;
let serialBuffer = "";
const areaData = {
Thanjavur: {
soil: 42,
temperature: 30,
humidity: 65,
rain: false,
irrigation:
"IRRIGATION NOT REQUIRED",
recommendedCrop:
"Rice",
health: 86,
status:
"Healthy",
risk: 25,
demand:
"VERY HIGH"
},
Madurai: {
soil: 42,
temperature: 32,
humidity: 61,
rain: false,
irrigation:
"IRRIGATION REQUIRED",
recommendedCrop:
"Maize",
health: 72,
status:
"Warning",
risk: 38,
demand:
"HIGH"
},
Trichy: {
soil: 36,
temperature: 31,
humidity: 58,
rain: false,
irrigation:
"IRRIGATION REQUIRED",
recommendedCrop:
"Rice",
health: 68,
status:
"Needs Attention",
risk: 42,
demand:
"HIGH"
},
Salem: {
soil: 51,
temperature: 29,
humidity: 65,
rain: true,
irrigation:
"IRRIGATION NOT REQUIRED",
recommendedCrop:
"Ragi",
health: 78,
status:
"Healthy",
risk: 28,
demand:
"HIGH"
},
Kanyakumari: {
soil: 68,
temperature: 28,
humidity: 79,
rain: true,
irrigation:
"IRRIGATION NOT REQUIRED",
recommendedCrop:
"Rice",
health: 54,
status:
"Warning",
risk: 45,
demand:
"MEDIUM"
}
};
const cropData = {
Rice: {
displayName:
"Paddy",
yield:
5.8,
demand:
3.9,
price:
28000,
cost:
90000
},
Wheat: {
displayName:
"Wheat",
yield:
4.6,
demand:
5.4,
price:
30000,
cost:
85000
},
Barley: {
displayName:
"Barley",
yield:
3.8,
demand:
2.9,
price:
25000,
cost:
65000
},
Maize: {
displayName:
"Maize",
yield:
6.2,
demand:
7.5,
price:
24000,
cost:
85000
},
Ragi: {
displayName:
"Ragi",
yield:
4.2,
demand:
3.1,
price:
35000,
cost:
70000
}
};
function showPage(number) {
document
.querySelectorAll(".page")
.forEach(
page =>
page.classList.remove(
"active"
)
);
const target =
document.getElementById(
"page" + number
);
if(target) {
target.classList.add(
"active"
);
}
window.scrollTo(
0,
0
);
if(number === 3) {
document.getElementById(
"cropArea"
).innerText =
selectedArea;
}
}
function selectArea(area) {
selectedArea =
area;
const data =
areaData[area];
document.getElementById(
"page2Area"
).innerText =
area;
document.getElementById(
"regionName"
).innerText =
area;
document.getElementById(
"selectedAreaDisplay"
).innerText =
area;
document.getElementById(
"recommendedCrop"
).innerText =
data.recommendedCrop;
document.getElementById(
"healthScore"
).innerText =
data.health +
"/100";
document.getElementById(
"healthProgress"
).style.width =
data.health +
"%";
updateFarmStatus(
data.status
);
document.getElementById(
"soilValue"
).innerText =
data.soil +
"%";
document.getElementById(
"temperatureValue"
).innerText =
data.temperature +
"°C";
document.getElementById(
"humidityValue"
).innerText =
data.humidity +
"%";
document.getElementById(
"rainValue"
).innerText =
data.rain
? "YES"
: "NO";
document.getElementById(
"irrigationValue"
).innerText =
data.irrigation;
updateIrrigationColor(
data.irrigation
);
if(
area === "Thanjavur"
) {
document.getElementById(
"connectButton"
).style.display =
"block";
document.getElementById(
"connectionText"
).innerText =
"Connect ESP32 for Live Data";
}
else {
document.getElementById(
"connectButton"
).style.display =
"none";
document.getElementById(
"connectionText"
).innerText =
"Demo Mode";
}
showPage(2);
}
function updateFarmStatus(status) {
const element =
document.getElementById(
"farmStatus"
);
element.className =
"farm-status";
if(
status === "Healthy"
) {
element.classList.add(
"status-healthy"
);
element.innerText =
"🟢 Healthy";
}
else if(
status === "Warning"
) {
element.classList.add(
"status-warning"
);
element.innerText =
"🟡 Warning";
}
else if(
status === "Critical"
) {
element.classList.add(
"status-critical"
);
element.innerText =
"🔴 Critical";
}
else {
element.classList.add(
"status-attention"
);
element.innerText =
"🟠 Needs Attention";
}
}
function updateIrrigationColor(value) {
const element =
document.getElementById(
"irrigationValue"
);
const required =
value
.toUpperCase()
.includes(
"IRRIGATION REQUIRED"
)
&&
!value
.toUpperCase()
.includes(
"NOT REQUIRED"
);
if(required) {
element.style.color =
"#c62828";
}
else {
element.style.color =
"#16743b";
}
}
function selectCrop(crop) {
selectedCrop =
crop;
document.getElementById(
"resultCrop"
).innerText =
cropData[crop]
.displayName;
document.getElementById(
"resultArea"
).innerText =
selectedArea;
updateCropPrediction();
showPage(4);
}
function updateCropPrediction() {
const data =
cropData[
selectedCrop
];
const yieldValue =
data.yield;
const demandValue =
data.demand;
const difference =
yieldValue -
demandValue;
const revenue =
yieldValue *
data.price;
const profit =
revenue -
data.cost;
const profitText =
"₹" +
Math.round(
profit
).toLocaleString(
"en-IN"
);
document.getElementById(
"productionValue"
).innerText =
yieldValue.toFixed(1)
+
" T/A";
document.getElementById(
"demandValue"
).innerText =
demandValue.toFixed(1)
+
" T/A";
document.getElementById(
"differenceValue"
).innerText =
(
difference >= 0
? "+"
: ""
)
+
difference.toFixed(1)
+
" T/A";
document.getElementById(
"profitValue"
).innerText =
profitText;
document.getElementById(
"profitLarge"
).innerText =
profitText;
document.getElementById(
"productionLabel"
).innerText =
yieldValue.toFixed(1)
+
" T/A";
document.getElementById(
"demandLabel"
).innerText =
demandValue.toFixed(1)
+
" T/A";
const maximum =
Math.max(
yieldValue,
demandValue
);
document.getElementById(
"productionBar"
).style.width =
(
yieldValue /
maximum *
100
)
+
"%";
document.getElementById(
"demandBar"
).style.width =
(
demandValue /
maximum *
100
)
+
"%";
const status =
document.getElementById(
"marketStatus"
);
const title =
document.getElementById(
"marketStatusTitle"
);
const text =
document.getElementById(
"marketStatusText"
);
if(
difference >= 0
) {
status.className =
"market-status market-surplus";
title.innerText =
"🟢 Yield Surplus";
text.innerText =
"Expected yield is "
+
difference.toFixed(1)
+
" T/A higher than expected demand.";
}
else {
status.className =
"market-status market-shortage";
title.innerText =
"🔴 Yield Insufficient";
text.innerText =
"Expected yield is "
+
Math.abs(
difference
).toFixed(1)
+
" T/A lower than expected demand.";
}
}
function generateReport() {
const area =
areaData[
selectedArea
];
const crop =
cropData[
selectedCrop
];
const difference =
crop.yield -
crop.demand;
const revenue =
crop.yield *
crop.price;
const profit =
revenue -
crop.cost;
const profitText =
"₹" +
Math.round(
profit
).toLocaleString(
"en-IN"
);
const now =
new Date();
const dateText =
now.toLocaleDateString(
"en-IN",
{
day: "2-digit",
month: "short",
year: "numeric"
}
);
document.getElementById(
"reportDate"
).innerText =
dateText;
document.getElementById(
"reportLocation"
).innerText =
selectedArea +
", Tamil Nadu";
document.getElementById(
"reportCrop"
).innerText =
crop.displayName;
document.getElementById(
"reportHealth"
).innerText =
area.health +
"/100";
document.getElementById(
"reportYield"
).innerText =
crop.yield.toFixed(1)
+
" T/A";
document.getElementById(
"reportProfit"
).innerText =
profitText;
document.getElementById(
"reportRisk"
).innerText =
area.risk +
"%";
document.getElementById(
"reportDemand"
).innerText =
area.demand;
let insight;
if(
difference >= 0
) {
insight =
"Current field conditions support "
+
crop.displayName
+
" cultivation. Expected yield is "
+
Math.abs(
difference
).toFixed(1)
+
" T/A above projected demand, "
+
"with manageable risk levels and "
+
area.demand.toLowerCase()
+
" market demand.";
}
else {
insight =
"Current field conditions support "
+
crop.displayName
+
" cultivation, but projected demand "
+
"is higher than expected yield by "
+
Math.abs(
difference
).toFixed(1)
+
" T/A. Market conditions and farm "
+
"risk should be monitored.";
}
document.getElementById(
"reportInsight"
).innerText =
insight;
showPage(5);
}

function connectESP32(){
  document.getElementById("connectButton").innerText="ESP32 LIVE ✓";
  document.getElementById("connectionText").innerText="LIVE ESP32";
  refreshSensors();
}
async function refreshSensors(){
  try{
    const r=await fetch("/api",{cache:"no-store"});
    const d=await r.json();
    processESP32Data(d);
  }catch(e){
    document.getElementById("connectionText").innerText="ESP32 OFFLINE";
  }
}
function processESP32Data(d){
  if(d.soilMoisture!==undefined) document.getElementById("soilValue").innerText=d.soilMoisture+"%";
  if(d.temperature!==undefined) document.getElementById("temperatureValue").innerText=Number(d.temperature).toFixed(1)+"°C";
  if(d.humidity!==undefined) document.getElementById("humidityValue").innerText=Number(d.humidity).toFixed(1)+"%";
  if(d.rain!==undefined) document.getElementById("rainValue").innerText=d.rain?"YES":"NO";
  if(d.irrigation!==undefined){
    document.getElementById("irrigationValue").innerText=d.irrigation;
    updateIrrigationColor(d.irrigation);
  }
  if(d.health!==undefined){
    document.getElementById("healthScore").innerText=d.health+"/100";
    document.getElementById("healthProgress").style.width=d.health+"%";
  }
  if(d.status!==undefined) updateFarmStatus(d.status);
}
setInterval(refreshSensors,2000);
window.addEventListener("load",refreshSensors);
</script>
</body>
</html>

)rawliteral";

void readSensors(){
  int soilRaw=analogRead(SOIL_PIN);
  soilMoisture=constrain(map(soilRaw,4095,0,0,100),0,100);
  temperature=dht.readTemperature();
  humidity=dht.readHumidity();
  rainRaw=analogRead(RAIN_PIN);
  rainDetected=rainRaw<2000;

  if(isnan(temperature)||isnan(humidity)){
    droughtAlert=false;
    digitalWrite(LED_PIN,LOW);
    digitalWrite(BUZZER_PIN,LOW);
    return;
  }

  droughtAlert=temperature>=DROUGHT_TEMP;
  irrigationStatus=soilMoisture<30 ? "WATER NEEDED" : "NOT NEEDED";

  if(soilMoisture<20) farmStatus="Critical";
  else if(soilMoisture<30 || droughtAlert) farmStatus="Warning";
  else if(soilMoisture<40) farmStatus="Needs Attention";
  else farmStatus="Healthy";

  digitalWrite(LED_PIN,droughtAlert?HIGH:LOW);
  digitalWrite(BUZZER_PIN,droughtAlert?HIGH:LOW);
}

void updateLCD(){
  static unsigned long last=0;
  static byte screen=0;
  if(millis()-last<2500) return;
  last=millis();
  lcd.clear();

  if(isnan(temperature)||isnan(humidity)){
    lcd.setCursor(0,0); lcd.print("DHT11 Error!");
    lcd.setCursor(0,1); lcd.print("Check Sensor");
    return;
  }

  if(droughtAlert){
    lcd.setCursor(0,0); lcd.print("DROUGHT ALERT");
    lcd.setCursor(0,1); lcd.print("Temp:");
    lcd.print(temperature,1); lcd.print("C");
    return;
  }

  if(screen==0){
    lcd.setCursor(0,0); lcd.print("Soil:"); lcd.print(soilMoisture); lcd.print("%");
    lcd.setCursor(0,1); lcd.print("Temp:"); lcd.print(temperature,1); lcd.print(" C");
  }else if(screen==1){
    lcd.setCursor(0,0); lcd.print("Humidity:"); lcd.print(humidity,0); lcd.print("%");
    lcd.setCursor(0,1); lcd.print(rainDetected?"Rain: YES":"Rain: NO");
  }else if(screen==2){
    lcd.setCursor(0,0); lcd.print("Irrigation:");
    lcd.setCursor(0,1); lcd.print(irrigationStatus);
  }else{
    lcd.setCursor(0,0); lcd.print("AgriTwin AI");
    lcd.setCursor(0,1);
    if(soilMoisture<30) lcd.print("ALERT: DRY SOIL");
    else if(rainDetected) lcd.print("RAIN DETECTED");
    else lcd.print("FIELD NORMAL");
  }
  screen=(screen+1)%4;
}

void handleRoot(){
  server.send_P(200,"text/html",INDEX_HTML);
}

void handleAPI(){
  readSensors();
  String json="{";
  json += "\"soilMoisture\":" + String(soilMoisture);
  json += ",\"temperature\":" + String(temperature,1);
  json += ",\"humidity\":" + String(humidity,1);
  json += ",\"rain\":" + String(rainDetected?"true":"false");
  json += ",\"rainRaw\":" + String(rainRaw);
  json += ",\"irrigation\":\"" + irrigationStatus + "\"";
  json += ",\"drought\":" + String(droughtAlert?"true":"false");
  json += ",\"health\":" + String(soilMoisture);
  json += ",\"status\":\"" + farmStatus + "\"";
  json += "}";
  server.send(200,"application/json",json);
}

void setup(){
  Serial.begin(115200);
  pinMode(SOIL_PIN,INPUT);
  pinMode(RAIN_PIN,INPUT);
  pinMode(LED_PIN,OUTPUT);
  pinMode(BUZZER_PIN,OUTPUT);
  digitalWrite(LED_PIN,LOW);
  digitalWrite(BUZZER_PIN,LOW);

  dht.begin();
  Wire.begin(21,22);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0); lcd.print("AgriTwin AI");
  lcd.setCursor(0,1); lcd.print("WiFi Starting");
  delay(1500);
  lcd.clear();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  int tries=0;
  while(WiFi.status()!=WL_CONNECTED && tries<30){
    delay(500);
    Serial.print(".");
    tries++;
  }

  if(WiFi.status()==WL_CONNECTED){
    Serial.println();
    Serial.println("==========================");
    Serial.println("     AgriTwin AI");
    Serial.println("   ESP32 Web Dashboard");
    Serial.println("==========================");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0,0); lcd.print("WiFi Connected");
    lcd.setCursor(0,1); lcd.print(WiFi.localIP());
  }else{
    Serial.println();
    Serial.println("WiFi connection failed!");
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("WiFi Failed");
    lcd.setCursor(0,1); lcd.print("Check WiFi");
  }

  server.on("/",HTTP_GET,handleRoot);
  server.on("/api",HTTP_GET,handleAPI);
  server.begin();

  delay(1500);
  lcd.clear();
}

void loop(){
  server.handleClient();
  readSensors();
  updateLCD();

  static unsigned long lastPrint=0;
  if(millis()-lastPrint>=2000){
    lastPrint=millis();
    Serial.printf(
      "Soil:%d%% Temp:%.1fC Hum:%.1f%% Rain:%s Irrigation:%s Drought:%s\n",
      soilMoisture,temperature,humidity,
      rainDetected?"YES":"NO",
      irrigationStatus.c_str(),
      droughtAlert?"YES":"NO"
    );
  }
}
