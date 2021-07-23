/*  
 *  RFID 13.56 MHz / NFC Module
 *  
 *  Copyright (C) Libelium Comunicaciones Distribuidas S.L. 
 *  http://www.libelium.com 
 */

#include "arduPi.h"


uint8_t init(uint8_t *UID , uint8_t *ATQ);
uint8_t authenticate(uint8_t *UID, uint8_t blockAddress, uint8_t *keyAccess);
uint8_t writeData(uint8_t address, uint8_t *blockData);
uint8_t readData(uint8_t address, uint8_t *readData);
bool getFirmware(void);
void print(uint8_t * _data, uint8_t length);
bool configureSAM(void);
void sendTX(uint8_t *dataTX, uint8_t length, uint8_t outLength);
void getACK(void);
void waitResponse(void);
void getData(uint8_t outLength);
void checkSum(uint8_t *dataTX);
uint8_t lengthCheckSum(uint8_t *dataTX);
uint8_t setKeys(uint8_t *CardID,uint8_t *keyOld,uint8_t *keyA,uint8_t *keyB,uint8_t *config,uint8_t *data,uint8_t add);

uint8_t dataRX[35];//Receive buffer.
uint8_t dataTX[35];//Transmit buffer.
//stores the status of the executed command:
short state;
//auxiliar buffer:
unsigned char aux[16];
//stores the UID (unique identifier) of a card:
unsigned char _CardID[4];
//stores the key or password:
unsigned char keyOld[]= {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};// Old key access.
unsigned char keyA[]= {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF1};//NEW KEY ACCESS.
unsigned char keyB[]= {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};//Secundary KEY ACCESS

//only edit config if strong knowledge about the RFID/NFC module
unsigned char config[] = {0xFF, 0x07, 0x80, 0x69};
int address = 3;

void setup()
{
	// start serial port 115200 bps:
	Serial.begin(115200);
	delay(100);

	printf("RFID/NFC @ 13.56 MHz module started\n");
 delay(1000);

//! It is needed to launch a simple command to sycnchronize
	getFirmware();
	configureSAM();
}

void loop()
{
	printf("\n++++++++++++++++++++++++++++++++++++\n");
	// **** init the RFID/NFC reader
	state = init(_CardID, aux);
	if (aux[0] == aux[1]) {// if so, there is no card on the EM field
		printf("Request error - no card found\n");
	} else { // a card was found

		printf("Request | Answer: \n");
		print(aux, 2);  //show the ATQ

		printf("AnticollSelect: \n");
		printf("%x",state);//show the status of the executed init command(should be 0)
		printf(" | Answer: ");
		print(_CardID, 4); //show the UID (Unique IDentifier) of the read card (4 bytes)

	//Write new key in block then authenticate this address and get the new value
		state = setKeys(_CardID, keyOld, keyA, keyB, config, aux, address);
		printf("\n**write new key, state: ");
		printf("%x",state);

		if (state == 0) {
			printf(" --> correct key change ");
		} else {
			printf(" --> ** ERROR: keys not changed ** ");
		}	 // ###### check block 3/7/11/15 (the keys will return 0s)
			// **** authenticate again, now with the new key A

		state = authenticate(_CardID, address , keyA);
		printf("\n**Authentication block: ");
		printf("%x",state);

		if (state == 0) {
			printf(" --> correct authentication ");
		}else {
			printf(" --> ** ERROR: bad authentication ** ");
		}

	}
	delay(5000); // wait 10 seconds in each loop
} 

