#define CYCLE_REPEATS 30
#define tol .5
#define HOT 5
#define FAN 4


#include <Wire.h>

void setup() {
  Wire.begin();        //join bus as a master
  Serial.begin(9600);  //start serial port to computer
  pinMode(HOT,OUTPUT); //set hot and fan pins as outputs.  Default is low.
  pinMode(FAN,OUTPUT);
}

//get raw value from sensor, convert it to temperature and return it as a float
float get_temp(int address) 
{
    unsigned short upper, lower, raw;
    float temp;
    Wire.requestFrom(address,2);
    while(Wire.available()) //read data from sensor as a 12 bit twos complement. since this application will always be at ambient temperature, we dont need to worry about the sign.
    {
      upper= Wire.read();
      lower= Wire.read();
      raw  = (upper << 4 ) ^ (lower >>4); //get rid of trailing 0s
      temp=(float)raw*.0625;              //each ADC "tick" is .0625 of a degree.
      return temp;
    }
}
  
  
//this is the function that decides what the machine will do- heat, cool, or idle.  the #define tol .5 at the top can be changed to an arbitrary tolerance.
//making the tolerence too small will result in your machine flipping between heating and cooling really fast, making it too big will result in more ringing
int temp_task(float target,float temp)   
{
  if(temp<(target-tol))
  {
    digitalWrite(FAN,LOW);
    digitalWrite(HOT,HIGH);
  }
  else if(temp>(target+tol))
  {
    digitalWrite(HOT,LOW);
    digitalWrite(FAN,HIGH);
  }
  else
  {
    digitalWrite(HOT,LOW);
    digitalWrite(FAN,LOW);
  }
  
  return temp>(target-tol-tol) && temp<(target+tol+tol); //return 1 if temp is within 2 tol- approaching switchover pt. from ramping to waiting for the timer
}


//this function ramps (heats or cools) to the desired temperature, then waits a certian amount of time while holding that temperature
void single_cycle(int seconds, float target)
{
   float temp;
   temp=get_temp(0b1001111);
   Serial.print("BEGIN\n"); //prints this at the beginning of every cycle, useful for debugging
   Serial.println(temp,DEC);
   while (!temp_task(target,temp)) //while temperature is not near the target, keep ramping and checking the temperature.  delay makes each cycle take about 1/8 of a second
   {
     temp=get_temp(0b1001111);
     Serial.println(temp,DEC);
     temp_task(target,temp);
     delay(125);
   }
   for(int i=0; i<seconds*8; i++){ //seconds*8 since this loop takes about 1/8 second.  holds at the desired temp for the desired number of seconds
     temp=get_temp(0b1001111);
     Serial.println(temp,DEC);
     temp_task(target,temp);
     delay(125);
   }
  
  
  
}

void loop() {
  float temp, target;
  target=25; //arb target temp, just so the thing doesent go crazy it is set to about room temperature
  
  //these wire commands set the prescision of the sensor to 12 bits
  
  delay(1000);
  Wire.beginTransmission(0b1001111);
  Wire.write(0b00000001);
  Wire.write(0b01100000);
  Wire.endTransmission();
  
  Wire.beginTransmission(0b1001111);
  Wire.write(0b00000000);
  Wire.endTransmission();
  
  int time=0;
  
  delay(1000);
  
  //prints start at the beginning of the cycle
  Serial.println("START");
  
   //add stuff here that you want to do before the cycle, eg hot start, initial denaturation
   
   single_cycle(30,98);
   
   //this for loop is what gets repeated over and over again, change #define CYCLE_REPEATS 30 to change it
   for (int i=0; i<CYCLE_REPEATS;i++){
     //change what is in here to change what the denature-anneal-extend cycle is.
     single_cycle(10,98);//denature
     single_cycle(30,71);//anneal
     single_cycle(30,72);//extend
   }
   //Final extension etc. goes here.  Note repeat to get correct time.
   single_cycle(30,72);
   single_cycle(30,72);
   single_cycle(30,72);
   single_cycle(30,72);
   single_cycle(30,72);
   single_cycle(30,72);
   single_cycle(30,72);
   single_cycle(30,72);
   single_cycle(30,72);
   single_cycle(30,72);
   
   //this block turns off the fan and bulb and waits for the device to be reset
   digitalWrite(HOT,LOW);
   digitalWrite(FAN,LOW);
   while(1){
   Serial.println("IDLE");
   }
      
  
}
