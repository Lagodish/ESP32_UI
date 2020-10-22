#include <Arduino.h>
#include <Wire.h>
#include <menu.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include <EEPROM.h>
#include <GyverButton.h>
#include <config.h>

GButton butt1(GP1); //GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);
GButton butt2(GP2);
GButton butt3(GP3);
GButton butt4(GP4);

//xSemaphoreCreateMutex

using namespace Menu;

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


bool oldRetCode; 
bool Hysteresis(double temp_now) {

  if(!Temp_mode){temp_now = int((temp_now*9/5)+32);}    

  bool retCode = false;
  if (temp_now > setted_temp+1.0) {
    retCode = true;
  } else if ( temp_now < setted_temp-PERF) {
    retCode = false;
  } else {
    retCode = oldRetCode;
  }
   
  oldRetCode = retCode;
  return retCode;
}

result action4(eventMask e,navNode& nav, prompt &item) {
  EEPROM.write(3,PERF); 
  EEPROM.commit();
  return proceed;
}

result action3(eventMask e,navNode& nav, prompt &item) {
  EEPROM.write(0, BRT_Disp);
  EEPROM.commit();
  return proceed;
}

result action2(eventMask e,navNode& nav, prompt &item) {
  EEPROM.write(5,Temp_mode);
  if(Temp_mode){ // C
    setted_temp = int((setted_temp-32)*5/9);
  }
  else{ //  F
    setted_temp = int((setted_temp*9/5)+32);
  }
  EEPROM.write(4,setted_temp);
  EEPROM.commit();
  return proceed;
}

result action1(eventMask e,navNode& nav, prompt &item) {  
  EEPROM.write(1,BRT_max);
  EEPROM.write(2,SPD_max); 
  EEPROM.write(6,LightCtrl);
  EEPROM.write(7,FanCtrl);
  EEPROM.write(9,Wireless);
  EEPROM.commit();
  return proceed;
}


TOGGLE(LightCtrl,setLight,text_2,action1,enterEvent,noStyle
  ,VALUE(text_8,HIGH,doNothing,noEvent)
  ,VALUE(text_9,LOW,doNothing,noEvent)
);


TOGGLE(FanCtrl,setFan,text_3,action1,enterEvent,noStyle
  ,VALUE(text_8,HIGH,doNothing,noEvent)
  ,VALUE(text_9,LOW,doNothing,noEvent)
);


TOGGLE(Temp_mode,TempMenu,text_4,action2,enterEvent,noStyle
  ,VALUE(text_22,HIGH,doNothing,noEvent)  //C
  ,VALUE(text_23,LOW,doNothing,noEvent)   //F
);

TOGGLE(Wireless,setWireless,text_6,action1,enterEvent,noStyle
  ,VALUE(text_9,0,doNothing,noEvent)
  ,VALUE("WiFi",1,doNothing,noEvent)
  ,VALUE("BLE",2,doNothing,noEvent)
);


TOGGLE(PERF,PerformanceMenu,text_7,action4,enterEvent,noStyle
  ,VALUE(text_19,3,doNothing,noEvent)
  ,VALUE(text_20,2,doNothing,noEvent)
  ,VALUE(text_21,1,doNothing,noEvent)
);

MENU(LightMenu,text_16,doNothing,noEvent,noStyle
  ,SUBMENU(setLight)
  ,FIELD(BRT_max,text_18,"%",0,100,10,0,action1,enterEvent,wrapStyle)
  ,EXIT(text_11)
);

MENU(FanMenu,text_15,doNothing,noEvent,noStyle
  ,SUBMENU(setFan)
  ,FIELD(SPD_max,text_17,"%",0,100,10,0,action1,enterEvent,wrapStyle)
  ,EXIT(text_11)
);
 
PADMENU(YMD_Menu,text_14,doNothing,noEvent,noStyle
  ,FIELD(year,"","/",1900,3000,20,1,doNothing,noEvent,noStyle)
  ,FIELD(month,"","/",1,12,1,0,doNothing,noEvent,wrapStyle)
  ,FIELD(day,"","",1,31,1,0,doNothing,noEvent,wrapStyle)
);

PADMENU(HM_Menu,text_13,doNothing,noEvent,noStyle
  ,FIELD(hrs,"",":",0,23,1,0,doNothing,noEvent,wrapStyle)
  ,FIELD(mins,"","",0,59,1,0,doNothing,noEvent,wrapStyle)
);

MENU(timeMenu,text_12,doNothing,noEvent,noStyle
  ,SUBMENU(HM_Menu)
  ,SUBMENU(YMD_Menu)
  ,EXIT(text_11)
);


//TODO работа на нагрев (если в комнате температура меньше чем нужно) + счетчик наработки
MENU(mainMenu, text_1 ,doNothing,noEvent,noStyle
  ,SUBMENU(timeMenu)
  ,SUBMENU(LightMenu)
  ,SUBMENU(FanMenu)
  ,SUBMENU(PerformanceMenu)
  ,SUBMENU(setWireless)
  ,SUBMENU(TempMenu)
  ,FIELD(BRT_Disp,text_10," %",0,100,10,0,action3,enterEvent,noStyle)
  ,EXIT(text_11)
);

serialIn serial(Serial);
MENU_INPUTS(in,&serial);

