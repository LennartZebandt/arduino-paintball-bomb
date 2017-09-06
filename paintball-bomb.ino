#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <TimeLib.h>

const byte keypadRows= 4; 
const byte keypadCols= 4; 
const byte lcdAdress = 0x27;
const char keymap[keypadRows][keypadCols]= {
  {'1', '2', '3', 'A'}, 
  {'4', '5', '6', 'B'}, 
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
const byte keypadRowPins[keypadRows] = {9,8,7,6};
const byte keypadColPins[keypadCols]= {5,4,3,2};

const char settingsDigit = '#';
const char timeSettings = '1';
const char codeSettings = '2';
const char validSettingsDigits[] = {'1', '2'};
const char validTimerDigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
const char validCodeDigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D'};
const String explodeText = "!!!!!!!!!!!!!!!!";
const String tText = "Terrorists";
const String ctText = "Counter Terrorists";
const String winText = "win!";
const String setTimerText = "Set the time:";
const String setCodeText = "Set the code:";
const int buzzerPin = 10;
const int greenLedPin = 11;
const int redLedPin = 12;
const int buzzerTickFrequency = 1000;
const int buzzerButtonFrequency = 500;
const int buzzerExplodeFrequency = 4000;
const int buzzerTickPeriod = 100;
const int buzzerButtonPeriod = 50;

Keypad myKeypad= Keypad(makeKeymap(keymap), keypadRowPins, keypadColPins, keypadRows, keypadCols);
LiquidCrystal_I2C lcd(lcdAdress, 16, 2);

time_t startTime;
time_t timeLeft;
time_t lastTimeLeft;
int greenLedState;
int redLedPinState;
enum BombState {
  idle,
  armed,
  finished,
  settings,
  setTimer,
  setCode
};
BombState currentBombState;
int timerVal = 45;
String codeVal = "123";//7355608
String input;

void setup(){
  //Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(greenLedPin, HIGH);
  currentBombState = idle;  
}

void loop(){
  char keyPressed = myKeypad.getKey();

  if(keyPressed != NO_KEY){
    tone(buzzerPin, buzzerButtonFrequency, buzzerButtonPeriod);
  }
  
  switch(currentBombState){
    case idle:
        if(validateDigitInput(keyPressed, validCodeDigits, sizeof(validCodeDigits)/sizeof(validCodeDigits[0]))){
            input += keyPressed;
            overrideLcd(input, "");
            if(input.length() == codeVal.length()){
              if(validCode()){
                startTime = now();
                digitalWrite(greenLedPin, LOW);
                currentBombState = armed;  
              }
            }
        }else if(keyPressed == settingsDigit && input.length() == 0){
          input += keyPressed;
          overrideLcd(input, "");
          currentBombState = settings;
        }
        break;
    case armed:
        timeLeft = timerVal - (now() - startTime);
        if(timeLeft != lastTimeLeft){
          bombTick();
          lastTimeLeft = timeLeft;
        }
        
        if(timeLeft <= 0){
          explode();
          currentBombState = finished;
        }

        if(validateDigitInput(keyPressed, validCodeDigits, sizeof(validCodeDigits)/sizeof(validCodeDigits[0]))){
          input += keyPressed;
          overrideLcd(String(timeLeft), input);
          if(input.length() == codeVal.length()){
            if(validCode()){
              digitalWrite(redLedPin, LOW);
              digitalWrite(greenLedPin, HIGH);
              overrideLcd(ctText, winText);
              currentBombState = finished;
            }
          }
        }
        
        break;
    case finished:
        if(keyPressed == settingsDigit){
          digitalWrite(greenLedPin, HIGH);
          digitalWrite(redLedPin, LOW);
          overrideLcd("", "");
          currentBombState = idle;
        }
        break;
    case settings:
        if(validateDigitInput(keyPressed, validSettingsDigits, sizeof(validSettingsDigits)/sizeof(validSettingsDigits[0]))){
            input += keyPressed;
            overrideLcd(input, "");
            input = "";
            switch(keyPressed){
                case timeSettings:
                  overrideLcd(setTimerText, "");
                  currentBombState = setTimer;
                  break;
                case codeSettings:
                 overrideLcd(setCodeText, "");
                 currentBombState = setCode;
                 break;
            }
        }
        break;
    case setTimer:
        if(validateDigitInput(keyPressed, validTimerDigits, sizeof(validTimerDigits)/sizeof(validTimerDigits[0]))){
          input += keyPressed;
          overrideLcd(setTimerText, input);
        }else if(keyPressed == settingsDigit){
            timerVal = input.toInt();
            input = "";
            overrideLcd("", "");
            currentBombState = idle;
        }
        break;
    case setCode:
        if(validateDigitInput(keyPressed, validCodeDigits, sizeof(validCodeDigits)/sizeof(validCodeDigits[0]))){
          input += keyPressed;
          overrideLcd(setCodeText, input);
        }else if(keyPressed == settingsDigit){
            codeVal = input;
            input = "";
            overrideLcd("", "");
            currentBombState = idle;
        }
        break;
  }
}

boolean validCode(){
    String enteredCode = input;
    input = "";  
    overrideLcd("", "");
    if(enteredCode == codeVal){
      return true;
    }else{
      return false;  
    }
}

void bombTick(){
  overrideLcd(String(timeLeft), input);
  tone(buzzerPin, buzzerTickFrequency, buzzerTickPeriod);
  digitalWrite(redLedPin,  !digitalRead(redLedPin));  
}

void explode(){
  overrideLcd(explodeText, explodeText);
  tone(buzzerPin, buzzerExplodeFrequency);
  digitalWrite(redLedPin, HIGH);
  delay(5000);
  noTone(buzzerPin);
  overrideLcd(tText, winText);
  currentBombState = finished;
}

void overrideLcd(String firstLine, String secondLine){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(firstLine);
  lcd.setCursor(0,1);
  lcd.print(secondLine);
}

boolean validateDigitInput(char digit, char allowedDigits[], int allowedDigitsLen){
  if(digit == NO_KEY) return false;
  for(int i=0;i<allowedDigitsLen;i++){
    if(digit == allowedDigits[i]){
      return true;  
    }
  } 
  return false; 
}


