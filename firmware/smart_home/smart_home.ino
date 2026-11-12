/**
 * SmartHome 华为云 IoTDA — 最终版
 * 已新增 WireTriggered 有线触发状态上报 + 标准云端命令应答
 * 喇叭=D2，按钮=A3, PIR=A0, 土壤=A2, LED=A4, 锁=D9
 */
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <mbedtls/md.h>

#define PPIR    36   // A0
#define PBUT    35   // A3
#define PSOIL   34   // A2
#define PLED    A4   // LED
#define PSPK    2    // 喇叭 D2
#define PLOCK   16   // 电磁锁 D9
#define PWIRE   27   // 有线触发 D7

#include "secrets.h"  // Copy secrets.example.h to secrets.h and fill in your own credentials.

WiFiClientSecure tls;
WiFiServer webServer(80);
float t1=25,h1=60,t2=22,h2=55;
int soil=0,ledVal=0,motion=0,pirRaw=0,lockCnt=0,butState=0,spkVal=0;
unsigned long lastRpt=0,lastPing=0;
uint16_t pktId=1;
bool appOverride=false;

// 新增：有线触发状态变量 1=触发(低电平) 0=未触发
int wireTriggered = 0;

String hmacHex(const String& k,const String& m){
  byte h[32];mbedtls_md_context_t c;mbedtls_md_init(&c);
  mbedtls_md_setup(&c,mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),1);
  mbedtls_md_hmac_starts(&c,(const byte*)k.c_str(),k.length());
  mbedtls_md_hmac_update(&c,(const byte*)m.c_str(),m.length());
  mbedtls_md_hmac_finish(&c,h);mbedtls_md_free(&c);
  char x[65];for(int i=0;i<32;i++)sprintf(x+i*2,"%02x",h[i]);return String(x);
}

void w16(uint8_t*b,int&p,uint16_t v){b[p++]=v>>8;b[p++]=v&0xFF;}
void wStr(uint8_t*b,int&p,const char*s){int l=strlen(s);w16(b,p,l);memcpy(b+p,s,l);p+=l;}
void wHdr(uint8_t*b,int&p,uint8_t typ,int rem){b[p++]=typ;do{uint8_t d=rem&0x7F;rem>>=7;if(rem)d|=0x80;b[p++]=d;}while(rem);}

bool mqttConnect(String cid,String usr,String pwd){
  uint8_t p[256];int i=0;
  int vh0=i;wStr(p,i,"MQTT");p[i++]=4;p[i++]=0xC2;w16(p,i,60);
  int pl0=i;wStr(p,i,cid.c_str());wStr(p,i,usr.c_str());wStr(p,i,pwd.c_str());
  int rem=i-vh0;uint8_t buf[280];int j=0;wHdr(buf,j,0x10,rem);
  memcpy(buf+j,p+vh0,rem);tls.write(buf,j+rem);tls.flush();
  unsigned long t0=millis();
  while(!tls.available()&&millis()-t0<15000){delay(100);yield();}
  if(!tls.available())return false;uint8_t r[4];tls.read(r,4);return r[3]==0;
}

void mqttPub(String t,String m){
  int rem=2+t.length()+m.length();uint8_t h[10];int hi=0;wHdr(h,hi,0x30,rem);tls.write(h,hi);
  uint8_t b[2];int bi=0;w16(b,bi,t.length());tls.write(b,2);
  tls.write((const uint8_t*)t.c_str(),t.length());tls.write((const uint8_t*)m.c_str(),m.length());tls.flush();
}

void mqttSub(String t){
  uint8_t p[160];int i=0,vh0=i;w16(p,i,pktId++);wStr(p,i,t.c_str());p[i++]=0;
  int rem=i-vh0;uint8_t buf[180];int j=0;wHdr(buf,j,0x82,rem);
  memcpy(buf+j,p+vh0,rem);tls.write(buf,j+rem);tls.flush();
}

void mqttPing(){uint8_t b[]={0xC0,0x00};tls.write(b,2);tls.flush();}

String gCid,gUsr,gPwd;
void genCred(){
  time_t now=time(nullptr);struct tm* utc=gmtime(&now);
  char ts[11];sprintf(ts,"%04d%02d%02d%02d",utc->tm_year+1900,utc->tm_mon+1,utc->tm_mday,utc->tm_hour);
  gCid=DID+"_0_0_"+String(ts);gUsr=DID;gPwd=hmacHex(String(ts),DSEC);
}

