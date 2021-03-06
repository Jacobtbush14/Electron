#include "fuelgauge.h"

FuelGauge::FuelGauge()
{

}

boolean FuelGauge::begin()
{
	// this should be unecessary since, begin is already called from pmic setup
	return 1;
}

// Read and return the cell voltage
float FuelGauge::getVCell() {

	byte MSB = 0;
	byte LSB = 0;

	readRegister(VCELL_REGISTER, MSB, LSB);
	int value = (MSB << 4) | (LSB >> 4);
	return map(value, 0x000, 0xFFF, 0, 50000) / 10000.0;
	//return value * 0.00125;
}

// Read and return the state of charge of the cell
float FuelGauge::getSoC() {

	byte MSB = 0;
	byte LSB = 0;

	readRegister(SOC_REGISTER, MSB, LSB);
	float decimal = LSB / 256.0;
	return MSB + decimal;

}

// Return the version number of the chip
int FuelGauge::getVersion() {

	byte MSB = 0;
	byte LSB = 0;

	readRegister(VERSION_REGISTER, MSB, LSB);
	return (MSB << 8) | LSB;
}

byte FuelGauge::getCompensateValue() {

	byte MSB = 0;
	byte LSB = 0;

	readConfigRegister(MSB, LSB);
	return MSB;
}

byte FuelGauge::getAlertThreshold() {

	byte MSB = 0;
	byte LSB = 0;

	readConfigRegister(MSB, LSB);
	return 32 - (LSB & 0x1F);
}

void FuelGauge::setAlertThreshold(byte threshold) {

	byte MSB = 0;
	byte LSB = 0;

	readConfigRegister(MSB, LSB);
	if(threshold > 32) threshold = 32;
	threshold = 32 - threshold;

	writeRegister(CONFIG_REGISTER, MSB, (LSB & 0xE0) | threshold);
}

// Check if alert interrupt was generated
boolean FuelGauge::getAlert() {

	byte MSB = 0;
	byte LSB = 0;

	readConfigRegister(MSB, LSB);
	return LSB & 0x20;
}

void FuelGauge::clearAlert() {

	byte MSB = 0;
	byte LSB = 0;

	readConfigRegister(MSB, LSB);
}

void FuelGauge::reset() {

	writeRegister(COMMAND_REGISTER, 0x00, 0x54);
}

void FuelGauge::quickStart() {

	writeRegister(MODE_REGISTER, 0x40, 0x00);
}


void FuelGauge::readConfigRegister(byte &MSB, byte &LSB) {

	readRegister(CONFIG_REGISTER, MSB, LSB);
}

void FuelGauge::readRegister(byte startAddress, byte &MSB, byte &LSB) {

    Wire3.beginTransmission(MAX17043_ADDRESS);
    Wire3.write(startAddress);
    Wire3.endTransmission(true);

    Wire3.requestFrom(MAX17043_ADDRESS, 2, true);
	MSB = Wire3.read();
	LSB = Wire3.read();
}

void FuelGauge::writeRegister(byte address, byte MSB, byte LSB) {

    Wire3.beginTransmission(MAX17043_ADDRESS);
    Wire3.write(address);
    Wire3.write(MSB);
    Wire3.write(LSB);
    Wire3.endTransmission(true);
}
