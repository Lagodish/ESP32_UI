#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include <menu.h>
#include <menuIO/u8g2Out.h>
// #include <menuIO/encoderIn.h>
// #include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>

using namespace Menu;

#define LEDPIN GPIO_NUM_0

  #define fontName u8g2_font_7x13_mf
  #define fontX 7
  #define fontY 16
  #define offsetX 0
  #define offsetY 0
  #define U8_Width 128
  #define U8_Height 64
  #define USE_HWI2C

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2,22,21,U8X8_PIN_NONE);

typedef u8g2_uint_t u8g_uint_t;

const colorDef<uint8_t> colors[6] MEMMODE={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};

result doAlert(eventMask e, prompt &item);


bool LightCtrl=HIGH;
TOGGLE(LightCtrl,setLight,"Auto Light: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);

bool FanCtrl=HIGH;
TOGGLE(FanCtrl,setFan,"Auto Fan Ctrl: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);

//TempMenu
bool Temp_mode=HIGH;
TOGGLE(Temp_mode,TempMenu,"Temperature: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("Celsius",HIGH,doNothing,noEvent)
  ,VALUE("Fahrenheit",LOW,doNothing,noEvent)
);

bool Silence=LOW;
TOGGLE(Silence,setSilence,"Silence Mode: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);

uint8_t Lang=1;
SELECT(Lang,LangueMenu,"Langue",doNothing,noEvent,noStyle
  ,VALUE("Ru",0,doNothing,noEvent)
  ,VALUE("En",1,doNothing,noEvent)
  ,VALUE("Ge",2,doNothing,noEvent)
);

uint8_t PERF=1;
SELECT(PERF,PerformanceMenu,"Perf",doNothing,noEvent,noStyle
  ,VALUE("Eco",0,doNothing,noEvent)
  ,VALUE("Balanced",1,doNothing,noEvent)
  ,VALUE("High",2,doNothing,noEvent)
);


uint8_t BRT = 80;
MENU(LightMenu,"Light",doNothing,noEvent,noStyle
  ,SUBMENU(setLight)
  ,FIELD(BRT,"Max brightness","%",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,EXIT("<Back")
);

uint8_t SPD = 80;
MENU(FanMenu,"Fan",doNothing,noEvent,noStyle
  ,SUBMENU(setFan)
  ,FIELD(SPD,"Max speed","%",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,EXIT("<Back")
);

uint16_t hrs=18;
uint16_t mins=30;
altMENU(menu,timeMenu,"Time",doNothing,noEvent,noStyle,(systemStyles)(_asPad|Menu::_menuData|Menu::_canNav|_parentDraw)
  ,FIELD(hrs,"","",0,23,1,0,doNothing,noEvent,wrapStyle)
  ,FIELD(mins,"","",0,59,1,0,doNothing,noEvent,wrapStyle)
);

uint8_t BRT_Disp = 20;
MENU(mainMenu,"Settings",doNothing,noEvent,wrapStyle
  ,SUBMENU(setSilence)
  ,SUBMENU(timeMenu)
  ,SUBMENU(LangueMenu)
  ,SUBMENU(LightMenu)
  ,SUBMENU(FanMenu)
  ,SUBMENU(TempMenu)
  ,SUBMENU(PerformanceMenu) 
  ,FIELD(BRT_Disp,"Disp Brt","%",0,100,10,1,doNothing,noEvent,wrapStyle) 
  ,OP("System test",doAlert,enterEvent)
  ,EXIT("Close")
);

#define MAX_DEPTH 2

serialIn serial(Serial);
MENU_INPUTS(in,&serial);


MENU_OUTPUTS(out,MAX_DEPTH
  ,U8G2_OUT(u8g2,colors,fontX,fontY,offsetX,offsetY,{0,0,U8_Width/fontX,U8_Height/fontY})
  ,SERIAL_OUT(Serial)
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

result alert(menuOut& o,idleEvent e) {
  if (e==idling) {
    o.setCursor(0,0);
    o.print("System test - OK!");
    o.setCursor(0,1);
    o.print("press [select]");
    o.setCursor(0,2);
    o.print("to continue...");
  }
  return proceed;
}

result doAlert(eventMask e, prompt &item) {
  nav.idleOn(alert);
  return proceed;
}
double temp = 9.0;
double tempPrint =0.0;
//when menu is suspended
result idle(menuOut& o,idleEvent e) {
  o.clear();
  const char DEGREE_SYMBOL[] = { 0xB0, '\0' };
  const char ALERT_SYMBOL[] = { 71, '\0' }; //DOTO как в Aplle
  const char bluetooth_SYMBOL[] = { 74, '\0' };
  const char wifi_SYMBOL[] = { 80, '\0' }; //Заменить значок (не вытянутый)
  const char setup_SYMBOL[] = { 66, '\0' };
  if(temp>11){temp=9;}
  tempPrint = temp;
  switch(e) {
    case idleStart:/*o.println("suspending menu!")*/;break;
    case idling:{
      
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
    if(Temp_mode){
    o.print("C");
    }else{o.print("F");}
    u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t); //{Eeeeeeeeeeeeeeeee}
    u8g2.drawUTF8(8, 64, ALERT_SYMBOL);
    u8g2.drawUTF8(40, 64, bluetooth_SYMBOL);
    u8g2.drawUTF8(72, 64, wifi_SYMBOL);
    u8g2.drawUTF8(106, 64, setup_SYMBOL);
    break;}
    case idleEnd:/*o.println("resuming menu.");*/u8g2.setFont(fontName);break;
  }

  return proceed;
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("");Serial.flush();
  Serial.println("menu 4.x test");Serial.flush();
  // encButton.begin();
  // encoder.begin();

  Wire.begin();
  u8g2.begin();

  u8g2.setFont(fontName);
  // u8g2.setBitmapMode(0);

  // disable second option
  //mainMenu[1].enabled=disabledStatus;
  nav.idleTask=idle;//point a function to be used when menu is suspended
  nav.timeOut=30;
  nav.idleOn(idle);
  Serial.println("Use keys + - * /");Serial.flush();
  Serial.println("to control the menu navigation");Serial.flush();
  Serial.println("setup done.");Serial.flush();
}

uint8_t refresh = 0;
void loop() {
  nav.doInput();
  if (nav.changed(0)||(refresh>3)) {//only draw if menu changed for gfx device
    //change checking leaves more time for other tasks
    refresh=0;
    temp+=0.1;
    int contrast = map(BRT_Disp, 0, 100, 0, 190);
    u8g2.setContrast(contrast);
    u8g2.firstPage();
    do nav.doOutput(); while(u8g2.nextPage());
  }
  else{refresh++;}

  delay(100);//simulate other tasks delay
}