void setLED(int v){
  ledVal=v;
  pinMode(PLED,OUTPUT);
  digitalWrite(PLED,v?HIGH:LOW);
}

void lockPulse(){
  digitalWrite(PLOCK,HIGH);
  delay(300);
  digitalWrite(PLOCK,LOW);
}

void spkToggle(){
  spkVal=!spkVal;
  if(spkVal){
    ledcSetup(0,800,8);
    ledcAttachPin(PSPK,0);
    ledcWrite(0,128);
  }
  else{
    ledcWrite(0,0);
    ledcDetachPin(PSPK);
  }
  Serial.print("SPK:");
  Serial.println(spkVal);
}

void notifyWx(String msg){
  WiFiClient w;
  if(w.connect("sctapi.ftqq.com",80)){
    String req = "GET /" + String(SCT_KEY) + ".send?title=SmartHome&desp=" + msg + " HTTP/1.1\r\nHost: sctapi.ftqq.com\r\nConnection: close\r\n\r\n";
    w.print(req);
    w.stop();
  }
}

void reportAll(){
  motion=0;
  if(soil>0&&soil<100)motion=1;

  // 实时读取有线引脚状态，赋值给上报变量
  wireTriggered = (digitalRead(PWIRE) == LOW) ? 1 : 0;

  char buf[600];
  snprintf(buf,sizeof(buf),
    "{\"services\":[{\"service_id\":\"SmartHome\",\"properties\":{"
    "\"IndoorTemperature\":%.1f,\"IndoorHumidity\":%.1f,"
    "\"OutdoorTemperature\":%.1f,\"OutdoorHumidity\":%.1f,"
    "\"MotionDetected\":%d,\"LEDSwitch\":%d,"
    "\"LockSwitch\":0,\"SpeakerSwitch\":%d,\"ButtonPressed\":%d,\"SoilMoisture\":%d,"
    "\"WireTriggered\":%d}}]}",
    t1,h1,t2,h2,motion,ledVal,spkVal,butState,soil,
    wireTriggered);
  mqttPub("$oc/devices/"+DID+"/sys/properties/report",String(buf));
  Serial.println("属性上报: " + String(buf));
}

// ========== 新增：标准命令应答函数 ==========
void sendCmdResponse(const char* reqId, int code, const char* desc){
  String respTopic = "$oc/devices/" + DID + "/sys/commands/response/request_id=" + String(reqId);
  String respData = "{\"result_code\":" + String(code) + ",\"result_desc\":\"" + String(desc) + "\"}";
  mqttPub(respTopic, respData);
  Serial.println("命令应答: " + respData);
}

// 处理云端下发命令 + 回复应答
void handleMsg(char* topic,int tlen,char* data,int dlen){
  data[dlen]=0;
  JsonDocument doc;
  if(deserializeJson(doc,data))return;

  // 提取 request_id
  char rid[64]="";
  char* rp=strstr(topic,"request_id=");
  if(rp){
    strncpy(rid,rp+11,36);
    rid[36]=0;
  }
  const char* cn=doc["command_name"]|doc["commandName"]|"";
  if(strlen(cn)==0)return;

  int v=doc["paras"]["value"]|0;
  Serial.print("收到云端命令: ");
  Serial.println(cn);

  // 执行对应命令
  if(strcmp(cn,"SetLED")==0){
    appOverride=true;
    setLED(v);
    sendCmdResponse(rid, 0, "LED命令执行成功");
  }
  else if(strcmp(cn,"SetLock")==0){
    if(v)lockPulse();
    sendCmdResponse(rid, 0, "开锁命令执行成功");
  }
  else if(strcmp(cn,"SetSpeaker")==0){
    if(v)spkToggle();
    sendCmdResponse(rid, 0, "喇叭命令执行成功");
  }
  else{
    sendCmdResponse(rid, -1, "未知命令");
  }

  // 命令执行后主动上报最新状态
  reportAll();
}

