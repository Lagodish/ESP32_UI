#include <Arduino.h>
#include <Wire.h>
#include <menu.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include <EEPROM.h>
#include <GyverButton.h>

#define GP1 36
#define GP2 39
#define GP3 2
#define GP4 4

GButton butt1(GP1);
GButton butt2(GP2);
GButton butt3(GP3);
GButton butt4(GP4);

//xSemaphoreCreateMutex

#define EEPROM_SIZE 30

using namespace Menu;

#define fontName u8g2_font_7x13_mf
#define fontX 7
#define fontY 16
#define offsetX 0
#define offsetY 0
#define U8_Width 128
#define U8_Height 64
#define USE_HWI2C

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0,22,21,U8X8_PIN_NONE);

typedef u8g2_uint_t u8g_uint_t;

const colorDef<uint8_t> colors[6] MEMMODE={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};

bool LightCtrl=HIGH;
bool FanCtrl=HIGH;
bool Temp_mode=HIGH;
bool Silence=LOW;
bool mainScreenOn = true;
bool showTemp = false;

uint8_t SetTemp=10; //Setted temperature
uint8_t Wireless=0;
uint8_t Lang=1;
uint8_t PERF=1;
uint8_t BRT_max = 80;
uint8_t SPD_max = 80;
uint8_t BRT_Disp = 20;
double setted_temp = 16.0;
uint8_t timer_1 = 0;

bool oldRetCode; 
bool Hysteresis(double temp_now) {
 
  bool retCode = false;
  if (temp_now > setted_temp+PERF) {
    retCode = true;
    //showLcd(" on");
  } else if ( temp_now < setted_temp-PERF) {
    retCode = false;
    //showLcd("off"); 
  } else {
    retCode = oldRetCode;
  }
   
  oldRetCode = retCode;
  return retCode;
}

result action1(eventMask e,navNode& nav, prompt &item) {
  EEPROM.write(0, BRT_Disp);
  EEPROM.write(1,BRT_max);
  EEPROM.write(2,SPD_max);
  EEPROM.write(3,PERF);
  EEPROM.write(4,Lang);
  EEPROM.write(5,Temp_mode);
  EEPROM.write(6,LightCtrl);
  EEPROM.write(7,FanCtrl);
  EEPROM.write(8,Silence);
  EEPROM.write(9,Wireless);

  EEPROM.commit();
  return proceed;
}


TOGGLE(LightCtrl,setLight,"Auto Light: ",action1,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);


TOGGLE(FanCtrl,setFan,"Auto Fan: ",action1,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);


TOGGLE(Temp_mode,TempMenu,"Temp: ",action1,enterEvent,noStyle
  ,VALUE("Celsius",HIGH,doNothing,noEvent)
  ,VALUE("Fahrenheit",LOW,doNothing,noEvent)
);


TOGGLE(Silence,setSilence,"Silence Mode: ",action1,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);

TOGGLE(Wireless,setWireless,"Wireless: ",action1,enterEvent,noStyle
  ,VALUE("Off",0,doNothing,noEvent)
  ,VALUE("WiFi",1,doNothing,noEvent)
  ,VALUE("BLE",2,doNothing,noEvent)
);


TOGGLE(Lang,LangueMenu,"Langue: ",action1,enterEvent,noStyle
  ,VALUE("Ru",0,doNothing,noEvent)
  ,VALUE("En",1,doNothing,noEvent)
  ,VALUE("Ge",2,doNothing,noEvent)
);


TOGGLE(PERF,PerformanceMenu,"Perf: ",action1,enterEvent,noStyle
  ,VALUE("Eco",3,doNothing,noEvent)
  ,VALUE("Balanced",2,doNothing,noEvent)
  ,VALUE("High",1,doNothing,noEvent)
);