MENU_OUTPUTS(out,MAX_DEPTH
  ,U8G2_OUT(u8g2,colors,fontX,fontY,offsetX,offsetY,{0,0,U8_Width/fontX,U8_Height/fontY})
  ,SERIAL_OUT(Serial)
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

//when menu is suspended
result MainScreen(menuOut& o,idleEvent e) {
  o.clear();
  const char DEGREE_SYMBOL[] = { 0xB0, '\0' };
  const char ALERT_SYMBOL[] = { 71, '\0' }; //DOTO как в Aplle
  const char bluetooth_SYMBOL[] = { 74, '\0' };
  const char wifi_SYMBOL[] = { 80, '\0' }; //Заменить значок (не вытянутый)
  const char setup_SYMBOL[] = { 66, '\0' };

  if(showTemp){tempPrint = setted_temp;}
  else{
    if(Temp_mode){tempPrint = temp;}
    else{tempPrint = int((temp*9/5)+32);}}
  
  switch(e) {
    case idleStart:/*o.println("suspending menu!")*/;break;
    case idling:{
    mainScreenOn=true;
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

void Test_1( void * parameter)
{ double y=0;
  while (1){
  y=y*0.1/100;
  y++;
  if(y>10000) y=0;
  if(y<-10000) y=0;
  Serial.println(y);
  vTaskDelay(1/portTICK_PERIOD_MS);
}}


void Test_2( void * parameter)
{ float i=0;
  while (1){
  
  i=i*0.1/100;
  i++;
  i=i*6;
  if(i>10000) i=0;
  if(i<-10000) i=0;
  Serial.println(i);
  vTaskDelay(10/portTICK_PERIOD_MS);
}}

void DisplayTask( void * parameter)
{
    Serial.println("Display");

    Wire.begin();
    u8g2.begin();
    u8g2.enableUTF8Print();	
    if (EEPROM.begin(EEPROM_SIZE)){
    delay(10);
    BRT_Disp = EEPROM.read(0);
    BRT_max = EEPROM.read(1);
    SPD_max = EEPROM.read(2);
    PERF = EEPROM.read(3);
    setted_temp = EEPROM.read(4);
    Temp_mode = EEPROM.read(5);
    LightCtrl = EEPROM.read(6);
    FanCtrl = EEPROM.read(7);
    Silence = EEPROM.read(8);
    Wireless = EEPROM.read(9);
    }

  u8g2.setFont(fontName);
  // u8g2.setBitmapMode(0);
  //mainMenu[1].enabled=disabledStatus; //disable second option
  nav.idleTask=MainScreen;//point a function to be used when menu is suspended
  nav.timeOut=30;
  nav.idleOn(MainScreen);

    while(1){
       
    butt1.tick();
    butt2.tick();
    butt3.tick();
    butt4.tick();
    if (butt1.isClick()){butt1_l = true;nav.doNav(enterCmd);Serial.println("enterCmd");}else{butt1_l = false;}
    if (butt2.isClick()){butt2_l = true;nav.doNav(upCmd);Serial.println("upCmd");}else{butt2_l = false;}
    if (butt3.isClick()){butt3_l = true;nav.doNav(downCmd);Serial.println("downCmd");} else{butt3_l = false;}
    if (butt4.isClick()){butt4_l = true;nav.doNav(escCmd);Serial.println("escCmd");} else{butt4_l = false;}
  
    if(mainScreenOn&&(butt2_l||butt2.isStep())){

    if(Temp_mode){
    setted_temp+=0.5;
    if(setted_temp>18){setted_temp=18;}}
    else{
    setted_temp+=1;
    if(setted_temp>64){setted_temp=64;}}

    showTemp=true;
    timer_1=1;}
  if(mainScreenOn&&(butt3_l||butt3.isStep())){

    if(Temp_mode){
    setted_temp-=0.5;
    if(setted_temp<5){setted_temp=5;}}
    else{
    setted_temp-=1;
    if(setted_temp<41){setted_temp=41;}}

    showTemp=true;
    timer_1=1;}

  nav.doInput();
  //if (nav.changed(0)) {
    int contrast = map(BRT_Disp, 0, 100, 0, 190);
    u8g2.setContrast(contrast);
    u8g2.firstPage();
    do nav.doOutput(); while(u8g2.nextPage());

    blink++;
  if(blink>=100){blink=0;}
  //}
  if(showTemp){timer_1++;if(timer_1>101){timer_1=0;showTemp=false;}}
  else{timer_1=0;showTemp=false;}

     vTaskDelay(10/portTICK_PERIOD_MS);
    }

    Serial.println("Ending Display");
    vTaskDelete( NULL );
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  
  xTaskCreatePinnedToCore(
    DisplayTask,             /* Task function. */
    "DisplayTask",           /* String with name of task. */
    8000,            /* Stack size in bytes. */
    NULL,             /* Parameter passed as input of the task */
    2,                /* Priority of the task. */
    NULL,           /* Task handle. */
    0);               /* Core 1 */ 

  xTaskCreate(
                    Test_1,          
                    "Test_1",        
                    8000,            
                    NULL,             
                    1,               
                    NULL);  

  xTaskCreate(
                    Test_2,          
                    "Test_2",        
                    8000,            
                    NULL,             
                    1,               
                    NULL);
}

void loop() {
  vTaskDelete( NULL );
}