#include <Bounce2.h>
#include <Rotary.h>
#include <WS2812.h>
#include <EEPROM.h>

#define MAIN_BUTTON_PIN 9
#define YELLOW_LIGHT_PIN 3 //PWM dla zoltej
#define WHITE_LIGHT_PIN 5 //PWM dla bialej

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

byte menuState = 0;	//Tryb swiatla zoltego
bool isLightOn = false; //Wylaczone jest wszystko

byte yellowLightPercentage = 100;
byte whiteLightPercentage = 100;

int colorNumber = 0;

void setup() {
	pinMode(MAIN_BUTTON_PIN,INPUT_PULLUP);
	pinMode(ENCODER_BUTTON_PIN,INPUT_PULLUP);

	pinMode(YELLOW_LIGHT_PIN,OUTPUT);//Ustawienie wyjsc
	pinMode(WHITE_LIGHT_PIN,OUTPUT);
	LED.setOutput(COLOR_LIGHT_PIN);
	LED.sync();

	//Wylaczenie wszystkiego
	digitalWrite(YELLOW_LIGHT_PIN,LOW);
	digitalWrite(WHITE_LIGHT_PIN,LOW);
	setColourRgb(0, 0, 0);

	randomSeed(analogRead(0));    
}

void loop() {
	unsigned char result = firstEncoder.process(); // Update enkodera DIR_CW - w prawo, DIR_CCW - lewo, DIR_NONE - nic
  
	if(mainButton.update()){//Update przycisku głównego
		if(mainButton.fell()){//Zmiana stanu z wysokiego na niski (przycisniecie)
			isLightOn = !isLightOn;
			if(isLightOn){
				//Wczytanie zapisanych ustawien
				yellowLightPercentage = EEPROM.read(0);
				whiteLightPercentage = EEPROM.read(1);
				rgbColour[0] = EEPROM.read(2);
				rgbColour[1] = EEPROM.read(3);
				rgbColour[2] = EEPROM.read(4);
        colorNumber = ((EEPROM.read(5) << 0) & 0xFF) + ((EEPROM.read(6) << 8) & 0xFF00);
			}
		}
	}
	
	if(rotaryButton.update()){//Update przycisku encodera
		if(rotaryButton.fell()){//Zmiana stanu z wysokiego na niski (przycisniecie)
			buttonTimer = millis();
		}
	}
	if(rotaryButton.rose()){
		if ((millis() - buttonTimer > longPressTime)) {//Dlugie przytrzymanie przycisku encodera - zapis ustawien
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
		else{//krotkie nacisniecie
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
		
	if(!isLightOn){//Wylaczenie wszystkiego - zgaszone oswietlenie
		digitalWrite(YELLOW_LIGHT_PIN,LOW);
		digitalWrite(WHITE_LIGHT_PIN,LOW);
		colorNumber = 0;
		setColourRgb(0, 0, 0);
		menuState = 0;
	}
	else{//Światła są włączone
		//menu
		if(menuState == 0){ //zolte światło
			if(result == DIR_CCW){//Przekrecenie w lewo
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
		if(menuState == 1){ // kolorowe swiatlo
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
		if(menuState == 2){ //biale światło
			if(result == DIR_CCW){//Przekrecenie w lewo
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