//**********************************************************************
//!The goal of this command is to detect as many targets (maximum MaxTg)
//  as possible in passive mode.
uint8_t init(uint8_t *CardID , uint8_t *ATQ)   //! Request InListPassive
{
  Serial.flush();

	dataTX[0] = 0x04; // Length
	lengthCheckSum(dataTX); // Length Checksum
	dataTX[2] = 0xD4;
	dataTX[3] = 0x4A; // Code
	dataTX[4] = 0x01; //MaxTarget
	dataTX[5] = 0x00; //BaudRate = 106Kbps
	dataTX[6] = 0x00; // Clear checkSum position
	checkSum(dataTX); 

	sendTX(dataTX , 7 ,23);	

	for (int i = 17; i < (21) ; i++){
		_CardID[i-17] = dataRX[i];
		CardID[i-17] = _CardID[i-17];
	}

	ATQ[0] = dataRX[13];
	ATQ[1] = dataRX[14];

	if ((dataRX[9]== 0xD5) & (dataRX[10] == 0x4B) & (dataRX[11] == 0x01)) {
		return 0;
	} else {
		return 1;
	}
}
//**********************************************************************
 //!A block must be authenticated before read and write operations
uint8_t authenticate(uint8_t *CardID, uint8_t blockAddress, uint8_t *keyAccess)
{
	dataTX[0] = 0x0F;
	lengthCheckSum(dataTX);
	dataTX[2] = 0xD4;
	dataTX[3] = 0x40; // inDataEchange
	dataTX[4] = 0x01; //Number of targets
	dataTX[5] = 0x60; // Authentication code
	dataTX[6] = blockAddress;
	for (int i = 0; i < 6 ; i++) {
		dataTX[i + 7] = keyAccess[i];
	}
	dataTX[13] = CardID[0];  dataTX[14] = CardID[1];
	dataTX[15] = CardID[2];  dataTX[16] = CardID[3];
	dataTX[17] = 0x00;
	checkSum(dataTX);
	sendTX(dataTX , 18 ,14);

	if ((dataRX[9]== 0xD5) & (dataRX[10] == 0x41) & (dataRX[11] == 0x00)) {
		return 0;
	} else {
		return 1;
	}
}
//**********************************************************************
//!Write 16 bytes in address .
uint8_t writeData(uint8_t address, uint8_t *blockData)  //!Writing
{
	Serial.print("                ");
	dataTX[0] = 0x15;
	lengthCheckSum(dataTX); // Length Checksum
	dataTX[2] = 0xD4;
	dataTX[3] = 0x40;  //inDataEchange CODE
	dataTX[4] = 0x01;  //Number of targets
	dataTX[5] = 0xA0; //Write Command
	dataTX[6] = address; //Address		

	for (int i = 0; i < 16; i++) {
		dataTX[i+7] = blockData[i];
	}

	dataTX[23] = 0x00;
	checkSum(dataTX);
	sendTX(dataTX , 24 ,14);  		

	if ((dataRX[9]== 0xD5) & (dataRX[10] == 0x41) & (dataRX[11] == 0x00)) {
		return 0;
	} else {
		return 1;
	}
}
//**********************************************************************
//!Read 16 bytes from  address .
uint8_t readData(uint8_t address, uint8_t *readData) //!Reading
{
	Serial.print("                ");		

	dataTX[0] = 0x05;
	lengthCheckSum(dataTX); // Length Checksum
	dataTX[2] = 0xD4; // Code
	dataTX[3] = 0x40; // Code
	dataTX[4] = 0x01; // Number of targets
	dataTX[5] = 0x30; //ReadCode
	dataTX[6] = address;  //Read address
	dataTX[7] = 0x00;
	checkSum(dataTX);
	sendTX(dataTX , 8, 30);
	memset(readData, 0x00, 16);  

	if ((dataRX[9]== 0xD5) & (dataRX[10] == 0x41) & (dataRX[11] == 0x00)) {
		for (int i = 12; i < 28; i++) {
			readData[i-12] = dataRX[i];
		}
		return 0;
	} else {
		return 1;
	}
}
//**********************************************************************
//!The PN532 sends back the version of the embedded firmware.
bool getFirmware(void)  //! It is needed to launch a simple command to sycnchronize
{
	Serial.print("                ");		

	memset(dataTX, 0x00, 35);
	dataTX[0] = 0x02; // Length
	lengthCheckSum(dataTX); // Length Checksum
	dataTX[2] = 0xD4; // CODE
	dataTX[3] = 0x02; //TFI
	checkSum(dataTX); //0x2A; //Checksum   

	sendTX(dataTX , 5 , 17);
	printf("\n");
	printf("Your Firmware version is : ");

	for (int i = 11; i < (15) ; i++){
		printf("%x",dataRX[i]);
		printf(" ");
	}
	printf("\n");
}

