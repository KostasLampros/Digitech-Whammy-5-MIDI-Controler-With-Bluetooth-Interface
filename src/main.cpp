#include <Arduino.h>
#include <MIDI.h>
#include <Preferences.h>
#include <BluetoothSerial.h> 


MIDI_CREATE_DEFAULT_INSTANCE(); 
// Permanent storage object 
Preferences prefs;

// Check if Bluetooth configs are enabled
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Bluetooth Serial object
BluetoothSerial SerialBT;

// Handle received and sent messages
String message = "";
char incomingChar;
String temperatureString = "";
char charBuf[90];
char *ptr = NULL;

//Preference variables
#define maxVal  22 //Max capacity of programms
byte prog[maxVal]; // Array to print programms used
#define tap_delay  200 // Delay before next change registration
#define duration  500 // Duration needed to change rotation
#define buttonPin  2 //Stomp switch pin
#define switch_  15 //Switch to change modes: playng/updating
#define led_norm  12 // Led for normal looping mode
#define led_rev  13 // Led for reversed looping mode
#define mode_switch 14 //Swith to change mode form Chords to Classic

//Static variables
//unsigned long blinkTimer = millis();
unsigned long startPressed = 0; //initial   // The moment the button was pressed
unsigned long last_call = -1000; //Last time the buuton was pressed
int press_time = 0;  // how long the button was hold
bool rev = false; // Looping mode
//int eeSize = 10; //Number of programms in memory
//int prog = 0; //pogramm curently in update mode
//int poss; //possition of existing programm
//bool in = false; //indicator if program is in memmory
int clLen =0; //Number of programms in classic mode memory
bool connected = 0; // Status of bluetooth connection
bool classic ;

bool modeState ; // current state of the mode swith
bool lastModeState; // previous state of the mode swith
int buttonState = 1;     // current state of the button
int lastButtonState = 1; // previous state of the button
int i = -1;
int j = 1;
int k;
int q = -1;
int w = 1; // mporei n bgei
bool once = true;
//int f = 0;


void parse_msg(char msg[]){
    byte index = 0;
    msg[0] = '0';
    classic = (msg[strlen(msg)-3] == 'A') ?  true : false ;
    msg[strlen(msg)-6] = '\0';
    Serial.println("Length of msg: " + String(strlen(msg)));
    Serial.println("msg load= " + String(msg[(strlen(msg)-3)]));
    for (int x=0; x < strlen(msg); x++) Serial.print(msg[x]);
    ptr = strtok(msg, ","); //pointers of message ints
    char *strings[25];
    //Serial.print("ptr = " + String(ptr));
  while (ptr != NULL)
   {  
      strings[index] = ptr;  
      index++;
      ptr = strtok(NULL, ",");
   }
   Serial.println("index = " + String(index));
   if(classic) Serial.println("CLASSIC");
   if(index<= maxVal){
      clLen = index;
      for (int x=0; x < clLen; x++) prog[x] = atoi(strings[x]);
      for (int x=clLen; x < maxVal; x++) prog[x] = 255;
      if(classic){
        prefs.begin("classic", false);
        prefs.putBytes("classic", prog, sizeof(prog));
        prefs.end();
        lastModeState = !lastModeState;
      }else{
        prefs.begin("chords", false);
        prefs.putBytes("chords", prog, sizeof(prog));
        prefs.end();
        lastModeState = !lastModeState;
      }
      //int prog[clLen];
      //prefs.getBytes("classic", prog, maxVal);
   }else{
     SerialBT.println("Too long array");
     delay(1000);
   }
}

void changeProg() {
  // the button has been just pressed
  if (buttonState == LOW) {
      startPressed = millis();

  // the button has been just released
  } else {
      if(millis()-last_call<tap_delay){  // Check from last press to avoid double tap
            return;
      }else{
        last_call = millis();
        press_time = last_call - startPressed;
  
        if (press_time >= duration) {
            if(!rev){  // check if it is in reverse mode or not
              j = clLen -1;  //Modification of i progression to reverse 
              rev = true;
              digitalWrite(led_rev, HIGH); // turn on reversed led
            }else{
              j = 1;
              rev = false;
              digitalWrite(led_rev, LOW); // turn off reversed led
            }
        }
        i+=j;
        MIDI.sendProgramChange(prog[i%clLen] -1,1); //sends variable of programms  in position i  in channel 1
        Serial.println(prog[i%clLen]);
  
      }
  }
}




// void blink_led(int led){
//     if(digitalRead(switch_) == HIGH){
//         if(millis()-blinkTimer <250){
//             digitalWrite(led,HIGH);
//         }else if(millis()-blinkTimer<500){
//             digitalWrite(led,LOW); 
//         }else{
//           blinkTimer = millis();
//         }
//     }    
// }

// Function to send programms via bluetooth for communication with the app
void send(int length){
  message = "";
  for (int k = 0; k < length; k++){
    if( prog[k]!=255 && k>0){
        message += "," ;
      }
    if(prog[k]!=255){
      message += prog[k] ;
    }
  }
  SerialBT.println(message);
  Serial.println("Send message "+ String(message));
}

