/*
 * ardrum - drum sensor input
 * atmega168P extended with a 74hc4067 multiplexer
 * (c) 2011-2038 wotwot
 * do i need a license to code this ?
 * license: cc-by-nc-sa
 * parts (c) MIT licensed and/or others

 * a setup with more than 1 mux will
 * likely also need a rewrite of the muxer function and timing
 * pins on the muxer
*/

#define DRUMZ 16
#define DRUMNAMELENGTH 10

#define RESISTORS 1 // 0 = use builtin pullups for analogue ports
#define LEDPIN     13     // status LED pin
#define HHPIN  9
#define HIHAT 97 // note for the pedal
#define SENSITIVITY 69 // sensitivity potentiometer
#define PIEZOTHRESHOLD    27 // default analog threshold, was 13
#define FMT DEC
// #define PINS 1 // ???
#define MUXERS 1          // amount of 74hc4067
#define MUXCOUNT 16		  // 4067 has 16 i/o ports
#define SPEED 115200 // (31250, 57600, 115200 etc)
#define DEBUG 1
#define MIDICHAN  0xa // chan 10 for drums
// http://www.arduino.cc/en/Reference/AnalogReference
#define ADREF DEFAULT // DEFAULT INTERNAL INTERNAL1V1 INTERNAL2V56 EXTERNAL
// pulldowns now 22k

char LeDrumz[DRUMZ][DRUMNAMELENGTH];
/*
void pullemup();
// void initialize_da_crap(); // done in setup()
*/
int sensorMax, hatstate, val, i, j, k, index;
int sensornoise[MUXERS * MUXCOUNT + 1];

//Mux control pins
int s0 = 5;
int s1 = 6;
int s2 = 7;
int s3 = 8;

//Mux in "SIG" pin // 23-27
int SIG_pin = 23; // A0 ?
int SENS_pin = 24; // A1 ??
int global_sensitivity, sensipot;

/* setup and calibrate */
void setup() {
  analogReference(ADREF);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(HHPIN, INPUT); // input is default

  if( RESISTORS == 0) pullemup();

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  digitalWrite(LEDPIN,HIGH);
  digitalWrite(HHPIN,LOW);
  Serial.begin(SPEED);
  initialize_da_crap();
  sensorMax = 0;
  delay(50);
  for(i = 0; i < MUXERS ; i++) {
    for(k = 0; k < MUXCOUNT ; k ++){
      index = (i * MUXCOUNT )+ k;
      val = readMux(k);
      sensornoise[index] = val;
/*      if (sensornoise[index] > sensorMax) {
        sensorMax = sensornoise[index];
      } */
      delay(500);
      if(DEBUG > 0){
//        Serial.print("sensorMax: ");
//        Serial.println(sensorMax);
        debugprint(index, val);
//        Serial.println("sensornoise:");
//        debugprint(index, sensornoise[index]);
      }
    }
    hatstate = digitalRead(HHPIN);
    digitalWrite(LEDPIN,LOW);
/*    global_sensitivity = analogRead(SENS_pin);
    if(DEBUG > 0) {
      Serial.println("sensi: ");
      debugprint(SENSITIVITY, global_sensitivity);
    } */
  }
  if(DEBUG > 0)
    Serial.println("calibration end");
}