//**********************************************************************
//!Print data stored in vectors .
void print(uint8_t * _data, uint8_t length)
{
	for (int i = 0; i < length ; i++){
		printf("%x",_data[i]);
		printf(" ");
	}
	printf("\n");
}
//**********************************************************************
//!This command is used to set internal parameters of the PN532,
bool configureSAM(void)//! Configure the SAM
{
	Serial.print("               ");

	dataTX[0] = 0x05; //Length
	lengthCheckSum(dataTX); // Length Checksum
	dataTX[2] = 0xD4;
	dataTX[3] = 0x14;
	dataTX[4] = 0x01; //Normal mode
	dataTX[5] = 0x14; // TimeOUT
	dataTX[6] = 0x00; // IRQ
	dataTX[7] = 0x00; // Clean checkSum position
	checkSum(dataTX);

	sendTX(dataTX , 8, 13);
}
//**********************************************************************
//!Send data stored in dataTX
void sendTX(uint8_t *dataTX, uint8_t length, uint8_t outLength)
{
	Serial.print(0x00, BYTE);
	Serial.print(0x00, BYTE);
	Serial.print(0xFF, BYTE); 

	for (int i = 0; i < length; i++) {
		Serial.print(dataTX[i], BYTE);
	}

	Serial.print(0x00, BYTE);
	getACK();
	waitResponse();    // 1C - receive response
	getData(outLength);
}
//**********************************************************************
//!Wait for ACK response and stores it in the dataRX buffer
void getACK(void)
{
	delay(5);
	waitResponse();
	for (int i = 0; i < 5 ; i++) {
		dataRX[i] = Serial.read();
	}
}
//**********************************************************************
//!Wait the response of the module
void waitResponse(void)
{
	int val = 0xFF;
	int cont = 0x00;
	while(val != 0x00) {
		val = Serial.read();
		delay(5);
		cont ++;
	}
}
//**********************************************************************
//!Get data from the module
void getData(uint8_t outLength)
{
	for (int i=5; i < outLength; i++) {
		dataRX[i] = Serial.read(); // read data from the module.
	}
}
//**********************************************************************
//!Calculates the checksum and stores it in dataTX buffer
void checkSum(uint8_t *dataTX)
{
	for (int i = 0; i < dataTX[0] ; i++) {
		dataTX[dataTX[0] + 2] += dataTX[i + 2];
	}
  byte(dataTX[dataTX[0] + 2]= - dataTX[dataTX[0] + 2]);
}
//**********************************************************************
//!Calculates the length checksum and sotres it in the buffer.
uint8_t lengthCheckSum(uint8_t *dataTX)
{
	dataTX[1] = byte(0x100 - dataTX[0]);
}
//**********************************************************************
//Changes both keys and access conditions to one card's sector
uint8_t setKeys(uint8_t *CardID,uint8_t *keyOld,uint8_t *keyA,uint8_t *keyB,uint8_t *config,uint8_t *data,uint8_t add)
{
	uint8_t state = 1; 

	if (((add+1) % 4) == 0){
		state = authenticate(CardID , add, keyOld);
		if (state == 0) {
			for (int i = 0; i < 6; i++) {
				data[i] = keyA[i];
				data[i+ 10] = keyB[i];
			}

			for (int i = 0; i < 4 ; i++) {
				data[i + 6] = config[i];
			}
			state = writeData(add, data);
		}
	}
	return state;
}

int main (){
	setup();
	while(1){
		loop();
	}
	return (0);
}

    