MENU(LightMenu,"Light control",doNothing,noEvent,noStyle
  ,SUBMENU(setLight)
  ,FIELD(BRT_max,"Max brt","%",0,100,10,1,action1,enterEvent,wrapStyle)
  ,EXIT("<Back")
);


MENU(FanMenu,"Fan control",doNothing,noEvent,noStyle
  ,SUBMENU(setFan)
  ,FIELD(SPD_max,"Max spd","%",0,100,10,1,action1,enterEvent,wrapStyle)
  ,EXIT("<Back")
);
 
uint16_t hrs=12;
uint16_t mins=0;

uint16_t year=2020;
uint16_t month=10;
uint16_t day=15;

PADMENU(YMD_Menu,"Date",doNothing,noEvent,noStyle
  ,FIELD(year,"","/",1900,3000,20,1,doNothing,noEvent,noStyle)
  ,FIELD(month,"","/",1,12,1,0,doNothing,noEvent,wrapStyle)
  ,FIELD(day,"","",1,31,1,0,doNothing,noEvent,wrapStyle)
);

PADMENU(HM_Menu,"Time",doNothing,noEvent,noStyle
  ,FIELD(hrs,"",":",0,23,1,0,doNothing,noEvent,wrapStyle)
  ,FIELD(mins,"","",0,59,1,0,doNothing,noEvent,wrapStyle)
);

MENU(timeMenu,"Time & Date",doNothing,noEvent,noStyle
  ,SUBMENU(HM_Menu)
  ,SUBMENU(YMD_Menu)
  ,EXIT("<Back")
);


MENU(mainMenu,"Settings",doNothing,noEvent,noStyle
  ,SUBMENU(setSilence)
  ,SUBMENU(timeMenu)
  ,SUBMENU(LightMenu)
  ,SUBMENU(FanMenu)
  ,SUBMENU(PerformanceMenu)
  ,SUBMENU(setWireless)
  ,SUBMENU(TempMenu)
  ,SUBMENU(LangueMenu)
  ,FIELD(BRT_Disp,"Display Brt","%",0,100,10,0,action1,enterEvent,noStyle)
  ,EXIT("<Back")
);

#define MAX_DEPTH 3

serialIn serial(Serial);
MENU_INPUTS(in,&serial);