/* ------------------- */
void loop() {
  // Loop through each piezo and send data
  // on the serial output if the force exceeds the threshold
  int fuzz = 5;
  int lo = 0;
  int hi = 0;
  for(i = 0; i < MUXERS ; i++) {
    for(k = 0; k < MUXCOUNT ; k ++){
      index = (i * MUXCOUNT ) + k;
      val = readMux(k);
      if(( val > sensornoise[index]) && (val > PIEZOTHRESHOLD)) {
        digitalWrite(LEDPIN,HIGH);
        if(DEBUG > 0){
          debugprint(index, val);
        } 
        else {
          out_fo_real(index, val);
        }
        digitalWrite(LEDPIN,LOW);
        delay(6);
      }
    }
    // delay(1);
  }
// toggle output of hihat pedal
// deactivated for not having the other hardware here - attraktor 20110827
/* rectivated using a smaller pcb 20110919 */

  val = digitalRead(HHPIN);
  if(val == hatstate){
    // noting 
  } else {
    hatstate = val;
    if(DEBUG > 0){
        debugprint(HIHAT, val);
        Serial.print("hatstate: ");
        Serial.println(hatstate);
        //delay(400);
      }
      else {
        out_fo_real(HIHAT, val);
      }
    } /* */
    // other non-muxed inputs
    // sensitivity pot
/*    val = analogRead(SENSITIVITY);
    lo = val - fuzz;
    hi = val + fuzz;
    sensipot = constrain(val, lo, hi);
    if(DEBUG > 0){
      Serial.print("val: ");
      Serial.println(val);
      Serial.print("sensipot: ");
      Serial.println(sensipot);
      Serial.print("sensornoise: ");
      Serial.println(sensipot);
      delay(2000);
    }
    if(val == sensipot){
      // noting
    } else {
      Serial.println("change");
      if(sensipot < PIEZOTHRESHOLD) {
        // nope
        if(DEBUG > 0)
          debugprint(SENSITIVITY, sensipot);
      } else {
        global_sensitivity = sensipot;
        // adjust sensitivity
//        for(i = 0; i < MUXCOUNT ; i ++){
//            sensornoise[i] = global_sensitivity;
//        }
      }
    } */
}

void noteOn(byte cmd, byte data1, byte data2) {
   Serial.print(cmd, BYTE);
   Serial.print(data1, BYTE);
   Serial.print(data2, BYTE);
}


int readMux(int channel){
  int controlPin[] = { s0, s1, s2, s3 };

  int muxChannel[16][4]={
    { 0,0,0,0  } , //channel 0
    { 1,0,0,0  } , //channel 1
    { 0,1,0,0  } , //channel 2
    { 1,1,0,0  } , //channel 3
    { 0,0,1,0  } , //channel 4
    { 1,0,1,0  } , //channel 5
    { 0,1,1,0  } , //channel 6
    { 1,1,1,0  } , //channel 7
    { 0,0,0,1  } , //channel 8
    { 1,0,0,1  } , //channel 9
    { 0,1,0,1  } , //channel 10
    { 1,1,0,1  } , //channel 11
    { 0,0,1,1  } , //channel 12
    { 1,0,1,1  } , //channel 13
    { 0,1,1,1  } , //channel 14
    { 1,1,1,1  }   //channel 15
  };

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_pin);
  if(DEBUG > 0){
    delay(1);
  }
  return val;
}


void debugprint(int index, int val) {
  Serial.print("!");
  Serial.print(index);
  //Serial.print(LeDrumz[index]);
  Serial.print("@");
  Serial.println(val);
}

void out_fo_real(int index, int val) {
  Serial.write("!");
  Serial.write(index);
  Serial.write("@");
  Serial.write(val);
}

void initialize_da_crap () {
// connection order
        strcpy(LeDrumz[0],"bd");                                                                                                           
        strcpy(LeDrumz[1],"hh1");                                                                                                          
        strcpy(LeDrumz[2],"hh2");                                                                                                          
        strcpy(LeDrumz[3],"sd1");                                                                                                          
        strcpy(LeDrumz[4],"sd2");                                                                                                          
        strcpy(LeDrumz[5],"sd3");                                                                                                          
        strcpy(LeDrumz[6],"ride1");                                                                                                        
        strcpy(LeDrumz[7],"ride2");                                                                                                        
        strcpy(LeDrumz[8],"crash1");                                                                                                       
        strcpy(LeDrumz[9],"crash2");                                                                                                       
        strcpy(LeDrumz[10],"crash3");                                                                                                      
        strcpy(LeDrumz[11],"rasen1");                                                                                                      
        strcpy(LeDrumz[12],"rasen2");                                                                                                      
        strcpy(LeDrumz[13],"cow");                                                                                                         
        strcpy(LeDrumz[14],"tom1");                                                                                                     
        strcpy(LeDrumz[15],"tom2");
}

void pullemup () {
 // this should help if no pullup resistors were present
 digitalWrite(A0, HIGH);
 digitalWrite(A1, HIGH);
 digitalWrite(A2, HIGH);
 digitalWrite(A3, HIGH);
 digitalWrite(A4, HIGH);
 digitalWrite(A5, HIGH);
}

int changed(int globalvar, int testval){
    if( globalvar != testval){
        return(1);
    }else return(0);
}

