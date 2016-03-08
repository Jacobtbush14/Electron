#include "application.h"
#include "modem/mdm_hal.h"

#include "Serial3/Serial3.h"
#include "pmic.h"
#include "fuelgauge.h"


PMIC PMIC;
FuelGauge FUEL;
double socValue = 0;
String deviceName = "-2g";

extern MDMElectronSerial electronMDM;

// ALL_LEVEL, TRACE_LEVEL, DEBUG_LEVEL, WARN_LEVEL, ERROR_LEVEL, PANIC_LEVEL, NO_LOG_LEVEL
SerialDebugOutput debugOutput(115200, ALL_LEVEL);

/* Function prototypes -------------------------------------------------------*/
int tinkerDigitalRead(String pin);
int tinkerDigitalWrite(String command);
int tinkerAnalogRead(String pin);
int tinkerAnalogWrite(String command);

SYSTEM_MODE(SEMI_AUTOMATIC);

/* This function is called once at start up ----------------------------------*/
void setup()
{
	if (System.deviceID() == "5b0033000e51343035353132")
		deviceName = "-3g";

	RGB.brightness(10);
	Serial.begin(9600);
	PMIC.begin();
	FUEL.quickStart();
	Serial.println("");

	//D0 to D7
	for(uint8_t i = 0;i<8;i++){
		pinMode(i,OUTPUT);
		digitalWrite(i,LOW);
	}

	//A0 to A5
	for(uint8_t i = 10;i<17;i++){
		pinMode(i,OUTPUT);
		digitalWrite(i,LOW);
	}

	//B0 to B5, C0 to C5
	for(uint8_t i = 24;i<36;i++){
		pinMode(i,OUTPUT);
		digitalWrite(i,LOW);
	}

	electronMDM.setDebug(3); // enable this for debugging issues

	delay(3000);
    DEBUG_D("\e[0;36mHello from the Electron! Boot time is: %d\r\n",millis());

	Particle.connect(); // blocking call to connect

	//Register all the Tinker functions
	Particle.function("digitalread", tinkerDigitalRead);
  Particle.function("digitalwrite", tinkerDigitalWrite);

	Particle.function("analogread", tinkerAnalogRead);
	Particle.function("analogwrite", tinkerAnalogWrite);
	Particle.variable("soc",&socValue,DOUBLE);
	Particle.variable("test","test-string",STRING);

	PMIC.setInputCurrentLimit(150);
}

unsigned long old_time = 0;
unsigned long pubSoc = 0;
unsigned long pubTS	 = 0;


/* This function loops forever --------------------------------------------*/
void loop()
{
	if (millis() - old_time >= 500){
		socValue = FUEL.getSoC();

		PMIC.setInputCurrentLimit(500);
	//D0 to D7
		for(uint8_t i = 0;i<8;i++){
			digitalWrite(i,!digitalRead(i));
		}

		//A0 to A5
		for(uint8_t i = 10;i<17;i++){
			digitalWrite(i,!digitalRead(i));
		}

		//B0 to B5, C0 to C5
		for(uint8_t i = 24;i<36;i++){
			digitalWrite(i,!digitalRead(i));
		}
		old_time = millis();
	}

	if (millis() - pubSoc >= 5*60000){
		Particle.publish("soc"+deviceName, String(FUEL.getSoC()), 120, PRIVATE);
		pubSoc = millis();
	}

}

/*******************************************************************************
 * Function Name  : tinkerDigitalRead
 * Description    : Reads the digital value of a given pin
 * Input          : Pin
 * Output         : None.
 * Return         : Value of the pin (0 or 1) in INT type
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerDigitalRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(pin.startsWith("D"))
	{
		pinMode(pinNumber, INPUT_PULLDOWN);
		return digitalRead(pinNumber);
	}
	else if (pin.startsWith("A"))
	{
		pinMode(pinNumber+10, INPUT_PULLDOWN);
		return digitalRead(pinNumber+10);
	}
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerDigitalWrite
 * Description    : Sets the specified pin HIGH or LOW
 * Input          : Pin and value
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerDigitalWrite(String command)
{
	bool value = 0;
	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(command.substring(3,7) == "HIGH") value = 1;
	else if(command.substring(3,6) == "LOW") value = 0;
	else return -2;

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		digitalWrite(pinNumber, value);
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		digitalWrite(pinNumber+10, value);
		return 1;
	}
	else return -3;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogRead
 * Description    : Reads the analog value of a pin
 * Input          : Pin
 * Output         : None.
 * Return         : Returns the analog value in INT type (0 to 4095)
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerAnalogRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(pin.startsWith("D"))
	{
		return -3;
	}
	else if (pin.startsWith("A"))
	{
		return analogRead(pinNumber+10);
	}
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogWrite
 * Description    : Writes an analog value (PWM) to the specified pin
 * Input          : Pin and Value (0 to 255)
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerAnalogWrite(String command)
{
	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	String value = command.substring(3);

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		analogWrite(pinNumber, value.toInt());
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		analogWrite(pinNumber+10, value.toInt());
		return 1;
	}
	else return -2;
}