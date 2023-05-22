#include <stdlib.h>
#include<LiquidCrystal_I2C.h>
#include<Wire.h>
#include "color_detection.h"
#include "stepper_control.h"

//module compile switches
#define EnableDebuggingLogsMain true

//Variables and data structures
LiquidCrystal_I2C lcd(0x27, 16,2);
byte Reset = 0;
String str;

struct DataRecorderVar
{
  byte LeftContainerCnt = 0x00, MiddleContainerCnt = 0x00, RightContainerCnt = 0x00;
  byte BallThrowsCnt = 0x00;
};
DataRecorderVar DataRecorder;

StepperCscDataVar StepperCscData;
StepperThDataVar StepperThData;

//Functions
void PinSetup() {
  pinMode(StepCS, OUTPUT);
  pinMode(DirCS , OUTPUT);
  pinMode(StepTH, OUTPUT);
  pinMode(DirTH , OUTPUT);
  pinMode(StepBA, OUTPUT);
  pinMode(DirBA , OUTPUT);
}
void WriteStepperDataToDataRec(StepperCscDataVar *StepperCSC, StepperThDataVar *StepperTh, DataRecorderVar *DataRec)
{
  DataRec->LeftContainerCnt = StepperCSC->LeftContainerCnt;
  DataRec->MiddleContainerCnt = StepperCSC->MiddleContainerCnt;
  DataRec->RightContainerCnt = StepperCSC->RightContainerCnt;
  DataRec->BallThrowsCnt = StepperTh->BallThrowsCnt;
}
void printDataRecToSerial(){
  Serial.println("Data Recorder Data:");
  Serial.print("LeftContainerCnt:");
  Serial.print(DataRecorder.LeftContainerCnt);
  Serial.print(" MiddleContainerCnt:");
  Serial.print(DataRecorder.MiddleContainerCnt);
  Serial.print(" RightContainerCnt:");
  Serial.print(DataRecorder.RightContainerCnt);
  Serial.print(" BallThrowsCnt:");
  Serial.println(DataRecorder.BallThrowsCnt);
  Serial.println();
}
void resetDataRec(){
  DataRecorder.LeftContainerCnt=0;
  DataRecorder.MiddleContainerCnt=0;
  DataRecorder.RightContainerCnt=0;
  DataRecorder.BallThrowsCnt=0;
}

void printDataRecToLCD(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Nr. colrs sorted");
    lcd.setCursor(0,1);
    str = "R:" + String(DataRecorder.LeftContainerCnt) + " Y:"+String(DataRecorder.MiddleContainerCnt)+ " B:"+String(DataRecorder.RightContainerCnt)+" T:"+String(DataRecorder.BallThrowsCnt);
    lcd.print(str);
}
void lcd_scrolling_display( char *str_buffer_row0,char *str_buffer_row1){
  int offset0,offset1;
  char lcd_row[16];
  for(offset0=0,offset1=0;offset0<strlen(str_buffer_row0),offset1<strlen(str_buffer_row1);offset0++,offset1++){
strncpy(lcd_row , str_buffer_row0+offset0, 16);
lcd.setCursor(0,0);
lcd.print(lcd_row);
strncpy(lcd_row , str_buffer_row1+offset1, 16);
lcd.setCursor(0,1);
lcd.print(lcd_row);
delay(100);
lcd.clear();
delay(100);
  }
}
void setup() {
  Serial.begin(115000);
  PinSetup();
  lcd.init();
  lcd.backlight();
  //Initial display message
  lcd_scrolling_display("     Ball Color Sorting System              ","     By Kafka Patrik, year 3 UPT CTI RO 2023");
  lcd.setCursor(0,0);
  lcd.print("Waiting for move");
  lcd.setCursor(0,1);
  lcd.print("ment....");
  setup_color_detect(5);//10 before, delay 5 miliseconds between each photodiode array read from color sensor
  StepperCscContainerWrite(&StepperCscData, ColorRed, ColorYellow, ColorBlue); //Default value
  StepperCscData.CurrentStepperPos = StepperCscData.MiddleContainerPos;//initial position
  delay(5000);
}

void loop() {
  StepperBaRun();
  color_detect_routine();
  StepperThRun(BallColorOut, &StepperCscData, &StepperThData); 
  StepperCsRun(BallColorOut,&StepperCscData, &StepperThData); 
  WriteStepperDataToDataRec(&StepperCscData, &StepperThData, &DataRecorder);
  //printDataRecToSerial();
  printDataRecToLCD();
  delay(500);
}
