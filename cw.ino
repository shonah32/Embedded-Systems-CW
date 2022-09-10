#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <EEPROM.h>

#define RED 1
#define GREEN 2
#define YELLOW 3
#define PURPLE 5
#define WHITE 7

#ifdef __arm__
extern "C" char* sbrk(int incr); 
#else // __ARM__
extern char *__brkval;
#endif // __arm__

typedef struct{
  char id;
  byte value; 
  byte min;
  byte max; 
  char desc[16]; 
  byte count;
  byte recent[10]; 
  int avg; 
  
} channel; //used to refer to all the variables above in the code 

typedef enum{
    INITIALISATION, 
    SYNCHRONISATION, 
    AFTERSYNC, 
    MAIN, 
    BEFORE_SEL, //select button before being held for one second 
    AT_SEL, //select button held for one second 
    AT_UP, //up button pressed 
    AT_DOWN, //down button pressed 
    AT_RIGHT, //HCI extension
    AFTER_RIGHT, //HCI extension
  
} state_e; 

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield(); 

channel *channels[26]; //creates an array of size 26 (A-Z)
channel *channelsMax[26]; //creates an array of size 26 (A-Z) 

bool errorDetectVal(String incomingMsg){
  if(incomingMsg.length() > 2 & incomingMsg.length() <= 5){
    return true;
  }
  else{
    return false; 
  }
}

bool errorDetectString(String incomingMsg){
  if(incomingMsg.length() > 2 & incomingMsg.length() <= 18){
    return true;
  }
  else{
    return false; 
  }
}

bool channelExists(char channelID){
   return channels[channelID - 'A'] != NULL; 
}

int freeMemory() {
  char top;
  #ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
  #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
  #else // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start; 
  #endif // __arm__
}

void eeprom(channel *channelPtr){
  
  char channelID = channelPtr -> id; 
  int index = (channelID - 'A'); 
  char *channelDesc = channelPtr -> desc; 
  char id = channelPtr -> id; 
  byte max = channelPtr -> max;
  byte min = channelPtr -> min; 

  if(EEPROM.read(0) == 64){
    Serial.println("DEBUG: Data not cleared from EEPROM");
    EEPROM.update((index+(index*18) + 1), channelID); //entering in data to EEPROM 
    EEPROM.update((1 + index+(index*18) + 1), min); 
    EEPROM.update((2 + index+(index*18) + 1), max);
  
    byte counter = 3;
    
    for(int j=0; j<16; j++){
      if(channelDesc[j] == NULL){
        EEPROM.update((counter + index + (index*18) + 1), 0); 
        counter++;
      }
      
      else{
        EEPROM.update((counter + index + (index*18) + 1), channelDesc[j]); 
        counter++;
      }
        
    }
    
  } //end of if statement 
  else{
    for(int i=0; i<1024; i++){
     EEPROM.update(i,0); 
   } //clears the EEPROM of previous data 
   
    Serial.println("DEBUG: Cleared EEPROM data"); 
    EEPROM.update(0, 64); //updates the first index of EEPROM with the '@' symbol 
    EEPROM.update((index+(index*18) + 1), channelID); //entering in data to EEPROM 
    EEPROM.update((1 + index+(index*18) + 1), min); 
    EEPROM.update((2 + index+(index*18) + 1), max);
  
    byte counter = 3;
    
    for(int j=0; j<16; j++){
      if(channelDesc[j] == NULL){
        EEPROM.update((counter + index + (index*18) + 1), 0); 
        counter++;
      }
      
      else{
        EEPROM.update((counter + index + (index*18) + 1), channelDesc[j]); 
        counter++;
      }
        
    }
   
  } //end of else statement 


//   for(int i=0; i<100; i++){
//     EEPROM.update(i,0); 
//   } //clears the EEPROM of previous data 

//   Serial.println("channel desc"); 
//   for(int i=0; i<20; i++){
//    Serial.println(channelDesc[i]);
//   }

//DEBUG TESTING 
//  Serial.println(channelDesc[6]); 
//  Serial.println(channelDesc[8]); 
  
//  for(int j=0; j<16; j++){
//    if(channelDesc[j] == NULL){
//      Serial.println(j); 
//      Serial.println("null");
//      Serial.println(" ");
//    }
//    else{
//      Serial.println(j); 
//      Serial.println(channelDesc[j]); 
//      Serial.println(" ");
//    }
     
//  }

//DEBUG TESTING 
//for(int i=3; i<19; i++){
//  Serial.println(i + index + (index*18) + 1); 
//}
    
//  EEPROM.update((index+(index*18) + 1), channelID); //entering in data to EEPROM 
//  EEPROM.update((1 + index+(index*18) + 1), min); 
//  EEPROM.update((2 + index+(index*18) + 1), max);
//
//  byte counter = 3;
//  
//  for(int j=0; j<16; j++){
//    if(channelDesc[j] == NULL){
//      EEPROM.update((counter + index + (index*18) + 1), 0); 
//      counter++;
//    }
//    
//    else{
//      EEPROM.update((counter + index + (index*18) + 1), channelDesc[j]); 
//      counter++;
//    }
//      
//  }
   
//  
//    for(int i=0; i<50; i++){
//      Serial.print(i);
//      Serial.print(" "); 
//      Serial.println(EEPROM.read(i));
//    } //checks what's inside the EEPROM 

//      for(int i=0; i<40; i++){ //displays 4*26 channels (put i<103) 
//        Serial.print(i);
//        Serial.println(EEPROM.read(i)); 
//      }

  
} //end of eeprom function 


