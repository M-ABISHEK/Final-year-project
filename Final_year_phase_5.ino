#include <Keypad.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;
#include <LiquidCrystal.h>
#define lock 45
#define lock1 46

SoftwareSerial mySerial(11,12);
const byte ROWS = 4;
const byte COLS = 3;
byte datacount=0 ;
char data[5];
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {2,3,4,5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6,7,8}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
long randNumber;
char buf[16];
char manager_buf[16];
int logout =0;
int count = 0;
int manager_count = 0;
int user_count = 0;
String a;
String manager = "AT+CMGS=\"+918903970069\"\r";
String user = "AT+CMGS=\"+917695907190\"\r";
int encryption_code = 1354;
int manager_login,user_login,user_lock,manager_lock = 0;
int Fmin,Fsec,Pmin,Psec =0;
int expired =0;
int EP =9;

void setup()
{
  mySerial.begin(9600);   // Setting the baud rate of GSM Module  
  Serial.begin(9600);    // Setting the baud rate of Serial Monitor (Arduino)
  randomSeed(analogRead(0));
  pinMode(44,OUTPUT);
  pinMode(44,HIGH);
  pinMode(lock, OUTPUT);
  digitalWrite(lock, LOW);
    pinMode(lock1, OUTPUT);
  digitalWrite(lock1, LOW);
  pinMode(EP, INPUT); //set EP input for measurment
  Serial.println("Welcome to SIET");
  
  while (!Serial); // for Leonardo/Micro/Zero
  if (! rtc.begin()) 
 {
   Serial.println("Couldn't find RTC");
   while (1);
 }
 if (! rtc.isrunning()) 
 {
   Serial.println("RTC is NOT running!");
   // following line sets the RTC to the date & time this sketch was compiled
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
 }
  
  if (Serial.available())
  {
     mySerial.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
     Serial.println("GSM status is set to receive the live sms");
     delay(2000);
  }
  mySerial.println("AT+CMGF=1\r");    //Sets the GSM Module in Text Mode
  Serial.println("GSM status is set to text mode");
  delay(2000);
}

void loop()
{
  if (mySerial.available())
    a = mySerial.readString();
    if (a.indexOf("Phenomenal")>=0 && a.indexOf("7695907190")>=0)
    {
//       count,user_count,manager_count,user_lock,logout =0;
       a="boo";
       random_number();
    }

    if ( logout == 1 )
    {
      impact_detection();
    }
    
    expiry_check();
    char key = keypad.getKey();
    // checking count to avoid the accidental key press
    if (key && count<3 && logout!=1)
    {
      Serial.println(key);
      data[datacount]=key;
      datacount++;
    } 
    else if(key && count>=3 && logout!=1)
    {
      Serial.println("Number of chances exceeded..."); 
      SOS_call();
      cleardata();
    }
    else if(key  && logout==1)
    {
      Serial.println("Someone tries to open the Vault...");
      SOS_call();
      cleardata();
    }
    
    if (datacount==4)
    {
      Serial.println(data);

      if(!strcmp(data,"****"))
      {
        digitalWrite(lock, LOW);
        Serial.println("User lock is locked");
        user_lock = 1;
        cleardata();
      }
      else if(!strcmp(data,"####")&&user_lock==1)
      {
        digitalWrite(lock1, LOW);
        Serial.println("Manager lock is locked");
        logout = 1;
        cleardata();
      }
      else if(!strcmp(data,manager_buf)&& expired ==0)
      {
        digitalWrite(lock1, HIGH);
        delay(1000);
        Serial.println("Access Granted to manager");
        Serial.println("Enter the user password");
        manager_count = 1;
        cleardata();
      }
      else if(!strcmp(data,buf)&&(manager_count==1)&& expired ==0)
      {
        digitalWrite(lock, HIGH);
        delay(1000);
        Serial.println("Access Granted to user");
        user_count = 1;
        cleardata();
      }
      else
      {
        count = count + 1;
        Serial.println("Access Denied");
        Serial.println("Check Your Password or your OTP is expired...");
        cleardata();
      }
    }
}

// OTP sending 
void send_sms_user(long randNumber,String person)
{
     mySerial.println(person);
     delay(1000);
     mySerial.println("OTP for Locker Number - 13544 : "+String(manager_buf)+
                      "Your OTP will expire in 90 seconds");// The SMS text you want to send
     delay(100);
     mySerial.println((char)26);// ASCII code of CTRL+Z for saying the end of sms to  the module 
     delay(1000);
     mySerial.println();
     delay(100);
}

// OTP Generation
void random_number()
{
  Fmin,Fsec,Pmin,Psec =0;
  count,user_count,manager_count,user_lock,logout,expired =0;
  long randNumber;
  randNumber = random(1000,8000);
  Serial.println(randNumber);
  delay(2000);
  itoa(randNumber,buf,10);
  itoa(randNumber+encryption_code,manager_buf,10);
  delay(1000);
  send_sms_user(randNumber,user);// send OTP to the User
  Serial.println("Message sent to user");
  delay(5000);
  send_sms_user(randNumber,manager); // send OTP to the Manager
  Serial.println("Message sent to manager");
  delay(5000);
  DateTime now = rtc.now();
  DateTime future (now + TimeSpan(0,0,1,30));
  Fmin = future.minute();
  Fsec = future.second();
  Serial.println("Enter the manager password");
}

// SOS Call
void SOS_call()
{
  mySerial.println("ATD+918903970069;"); // ATDxxxxxxxxxx; -- watch out here for semicolon at the end
  Serial.println("Calling to manager "); // print response over serial port
  delay(30000);
  mySerial.println("ATD+917695907190;"); // ATDxxxxxxxxxx; -- watch out here for semicolon at the end
  Serial.println("Calling to user "); // print response over serial port
  delay(30000);
}

// Expiry check
void expiry_check()
{
 DateTime now = rtc.now();

 Pmin = now.minute();
 Psec = now.second();
 while (Pmin==Fmin && Psec==Fsec && expired ==0)
 {
  expired =1;
  break;
 }
}

// Impact Detection
void impact_detection()
{
  long measurement = pulseIn (EP, HIGH);
//  delay(500);
//  Serial.print("measurment = ");
//  Serial.println(measurement);
//  delay(1000);
  if (measurement > 2000)// to change
  {
    Serial.println("Intruder action......");
    SOS_call();
    measurement = 0; 
  }
}

// To clear the typed password through the keypad
void cleardata()
{
  while (datacount != 0)
  { 
    data[datacount--] = 0; 
  }
  return;
}
