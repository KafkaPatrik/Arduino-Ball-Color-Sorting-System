#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#define EnableDebuggingLogsStepper true

//cnc shield v3 setup
//arduino pins corresponding to shield pins
const byte StepCS = 2;//x axis
const byte DirCS = 5;
const byte StepTH = 3;//y axis
const byte DirTH = 6;
const byte StepBA = 4;//z axis
const byte DirBA = 7;

//Variables and data structures
const byte DirClockwise = HIGH, DirCounterClockwise = LOW;

struct StepperCscDataVar
{
  const byte LeftContainerPos = 0x00, MiddleContainerPos = 0x01, RightContainerPos = 0x02;
  byte LeftContainerColor = ColorRed, MiddleContainerColor = ColorGreen, RightContainerColor = ColorBlue; //default setup
  byte LeftContainerCnt = 0x00, MiddleContainerCnt = 0x00, RightContainerCnt = 0x00;
  byte CurrentStepperPos = MiddleContainerPos; //Stepper is in 'home' position, pointing the ramp to the middle container
};

struct StepperThDataVar
{
  byte BallThrowsCnt = 0x00;
  bool ThrowRequestActive = false;
};

void MoveStepper(byte StepperPin, int StepsNumber, int StepDelay, byte DirectionPin, byte Direction)
{
  digitalWrite(DirectionPin, Direction);
  StepDelay = (StepDelay < 300) ? 300 : StepDelay; // minimum step delay >=300 to avoid overstepping
  for (int steps = 0; steps < StepsNumber; steps++)
  {
    digitalWrite(StepperPin, HIGH);
    delayMicroseconds(StepDelay);
    digitalWrite(StepperPin, LOW);
    delayMicroseconds(StepDelay);
  }
}
//After using this function, please use resetDataRec() function to clear the data recorder
void StepperCscContainerWrite(StepperCscDataVar *StepperCSC, byte ContainerLeftColor, byte ContainerMiddleColor, byte ContainerRightColor) {
  StepperCSC->LeftContainerColor  = ContainerLeftColor;
  StepperCSC->MiddleContainerColor = ContainerMiddleColor;
  StepperCSC->RightContainerColor = ContainerRightColor;
}
/*StepperColorSortingControl
  Function to control the the stepper responsible for directing the ball to the required color
*/
void StepperCsMoveHome(StepperCscDataVar *StepperCSC, int StepDelayHome, int StepNo)
{
  if (StepperCSC->CurrentStepperPos != StepperCSC->MiddleContainerPos)
  {
    if (StepperCSC->CurrentStepperPos == StepperCSC->LeftContainerPos)
    {
      MoveStepper(StepCS, StepNo, StepDelayHome, DirCS, DirCounterClockwise);
    } else if (StepperCSC->CurrentStepperPos == StepperCSC->RightContainerPos)
    {
      MoveStepper(StepCS, StepNo, StepDelayHome, DirCS, DirClockwise);
    }
    StepperCSC->CurrentStepperPos = StepperCSC->MiddleContainerPos;
  }
}
void StepperCsControl(byte BallColor, StepperCscDataVar *StepperCSC, int StepNoCs, int StepDelayCs, int PositionHoldDelay)
{
  //StepNo = 25; full step mode corresponding number of steps for 45 degree rotation of the stepper
  //StepDelay=500; // Very high speed is not needed, 500 microseconds delay between steps is enough

  if (StepperCSC->CurrentStepperPos == StepperCSC->MiddleContainerPos)
  {
    if (BallColor == StepperCSC->MiddleContainerColor)
    {
      StepperCSC->MiddleContainerCnt++;
    } else if (BallColor == StepperCSC->LeftContainerColor)
    {
      StepperCSC->CurrentStepperPos = StepperCSC->LeftContainerPos;
      MoveStepper(StepCS, StepNoCs, StepDelayCs, DirCS, DirClockwise);
      StepperCSC->LeftContainerCnt++;
    } else if (BallColor == StepperCSC->RightContainerColor)
    {
      StepperCSC->CurrentStepperPos = StepperCSC->RightContainerPos;
      MoveStepper(StepCS, StepNoCs, StepDelayCs, DirCS, DirCounterClockwise);
      StepperCSC->RightContainerCnt++;
    }
    Serial.print("Color sorted to container: ");
    Serial.println(BallColor);
  }
  delay(PositionHoldDelay);
  StepperCsMoveHome(StepperCSC, StepDelayCs, StepNoCs); //Always moving the stepper in the middle position for simpler logic
}
/*StepperThrowControl
  Function to control the the stepper responsible for throwing the balls with the wrong colors off the ramp
*/
void StepperThControl(int ThrowDelay, int ReturnDelay, int StepNoTH, int StepDelayTh)
{
  delay(ThrowDelay);/*the ramp incline is constant therefore we asume a constant delay from when the ball is released
  to the moment the throw comand is activated*/
  MoveStepper(StepTH, StepNoTH, StepDelayTh, DirTH, DirClockwise);
  delay(ReturnDelay);//delay for the motor to return to it's initial positon
  MoveStepper(StepTH, StepNoTH, StepDelayTh, DirTH, DirCounterClockwise );
  Serial.println("Th actuator activated");
}
/*StepperBallActuationControl
  Function to control the the stepper responsible for releasing the balls from the funnel to the ramp
*/
void StepperBaControl(int StepDelayBa, int StepNoBa) {
  MoveStepper(StepBA, StepNoBa, StepDelayBa, DirBA, DirCounterClockwise );
}
void StepperCsRun(int color,StepperCscDataVar *StepperCSC,StepperThDataVar *StepperTHC)
{
  if (StepperTHC->ThrowRequestActive == false){
   StepperCsControl(color,StepperCSC, 20, 1400, 4000); //(byte BallColor, StepperCscDataVar StepperCSC, byte StepNoCs, int StepDelayCs, int PositionHoldDelay)
  }
  else{
    StepperTHC->ThrowRequestActive = false;
  }
}
void StepperThRun(byte ball_color, StepperCscDataVar *StepperCSC, StepperThDataVar *StepperTHC)
{
  if ((ball_color != StepperCSC->LeftContainerColor && ball_color != StepperCSC->MiddleContainerColor && ball_color != StepperCSC->RightContainerColor )|| ball_color == ColorUndefined)
  {
    StepperTHC->ThrowRequestActive = true;
    StepperThControl(400, 500, 40, 500); //(int ThrowDelay,int ReturnDelay, byte StepNoTH, byte StepDelayTh)
    StepperTHC->BallThrowsCnt++;
  }
}
void StepperBaRun()
{
  StepperBaControl( 1000, 200);//(int StepDelayBa,int StepNoBa)
}
#endif
