#include <Arduino.h>

#define MAX_DEPTH 3

#define  language_RU 
//#define language_EU

#ifdef language_EU
  #define text_1    "Settings"
  #define text_2    "Auto Light: "
  #define text_3    "Auto Fan: "
  #define text_4    "Temp: "
  #define text_5    "???"
  #define text_6    "Wireless: "
  #define text_7    "Perf: "
  #define text_8    "On"
  #define text_9    "Off"
  #define text_10   "Display Brt"
  #define text_11   "<Back"
  #define text_12   "Time & Date"
  #define text_13   "Time"
  #define text_14   "Date"
  #define text_15   "Fan control"
  #define text_16   "Light control"
  #define text_17   "Max spd"
  #define text_18   "Max brt"
  #define text_19   "Eco"
  #define text_20   "Balanced"
  #define text_21   "High"
  #define text_22   "Celsius"
  #define text_23   "Fahrenheit"
#endif

#ifdef language_RU
  #define text_1    "Настройки"
  #define text_2    "Авто: "
  #define text_3    "Авто: "
  #define text_4    "Шкала: "
  #define text_5    "???"
  #define text_6    "Сеть: "
  #define text_7    "Режим: "
  #define text_8    "Вкл"
  #define text_9    "Выкл"
  #define text_10   "Яркость"
  #define text_11   "<Назад"
  #define text_12   "Время & Дата"
  #define text_13   "Время"
  #define text_14   "Дата"
  #define text_15   "Верниляторы" 
  #define text_16   "Подсветка"
  #define text_17   "Обороты"
  #define text_18   "Яркость"
  #define text_19   "Эко"
  #define text_20   "Средний"
  #define text_21   "Высокий"
  #define text_22   "Цельсия"
  #define text_23   "Фаренгейта"
#endif

uint16_t hrs=12;
uint16_t mins=0;

uint16_t year=2020;
uint16_t month=10;
uint16_t day=15;

bool LightCtrl=HIGH;
bool FanCtrl=HIGH;
bool Temp_mode=HIGH;
bool Silence=LOW;
bool mainScreenOn = true;
bool showTemp = false;

uint8_t Wireless=0;
uint8_t PERF=1;
uint8_t BRT_max = 80;
uint8_t SPD_max = 80;
uint8_t BRT_Disp = 20;
double setted_temp = 16.0;
uint8_t timer_1 = 0;