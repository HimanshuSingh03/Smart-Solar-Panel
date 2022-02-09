#include <Servo.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_SSD1306.h>
#include <dht.h>
#include <SoftwareSerial.h>

//----------------------------------------------------------
//the next 4 lines look really complicated but they are just to set up the wireless connection with the app
#define BLYNK_PRINT DebugSerial
SoftwareSerial DebugSerial(2, 3); // RX, TX
char auth[] = "oPjH1NjEcitLKorij1H9qdy4M13NU_yZ";
#include <BlynkSimpleStream.h>
//----------------------------------------------------------


Adafruit_INA219 ina219; //Current sensor
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET); //OLED display

dht DHT; //temperature sensor
#define DHT11_PIN 11


//defining Servos
Servo servohori; //horizontal servo
int servoh = 0;
int servohLimitHigh = 160;
int servohLimitLow = 20;

Servo servoverti; //Vertical Servo
int servov = 0; 
int servovLimitHigh = 160;
int servovLimitLow = 20;

//Assigning LDRs
int ldrtopl = 3; //top left LDR white
int ldrtopr = 0; //top right LDR blue
int ldrbotl = 2; // bottom left LDR black
int ldrbotr = 1; // bottom right LDR orange

float shuntvoltage = 0; // value returned by INA219
float busvoltage = 0; // value returned by INA219
float loadvoltage = 0; 
float current_mA = 0; // value returned by INA219
float power = 0;
unsigned long previousMillis = 0;
unsigned long interval = 100;
float energy = 0;
float temperature=0;

int refreshrate = 25; //higher means faster panel
int cycle;



 void setup () 
 {
  ina219.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.display();
  
  servohori.attach(6);   //attach servo to digital pin 6
  servohori.write(90);
  servoverti.attach(7);  //attach servo to digital pin 7
  servoverti.write(90);
  delay(500);

  DebugSerial.begin(9600);
  Serial.begin(9600);
  Blynk.begin(Serial, auth);
 }




void loop()
{
  Blynk.run();
  
  servoh = servohori.read();
  servov = servoverti.read();
  
  //capturing analog values of each LDR
  int topl = analogRead(ldrtopl);
  int topr = analogRead(ldrtopr);
  int botl = analogRead(ldrbotl);
  int botr = analogRead(ldrbotr);

  //print values (this is just for me to see them during testing)
  Serial.println(topl);
  Serial.println(topr);
  Serial.println(botl);
  Serial.println(botr);
  
  // calculating averages
  int avgtop = (topl + topr) / 2; //average of top LDRs
  int avgbot = (botl + botr) / 2; //average of bottom LDRs
  int avgleft = (topl + botl) / 2; //average of left LDRs
  int avgright = (topr + botr) / 2; //average of right LDRs


  //if light is coming from the bottom direction
  if (avgtop < avgbot) 
  {
    servoverti.write(servov +1);
    if (servov > servovLimitHigh) 
     {
      servov = servovLimitHigh;
     }
    delay(10);
  }
  
  //if light is coming from the top direction
  else if (avgbot < avgtop)
  {
    servoverti.write(servov -1);
    if (servov < servovLimitLow)
  {
    servov = servovLimitLow;
  }
    delay(10);
  }
  else 
  {
    servoverti.write(servov);
  }


  //if light is coming from the left direction
  if (avgleft > avgright)
  {
    servohori.write(servoh +1);
    if (servoh > servohLimitHigh)
    {
    servoh = servohLimitHigh;
    }
    delay(10);
  }

  //if light is coming from the right direction
  else if (avgright > avgleft)
  {
    servohori.write(servoh -1);
    if (servoh < servohLimitLow)
     {
     servoh = servohLimitLow;
     }
    delay(10);
  }
  else 
  {
    servohori.write(servoh);
  }
  delay(30);
  
//-------------------------------------------------------------- Calculator

  cycle++;
  if(cycle == refreshrate){

        unsigned long currentMillis = millis();
          if (currentMillis - previousMillis >= interval)
          {
            previousMillis = currentMillis; 
            read_sensor_data();
            display_data();  
              }
      cycle = 0;
  }


}


void read_sensor_data(){  //reads data from the INA219
      shuntvoltage = ina219.getShuntVoltage_mV();
      busvoltage = ina219.getBusVoltage_V();
      current_mA = ina219.getCurrent_mA();
      
      loadvoltage = busvoltage + (shuntvoltage / 1000);
      energy = energy + loadvoltage * current_mA / 3600; //not sure if this calculation is right
      power = loadvoltage*current_mA;
        
      //read temperature sensor
      int chk = DHT.read11(DHT11_PIN);
      temperature = DHT.temperature;
}

void display_data(){   // show values from the above method on the OLED
      
      display.clearDisplay();  
      display.setCursor(0, 0);
      display.print(busvoltage);
      display.print(" V");
    
      display.setCursor(0, 10);
      display.print(current_mA);
      display.print(" mA");
    
      display.setCursor(0, 20);
      display.print(power);
      display.print(" mW");
    
      display.setCursor(65,0);
      display.print(energy);
      display.println(" mWh");
      
      display.setCursor(65,20);
      display.print(temperature);  
      display.println(" C");
      
      display.display();
 }