void channelScreenTop(channel *channelPtr){
  lcd.setCursor(1,0); //sets the cursor to the top line 
  lcd.print(channelPtr -> id);
  if(channelPtr -> value == NULL){
    lcd.print("");
  }
  else{
    lcd.print(rightJust(channelPtr -> value)); //passes the value through the rightJust function to be formatted so it's aligned to the right 
  }
  lcd.setCursor(5,0);
  lcd.print(","); 
  for(int i=6; i<=8; i++) {
    lcd.setCursor(i,0); 
    lcd.print(' '); //shows spaces to 'hide' the 'old' average 
  }
  lcd.setCursor(6,0);
  if(channelPtr -> count == 0){
    lcd.print(""); 
  } 
  else{
    lcd.print(rightJust(channelPtr -> avg)); 
  }
  lcd.setCursor(10,0); 
  String desc = (channelPtr -> desc);
  if (desc.length() >= 7){
    scrollingTop(desc);
  }
  else{
    for(int i = 10; i < 16; i++) {
      lcd.setCursor(i,0); 
      lcd.print(' '); //shows spaces to 'hide' the 'old' description
    }
    lcd.setCursor(10,0); 
    lcd.print(desc); 
  }

}

void channelScreenBot(channel *channelPtr){
  lcd.setCursor(1,1); //sets the cursor to the bottom line 
  lcd.print(channelPtr -> id); 
  if(channelPtr -> value == NULL){
    lcd.print("");
  }
  else{
    lcd.print(rightJust(channelPtr -> value)); //passes the value through the rightJust function to be formatted so it's aligned to the right 
  }
  lcd.setCursor(5,1);
  lcd.print(",");  
  for(int i=6; i<=8; i++) {
    lcd.setCursor(i,1); 
    lcd.print(' '); //shows spaces to 'hide' the 'old' average
  }
  lcd.setCursor(6,1); 
  if(channelPtr -> count == 0){
    lcd.print(""); 
  }
  else{
    lcd.print(rightJust(channelPtr -> avg)); 
  }
  lcd.setCursor(10,1); 
  String desc = (channelPtr -> desc);
  if (desc.length() >= 7){
    scrollingBot(desc);
  }
  else{
    for(int i = 10; i < 16; i++) {
      lcd.setCursor(i,1); 
      lcd.print(' '); //shows spaces to 'hide' the 'old' description
    }
    lcd.setCursor(10,1); 
    lcd.print(desc); 
  }

} //end of function 


