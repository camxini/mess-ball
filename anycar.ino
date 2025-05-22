#define ENA 6
#define ENB 10 //ENB 3
#define IN1 4 
#define IN2 5
#define IN3 9 //IN3 2
#define IN4 7
#define RIN1 11
#define RIN2 3 //RIN2 10
#define RIN3 2 //RIN3 9
#define RIN4 8

#define DEFSPDA 150
#define DEFSPDB 150
#define ADPSPD 50

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <string.h>

#include<Wire.h>

#include <SoftwareSerial.h>  
// 设置Arduino软件串口，12-RX,13-TX     
// Pin12为RX，接HC05的TXD 
// Pin13为TX，接HC05的RXD 
SoftwareSerial BT(12, 13);  

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, A5, A4);

int pin_r[4] = {RIN1,RIN2,RIN3,RIN4};
int sens[4] = {0,0,0,0};
int sens_r[4] = {-3,-1,1,3};
int pos_rev[2] = {0,0};//left right wheel with 0 pos 1 neg
int time = 0;

char val;  
char a = 'B';
char anomoly[7][100] = {"A sent\n","B sent\n","C sent\n","D sent\n","Stop triggered\n","L triggered\n","R triggered\n"};
int anomoly_number = -1;

int SpdA = 0,SpdB = 0,Spd;

void setup() {
  // put your setup code here, to run once:
  pinMode(ENA,OUTPUT);
  pinMode(ENB,OUTPUT);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA,100);
  analogWrite(ENB,100);

  u8g2.begin(); 

  Serial.begin(9600); 
   //初始化Arduino串口，波特率自定，这里选38400   
   Serial.println("BT is ready!");   
   //测试与PC之间串口是否正常，正常则显示上述文字，异常则显示乱码  
   BT.begin(9600);
   // HC-05的AT模式默认通信波特率为38400    
   //pinMode(13,OUTPUT);   
   //pinMode(8,INPUT);
   //用来使能HC-05并读取HC-05状态，这里没用到

  attachInterrupt(1,B_avoid,CHANGE);
  attachInterrupt(0,B_avoid,CHANGE);
}

void read_sens()
{
  int i = 0;
  for(i=0;i<4;i++){
    sens[i]=digitalRead(pin_r[i]);
  }

  /*double error = 0.0;
  for(i=0;i<4;i++){
    error += sens_r[i]*sens[i];
  }
  error /= 4;
  return error;*/
}

void B_avoid()
{

  pos_rev[0] = 1;
  pos_rev[1] = 1;

  SpdA = 255;
  SpdB = 255;

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA,SpdA);
  analogWrite(ENB,SpdB);
  delay(40);

  anomoly_number = -1;

  a = 'B';

}

void loop() {
  // put your main code here, to run repeatedly:
  /*double error = 0;
  error = read_sens();*/ //如有巡线可魔改此
  read_sens();

  /*wheel turn direction*/
  if ( pos_rev[1] == 0 )
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }else
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }

  if ( pos_rev[0] == 0 )
  {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }else
  {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  /*end that*/

  if( sens[1] == 0 || sens[2] == 0 )
  {
    a = 'B';
    anomoly_number = 4;
  }else if(sens[0] == 0 && (a == 'A' || a == 'C' || a == 'D') )
  {
    a = 'L';
    anomoly_number = 5;
  }else if(sens[3] == 0 && (a == 'A' || a == 'C' || a == 'D') )
  {
    a = 'R';
    anomoly_number = 6;
  }

  SpdA = constrain(SpdA,0,255);
  SpdB = constrain(SpdB,0,255);

  Spd = ( SpdA + SpdB ) / 2;

  if (Serial.available()) {     
     val = Serial.read(); 
     if (val != '\0' && val != '\n' && val != -1 && val != '\r') { // 仅当incomingVal不为空字符时更新val
      a = val;
      }    
     BT.print(val);
     //将PC发来的数据存在val内，并发送给HC-05模块   
   }
   if( anomoly_number != -1 )
   {
      BT.print(anomoly[anomoly_number]);
      anomoly_number = -1;
   }   
  if (BT.available()) {     
     val = BT.read(); 
      if (val != '\0' && val != '\n' && val != -1 && val != '\r') { // 仅当incomingVal不为空字符时更新val
      a = val;

      if( a == 'A' )
      {
        anomoly_number = 0;
      }else if( a == 'B' )
      {
        anomoly_number = 1;
      }else if( a == 'C' )
      {
        anomoly_number = 2;
      }else if( a == 'D' )
      {
        anomoly_number = 3;
      }
      }  
     Serial.print(val); 
     //将HC-05模块发来的数据存在val内，并发送给PC   
   }   

  u8g2.firstPage();
  do{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(0,15);
    u8g2.print("Speed:");
    u8g2.print(Spd);

    u8g2.setCursor(0,35);
    u8g2.print("BTstatus:");
    u8g2.print(a);

    u8g2.sendBuffer();
  } while(u8g2.nextPage());

  if ( a == 'A' )
  {
    time = 0;

    pos_rev[0] = 0;
    pos_rev[1] = 0;

    SpdA = DEFSPDA /*+ error * ADPSPD;*/;
    SpdB = DEFSPDB /*+ error * ADPSPD;*/;
  }else if( a == 'B' )
  {
    time = 0;
    
    pos_rev[0] = 0;
    pos_rev[1] = 0;

    SpdA = 0;
    SpdB = 0;
  }else if( a == 'L' )
  {
    pos_rev[0] = 1;
    pos_rev[1] = 0;
    time++;
    SpdA = DEFSPDA;
    SpdB = DEFSPDB;
    if( time == 8 )
    {
      a = 'B';
      time = 0;
    }
  }else if( a == 'R' )
  {
    pos_rev[0] = 0;
    pos_rev[1] = 1;
    time++;
    SpdA = DEFSPDA;
    SpdB = DEFSPDB;
    if( time == 8 )
    {
      a = 'B';
      time = 0;
    }
  }else if( a == 'C' )
  {
    pos_rev[0] = 1;
    pos_rev[1] = 0;
    time = 0;
    SpdA = DEFSPDA;
    SpdB = DEFSPDB;
  }else if( a == 'D' )
  {
    pos_rev[0] = 0;
    pos_rev[1] = 1;
    time = 0;
    SpdA = DEFSPDA;
    SpdB = DEFSPDB;
  }

  analogWrite(ENA,SpdA);
  analogWrite(ENB,SpdB);

}
