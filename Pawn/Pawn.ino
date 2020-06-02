
#define teal makeColorHSB(100,255,255)

#define CHOOSE_COLOR_DURATION 500
#define LAP_DURATION 100
#define DEAD_DURATION 300

//bits [A][B][C][D][E][F]
enum signalStates {INERT, RESET, RESOLVE};
byte signalState = INERT;
byte color=0;
Color colors[6]={BLUE,teal,YELLOW,MAGENTA,GREEN,RED};
byte randomColor;
byte displayFace=0;
byte deadBrightness;
byte BLINKS=6;
Timer chooseColorTimer;
Timer spinTimer;
Timer deadTimer;
bool isDead=false;
bool isDecrease=false;



void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  switch (signalState) {
    case INERT:
      inertLoop();
      break;
    case RESET:
      resetLoop();
      break;
    case RESOLVE:
      resolveLoop();
      break;
  }

 
  if(!chooseColorTimer.isExpired()){
    chooseColor();
  }else if(isDead){
   deadDisplay();
  }else{
   displayColor();
  }
 
  byte sendData= (signalState << 4) + (color);
  setValueSentOnAllFaces(sendData);
}

//all the gameplay happens here
void inertLoop() {
  
  //set myself to RESET
  if (buttonMultiClicked()) {
    byte clicks=buttonClickCount();
    if(clicks>=3){
      color=clicks+6; //everything shifted so 9=3 options
      byte sendData= (signalState << 4) + (color);
      setValueSentOnAllFaces(sendData);
      BLINKS=clicks;
      signalState=RESET;
    }
  }

  if(buttonDoubleClicked()){
    if(isAlone()){
      chooseColorTimer.set(CHOOSE_COLOR_DURATION);
      randomColor=(random(BLINKS-1)+millis())%BLINKS; //a random number determined by how many blinks you have
      color=randomColor;
      if(color==2){
        isDead=true;
      }
    }
  }

  
  
  

  //listen for neighbors in RESET
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == RESET) {//a neighbor saying RESET!
        signalState = RESET;
      }
    }
  }
}

void resetLoop() {
  signalState = RESOLVE;//I default to this at the start of the loop. Only if I see a problem does this not happen

  
 
  isDead=false;

  FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f)){
      if(getColor(getLastValueReceivedOnFace(f))>5){
        BLINKS=getColor(getLastValueReceivedOnFace(f))-6;
      }
    }
  }
  

  //look for neighbors who have not heard the RESET news
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == INERT) {//This neighbor doesn't know it's RESET time. Stay in RESET
        signalState = RESET;
      }
    }
  }
}

void resolveLoop() {
  signalState = INERT;//I default to this at the start of the loop. Only if I see a problem does this not happen

  //look for neighbors who have not moved to RESOLVE
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == RESET) {//This neighbor isn't in RESOLVE. Stay in RESOLVE
        signalState = RESOLVE;
      }
    }
  }
}

//random animation
void chooseColor(){
  FOREACH_FACE(f){
    setColorOnFace(makeColorHSB(43, 255, random(155) + 100), f);
  }
}

//for when a piece dies
void deadDisplay(){
  if(deadTimer.isExpired()){
    displayFace++;
    deadTimer.set(DEAD_DURATION);
  }
  setColor(dim(YELLOW,130));
  setColorOnFace(YELLOW,displayFace%6);
  setColorOnFace(YELLOW,(displayFace+3)%6);
}



//displays current color
void displayColor(){
  if(isAlone()){
    if(deadTimer.isExpired()){
    if(!isDecrease){
      deadBrightness+=10;
    }else{
      deadBrightness-=10;
    }
    if(deadBrightness>240){
      deadBrightness=240;
      isDecrease=true;
    }else if(deadBrightness<150){
      deadBrightness=140;
      isDecrease=false;
    }
    deadTimer.set(LAP_DURATION);
  }
  setColor(dim(colors[color],deadBrightness));
  }else{
    setColor(colors[color]);
  }
}

//team at [A], signalState at [C][D], blinkMode at [E][F]
byte getSignalState(byte data) {
  return (data>>4);
}

byte getColor(byte data) {
  return (data & 15);
}
