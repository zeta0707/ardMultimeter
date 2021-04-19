#include <LiquidCrystal.h>
//Test for git push
//LCD interface
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//Key message, A0 read -> 5 keys
char msgs[5][15] = {"Right Key OK ", 
					"Up Key OK    ", 
					"Down Key OK  ", 
					"Left Key OK  ", 
					"Select Key OK" };
#define __Due__
#if defined(__Due__)
    static float adcRef = 3.3;
    int adc_key_val[5] ={100, 300, 600, 900, 1023};
  
#else
    static float adcRef = 5.0;
    int adc_key_val[5] ={30, 150, 360, 535, 760 ;  
#endif

int NUM_KEYS = 5;
int adc_key_in;
int key=-1;			//debounce
int oldkey=-1;		//debounce
const int KEY_PIN = A0;

//capcitance check
const int OUT_PIN = A2;
const int IN_PIN = A1;
const float IN_STRAY_CAP_TO_GND = 24.48;
const float IN_CAP_TO_GND  = IN_STRAY_CAP_TO_GND;
const float R_PULLUP = 34.8;  
const int MAX_ADC_VALUE = 1023;

//voltage check
float input_voltage = 0.0;
float temp=0.0;
float r1=10000.0;
float r2=5000.0;
//resistance check
float input_res = 0.0;
float r3=15000.0;

//Multimeter functions
int iFunction=0;
const int MAX_FUNCTION=3;				//1: voltage, 2: capacitance, 3: resistance

void setup()
{
    Serial.begin(9600);
    pinMode(OUT_PIN, OUTPUT);			//initial settup
    pinMode(IN_PIN, OUTPUT);			//initial settup
    pinMode(KEY_PIN, INPUT);	
    lcd.begin(16, 2);
    lcd.setCursor(0,0);
    lcd.print("DIGIT Multimeter");
    lcd.setCursor(0,1);
    lcd.print("By ZETA");
}

void loop()
{		
    adc_key_in = analogRead(KEY_PIN);   // read the value from the sensor
    key = get_key(adc_key_in); 			// convert into key press
    Serial.print(adc_key_in); Serial.print(":"); Serial.println(key);   
       
    if (key != oldkey) 					// if keypress is detected
    {
        delay(50);  					// wait for debounce time
        adc_key_in = analogRead(KEY_PIN);   	// read the value from the sensor
        key = get_key(adc_key_in);    	// convert into key press
        
        if (key != oldkey)		oldkey = key;
        if(key==1) iFunction++;
        if(key==2) iFunction--;
        if (iFunction <1) iFunction = 1;
        else if(iFunction > MAX_FUNCTION) iFunction = MAX_FUNCTION;
        lcd.clear();
    }

    if (iFunction == 1) { //voltage meter
        pinMode(A2, INPUT);
        //Conversion formula
        int analog_value = analogRead(A2);
        temp = (analog_value * adcRef) / 1024.0; 
        input_voltage = temp / (r2/(r1+r2));
        
        if (input_res < 0.1) input_voltage=0.0;
         
        lcd.setCursor(0,0);
        lcd.print(F("Voltage = "));  
        lcd.setCursor(0, 1);
        lcd.print(input_voltage,3);
        lcd.print("Volt");
        //Serial.print("Volt =");Serial.println(input_voltage); 
    }
    else if (iFunction == 2) { //resistance meter
        pinMode(A2, INPUT);
        //Conversion formula
        int analog_value = analogRead(A2);
        temp = (analog_value * adcRef) / 1024.0; 
        input_res = ((adcRef - temp) * r3) / temp;
        //R1 = (Vref - Vout) * R2 /Vout
        if (input_res < 0.1) input_res=0.0;
         
        lcd.setCursor(0,0);
        lcd.print(F("Resitance = "));  
        lcd.setCursor(0, 1);
        lcd.print(input_res,2);
        lcd.print("Ohm");
        //Serial.print("Resitance =");Serial.println(input_res); 
    } 

    else if(iFunction == 3) {

        pinMode(IN_PIN, INPUT);
        digitalWrite(OUT_PIN, HIGH);
        int val = analogRead(IN_PIN);
        digitalWrite(OUT_PIN, LOW);
    
        if (val < 1000)
        {
            pinMode(IN_PIN, OUTPUT);
            
            float capacitance = (float)val * IN_CAP_TO_GND / (float)(MAX_ADC_VALUE - val);
            
            lcd.setCursor(0,0);
            lcd.print(F("Capacitance = "));
            lcd.setCursor(0,1);
            lcd.print(capacitance, 3);
            lcd.print(F("pF "));
            lcd.print(val);
            lcd.print("mS");
        }
        
        else
        {
            pinMode(IN_PIN, OUTPUT);
            delay(1);
            pinMode(OUT_PIN, INPUT_PULLUP);
            unsigned long u1 = micros();
            unsigned long t;
            int digVal;
            
            do
            {
                digVal = digitalRead(OUT_PIN);
                unsigned long u2 = micros();
                t = u2 > u1 ? u2 - u1 : u1 - u2;
            } 
        
            while ((digVal < 1) && (t < 400000L));
            
            pinMode(OUT_PIN, INPUT);  
            val = analogRead(OUT_PIN);
            digitalWrite(IN_PIN, HIGH);
            int dischargeTime = (int)(t / 1000L) * 5;
            delay(dischargeTime);   
            pinMode(OUT_PIN, OUTPUT);  
            digitalWrite(OUT_PIN, LOW);
            digitalWrite(IN_PIN, LOW);
        
            float capacitance = -(float)t / R_PULLUP
                          / log(1.0 - (float)val / (float)MAX_ADC_VALUE);
            
            lcd.setCursor(0,0);
            lcd.print(F("Capacitance = "));
            if (capacitance > 1000.0)
            {
                lcd.setCursor(0,1);
                lcd.print(capacitance / 1000.0, 2);
                lcd.print(F("uF "));
                lcd.print(val);
                lcd.print("mS");
            }
            
            else
            {
                lcd.setCursor(0,1);
                lcd.print(capacitance, 2);
                lcd.print(F("nF "));
                lcd.print(val);
                lcd.print("mS");
            }
        }
        while (millis() % 1000 != 0);
    }
    delay(300); 
}

// Convert ADC value to key number
int get_key(unsigned int input)
{
	int k;    
	for (k = 0; k < NUM_KEYS; k++)
	{
		if (input < adc_key_val[k])	{           
		    return k;
        }  
	}		
    if (k >= NUM_KEYS)	k = -1;     		// No valid key pressed
    return k;
}