void scrollingTop(String scrollingMessage){
  scrollingMessage = scrollingMessage + " "; 
  static unsigned int scrollPos = 10;
  static unsigned long now = millis();

  if (millis() - now > 500) {
    now = millis();
    scrollPos++;
    if (scrollPos > scrollingMessage.length()) {
      scrollPos = 0;
    }
  }
  
  lcd.setCursor(10, 0);
  lcd.print(scrollingMessage.substring(scrollPos, scrollPos + 10));
  lcd.setCursor(10, 0);
}

void scrollingBot(String scrollingMessage){
  scrollingMessage = scrollingMessage + " "; 
  static unsigned int scrollPos = 10;
  static unsigned long now = millis();

  if (millis() - now > 500) {
    now = millis();
    scrollPos++;
    if (scrollPos > scrollingMessage.length()) {
      scrollPos = 0;
    }
  }
  
  lcd.setCursor(10, 1);
  lcd.print(scrollingMessage.substring(scrollPos, scrollPos + 10));
  lcd.setCursor(10, 1);
}

String rightJust(byte value){ //function to align the value to the right
  if(value >= 100){
    return String(value);
  }
  
  if(value >= 10){
    return String(" ") + String(value); //moves the value 1 space to the right if the value is 2 digits
  }
  
  else{
    return String("  ") + String(value); //moves the value 2 spaces to the right if the value is a single digit
  }
  
}

void backlight(){ //function to set the backlight according to minimum & maximum values 
  bool red = false;
  bool green = false; 
  
  for(int i=0; i<26; i++){
    channel *ptr = channels[i];
    if(ptr == NULL){
      continue;  
    } 
    if(ptr -> value > ptr -> max){ //if the value is greater than the set maximum 
      red = true; 
      channelsMax[i] = ptr; 
    }
    else if(ptr -> value < ptr -> min){ //if the value is less than the set minimum 
      green = true; 
    }  
  }

  if(red && green){
    lcd.setBacklight(YELLOW); 
  }

  else if(red){
    lcd.setBacklight(RED); 
  }
  
  else if(green){
    lcd.setBacklight(GREEN);   
  }
  
  else{
    lcd.setBacklight(WHITE); 
  }
  
} 

channel *getChannelBefore(channel *channelPtr, bool right, bool left){
  if(channelPtr == NULL){
    return NULL; 
  }
  char id = channelPtr -> id; 
  int index = (id - 'A'); //gets the index of the channel wanted e.g. ('B'-'A') = 1 which means that B is stored in channel[1] 
  if(index < 0 || index > 25){
    return NULL; 
  }
  for(int i=index-1; i>=0; i--){
    if(right == true){
      if(channels[i] != NULL & channelPtr -> value > channelPtr -> max){
      return channels[i];
      }
    }
    else if(left == true){
      if(channels[i] != NULL & channelPtr -> value < channelPtr -> min){
      return channels[i];
      }
    }
    else{
      if(channels[i] != NULL){ 
        return channels[i];
      }
    }
  }
  return NULL; 
}

channel *getChannelAfter(channel *channelPtr, bool right, bool left){
  if(channelPtr == NULL){
  return NULL; 
  }
  char id = channelPtr -> id; 
  int index = (id - 'A'); //gets index of the channel wanted 
  if(index < 0 || index > 25){
    return NULL; 
  }
  for(int i=index+1; i<26; i++){
    if(channels[i] != NULL){
      return channels[i]; 
    }
  }
  return NULL; 
}

void setup() {
  //Set up 
  Serial.begin(9600);
  lcd.begin(16,2); 
  lcd.clear(); 
  Serial.setTimeout(100); 

} //end of setup function 

