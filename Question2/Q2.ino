/*This code was written and tested for Arduino Uno Board, simulated in Tinkercad. 
It is possible to run this code deploying on Arduino or please refer to the link below to run the design in Tinkercad
https://www.tinkercad.com/things/dFszccHorHI-question-2-circuit-implementation?sharecode=MWWaLcYScGQwvd8dmtPLBQibSqOFq5M_jG-IDzLy7gM
*/

const int buttonPin = 2;
const unsigned long protectionTime = 10000;

unsigned long pressStartTime = 0;
unsigned long pressedTime = 0;
unsigned long protectionStartTime = 0;

//Button reading and debounce ------------------
int lastButtonState = LOW;
int buttonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

int buttonIsPressed(){
  	int reading = digitalRead(buttonPin);
    if (reading != lastButtonState) lastDebounceTime = millis();  
  	if ((millis() - lastDebounceTime) >= debounceDelay){
      buttonState = reading;
  	}
 	lastButtonState = reading;
	return buttonState;
}

//FSM------------------------------------------
enum State{
	OFF,
  	ON,
  	PROTECTED
};

State currentState;

void setup(){
  Serial.begin(9600);
  Serial.println("Initial state is OFF");
  
  pinMode(buttonPin, INPUT);
  
  currentState = OFF;
}

void loop(){
  
  int pressed = buttonIsPressed();
  unsigned long currentTime = millis();
  
  switch(currentState){
    case OFF: {
      if (pressed){
        currentState = ON;
        Serial.println("Switched to ON");
        pressStartTime = currentTime;
      }
    } break;
    
    case ON: {
      if (!pressed){
        currentState = PROTECTED;
        protectionStartTime = currentTime;
        pressedTime = currentTime - pressStartTime;
        
        Serial.println("Switched to PROTECTED");
        Serial.print("Pressed time: ");
		Serial.print(pressedTime);
        Serial.println(" ms");
      }
    } break;
    
    case PROTECTED: {
      if((currentTime - protectionStartTime) >= protectionTime){
        currentState = OFF;
        Serial.println("Switched to OFF");
        Serial.println("================================");
      }
    } break;
  }
}