void setup(){
  Serial.begin(115200);
  delay(500);
  Serial.println("\n===== SmartHome =====");
  pinMode(PLED,OUTPUT);digitalWrite(PLED,LOW);
  pinMode(PLOCK,OUTPUT);digitalWrite(PLOCK,LOW);
  pinMode(PPIR,INPUT);
  pinMode(PBUT,INPUT);
  pinMode(PWIRE,INPUT_PULLUP);

  Serial.print("WiFi:");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WSSID,WPASS);
  for(int i=0;i<60&&!WiFi.isConnected();i++){delay(500);Serial.print(".");yield();}
  if(!WiFi.isConnected()){Serial.println("WiFi 连接失败");return;}
  Serial.printf("WiFi 已连接 IP:%s\n",WiFi.localIP().toString().c_str());

  Serial.print("NTP 时间同步");
  configTime(8*3600,0,"ntp.aliyun.com","pool.ntp.org");
  time_t now=0;
  for(int i=0;i<60&&now<1000000000;i++){delay(500);Serial.print(".");now=time(nullptr);yield();}
  if(now<1000000000){Serial.println(" 时间同步失败");return;}
  Serial.println(" 时间同步成功");

  tls.setInsecure();
  if(!tls.connect(MHOST,MPORT)){Serial.println("TLS 服务器连接失败");return;}
  genCred();
  if(!mqttConnect(gCid,gUsr,gPwd)){Serial.println("MQTT 登录失败");return;}
  Serial.println("MQTT 接入华为云成功");

  // 订阅云端命令主题
  mqttSub("$oc/devices/"+DID+"/sys/commands/#");
  webServer.begin();
  reportAll();
  lastRpt=lastPing=millis();
  Serial.println("系统就绪！");
}

