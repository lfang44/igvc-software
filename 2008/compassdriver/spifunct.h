#ifndef SPIFUNCT_H
#define SPIFUNCT_H

#include "compassTypes.h"
//#include "peekpoke.h"

//these were included in button.c, don't know if we need them
//the spi pins are on the dio header

#include <unistd.h>
#include <sys/compassTypes.h>
#include <sys/mman.h>
#include <fcntl.h>



sint_32 bytes2sint32LE(uint_8 * array) {//give it pointer to least signifiacant byte first (little endian)
	
	union sint_32_uint_8 {
		sint_32 sint32;
		uint_8 uint8[4];
	} outputunion;
	
	outputunion.uint8[0] = array[0];//lsb
	outputunion.uint8[1] = array[1];
	outputunion.uint8[2] = array[2];
	outputunion.uint8[3] = array[3];//msb

	return( outputunion.sint32 );
}

uint_8 * sint32_bytesLE(sint_32 input) {//give it sint_32, returns pointer to byte array, lsb first (little endian)
	union sint_32_uint_8 {
		sint_32 sint32;
		uint_8 uint8[4];
	} inputunion;
	
	inputunion.sint32 = input;

	uint_8 out1 = inputunion.uint8[0];//lsb
	uint_8 out2 = inputunion.uint8[1];
	uint_8 out3 = inputunion.uint8[2];
	uint_8 out4 = inputunion.uint8[3];//msb

	uint_8 output[4] = {out1, out2, out3, out4};
	return(output);
}

float bytes2floatLE(uint_8 * array) {
union float_uint_8 {
	float flt;
		uint_8 uint8[4];
	} outputunion;
		
	outputunion.uint8[0] = array[3];//haven't checked lsb order, but result seems correct
	outputunion.uint8[1] = array[2];
	outputunion.uint8[2] = array[1];
	outputunion.uint8[3] = array[0];
	
	return(outputunion.flt);
}

uint_8 * float2bytesLE(float input) {//lsb returned first (0=sign; 1-8=exponent;9-31=mantissa) -- this works, but don't know about order
	
	union float_uint_8 {
	float flt;
		uint_8 uint8[4];
	} inputunion;

	inputunion.flt = input;
	
	uint_8 out1 = inputunion.uint8[0];//lsb??
	uint_8 out2 = inputunion.uint8[1];
	uint_8 out3 = inputunion.uint8[2];
	uint_8 out4 = inputunion.uint8[3];

	uint_8 output[4] = {out1, out2, out3, out4};
	return(output);
}

bool checkbitset(short int foo, int i) {//from http://www.cs.umd.edu/class/spring2003/cmsc311/Notes/BitOp/bitI.html
	short int mask = 1 << i;
	return(mask & foo);
}

uint_8 char2uint_8(char foo) {
	union char2uint8 {
		uint_8 uint8;
		char chr;
		}out;
	out.chr = foo;
	return(out.uint8);
}

char uint_82char2(uint_8 * foo) {
	union char2uint8 {
		uint_8 uint8;
		char chr;
		}out;
	out.uint8 = foo[0];
	return(out.chr);
}


int CompassDriver::spiSend(uint_8 * data, int size) {
	spi_status_register status;
	status = spigetstatus();
	
	for(int i=0; i < size; i++) {
		while(!(status.transmit_empty_interrupt_flag)) {
			//wait for the send buffer to clear -- more than likly a better way to do this
		}
		*datareg = data[i];
	}
	return(0);
}


int CompassDriver::spiGet(uint_8 * dataresp, int size) {
	spi_status_register status;
	status = spigetstatus();
	//should i load 0x00 into the send buffer before i recive?
	for(int i=0; i < size; i++) {
		while(!(status.interrupt_flag)) {
			//wait for new data -- more than likly a better way to do this
		}
		dataresp[i] = *datareg;
	}
	return(0);
}


spi_status_register CompassDriver::spigetstatus() {
	spi_status_register statusval;
	statusval.byte = *statusreg;
	return(statusval);
}



int CompassDriver::spiinit() {
	//volatile unsigned int ctrlreg1, ctrlreg2, datareg, statusreg, clockprescalereg, interuptclearreg;//this is now in the class
	unsigned char *start;
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	start = (unsigned char*)mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x80840000);//this doesn't need to be cast from void in the example -- is this correct?

	//name the dio registers -- direct from button.c -- do i want these to be pointers?  the poke func expects an address
	volatile unsigned int *PEDR, *PEDDR, *PBDR, *PBDDR, *GPIOBDB;
	//these registers are for the ts-7200 dio pins, do i really need these? they are in the spi example, but the spi pins are elsewhere...
	PBDR = (unsigned int *)(start + 0x04);     // port b
	PBDDR = (unsigned int *)(start + 0x14);    // port b direction register
	PEDR = (unsigned int *)(start + 0x20);     // port e data
	PEDDR = (unsigned int *)(start + 0x24);    // port e direction register
	GPIOBDB = (unsigned int *)(start + 0xC4);  // debounce on port b

	*PBDDR = 0xf0;			      // upper nibble output, lower nibble input
	*PEDDR = 0xff;			     // all output (just 2 bits)
	*GPIOBDB = 0x01;			      // enable debounce on bit 0
	

	//name the spi registers -- these are the ones for the ts-7200, change <strike>if</strike> when we get docs for the board
	ctrlreg0 = (unsigned int *)(start + 0x6000);
	ctrlreg1 = (unsigned int *)(start + 0x6004);
	datareg = (unsigned int *)(start + 0x6008);
	statusreg = (unsigned int *)(start + 0x600C);
	clockprescalereg = (unsigned int *)(start + 0x6010);
	interuptclearreg = (unsigned int *)(start + 0x6014);
	
	//	
	close(fd);//is this the right place?

	//set config
	spi_control_0 spicntr0 = {0};
	spi_control_1 spicntr1 = {0};
	spi_baud_rate spibaudrt = {0};
	spi_status_register spistatus = {0};

	spicntr0.interupt_enable=0;
	spicntr0.sytem_enable=1;
	spicntr0.transmit_interrupt_enable=0;
	spicntr0.mode_select=1;
	spicntr0.clock_polarity=0;
	spicntr0.clock_phase=0;
	spicntr0.slave_select_out_enable=0;
	spicntr0.LSB_first=1;

	spicntr1.MODFEN=1;
	spicntr1.BIDROE=1;
	spicntr1.SPISWAI=1;
	spicntr1.SPC0=0;
	
	//spibaudrt.baud_rate_preselect=;
	//spibaudrt.baud_rate_select=;
	

	//load config -- use poke, or just directly '='?
	//poke8(clockprescalereg, spibaudrate);//verify that this is the right register before the test.  i'm lazy right now
	//poke8(ctrlreg0, spicntr1);
	//poke8(ctrlreg1, spicntr0);

	//*clockprescalereg = spibaudrate;//verify register
	*ctrlreg1 = spicntr1.byte;
	*ctrlreg0 = spicntr0.byte;

}

int CompassDriver::spioff() {
	//close(fd);//will this work across scope like this? -- in the example this is right after the dio pins are named, moved there
}

#endif // SPIFUNCT_H
