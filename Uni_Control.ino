#define max_he_temp 245 
#define max_hb_temp  120
#define times_to_turn_off  10

#include <avr/wdt.h>
#define CLK 2
#define DIO 3
#include "GyverTM1637.h"
GyverTM1637 disp(CLK, DIO);

int display_info;
int display_info1;
int alarm_he;
int alarm_hb;

void setup(){

    // реле
    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);

  // Терморезисторы
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);

  // инициализируем дисплей
  disp.clear();
  disp.brightness(7);  // яркость, 0 - 7 (минимум - максимум)

  //disp.displayInt(0);
  //delay(2000);

  //disp.displayInt(123);
  //delay(2000);
  //digitalWrite(16, LOW);
  
  // wdt
    wdt_enable(WDTO_2S);

  // обнуляем счетчики
    display_info = 1;
    display_info1 = 0;    
    alarm_he = 0;
    alarm_hb = 0;
        
    //Serial.begin(9600);
    //Serial.println("Start");
}
void loop(){
    // Сброс таймера watchdog
    wdt_reset();

    // измерения
    int t1 = analogRead(A1);   // Корпус
    int t2 = analogRead(A2);   // хотэенд
    int t3 = analogRead(A3);   // стол
    //Serial.print(v);
    //Serial.print("t");
    //Serial.println("A1- "+String(analog2temp1(t1))+ "   A2- "+String(analog2temp2(t2))  +"   A3- "+String(analog2temp3(t3))   +"   A1- "+String(analog2tempBed(t3))   );
    //Serial.println("A1- "+String(t1)+ "   A2- "+String(t2)  +"   A3- "+String(t3));

    

    // отображение температуры
    if (display_info==1)
      {
          disp.clear();
          disp.displayByte(0x00, 0x76, 0x79, 0x00);
      }
    else if (display_info==2)
      {
          disp.clear();
          disp.displayInt(analog2temp2(t2));
      }      
    else if (display_info==3)
      {
          disp.clear();
          disp.displayByte(0x00, 0x76, 0x7f, 0x00);
      }
    else if (display_info==4)
      {
          disp.clear();
          disp.displayInt(analog2temp3(t3));
      }      
    else if (display_info==5)
      {
          disp.clear();
          disp.displayByte(0x00, 0x73, 0x6d, 0x00);
      }
    else if (display_info==6)
      {
          disp.clear();
          disp.displayInt(analog2temp1(t1));
      }          

    // Меняем показатель раз в 3 сек
    if (display_info1<5)
      display_info1=display_info1+1;
    else 
      display_info1=0;

    if (display_info1==0)
      if (display_info<6)
        display_info=display_info+1;
      else
        display_info=1;


    //  проверяем температуры
    if (analog2temp2(t2)>max_he_temp)
      alarm_he=alarm_he+1;
    else
      alarm_he = 0;

    if (analog2temp3(t3)>max_hb_temp)
      alarm_hb=alarm_hb+1;
    else
      alarm_hb = 0;

    // Выключаем при аварии
    if (alarm_he>times_to_turn_off)
      digitalWrite(16, LOW);
      
    if (alarm_hb>times_to_turn_off)
      digitalWrite(16, LOW);
   
    delay(500);
}

// Корпус
const short temptable_1[][2] PROGMEM = {
   {1, 841},
   {54, 255},
   {84, 235},      
   {89, 230},      
   {106, 220},   
   {124, 210},   
   {145, 200},
   {176, 190},
   {451, 130},
   {480, 115},
   {520, 110},
   {595, 100},
   {669, 90},
   {800, 70},
   {914, 50},
   {968, 30},
   {1008, 3}
};

// хотэенд
const short temptable_2[][2] PROGMEM = {
   {1, 841},
   {54, 255},
   {84, 235},      
   {89, 230},      
   {106, 220},   
   {124, 210},   
   {145, 200},
   {176, 190},
   {239, 170},
   {334, 150},
   {451, 130},
   {582, 110},
   {715, 90},
   {829, 70},
   {914, 50},
   {968, 30},
   {1008, 3}
};