MENU_OUTPUTS(out,MAX_DEPTH
  ,U8G2_OUT(u8g2,colors,fontX,fontY,offsetX,offsetY,{0,0,U8_Width/fontX,U8_Height/fontY})
  ,SERIAL_OUT(Serial)
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

int blink=0;
double temp = 9.0;
double tempPrint =0.0;

//when menu is suspended
result MainScreen(menuOut& o,idleEvent e) {
  o.clear();
  const char DEGREE_SYMBOL[] = { 0xB0, '\0' };
  const char ALERT_SYMBOL[] = { 71, '\0' }; //DOTO как в Aplle
  const char bluetooth_SYMBOL[] = { 74, '\0' };
  const char wifi_SYMBOL[] = { 80, '\0' }; //Заменить значок (не вытянутый)
  const char setup_SYMBOL[] = { 66, '\0' };
  //if(temp>12){temp=9;}
  if(showTemp){tempPrint = setted_temp;}
  else{tempPrint = temp;}
  
  switch(e) {
    case idleStart:/*o.println("suspending menu!")*/;break;
    case idling:{
    mainScreenOn=true;
    if(!Temp_mode){tempPrint =(tempPrint*9/5) + 32;}
    if(tempPrint<9.95){
    u8g2.setFont(u8g2_font_ncenB24_tf);
    u8g2.drawUTF8(83, 33, DEGREE_SYMBOL);
    u8g2.setFont(u8g2_font_fur35_tr);
    o.setCursor(2,2); 
    o.print(String(tempPrint,1)); 
    u8g2.setFont(u8g2_font_fur30_tr);
    o.setCursor(13,2);
    }
    else{
    u8g2.setFont(u8g2_font_ncenB24_tf);
    u8g2.drawUTF8(91, 33, DEGREE_SYMBOL);
    u8g2.setFont(u8g2_font_fur35_tr);
    o.setCursor(0,2);
    o.print(String(tempPrint,1)); 
    u8g2.setFont(u8g2_font_fur30_tr);
    o.setCursor(14,2);
    }
    if(Temp_mode){o.print("C");}else{o.print("F");}
    u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t); //{Eeeeeeeeeeeeeeeee}
    if(timer_1>0){u8g2.drawUTF8(8, 64, setup_SYMBOL);}
    if(Wireless==1){u8g2.drawUTF8(72, 64, wifi_SYMBOL);}
    if(Wireless==2){u8g2.drawUTF8(72, 64, bluetooth_SYMBOL);}
    //u8g2.drawUTF8(72, 64, SYMBOL);
    if(!Hysteresis(temp)){u8g2.drawUTF8(106, 64, ALERT_SYMBOL);}
    break;}
    case idleEnd:/*o.println("resuming menu.");*/mainScreenOn=false;u8g2.setFont(fontName);break;
  }

  return proceed;
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Wire.begin();
  u8g2.begin();

  EEPROM.begin(EEPROM_SIZE);
  delay(10);
  BRT_Disp = EEPROM.read(0);
  BRT_max = EEPROM.read(1);
  SPD_max = EEPROM.read(2);
  PERF = EEPROM.read(3);
  Lang = EEPROM.read(4);
  Temp_mode = EEPROM.read(5);
  LightCtrl = EEPROM.read(6);
  FanCtrl = EEPROM.read(7);
  Silence = EEPROM.read(8);
  Wireless = EEPROM.read(9);
  
  u8g2.setFont(fontName);
  // u8g2.setBitmapMode(0);

  // disable second option
  //mainMenu[1].enabled=disabledStatus;
  nav.idleTask=MainScreen;//point a function to be used when menu is suspended
  nav.timeOut=30;
  nav.idleOn(MainScreen);

  //butt1.setTickMode(AUTO);
  //butt2.setTickMode(AUTO);
  //butt3.setTickMode(AUTO);
  //butt4.setTickMode(AUTO);
}

bool butt1_l = false;
bool butt2_l = false;
bool butt3_l = false;
bool butt4_l = false;


void loop() {
  butt1.tick();
  butt2.tick();
  butt3.tick();
  butt4.tick();
  if (butt1.isClick()){butt1_l = true;nav.doNav(enterCmd);Serial.println("enterCmd");}else{butt1_l = false;}
  if (butt2.isClick()){butt2_l = true;nav.doNav(upCmd);Serial.println("upCmd");}else{butt2_l = false;}
  if (butt3.isClick()){butt3_l = true;nav.doNav(downCmd);Serial.println("downCmd");} else{butt3_l = false;}
  if (butt4.isClick()){butt4_l = true;nav.doNav(escCmd);Serial.println("escCmd");} else{butt4_l = false;}
  
  if(mainScreenOn&&(butt2_l||butt2.isStep())){setted_temp+=0.5;if(setted_temp>18){setted_temp=18;}showTemp=true;timer_1=1;}
  if(mainScreenOn&&(butt3_l||butt3.isStep())){setted_temp-=0.5;if(setted_temp<5){setted_temp=5;}showTemp=true;timer_1=1;}

  nav.doInput();
  //if (nav.changed(0)) {
    //temp+=0.1;
    int contrast = map(BRT_Disp, 0, 100, 0, 190);
    u8g2.setContrast(contrast);
    u8g2.firstPage();
    do nav.doOutput(); while(u8g2.nextPage());

    blink++;
  if(blink>=100){blink=0;}
  //}
  if(showTemp){timer_1++;if(timer_1>101){timer_1=0;showTemp=false;}}
  else{timer_1=0;showTemp=false;}
  
  //delay(10);//simulate other tasks delay
}