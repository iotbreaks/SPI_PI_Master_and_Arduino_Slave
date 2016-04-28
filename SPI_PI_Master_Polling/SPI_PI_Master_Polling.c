//#########################################################
//## PI_SPI_Master_Polling.c 
//## Created by Kenny from www.iotbreaks.vn, April 24, 2016.
//## Released into the public domain.
//## Tutorial: 
//#########################################################
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <wiringPi.h>
#include <errno.h>

#include <wiringPiSPI.h>
#define MOTOR_SWITCH_INPUT_PIN 21
#define LED_OUTPUT_PIN 22
#define TRANSMIT_BUTTON_INPUT_PIN 26

#define SPI_AMONG_WORDS_MICRO_SECONDS_DELAY 10


static int initializePiAsMaster();
static uint8_t* transceiver(int fileDescriptor, uint8_t sendingBytes[], uint8_t sendingByteLen);
void transmitTrigger(void);
void wiPiTransmitTrigger(void);
static uint32_t speed = 9600; // SPI clock Speed

int sendByteCount = 0;
uint8_t ledValue = 0;
uint8_t readLedPeriod = 0;
int realDelay=100;

unsigned char motorSwitchChar;
int main(int argc, char *argv[]) {
  	if (wiringPiSetup () < 0) {
  	    fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
  	    return 1;
  	}
	
	//if (wiringPiISR (TRANSMIT_BUTTON_INPUT_PIN, INT_EDGE_RISING, &transmitTrigger) < 0 ) {
    //	fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
    //	return 1;
  	//}
	
    pinMode(LED_OUTPUT_PIN, OUTPUT);     
    pinMode(MOTOR_SWITCH_INPUT_PIN, INPUT);

	while ( 1 ) {
		delay(readLedPeriod);
		ledValue = !ledValue ;
		digitalWrite(LED_OUTPUT_PIN, ledValue);
		if(digitalRead(TRANSMIT_BUTTON_INPUT_PIN)){
			delay(100);
			transmitTrigger();			
		}
	}
	return 1;
}

void transmitTrigger(void) {
	printf("Transmit Start\n");
	uint8_t motorSwitchValue = digitalRead(MOTOR_SWITCH_INPUT_PIN);
	unsigned char motorSwitchChar = motorSwitchValue?'H':'L';
	printf("motorSwitchValue = %d", motorSwitchChar);
	int fileDescriptor = initializePiAsMaster();
	if (fileDescriptor == 0) return ;
	unsigned char sendingData[] = {motorSwitchChar};		
	uint8_t sendingByteLen = 1;
	uint8_t* receivedBytes ;
	receivedBytes = transceiver(fileDescriptor, sendingData, sendingByteLen);
	readLedPeriod = receivedBytes[0];	
	close(fileDescriptor);
	printf("Transmit Finish\n");
	printf("readLedPeriod = %d\n", readLedPeriod);
	printf("sendByteCount = %d\n======================", ++sendByteCount);
}


// Return file descriptor of SPI
static int initializePiAsMaster(){ 
	const char* device = "/dev/spidev0.0";
	int fileDescriptor = open("/dev/spidev0.0", O_RDWR);
	if (fileDescriptor < 0){
		printf("can't open device\n");
		return 0;
	} else {
		printf("open device spidev0.0 successfully with file description = %d\n", fileDescriptor);
	}
	uint8_t mode =   ((uint8_t)((uint8_t)0 & SPI_READY))
				   | ((uint8_t)((uint8_t)0 & SPI_NO_CS)) 
				   | ((uint8_t)((uint8_t)0 & SPI_LOOP)) 
				   | ((uint8_t)((uint8_t)0 & SPI_3WIRE))
				   | ((uint8_t)((uint8_t)0 & SPI_LSB_FIRST))
				   | ((uint8_t)((uint8_t)0 & SPI_CS_HIGH))
				   | ((uint8_t)((uint8_t)0 & SPI_CPOL)) 
				   | ((uint8_t)((uint8_t)0 & SPI_CPHA)); 

	mode |= SPI_LOOP;
	int ret;
	ret = ioctl(fileDescriptor, SPI_IOC_WR_MODE, &mode);
	ret = ioctl(fileDescriptor, SPI_IOC_RD_MODE, &mode);
	
	uint8_t bitPerWord = 8;
	ret = ioctl(fileDescriptor, SPI_IOC_WR_BITS_PER_WORD, &bitPerWord); //8 bits/word
	ret = ioctl(fileDescriptor, SPI_IOC_RD_BITS_PER_WORD, &bitPerWord);
	
	ret = ioctl(fileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	ret = ioctl(fileDescriptor, SPI_IOC_RD_MAX_SPEED_HZ, &speed);

	return fileDescriptor;
}

// Send and Receive SPI use spidev
static uint8_t* transceiver(int fileDescriptor, uint8_t sendingBytes[],uint8_t sendingByteLen)
{
	static uint8_t receivedBytes[255] = {0, }; 
	struct spi_ioc_transfer transceiverMessage = {
		.tx_buf = (unsigned long)sendingBytes,
		.rx_buf = (unsigned long)receivedBytes,
		.len = sendingByteLen,
		.delay_usecs = SPI_AMONG_WORDS_MICRO_SECONDS_DELAY,
		.speed_hz = speed,
		.bits_per_word = 8,
	};

	int ret = ioctl(fileDescriptor, SPI_IOC_MESSAGE(1), &transceiverMessage);
	if (ret < 1)
		printf("can't send spi message");

	int i=0;
	for (i = 0; i < sendingByteLen; i++) {
		printf("Sending Byte: %d, char: %c", sendingBytes[i], sendingBytes[i]);
	}
	puts("");
	
	for (i = 0; i < sendingByteLen; i++) {
		printf("Received Byte: %d, char: %c", receivedBytes[i], receivedBytes[i]);
	}
	puts("");

	return receivedBytes;
}