// стол
const short temptable_3[][2] PROGMEM = {
   {1, 841},
   {54, 255},
   {84, 235},      
   {89, 230},      
   {106, 220},   
   {124, 210},   
   {145, 200},
   {176, 190},
   {451, 130},
   {480, 115},
   {520, 110},
   {595, 100},
   {669, 90},
   {800, 70},
   {914, 50},
   {968, 30},
   {1008, 3}
};


# define BEDTEMPTABLE_LEN1 (sizeof(temptable_1)/sizeof(*temptable_1))
# define BEDTEMPTABLE_LEN2 (sizeof(temptable_2)/sizeof(*temptable_2))
# define BEDTEMPTABLE_LEN3 (sizeof(temptable_3)/sizeof(*temptable_3))
#define PGM_RD_W(x)   (short)pgm_read_word(&x)


static float analog2temp1(int raw) {
    float celsius = 0;
    byte i;
 
    for (i = 1; i < BEDTEMPTABLE_LEN1; i++)
    {
        if (PGM_RD_W(temptable_1[i][0]) > raw)
        {
            celsius = PGM_RD_W(temptable_1[i - 1][1]) +
                (raw - PGM_RD_W(temptable_1[i - 1][0])) *
                (float)(PGM_RD_W(temptable_1[i][1]) - PGM_RD_W(temptable_1[i - 1][1])) /
                (float)(PGM_RD_W(temptable_1[i][0]) - PGM_RD_W(temptable_1[i - 1][0]));
            break;
        }
    }
 
    // Overflow: Set to last value in the table
    if (i == BEDTEMPTABLE_LEN1) celsius = PGM_RD_W(temptable_1[i - 1][1]);
 
    return celsius;
}

static float analog2temp2(int raw) {
    float celsius = 0;
    byte i;
 
    for (i = 1; i < BEDTEMPTABLE_LEN2; i++)
    {
        if (PGM_RD_W(temptable_2[i][0]) > raw)
        {
            celsius = PGM_RD_W(temptable_2[i - 1][1]) +
                (raw - PGM_RD_W(temptable_2[i - 1][0])) *
                (float)(PGM_RD_W(temptable_2[i][1]) - PGM_RD_W(temptable_2[i - 1][1])) /
                (float)(PGM_RD_W(temptable_2[i][0]) - PGM_RD_W(temptable_2[i - 1][0]));
            break;
        }
    }
 
    // Overflow: Set to last value in the table
    if (i == BEDTEMPTABLE_LEN2) celsius = PGM_RD_W(temptable_2[i - 1][1]);
 
    return celsius;
}


static float analog2temp3(int raw) {
    float celsius = 0;
    byte i;
 
    for (i = 1; i < BEDTEMPTABLE_LEN3; i++)
    {
        if (PGM_RD_W(temptable_3[i][0]) > raw)
        {
            celsius = PGM_RD_W(temptable_3[i - 1][1]) +
                (raw - PGM_RD_W(temptable_3[i - 1][0])) *
                (float)(PGM_RD_W(temptable_3[i][1]) - PGM_RD_W(temptable_3[i - 1][1])) /
                (float)(PGM_RD_W(temptable_3[i][0]) - PGM_RD_W(temptable_3[i - 1][0]));
            break;
        }
    }
 
    // Overflow: Set to last value in the table
    if (i == BEDTEMPTABLE_LEN3) celsius = PGM_RD_W(temptable_3[i - 1][1]);
 
    return celsius;
}



/*

void ledDigitDisplay(int value)
  {

    String qq = String("0000")+String(value);
    //Serial.println(qq);
    qq=qq.substring(qq.length() - 4);
    //Serial.println(qq);
    //Serial.print("aaa ");
    //Serial.println(qq.substring(3,4));
    
    tm1637.display( 0, qq.substring(0,1).toInt() );
    tm1637.display( 1, qq.substring(1,2).toInt() );
    tm1637.display( 2, qq.substring(2,3).toInt() );
    tm1637.display( 3, qq.substring(3,4).toInt() );
  }

*/  
