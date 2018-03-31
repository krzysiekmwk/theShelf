#include <Bounce2.h>
#include <Rotary.h>
#include <WS2812.h>
#include <EEPROM.h>

#define MAIN_BUTTON_PIN 9
#define YELLOW_LIGHT_PIN 3 //PWM for YELLOW
#define WHITE_LIGHT_PIN 5 //PWM for WHITE

#define COLOR_LIGHT_PIN 4 //WS2812b
#define COLOR_LED_COUNT 60

#define ENCODER_LEFT_PIN 6
#define ENCODER_RIGHT_PIN 7
#define ENCODER_BUTTON_PIN 8

WS2812 LED(COLOR_LED_COUNT); 
cRGB value;
unsigned int rgbColour[3];
 
Bounce mainButton(MAIN_BUTTON_PIN,15);

Bounce rotaryButton(ENCODER_BUTTON_PIN,15);
long buttonTimer = 0;
long longPressTime = 500;

Rotary firstEncoder = Rotary(ENCODER_LEFT_PIN, ENCODER_RIGHT_PIN);

byte menuState = 0;	//the yellow state
bool isLightOn = false; //everything is turned off

byte yellowLightPercentage = 100;
byte whiteLightPercentage = 100;

int colorNumber = 0;

void setup() {
	pinMode(MAIN_BUTTON_PIN,INPUT_PULLUP);
	pinMode(ENCODER_BUTTON_PIN,INPUT_PULLUP);

	pinMode(YELLOW_LIGHT_PIN,OUTPUT);//Setup outputs
	pinMode(WHITE_LIGHT_PIN,OUTPUT);
	LED.setOutput(COLOR_LIGHT_PIN);
	LED.sync();

	//turning off everything
	digitalWrite(YELLOW_LIGHT_PIN,LOW);
	digitalWrite(WHITE_LIGHT_PIN,LOW);
	setColourRgb(0, 0, 0);

	randomSeed(analogRead(0));    
}