//Function to print prpogramms to Serial monitor for debugging 
void show(int length){
  for (int k = 0; k < length; k++){
    if( prog[k]!=255 && k>0){
        Serial.print(",");
      }
    if(prog[k]!=255){
      Serial.print(prog[k]);
    }
  }
  Serial.println("");
}

void setup() 
{ 
  pinMode(buttonPin, INPUT_PULLUP); // initialize the button pin as a input
  Serial.begin(115200);        // initialize serial communication
  pinMode(led_norm, OUTPUT); // initialize the LED pin as a output
  pinMode(led_rev, OUTPUT); // initialize the LED pin as a output
  pinMode(switch_, INPUT); // initialize the switch pin as a input
  pinMode(mode_switch, INPUT_PULLUP); // initialize the switch pin as a input
  //MIDI.begin(MIDI_CHANNEL_OFF);

  prefs.begin("classic");
  

  

  //byte clLen = prefs.getBytesLength("classic"); //See length of saved programms in classic
  
  //byte prog[clLen];
  prefs.getBytes("classic", prog, maxVal);
  
  while (prog[clLen] != 255){
    clLen++;
  }
  
  
}

void loop(){
  //playing mode
  if(digitalRead(switch_) == LOW){
        if (i==-1){
          q = -1;
          w = 1;
          SerialBT.println("BT stopping");//
          SerialBT.flush();               // Disconect from bluetooth to save power
          SerialBT.disconnect();          //
          SerialBT.end();                 //      
          once = true;
          connected = 0;
          digitalWrite(led_norm,LOW); //In case blink() ended with HIGH value
          if( rev == true){
            j = clLen-1;
          }
        }
        
        modeState = digitalRead(mode_switch);  // Read mode switch possition

        // If mode state has changed load the prog array with the apropriate values
        if(modeState != lastModeState){
          lastModeState = modeState;
          i = -1; // Start array from the begining
          clLen = 0;
          if(modeState){
            prefs.begin("classic",true);
            prefs.getBytes("classic", prog, maxVal);
            prefs.end();
            while (prog[clLen] != 255){
              clLen++;
            }
            Serial.println(clLen);
            show(clLen);
          }else{
            prefs.begin("chords",true);
            prefs.getBytes("chords", prog, maxVal);
            prefs.end();
            while (prog[clLen] != 255){
              clLen++;
            }
            j = !rev ? 1 : (clLen -1);
            Serial.println(clLen);
            show(clLen);
          }
        }
          
        buttonState = digitalRead(buttonPin); // read the button input
        
        if (buttonState != lastButtonState) { // button state changed
            changeProg();
        }

        lastButtonState = buttonState;        // save state for next loop
  
  //bluetooth mode
  }else{
        if (q==-1){
          i = -1;
          j = 1;
          digitalWrite(led_norm,LOW); // In case blink() ended with HIGH value
          if( rev == true){
            w = 21;
          }
        }

        //recieves message 
        if(once){
          SerialBT.begin("ESP32"); // Bluetooth device name
          while(!connected){
            digitalWrite(led_rev, HIGH);
            delay(250);
            digitalWrite(led_rev, LOW);
            delay(250);
            if(SerialBT.read() == 1) connected = 1;
            if(digitalRead(switch_) == LOW) break;
          }
          send(clLen);
          SerialBT.println();
          delay(200); //Delay to seperate mesages in app
          //Diferentiation for classic and chords mode
          if(prog[0] > 42){
            Serial.println("Changed to classic");
            prefs.begin("classic",true);
            prefs.getBytes("classic", prog, maxVal);
            prefs.end();
            while (prog[clLen] != 255){
              clLen++;
            }
            send(clLen);
            lastModeState = !lastModeState;
          }else{
            Serial.println("Changed to chords");
            prefs.begin("chords",true);
            prefs.getBytes("chords", prog, maxVal);
            prefs.end();
            while (prog[clLen] != 255){
              clLen++;
            }
            send(clLen);
            lastModeState = !lastModeState;
          }
          //SerialBT.println("Send the porgrams you want"); 
          once = !once; 
          message =""; //delete previous message     
        }
        
        if (SerialBT.available()>0){
          char incomingChar = SerialBT.read();
          if (incomingChar != '\n'){
            message += String(incomingChar);
        }else{
          message.toCharArray(charBuf, 90);
          message = "";
          once = true;
        }
        Serial.write(incomingChar);
        }

        
        
        if(strlen(charBuf) != 0 ){
          Serial.println("Len of char Buff: " + String(strlen(charBuf)));
          parse_msg(charBuf); //send the pessage to be parsed by the function
          charBuf [0] = '\0';
          message = ""; //delete previous message  
          SerialBT.println("Change complete"); // Text to show completed task in the app
          delay(100); //Delay to seperate mesages in app
        }
        
        
      
  }
  
}