void loop(){
  yield();

  // 1. PIR -> LED
  static int lastPir=-1;
  int pir=digitalRead(PPIR);
  if(lastPir==-1)lastPir=pir;
  else if(pir!=lastPir){
    lastPir=pir;
    pirRaw=pir;
    if(!appOverride){
      setLED(pir);
      reportAll();
      notifyWx(pir?"PIR_有人":"PIR_无人");
    }
  }

  // 2. Button -> 本地喇叭 + 云规则双路
  static int lastBut=-1;
  int but=digitalRead(PBUT);
  if(lastBut==-1)lastBut=but;
  else if(but!=lastBut){
    lastBut=but;
    butState=but;
    if(but==HIGH){
      spkToggle();
      reportAll();
    }
  }

  // 3. Wire trigger -> lock 有线触发开锁联动
  static int lastWire=-1;
  int wire=digitalRead(PWIRE);
  if(lastWire==-1)lastWire=wire;
  else if(wire!=lastWire){
    lastWire=wire;
    if(wire==LOW){
      lockPulse();
      lockCnt++;
      reportAll();
      notifyWx("开锁!");
    }
  }

  // 4. Soil 土壤检测
  static unsigned long lastSoil=0;
  if(millis()-lastSoil>2000){
    lastSoil=millis();
    soil=analogRead(PSOIL);
    static bool w=false;
    if(soil>0&&soil<100&&!w){
      w=true;
      motion=1;
      reportAll();
    }
    if(soil>=150)w=false;
  }

  // 5. 本地网页服务
  WiFiClient c=webServer.available();
  if(c&&c.connected()){
    String r="";
    unsigned long wt=millis();
    while(!c.available()&&millis()-wt<200)delay(5);
    while(c.available()&&r.length()<300){r+=(char)c.read();}
    if(r.length()>=5){
      if(r.startsWith("GET /api/data")){
        char j[350];
        snprintf(j,sizeof(j),"{\"t1\":%.1f,\"h1\":%.1f,\"soil\":%d,\"led\":%d,\"spk\":%d,\"motion\":%d,\"pir\":%d,\"lock\":%d,\"btn\":%d}",t1,h1,soil,ledVal,spkVal,motion,pirRaw,lockCnt,butState);
        c.print("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n");
        c.print(j);
      }else if(r.startsWith("GET /api/cmd")){
        if(r.indexOf("type=led")>0){appOverride=true;setLED(!ledVal);}
        if(r.indexOf("type=lock")>0)lockPulse();
        if(r.indexOf("type=spk")>0)spkToggle();
        if(r.indexOf("type=auto")>0)appOverride=false;
        c.print("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n{\"ok\":1}");
      }else{
        bool isAPP=r.indexOf("Dalvik")>0||r.indexOf("Android")>0;
        String h="HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n\r\n";
        if(isAPP){
          // 只需要替换代码中 `h+="<!DOCTYPE html>..."` 这一大段即可
h+="<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
h+="<title>SmartHome</title><style>*{margin:0;padding:0}body{font-family:Arial;background:#121212;color:#eee;padding:12px}";
h+=".bar{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px}";
h+=".bar h3{color:#ff6b6b}.bar span{font-size:10px;color:#888}";
h+=".row{display:flex;gap:8px;margin:6px 0}";
h+=".card{flex:1;background:#1e1e1e;border-radius:14px;padding:14px 8px;text-align:center;border:1px solid #333}";
h+=".card .val{font-size:24px;font-weight:bold;margin:4px 0}";
h+=".card .lbl{font-size:10px;color:#888}";
h+=".btn{display:block;width:100%;padding:16px;margin:6px 0;border:none;border-radius:25px;font-size:16px;font-weight:bold;cursor:pointer;transition:all .2s}";
h+=".btn:active{transform:scale(.96)}";
h+=".red{background:linear-gradient(135deg,#e94560,#c0392b);color:#fff}.red.on{background:linear-gradient(135deg,#4ecca3,#27ae60)}";
h+=".purple{background:linear-gradient(135deg,#7b2ff7,#5b1fc4);color:#fff}";
h+=".orange{background:linear-gradient(135deg,#f0a500,#d68910);color:#000}";
h+="input{width:100%;padding:14px;font-size:16px;border-radius:25px;border:2px solid #7b2ff7;background:#1e1e1e;color:#eee;margin:6px 0}";
h+="canvas{width:100%;height:130px;background:#1e1e1e;border-radius:14px;margin:6px 0;border:1px solid #333}";
h+="#st{text-align:center;font-size:10px;color:#888;margin:4px 0}</style></head><body>";
h+="<div class='bar'><h3>SmartHome</h3><span id='st'>连接中...</span></div>";
h+="<div class='row'><div class='card'><div class='lbl'>🌡 温度</div><div class='val' id='t1' style='color:#e94560'>--</div><div class='lbl'>°C</div></div>";
h+="<div class='card'><div class='lbl'>💧 湿度</div><div class='val' id='h1' style='color:#4e8cff'>--</div><div class='lbl'>%</div></div></div>";
h+="<div class='row'><div class='card'><div class='lbl'>🌿 土壤</div><div class='val' id='so' style='color:#4ecca3'>--</div></div>";
h+="<div class='card'><div class='lbl'>👤 PIR</div><div class='val' id='mo' style='color:#f0a500'>--</div></div></div>";
h+="<div class='row'><div class='card'><div class='lbl'>🔓 开锁次数</div><div class='val' id='lk' style='color:#f0a500'>--</div></div>";
h+="<div class='card'><div class='lbl'>🔄 模式</div><div class='val' id='md' style='color:#888;font-size:14px'>--</div></div></div>";
h+="<canvas id='cv'></canvas>";
h+="<input id='vc' placeholder='🎤 说:开灯/关灯/开锁/喇叭' onkeydown='if(event.key==\"Enter\")voice()'>";
h+="<div class='row'><button class='btn red' id='lb' onclick='cmd(\"led\")' style='flex:1'>LED</button>";
h+="<button class='btn purple' id='sb' onclick='cmd(\"spk\")' style='flex:1'>喇叭</button></div>";
h+="<button class='btn orange' onclick='cmd(\"lock\")'>开锁</button>";
h+="<button class='btn' style='background:#555;color:#aaa;width:100%;padding:12px;margin:6px 0;border:none;border-radius:25px;font-size:14px;cursor:pointer' onclick='cmd(\"auto\")'>🔄 恢复自动</button>";
h+="<script>var led=0,spk=0,hist=[],retryCount=0;const maxRetry=3;function $(id){return document.getElementById(id);}";
h+="function voice(){var t=$('vc').value;$('vc').value='';if(t.indexOf('开灯')>=0)cmd('led');else if(t.indexOf('关灯')>=0)cmd('led');else if(t.indexOf('开锁')>=0)cmd('lock');else if(t.indexOf('喇叭')>=0)cmd('spk');}";
h+="function fmt(v,d){return v!=null?v.toFixed(d):'--';}";
h+="async function poll(){try{var r=await fetch('/api/data');if(!r.ok) throw new Error('请求失败');var d=await r.json();retryCount=0;$('st').textContent='在线';$('t1').textContent=fmt(d.t1,1);$('h1').textContent=fmt(d.h1,1);$('so').textContent=d.soil;$('mo').textContent=d.pir?'有人':'无人';$('lk').textContent=d.lock;$('md').textContent=d.led!=null?'APP手动':'--';led=d.led;spk=d.spk;$('lb').textContent='LED'+(led?' ●':' ○');$('lb').className='btn red'+(led?' on':'');$('sb').textContent='喇叭'+(spk?' 🔊':' 🔇');$('sb').className='btn purple'+(spk?' on':'');hist.push({t:new Date().toLocaleTimeString(),v:d.soil});if(hist.length>40)hist.shift();var cv=$('cv'),cx=cv.getContext('2d');cv.width=cv.parentNode.clientWidth-24;cx.clearRect(0,0,cv.width,cv.height);if(hist.length>1){cx.beginPath();cx.strokeStyle='#4ecca3';cx.lineWidth=2;for(var i=0;i<hist.length;i++){var x=i*(cv.width/hist.length);var y=cv.height-(hist[i].v/500)*cv.height;if(i==0)cx.moveTo(x,y);else cx.lineTo(x,y);}cx.stroke();}}catch(e){retryCount++;if(retryCount>maxRetry){$('st').textContent='离线（请求失败）';}else{$('st').textContent='离线，正在重试...';}}}";
h+="async function cmd(t){try{await fetch('/api/cmd?type='+t);setTimeout(poll,200);}catch(e){alert('指令发送失败，请重试');}}";
h+="poll();setInterval(poll,3000);</script></body></html>";
        }
        else {
          // 桌面浏览器：白蓝新设计
          h+="<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
          h+="<title>SmartHome</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:'Segoe UI',Arial,sans-serif;background:linear-gradient(180deg,#e8f4fd 0%,#f0f8ff 30%,#fff 100%);color:#2c3e50;min-height:100vh;padding:0}";
          h+=".topbar{background:linear-gradient(135deg,#1565c0,#1e88e5);color:#fff;padding:16px 20px;text-align:center;font-size:20px;font-weight:bold;box-shadow:0 2px 10px rgba(0,0,0,.1)}";
          h+=".topbar span{font-size:12px;opacity:.8;display:block}";
          h+=".wrap{max-width:600px;margin:0 auto;padding:16px}";
          h+=".grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:12px 0}";
          h+=".card{background:#fff;border-radius:16px;padding:16px;text-align:center;box-shadow:0 2px 8px rgba(0,0,0,.06);border:1px solid #e8e8e8}";
          h+=".card .val{font-size:30px;font-weight:700;margin:6px 0}";
          h+=".card .lbl{font-size:12px;color:#999;text-transform:uppercase;letter-spacing:1px}";
          h+=".btn{display:block;width:100%;padding:14px;margin:8px 0;border:none;border-radius:30px;font-size:16px;font-weight:600;cursor:pointer;transition:.2s;box-shadow:0 2px 6px rgba(0,0,0,.1)}";
          h+=".btn:active{transform:scale(.96);opacity:.9}";
          h+=".led{background:#fff;color:#e53935;border:2px solid #e53935}.led.on{background:#e53935;color:#fff}";
          h+=".spk{background:#fff;color:#7b1fa2;border:2px solid #7b1fa2}.spk.on{background:#7b1fa2;color:#fff}";
          h+=".lock{background:#fff;color:#ef6c00;border:2px solid #ef6c00}.lock.on{background:#ef6c00;color:#fff}";
          h+=".auto-btn{background:#fff;color:#555;border:2px solid #ddd}";
          h+="canvas{width:100%;height:180px;background:#f8fbff;border-radius:16px;margin:10px 0;border:1px solid #e0e8f0}";
          h+="input{width:100%;padding:14px;border-radius:30px;border:2px solid #e0e0e0;font-size:14px;background:#fff;text-align:center;outline:none;transition:.2s}";
          h+="input:focus{border-color:#1e88e5;box-shadow:0 0 0 3px rgba(30,136,229,.15)}";
          h+=".toggle-wrap{display:flex;gap:10px}.toggle-wrap .btn{flex:1}";
          h+="#st{text-align:center;font-size:11px;color:#aaa;margin:8px 0}</style></head><body>";
          h+="<div class='topbar'>🏠 SmartHome<span id='st'>连接中...</span></div>";
          h+="<div class='wrap'><div class='grid'><div class='card'><div class='lbl'>🌡 温度</div><div class='val' id='t1' style='color:#e53935'>--</div><div class='lbl'>°C</div></div>";
          h+="<div class='card'><div class='lbl'>💧 湿度</div><div class='val' id='h1' style='color:#1e88e5'>--</div><div class='lbl'>%</div></div>";
          h+="<div class='card'><div class='lbl'>🌿 土壤</div><div class='val' id='so' style='color:#43a047'>--</div></div>";
          h+="<div class='card'><div class='lbl'>👤 人体</div><div class='val' id='mo' style='color:#ef6c00'>--</div></div></div>";
          h+="<canvas id='cv'></canvas>";
          h+="<input id='vc' placeholder='⌨️ 输入指令或说: 开灯/关灯/开锁/喇叭' onkeydown='if(event.key===\"Enter\")voice()'>";
          h+="<div class='toggle-wrap'><button class='btn led' id='lb' onclick='cmd(\"led\")'>💡 LED</button>";
          h+="<button class='btn spk' id='sb' onclick='cmd(\"spk\")'>🔊 喇叭</button></div>";
          h+="<div class='toggle-wrap'><button class='btn lock' id='lk' onclick='cmd(\"lock\")'>🔓 开锁</button>";
          h+="<button class='btn auto-btn' onclick='cmd(\"auto\")'>🔄 自动</button></div></div>";
          h+="<script>var led=0,spk=0,hist=[],retryCount=0;const maxRetry=3;function $(id){return document.getElementById(id);}";
          h+="function voice(){var t=$('vc').value;$('vc').value='';if(t.indexOf('开灯')>=0)cmd('led');else if(t.indexOf('关灯')>=0)cmd('led');else if(t.indexOf('开锁')>=0)cmd('lock');else if(t.indexOf('喇叭')>=0)cmd('spk');}";
          h+="function fmt(v,d){return v!=null?v.toFixed(d):'--';}";
          h+="async function poll(){try{var r=await fetch('/api/data');if(!r.ok)throw new Error('请求失败');var d=await r.json();retryCount=0;$('st').textContent='在线 | '+new Date().toLocaleTimeString();$('t1').textContent=fmt(d.t1,1);$('h1').textContent=fmt(d.h1,1);$('so').textContent=d.soil;$('mo').textContent=d.pir?'🔴有人':'🟢无人';led=d.led;spk=d.spk;$('lb').textContent='💡 LED'+(led?' [开]':' [关]');$('lb').className='btn led'+(led?' on':'');$('sb').textContent='🔊 喇叭'+(spk?' [开]':' [关]');$('sb').className='btn spk'+(spk?' on':'');hist.push({t:new Date().toLocaleTimeString(),v:d.soil});if(hist.length>40)hist.shift();var cv=$('cv'),cx=cv.getContext('2d');cv.width=cv.parentNode.clientWidth-32;cx.clearRect(0,0,cv.width,cv.height);if(hist.length>1){cx.beginPath();cx.strokeStyle='#1e88e5';cx.lineWidth=2.5;for(var i=0;i<hist.length;i++){var x=i*(cv.width/hist.length);var y=cv.height-(hist[i].v/500)*cv.height;if(i==0)cx.moveTo(x,y);else cx.lineTo(x,y);}cx.stroke();}}catch(e){retryCount++;if(retryCount>maxRetry){$('st').textContent='离线';}else{$('st').textContent='离线，正在重试...';}}}";
          h+="async function cmd(t){try{await fetch('/api/cmd?type='+t);setTimeout(poll,200);}catch(e){alert('指令发送失败，请重试');}}";
          h+="poll();setInterval(poll,1000);</script></body></html>";
        }
        c.print(h);
      }
      c.flush();delay(5);
    }
    c.stop();
  }

  // 6. MQTT 消息解析
  if(tls.available()){
    uint8_t h[5];int hi=0;while(tls.available()&&hi<5)h[hi++]=tls.read();
    if(hi>=2){
      uint8_t typ=h[0]&0xF0;
      int rem=0,mult=1,ri=1;
      while(ri<hi){rem+=(h[ri]&0x7F)*mult;mult*=128;if(!(h[ri]&0x80))break;ri++;}
      if(rem<=200){
        uint8_t p[200];int pl=0;while(pl<rem&&tls.available())p[pl++]=tls.read();
        if(typ==0x30&&pl>=2){
          int qos=(h[0]&0x06)>>1,off=(qos>0)?2:0,tlen=(p[off]<<8)|p[off+1];
          if(tlen>0&&tlen<120){
            char tp[128];memset(tp,0,128);memcpy(tp,p+off+2,tlen);
            int dl=pl-off-2-tlen;
            if(dl>0&&dl<120){
              char dt[128];memset(dt,0,128);
              memcpy(dt,p+off+2+tlen,dl);
              handleMsg(tp,tlen,dt,dl);
            }
          }
        }
      }else{
        while(tls.available())tls.read();
      }
    }
  }

  if(millis()-lastPing>50000){mqttPing();lastPing=millis();}
  if(millis()-lastRpt>=30000){lastRpt=millis();reportAll();}
  delay(100);
}