void loop() {
	unsigned char result = firstEncoder.process(); // Update enkoder DIR_CW - right, DIR_CCW - left, DIR_NONE - nothing
  
	if(mainButton.update()){//Update main button
		if(mainButton.fell()){//Change state to LOW (button pressed)
			isLightOn = !isLightOn;
			if(isLightOn){
				//read saved settings
				yellowLightPercentage = EEPROM.read(0);
				whiteLightPercentage = EEPROM.read(1);
				rgbColour[0] = EEPROM.read(2);
				rgbColour[1] = EEPROM.read(3);
				rgbColour[2] = EEPROM.read(4);
        			colorNumber = ((EEPROM.read(5) << 0) & 0xFF) + ((EEPROM.read(6) << 8) & 0xFF00);
			}
		}
	}
	
	if(rotaryButton.update()){//update encoder button
		if(rotaryButton.fell()){//Change state to LOW (button pressed)
			buttonTimer = millis();
		}
	}
	if(rotaryButton.rose()){
		if ((millis() - buttonTimer > longPressTime)) {//long pressend encoder button - save settings
			if(menuState == 0){
				EEPROM.write(0, yellowLightPercentage);
			}
			if(menuState == 1){
				EEPROM.write(2, rgbColour[0]);
				EEPROM.write(3, rgbColour[1]);
				EEPROM.write(4, rgbColour[2]);
				EEPROM.write(5, (colorNumber >> 0) & 0xFF);
				EEPROM.write(6, (colorNumber >> 8) & 0xFF);
			}
			if(menuState == 2){
				EEPROM.write(1, whiteLightPercentage);
			}
		}
		else{//short click
			if(!isLightOn){
				isLightOn = !isLightOn;
				//Wczytanie zapisanych ustawien
				yellowLightPercentage = EEPROM.read(0);
				whiteLightPercentage = EEPROM.read(1);
				rgbColour[0] = EEPROM.read(2);
				rgbColour[1] = EEPROM.read(3);
				rgbColour[2] = EEPROM.read(4);
				colorNumber = ((EEPROM.read(5) << 0) & 0xFF) + ((EEPROM.read(6) << 8) & 0xFF00);
			}
      			else{
        			switch(menuState){
					case 0: menuState++; break;
					case 1: menuState++; break;
					case 2: menuState = 0; break;
				}
      			}
		}
	}
		
	if(!isLightOn){//turning everything off
		digitalWrite(YELLOW_LIGHT_PIN,LOW);
		digitalWrite(WHITE_LIGHT_PIN,LOW);
		colorNumber = 0;
		setColourRgb(0, 0, 0);
		menuState = 0;
	}
	else{//ligt are on
		//menu
		if(menuState == 0){ //yellow light
			if(result == DIR_CCW){//rotary in left
				if(yellowLightPercentage > 0)
					yellowLightPercentage -= 5;
				else
					yellowLightPercentage = 0;
			}
			if(result == DIR_CW){
				if(yellowLightPercentage < 100)
					yellowLightPercentage += 5;
				else
					yellowLightPercentage = 100;
			}
			analogWrite(YELLOW_LIGHT_PIN,map(yellowLightPercentage, 0, 100, 0, 255));
			digitalWrite(WHITE_LIGHT_PIN,LOW);
			setColourRgb(0, 0, 0);
		}
		if(menuState == 1){ // colorfull lights (ws2812b)
			digitalWrite(YELLOW_LIGHT_PIN,LOW);
			digitalWrite(WHITE_LIGHT_PIN,LOW);
			if(result == DIR_CCW){
       				colorNumber+=10;
				if(colorNumber > 750)
					colorNumber = 0;

				if(colorNumber == 0 || colorNumber == 750){
					rgbColour[0] = 0;//RED
					rgbColour[1] = 0;//GREEN
					rgbColour[2] = 250;//BLUE
				}
				if(colorNumber > 0 && colorNumber < 250){//R = 0, G = 0, B = 255
					rgbColour[1] += 10;//GREEN
					rgbColour[2] -= 10;//BLUE
				}
				if(colorNumber == 250){
					rgbColour[0] = 0;//RED
					rgbColour[1] = 250;//GREEN
					rgbColour[2] = 0;//BLUE
				}
				if(colorNumber > 250 && colorNumber < 500){//R = 0, G = 255, B = 0
					rgbColour[0] += 10;//RED
					rgbColour[1] -= 10;//GREEN
				}
				if(colorNumber == 500){
					rgbColour[0] = 250;//RED
					rgbColour[1] = 0;//GREEN
					rgbColour[2] = 0;//BLUE
				}
				if(colorNumber > 500 && colorNumber < 750){//R = 255, G = 0, B = 0
					rgbColour[2] += 10;//BLUE
					rgbColour[0] -= 10;//RED
				}
			}

			if(result == DIR_CW){
				colorNumber-=10;
				if(colorNumber < 0)
					colorNumber = 750;

				if(colorNumber == 0 || colorNumber == 750){
					rgbColour[0] = 0;//RED
					rgbColour[1] = 0;//GREEN
					rgbColour[2] = 250;//BLUE
				}
				if(colorNumber > 500 && colorNumber < 750){//R = 0, G = 0, B = 255
					rgbColour[0] += 10;//RED
					rgbColour[2] -= 10;//BLUE
				}
				if(colorNumber == 500){
					rgbColour[0] = 250;//RED
					rgbColour[1] = 0;//GREEN
					rgbColour[2] = 0;//BLUE
				}
				if(colorNumber > 250 && colorNumber < 500){//R = 255, G = 0, B = 0
					rgbColour[0] -= 10;//RED
					rgbColour[1] += 10;//GREEN
				}
				if(colorNumber == 250){
					rgbColour[0] = 0;//RED
					rgbColour[1] = 250;//GREEN
					rgbColour[2] = 0;//BLUE
				}
				if(colorNumber > 0 && colorNumber < 255){//R = 0, G = 255, B = 0
					rgbColour[1] -= 10;//GREEN
					rgbColour[2] += 10;//BLUE
				}

			}
			setColourRgb(rgbColour[0], rgbColour[1], rgbColour[2]);
		}
		if(menuState == 2){ //white light
			if(result == DIR_CCW){//turn in left
				if(whiteLightPercentage > 0)
					whiteLightPercentage -= 5;
				else
					whiteLightPercentage = 0;
			}
			if(result == DIR_CW){
				if(whiteLightPercentage < 100)
					whiteLightPercentage += 5;
				else
					whiteLightPercentage = 100;
			}
			analogWrite(WHITE_LIGHT_PIN,map(whiteLightPercentage, 0, 100, 0, 255));
			digitalWrite(YELLOW_LIGHT_PIN,LOW);
			setColourRgb(0, 0, 0);
		}
	}
}

void setColourRgb(unsigned int red, unsigned int green, unsigned int blue) {
	value.b = blue; 
	value.g = green; 
	value.r = red;

	int i = 0;
	while (i < COLOR_LED_COUNT){
		LED.set_crgb_at(i, value);
		i++;
	}


	LED.sync(); // Sends the data to the LEDs
}
