#include <U8g2lib.h> 
#include <ESP32Servo.h>           //匯入ESP32Servo程式庫
#define BITS 10                   //10位元解析度    //解析度:訊號週期的細分程度

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
const uint8_t myFont[575] U8G2_FONT_SECTION("myFont") = 
  "\25\0\3\2\4\5\3\5\6\17\20\0\376\16\376\16\376\0\0\0\0\0\240 \5\0\350Q\60\21\246\12"
  "QiQ\22jS\242D\233\230D\231\4\61\13\245\14QeR\22\366i\20\62\16\246\12\321\14I("
  "\246\231\26V\323a\63\20\246\12\321\14I(\246\321\234\212b\62$\0\64\20\246\12Q\206Z\22U\262"
  "$K\206\61\255\0\65\17\246\12Q\34\322\352 \247\251\230\14\11\0\66\17\246\12QMa\232\16J\350"
  "\230\14\11\0\67\13\246\12Q\134\213i\61m\2\70\17\246\12\321\14IhL\206$tL\206\4\71\16"
  "\246\12\321\14IhL\6\265\61\232\0=\11V*Q\334\211\303\0\0\0\0\4\377\377P<+\17\351"
  "`\326\241\60\207\206C\230\346H\232\3\322\60\204R\232%\321\60D\255\245aH\243\264\64\14i\224\226"
  "\322tx\313A\0^\246%\17\351\340\3\71%G\206\207(+g\345\341\255\71+g\203\234\23\222a"
  "P\243\64\254\245\351\32j\352\16\14e\257\62\17\351`e\71V\216\206!\31\302,\312\201,\312\201A"
  "\312\201,\32\246,*\16R\61\213\212YT\32\6%\312\201,\12\263$\253jQ\34F\0o\303\63"
  "\17\351\340#Q\232\15\203\30%Q\22F\303\240\244I\224D\341\60HuB\62\14b\224\203Q\62H"
  "[\216e\303\251\22\305Q\222%Y\224\224\342H\24t\346#\357\370`\374\232\23rBN\270#i\216"
  "\324r$\252#Q\35Is \316JR\226\215Y\35\30\2uL$\377\350\340\15\347\254\234\225\207s"
  "V\316\312\303\35Jr\314\254d\211\244\325\324,\207\302\34\12s$M\1{I'\17\351`\305\71p"
  "\31\226(\213\262\60\11s(\207\206\203\16\345\204\34\370\235\61\35\36\322:\24\346P%\207S\0}\32"
  ".\17\351\340\345\204h\30\264\64\314\242ZS\26\15Z\24\207\321\226\206QK\226\14K-N\242$\207"
  "*Q\322\230%mI\24fuD\15\201\350\61\16\353\340#\351\60\245Q\70\14Q\26Gu\340\64("
  "a\224%a\224%a\64(\303\216D\331\62DYR\221\262\244\42eIe\30\222;\220\64\0";
  
int gasVal = 0;
int level = 0;                    //氣體濃度的等級
int criticalVal = 5;              //警報的臨界值
const byte red = 2;
const byte yellow = 15;
const byte SW_UP = 12; //27
const byte SW_DOWN = 14;
const byte buzzer = 27; //12
bool red_on = true;
Servo servo1; //建立伺服馬達物件

//把氣體濃度分成10個等級
int gasLevel[10] = {0, 70, 140, 210, 280, 350, 420, 490, 560, 630};

// numberShow儲存顯示數字0~9七段LED各自的點亮方式
// 例：顯示0 (g=0,f=1,e=1,d=1,c=1,b=1,a=1)
bool numberShow[10][7] = { 
                           {0,1,1,1,1,1,1}, 
                           {0,0,0,0,1,1,0}, 
                           {1,0,1,1,0,1,1}, 
                           {1,0,0,1,1,1,1}, 
                           {1,1,0,0,1,1,0}, 
                           {1,1,0,1,1,0,1}, 
                           {1,1,1,1,1,0,1}, 
                           {0,0,0,0,1,1,1}, 
                           {1,1,1,1,1,1,1}, 
                           {1,1,0,1,1,1,1}  
                         };
                          
//7_segLED儲存七段顯示器各個接腳的輸入腳位(g、f、e、d、c、b、a)
int seven_segLED[7] = {19,18,5,17,16,4,0};

//MQ2感測器體濃度
void MQ2(){
  gasVal = analogRead(A0);        //讀取A0腳位的類比值  
  Serial.printf("MQ-2 感測值 : %u\n", gasVal);
}

//計算氣體濃度的等級
//例：氣體濃度70~139，level = 1
void calculate_gasLevel(){
  for(int i = 0; i < 10; i++){
    if(gasVal >= gasLevel[i]) level = i;
  }
}

//七段顯示器顯示氣體濃度的等級0~9
void seven_segment_LED(){
  for(int i=0 ; i<7 ; i++){
      if(numberShow[level][i]){
        digitalWrite(seven_segLED[i], HIGH);
      }
      else{
        digitalWrite(seven_segLED[i], LOW);
      }
   }
}

//OLED顯示氣體濃度的等級0~9
//OLED顯示目前的警報臨界值0~9
void OLED(){
  u8g2.firstPage();
  do{
    u8g2.setCursor(1,16);
    u8g2.print("瓦斯濃度等級 = "+ (String)level);
    u8g2.setCursor(1,50);
    u8g2.print("臨界值 = " + (String)criticalVal);
    u8g2.sendBuffer();
  }while(u8g2.nextPage());
}

//開啟LED閃爍
void LED_ON(){
  if(red_on){
    digitalWrite(red,HIGH);
    digitalWrite(yellow,LOW);
    red_on = false;
  }else{
    digitalWrite(red,LOW);
    digitalWrite(yellow,HIGH);
    red_on = true;
  }
}

//關閉LED
void LED_OFF(){
   digitalWrite(red,LOW);
   digitalWrite(yellow,LOW);
}

//以按鈕調整警報臨界值
//黃色按鈕：提高警報臨界值
//藍色按鈕：降低警報臨界值
void control_criticalVal(){
  if(!digitalRead(SW_UP)){
    if(criticalVal < 9) criticalVal++;
  }
  if(!digitalRead(SW_DOWN)){
    if(criticalVal >0) criticalVal--;
  }  
}

//蜂鳴器
void buzzer_on(){
  for(int i=0;i<100;i++){ 
      digitalWrite(buzzer,HIGH);
      delay(3);
      digitalWrite(buzzer,LOW);
      delay(3);
  } 
}

//警報
void alert(){
  if(level >= criticalVal){
    LED_ON(); 
    servo1.write(90);
    buzzer_on(); 
  }else{
    LED_OFF();
    servo1.write(0);
  }
}

void setup() {
  Serial.begin(115200);           //開啟序列埠，並設定通訊速率為115200 (baud rate)
  
  analogSetAttenuation(ADC_11db); //設定類比輸入電壓上限為 3.6 V
  analogSetWidth(BITS);           //設定ADC的取樣位元數    //ADC:數位轉換器
  
  servo1.attach(33, 500, 2400);
  
  u8g2.begin();
  u8g2.setFont(myFont);
  u8g2.enableUTF8Print();
 
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(SW_UP, INPUT_PULLUP);
  pinMode(SW_DOWN, INPUT_PULLUP);
  for(int i = 0; i < 7; i++) pinMode(seven_segLED[i], OUTPUT);
  
  seven_segment_LED();
}

void loop() {
  MQ2();
  calculate_gasLevel();
  control_criticalVal();
  alert();
  OLED();
  seven_segment_LED();
  delay(500);
}
