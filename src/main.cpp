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

uint8_t BRT_Disp = 30;
MENU(mainMenu,"Settings",doNothing,noEvent,wrapStyle
  ,SUBMENU(setSilence)
  ,SUBMENU(timeMenu)
  ,SUBMENU(LangueMenu)
  ,SUBMENU(LightMenu)
  ,SUBMENU(FanMenu)
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

//when menu is suspended
result idle(menuOut& o,idleEvent e) {
  o.clear();
  const char DEGREE_SYMBOL[] = { 0xB0, '\0' };
  const char ALERT_SYMBOL[] = { 71, '\0' };
  const char bluetooth_SYMBOL[] = { 74, '\0' };
  const char wifi_SYMBOL[] = { 80, '\0' };
  const char setup_SYMBOL[] = { 66, '\0' };
  switch(e) {
    case idleStart:o.println("suspending menu!");break;
    case idling:{
    
 /*   o.setCursor(0,0);
    o.print("Main Screen");
    o.setCursor(0,1);
    o.print("Temp=5.5 C");
    o.setCursor(0,3);
    o.print("Press *->Settings");*/
    //open_iconic_all_2x
    u8g2.setFont(u8g2_font_fub30_tf);
    o.setCursor(2,2); //(x,y 0,0 at top left)
    o.print("5.5 C");
    u8g2.drawUTF8(75, 45, DEGREE_SYMBOL);

    u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);

    u8g2.drawUTF8(8, 60, ALERT_SYMBOL);
    u8g2.drawUTF8(40, 60, bluetooth_SYMBOL);
    u8g2.drawUTF8(72, 60, wifi_SYMBOL);
    u8g2.drawUTF8(106, 60, setup_SYMBOL);
    break;}
    case idleEnd:o.println("resuming menu.");u8g2.setFont(fontName);break;
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

void loop() {
  nav.doInput();
  
  if (nav.changed(0)) {//only draw if menu changed for gfx device
    //change checking leaves more time for other tasks
    u8g2.setContrast(BRT_Disp);
    u8g2.firstPage();
    do nav.doOutput(); while(u8g2.nextPage());
  }

  delay(100);//simulate other tasks delay
}