void loop() {

  static state_e state = INITIALISATION; //sets the initial state to INITIALISATION 
  static long select_press_time; 
  int button_state;
  int released; 
  static int last_b = 0; 
  static byte numChannels = 0; 
  static channel *topChannel = NULL;
  static channel *botChannel = NULL;
  
  static channel *topChannel2 = NULL;
  static channel *botChannel2 = NULL; 
  
  
  switch(state){
    case INITIALISATION:
      state = SYNCHRONISATION;

      for(int i=0; i<26; i++) {
        channels[i] = NULL; 
      }

      for(int i=0; i<26; i++){
        channelsMax[i] = NULL; 
        }
        
      break; 
    
    case SYNCHRONISATION:
      lcd.setBacklight(PURPLE);
      
      while(true){ 
        Serial.print("Q"); 
        if(Serial.available() > 0) {
          String incomingMsg = Serial.readString(); //reads the incoming string 
          if(incomingMsg == "X"){
            break; //breaks the loop if an X is received 
          }
        }
         delay(1000); //prints a Q every second 
      }

      state = AFTERSYNC; //points to the next state, which is AFTERSYNC
      break; 
    
    case AFTERSYNC:
      Serial.println(" UDCHARS, FREERAM, NAMES, SCROLL, RECENT, EEPROM"); //prints out the completed extension tasks 
      lcd.setBacklight(WHITE); 
      state = MAIN; //points to the next state, which is MAIN 
      break; 

    case MAIN:
      if(topChannel != NULL){
        channelScreenTop(topChannel); 
      }
      if(botChannel != NULL){
        channelScreenBot(botChannel);
      } 

      if (getChannelBefore(topChannel, false, false) != NULL) {
        lcd.setCursor(0, 0);
        byte upArrow[] = { B00100, B01110, B11111, B00100, B00100, B00100, B00100, B00000 }; //UDCHARS extension, creates up arrow 
        lcd.createChar(1,upArrow); 
        lcd.write(1);
      } 
      
      else {
        lcd.setCursor(0, 0);
        lcd.print(' ');
      }

      if (getChannelAfter(topChannel, false, false) != NULL) { 
        byte downArrow[] = { B00000, B00100, B00100, B00100, B00100, B11111, B01110, B00100 }; //UDCHARS extension, creates down arrow 
        lcd.createChar(0, downArrow);
        lcd.setCursor(0, 1);
        lcd.write(0);
      } 
      
      else {
        lcd.setCursor(0, 1);
        lcd.print(' ');
      }

      button_state = lcd.readButtons();
      last_b = button_state; 
      
      if(button_state & BUTTON_SELECT){
       select_press_time = millis(); 
       state = BEFORE_SEL;
      } 

      else if(button_state & BUTTON_UP){
        channel *beforeChannel = getChannelBefore(topChannel, false, false); 
        if(beforeChannel != NULL){
          botChannel = topChannel;
          topChannel = beforeChannel; //switches the top channel to the bottom line, and gets the channel before to be displayed on the top line  
        }
        state = AT_UP; 
      }
      
      else if(button_state & BUTTON_DOWN){
        channel *afterChannel = getChannelAfter(topChannel, false, false); //calls on the getChannelAfter function
        if(botChannel != NULL){ 
          topChannel = botChannel; 
          botChannel = getChannelAfter(topChannel, false, false); //switches the bottom channel to the top line, and gets the channel after to be displayed on the bottom line 
          if (botChannel == NULL) { 
            lcd.setCursor(1, 1); 
            lcd.print(F("                ")); 
          }
        }
        state = AT_DOWN; 
      }

      else if(button_state & BUTTON_RIGHT){
        state = AT_RIGHT; 
      }

        if(Serial.available() > 0) {
          String incomingMsg = Serial.readStringUntil('\n'); //reads the incoming string 
          
          if(incomingMsg.startsWith("C")){ //if the message starts with 'C' - to create a channel 
            if(errorDetectString(incomingMsg) == true){
              char channelID = incomingMsg[1];
              String channelDesc = incomingMsg.substring(2, 17); //takes the desc from the second index onwards 
              int index = (channelID - 'A'); 
              char desc[16]; 
              
              if(channelExists(channelID)){ //if the channel is already created
                channelDesc.toCharArray(channels[index] -> desc, 16); 
              }
  
              else {
                channel *createChannel = (channel*) malloc(sizeof(channel)); //allocates memory for one channel 
                channels[index] = createChannel;
                createChannel -> id = channelID; 
                createChannel -> value = NULL;
                createChannel -> min = 0;
                createChannel -> max = 255;
                channelDesc.toCharArray(createChannel -> desc, 16); //changes the channelDesc (String) to Char through the toCharArray function 
                createChannel -> count = 0; 
                createChannel -> avg = NULL; 
                
                for(int i=0; i<100; i++){
                  createChannel -> recent[i] = NULL;
                } //clears the array from any previous data 
                
                eeprom(createChannel); 
                
                if(numChannels == 0){
                  topChannel = createChannel;
                }  
                
                else if(botChannel == NULL){
                    if(channelID > topChannel -> id){
                      botChannel = createChannel; 
                    }
                }
                
                numChannels++;
                
              }
            }
            
              else{
                Serial.println("Error: " + incomingMsg + " - Not a valid channel name"); 
              }
            }
               

          else if(incomingMsg.startsWith("V")){ //if the message starts with 'V' - to indicate a value 
            if(errorDetectVal(incomingMsg) == true){
              char channelID = incomingMsg[1]; //assigns the second value of incomingMsg to channelID 
              byte value = incomingMsg.substring(2).toInt(); //takes incomingMsg from the third value onwards and converts the value to an Int 
              int index = (channelID - 'A'); //calculates the index number by subtracting 'A' from the channelID
              
              channel *setValChannel = channels[index];
              
              if(setValChannel != NULL){
                setValChannel -> value = value; 
                
                if(setValChannel -> count >= 10){
                  byte N = 10; 
                   
                  for(int i=0; i<N; i++){
                    setValChannel -> recent[i] = setValChannel -> recent[i+1];  
                  }
                  setValChannel -> recent[N-1] = setValChannel -> value; 
                } //shifts everything in the array to the left by one, 
                //then appends the new value to the end of the array (and gets rid of first array value)
                
                else{
                  setValChannel -> recent[setValChannel -> count] = value; 
                } //adds the value to the array if the array is not full 
                
                Serial.println("DEBUG: Recent extension");
//                for(int i=0; i<10; i++){
//                  Serial.print(setValChannel -> recent[i]); 
//                  Serial.print(", "); 
//                } //prints out everything in the recent array for the channel 
                
                (setValChannel -> count)++; //increments the count variable 
                
                int sum = 0; 
                byte counter = 10; 
                
                for(int i=0; i<10; i++){
                  sum = sum + (setValChannel -> recent[i]); 
                  if(setValChannel -> recent[i] == NULL){
                    counter--; 
                  }
                }
                
                setValChannel -> avg = (sum/counter); 
  //              Serial.println("DEBUG: Average"); 
  //              Serial.println(setValChannel -> avg); 
                
                backlight(); 
                
              }

            }//errorDetectVal 
            
            else{
              Serial.println("Error: " + incomingMsg); 
            }
            
          }

          else if(incomingMsg.startsWith("X")){ //if the message starts with 'X' - to specify maximum value 
            if(errorDetectVal(incomingMsg) == true){
              char channelID = incomingMsg[1];
              byte value = incomingMsg.substring(2).toInt();
              int index = (channelID - 'A'); 
              
              channel *setValChannel = channels[index]; 
              
              if(setValChannel != NULL){
                setValChannel -> max = value; 
                backlight(); 
              }
  
              eeprom(setValChannel); 
            }
            else{
              Serial.println("Error: " + incomingMsg); 
            }
            
          }

          else if(incomingMsg.startsWith("N")){ //if the message starts with 'N' - to specify a minimum value 
            if(errorDetectVal(incomingMsg) == true){
              char channelID = incomingMsg[1];
              byte value = incomingMsg.substring(2).toInt();
              int index = (channelID - 'A'); 
              
              channel *setValChannel = channels[index];
              
              if(setValChannel != NULL){
                setValChannel -> min = value; 
                backlight(); 
              }
  
              eeprom(setValChannel); 
              
              }
              else{
                Serial.println("Error: " + incomingMsg); 
              }
              
            }
                
        } //end of outside if statement 
      
      break; 

    case BEFORE_SEL:
      button_state = lcd.readButtons();
      if((millis() - select_press_time) >= 1000) {
        lcd.clear();
        lcd.setBacklight(PURPLE); 
        lcd.setCursor(0,0); 
        lcd.print("F127995"); //prints my Student ID Number 
        lcd.setCursor(0,1);
        lcd.print(freeMemory()); //extension task FREERAM 
        state = AT_SEL;
      } 
      else if(!(button_state & BUTTON_SELECT)) {
        state = MAIN; //returns to the MAIN state if the select button is released after less than 1 second 
      }
      
      break;

    case AT_SEL:
      button_state = lcd.readButtons();
      if(!(button_state & BUTTON_SELECT)) {
        backlight(); 
        state = MAIN;
        lcd.clear();
      }
      
      break; 

    case AT_UP:
      button_state = lcd.readButtons(); 
      released = ~button_state & last_b; 
      last_b = button_state; 

      if(released & BUTTON_UP){
        state = MAIN; 
        Serial.println("DEBUG: Up released"); 
      }
      
      break; 


    case AT_DOWN:
      button_state = lcd.readButtons(); 
      released = ~button_state & last_b; 
      last_b = button_state; 

      if(released & BUTTON_DOWN){
        state = MAIN; 
        Serial.println("DEBUG: Down released"); 
      }
      
      break; 


    case AT_RIGHT: 
      button_state = lcd.readButtons(); 
      released = ~button_state & last_b; 
      last_b = button_state; 

      if(released & BUTTON_RIGHT){
        Serial.println("DEBUG: Right pressed & released - AT_RIGHT"); 
        lcd.clear(); 

        for(int i=0; i<26; i++){
          channel *topChannel2 = channelsMax[i];
          channel *botChannel2 = channelsMax[i]; 
          Serial.println("DEBUG: "); 
          Serial.print(topChannel2 -> id); 
          Serial.print(botChannel2 -> id); 
          
          if(topChannel2 == NULL){
            continue;
          }
          else{
            if(topChannel2 != NULL){
              channelScreenTop(topChannel2); 
            }
            break;
            
            if(topChannel2 == NULL){
              channelScreenBot(botChannel2);
            } 
      
            if (getChannelBefore(topChannel2, true, false) != NULL) {
              lcd.setCursor(0, 0);
              byte upArrow[] = { B00100, B01110, B11111, B00100, B00100, B00100, B00100, B00100 }; //UDCHARS extension, creates up arrow 
              lcd.createChar(1,upArrow); 
              lcd.write(1);
            } 
            
            else {
              lcd.setCursor(0, 0);
              lcd.print(' ');
            }
      
            if (getChannelAfter(topChannel2, true, false) != NULL) { 
              byte downArrow[] = { B00100, B00100, B00100, B00100, B00100, B11111, B01110, B00100 }; //UDCHARS extension, creates down arrow 
              lcd.createChar(0, downArrow);
              lcd.setCursor(0, 1);
              lcd.write(0);
            } 
            
            else {
              lcd.setCursor(0, 1);
              lcd.print(' ');
            }
      
            button_state = lcd.readButtons();
            last_b = button_state; 
            
            if(button_state & BUTTON_SELECT){
             select_press_time = millis(); 
             state = BEFORE_SEL;
            } 
      
            else if(button_state & BUTTON_UP){
              channel *beforeChannel = getChannelBefore(topChannel, true, false); 
              if(beforeChannel != NULL){
                botChannel = topChannel;
                topChannel = beforeChannel; //switches the top channel to the bottom line, and gets the channel before to be displayed on the top line  
              }
              state = AT_UP; 
            }
            
            else if(button_state & BUTTON_DOWN){
              channel *afterChannel = getChannelAfter(topChannel, true, false); //calls on the getChannelAfter function
              if(botChannel != NULL){ 
                topChannel = botChannel; 
                botChannel = getChannelAfter(topChannel, true, false); //switches the bottom channel to the top line, and gets the channel after to be displayed on the bottom line 
                if (botChannel == NULL) { 
                  lcd.setCursor(1, 1); 
                  lcd.print(F("                ")); 
                }
              }
              state = AT_DOWN; 
            }
          }
       
        }

      state = AFTER_RIGHT; 
      
      }
      
      break;


    case AFTER_RIGHT:
      button_state = lcd.readButtons(); 
      released = ~button_state & last_b; 
      last_b = button_state; 

      if(released & BUTTON_RIGHT){
        Serial.println("DEBUG: Right pressed & released - AFTER_RIGHT"); 
        state = MAIN; 
      } 

      
  } //end of switch-case 


} //end of loop function 
