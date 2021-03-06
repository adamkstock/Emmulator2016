

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "15034312"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500
#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x10
#define FLAG_Z  0x04
#define FLAG_N  0x02
#define FLAG_C  0x01
#define REGISTER_A	3
#define REGISTER_B	2
#define REGISTER_H	1
#define REGISTER_L	0
#define REGISTER_M	4
#define REGISTER_X 0
#define REGISTER_Y 1

BYTE Index_Registers[2];
BYTE Registers[5];
BYTE Flags;

WORD ProgramCounter;
WORD StackPointer;


////////////
// Memory //
////////////

#define MEMORY_SIZE	65536

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x01FA
#define TEST_ADDRESS_2  0x01FB
#define TEST_ADDRESS_3  0x01FC
#define TEST_ADDRESS_4  0x01FD
#define TEST_ADDRESS_5  0x01FE
#define TEST_ADDRESS_6  0x01FF
#define TEST_ADDRESS_7  0x0200
#define TEST_ADDRESS_8  0x0201
#define TEST_ADDRESS_9  0x0202
#define TEST_ADDRESS_10  0x0203
#define TEST_ADDRESS_11  0x0204
#define TEST_ADDRESS_12  0x0205


///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;


///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][14] =
{
"ILLEGAL     ", 
"DECX impl    ", 
"INCX impl    ", 
"DEY impl     ", 
"INCY impl    ", 
"CLC impl     ", 
"STC impl     ", 
"CLI impl     ", 
"STI impl     ", 
"ILLEGAL     ", 
"LDAA  #      ", 
"LDAB  #      ", 
"LX  #,L      ", 
"LX  #,L      ", 
"LDX  #       ", 
"LDY  #       ", 

"JMP abs      ", 
"JCC abs      ", 
"JCS abs      ", 
"JNE abs      ", 
"JEQ abs      ", 
"JMI abs      ", 
"JPL abs      ", 
"JHI abs      ", 
"JLE abs      ", 
"ILLEGAL     ", 
"LDAA abs     ", 
"LDAB abs     ", 
"MVI  #,L     ", 
"MVI  #,H     ", 
"LDX abs      ", 
"LDY abs      ", 

"LODS  #      ", 
"JSR abs      ", 
"CCC abs      ", 
"CCS abs      ", 
"CNE abs      ", 
"CEQ abs      ", 
"CMI abs      ", 
"CPL abs      ", 
"CHI abs      ", 
"CLE abs      ", 
"LDAA abs,X   ", 
"LDAB abs,X   ", 
"NOP impl     ", 
"HLT impl     ", 
"LDX abs,X    ", 
"LDY abs,X    ", 

"LODS abs     ", 
"ADC A,L      ", 
"SBC A,L      ", 
"ADD A,L      ", 
"SUB A,L      ", 
"CMP A,L      ", 
"OR A,L       ", 
"AND A,L      ", 
"XOR A,L      ", 
"BIT A,L      ", 
"LDAA abs,Y   ", 
"LDAB abs,Y   ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"LDX abs,Y    ", 
"LDY abs,Y    ", 

"LODS abs,X   ", 
"ADC A,H      ", 
"SBC A,H      ", 
"ADD A,H      ", 
"SUB A,H      ", 
"CMP A,H      ", 
"OR A,H       ", 
"AND A,H      ", 
"XOR A,H      ", 
"BIT A,H      ", 
"LDAA (ind)   ", 
"LDAB (ind)   ", 
"RET impl     ", 
"ILLEGAL     ", 
"LDX (ind)    ", 
"LDY (ind)    ", 

"LODS abs,Y   ", 
"ADC A,M      ", 
"SBC A,M      ", 
"ADD A,M      ", 
"SUB A,M      ", 
"CMP A,M      ", 
"OR A,M       ", 
"AND A,M      ", 
"XOR A,M      ", 
"BIT A,M      ", 
"LDAA (ind,X) ", 
"LDAB (ind,X) ", 
"SWI impl     ", 
"RTI impl     ", 
"LDX (ind,X)  ", 
"LDY (ind,X)  ", 

"LODS (ind)   ", 
"ADC B,L      ", 
"SBC B,L      ", 
"ADD B,L      ", 
"SUB B,L      ", 
"CMP B,L      ", 
"OR B,L       ", 
"AND B,L      ", 
"XOR B,L      ", 
"BIT B,L      ", 
"STOS abs     ", 
"MOVE A,A     ", 
"MOVE B,A     ", 
"MOVE L,A     ", 
"MOVE H,A     ", 
"MOVE M,A     ", 

"LODS (ind,X) ", 
"ADC B,H      ", 
"SBC B,H      ", 
"ADD B,H      ", 
"SUB B,H      ", 
"CMP B,H      ", 
"OR B,H       ", 
"AND B,H      ", 
"XOR B,H      ", 
"BIT B,H      ", 
"STOS abs,X   ", 
"MOVE A,B     ", 
"MOVE B,B     ", 
"MOVE L,B     ", 
"MOVE H,B     ", 
"MOVE M,B     ", 

"ILLEGAL     ", 
"ADC B,M      ", 
"SBC B,M      ", 
"ADD B,M      ", 
"SUB B,M      ", 
"CMP B,M      ", 
"OR B,M       ", 
"AND B,M      ", 
"XOR B,M      ", 
"BIT B,M      ", 
"STOS abs,Y   ", 
"MOVE A,L     ", 
"MOVE B,L     ", 
"MOVE L,L     ", 
"MOVE H,L     ", 
"MOVE M,L     ", 

"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"SBIA  #      ", 
"SBIB  #      ", 
"CPIA  #      ", 
"CPIB  #      ", 
"ORIA  #      ", 
"ORIB  #      ", 
"ILLEGAL     ", 
"STOS (ind)   ", 
"MOVE A,H     ", 
"MOVE B,H     ", 
"MOVE L,H     ", 
"MOVE H,H     ", 
"MOVE M,H     ", 

"INC abs      ", 
"DEC abs      ", 
"RRC abs      ", 
"RLC abs      ", 
"SAL abs      ", 
"SAR abs      ", 
"LSR abs      ", 
"COM abs      ", 
"ROL abs      ", 
"RR abs       ", 
"STOS (ind,X) ", 
"MOVE A,M     ", 
"MOVE B,M     ", 
"MOVE L,M     ", 
"MOVE H,M     ", 
"MOVE -,-     ", 

"INC abs,X    ", 
"DEC abs,X    ", 
"RRC abs,X    ", 
"RLC abs,X    ", 
"SAL abs,X    ", 
"SAR abs,X    ", 
"LSR abs,X    ", 
"COM abs,X    ", 
"ROL abs,X    ", 
"RR abs,X     ", 
"STORA abs    ", 
"STORB abs    ", 
"STOX abs     ", 
"STOY abs     ", 
"PUSH  ,A     ", 
"POP A,       ", 

"INC abs,Y    ", 
"DEC abs,Y    ", 
"RRC abs,Y    ", 
"RLC abs,Y    ", 
"SAL abs,Y    ", 
"SAR abs,Y    ", 
"LSR abs,Y    ", 
"COM abs,Y    ", 
"ROL abs,Y    ", 
"RR abs,Y     ", 
"STORA abs,X  ", 
"STORB abs,X  ", 
"STOX abs,X   ", 
"STOY abs,X   ", 
"PUSH  ,B     ", 
"POP B,       ", 

"INCA A,A     ", 
"DECA A,A     ", 
"RRCA A,A     ", 
"RLCA A,A     ", 
"SALA A,A     ", 
"SARA A,A     ", 
"LSRA A,A     ", 
"COMA A,A     ", 
"ROLA A,A     ", 
"RRA A,A      ", 
"STORA abs,Y  ", 
"STORB abs,Y  ", 
"STOX abs,Y   ", 
"STOY abs,Y   ", 
"PUSH  ,s     ", 
"POP s,       ", 

"INCB B,B     ", 
"DECB B,B     ", 
"RRCB B,B     ", 
"RLCB B,B     ", 
"SALB B,B     ", 
"SARB B,B     ", 
"LSRB B,B     ", 
"COMB B,B     ", 
"ROLB B,B     ", 
"RRB B,B      ", 
"STORA (ind)  ", 
"STORB (ind)  ", 
"STOX (ind)   ", 
"STOY (ind)   ", 
"PUSH  ,L     ", 
"POP L,       ", 

"CAY impl     ", 
"MYA impl     ", 
"CSA impl     ", 
"ABA impl     ", 
"SBA impl     ", 
"AAB impl     ", 
"SAB impl     ", 
"ADCP A,L     ", 
"SBCP A,L     ", 
"XCHG A,L     ", 
"STORA (ind,X)", 
"STORB (ind,X)", 
"STOX (ind,X) ", 
"STOY (ind,X) ", 
"PUSH  ,H     ", 
"POP H,       ", 

}; 

////////////////////////////////////////////////////////////////////////////////
//                           Simulator/Emulator (Start)                       //
////////////////////////////////////////////////////////////////////////////////
BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}
void set_flag_z(BYTE inReg) {
	BYTE reg; 
	reg = inReg; 

	if ((reg & 0x80) != 0) // msbit set 
	{ 
		Flags = Flags | FLAG_N;
	}
	else 
	{ 
		Flags = Flags & (0xFF - FLAG_N);
	}
}

void Group_1(BYTE opcode){
	BYTE LB = 0;
	BYTE HB = 0;
	BYTE destination = opcode >> 4;
	BYTE source = opcode & 0x0F;

	WORD address = 0;
	WORD data = 0;

	int destReg = 0;
	int sourceReg = 0;




	switch(opcode) 
	{
		//LDAA
		case 0x0A: //LDAA Immidiate
			data = fetch();
			Registers[REGISTER_A] = data;
			break;

		case 0x1A:
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Registers[REGISTER_A] = Memory[address];
			}
			break;

		case 0x2A:
			address += Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE) 
			{
				Registers[REGISTER_A] = Memory[address];
			}
			break;

		case 0x3A:
			address += Index_Registers[REGISTER_Y];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE) 
			{
				Registers[REGISTER_A] = Memory[address];
			}
			break;

		case 0x4A:
			HB = fetch();
			LB = fetch();
			address = (WORD)((WORD)HB << 8) + LB;
			HB = Memory[address];
			LB = Memory[address + 1];
			address = (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE) 
			{
				Registers[REGISTER_A] = Memory[address];
			}
			break;

		case 0x5A:
			HB = fetch();
			LB = fetch();
			address = (WORD)((WORD)HB << 8) + LB;
			HB = Memory[address];
			LB = Memory[address + 1];
			address = (WORD)((WORD)HB << 8) + LB;
			address += Index_Registers[REGISTER_X];
			if (address >= 0 && address < MEMORY_SIZE) 
			{
				Registers[REGISTER_A] = Memory[address];
			}
			break;

			//STORA

		case 0xBA:
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE) 
			{
				Memory[address] = Registers[REGISTER_A];
			}
			break;

		case 0xCA:
			address += Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE) 
			{
				Memory[address] = Registers[REGISTER_A];
			}
			break;

		case 0xDA:
			address += Index_Registers[REGISTER_Y];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE) 
			{
				Memory[address] = Registers[REGISTER_A];
			}
			break;

		case 0xEA:
			HB = fetch();
			LB = fetch();
			address = (WORD)((WORD)HB << 8) + LB;
			HB = Memory[address];
			LB = Memory[address + 1];
			address = (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE) 
			{
				Memory[address] = Registers[REGISTER_A];
			}
			break;

		case 0xFA:
			HB = fetch();
			LB = fetch();
			address = (WORD)((WORD)HB << 8) + LB;
			HB = Memory[address];
			LB = Memory[address + 1];
			address = (WORD)((WORD)HB << 8) + LB;
			address += Index_Registers[REGISTER_X];
			if (address >= 0 && address < MEMORY_SIZE) 
			{
				Memory[address] = Registers[REGISTER_A];
			}
			break;

		//LDAB
		
	case 0x0B: //LDAB Immidiate
		data = fetch();
		Registers[REGISTER_B] = data;
		break;

	case 0x1B:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_B] = Memory[address];
		}
		break;

	case 0x2B:
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_B] = Memory[address];
		}
		break;

	case 0x3B:
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_B] = Memory[address];
		}
		break;

	case 0x4B:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_B] = Memory[address];
		}
		break;

	case 0x5B:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_B] = Memory[address];
		}
		break;

		//STORB

	case 0xBB:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_B];
		}
		break;

	case 0xCB:
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_B];
		}
		break;

	case 0xDB:
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_B];
		}
		break;

	case 0xEB:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_B];
		}
		break;

	case 0xFB:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_B];
		}
		break;
	//MVI 

	case 0x1C:
		data = fetch();
		Registers[REGISTER_L] = data;
		break;

	case 0x1D:
		data = fetch();
		Registers[REGISTER_H] = data;
		break;

		//LODS

	case 0x20:
		data = fetch();
		StackPointer = data << 8; StackPointer += fetch();
		break;

	case 0x30:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}

	case 0x40:
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}

	case 0x50:
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}

	case 0x60:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}

	case 0x70:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		
	}
}

void Group_2_Move(BYTE opcode)
{
	BYTE destination = opcode >> 4;
	BYTE source = opcode & 0x0F;
	BYTE LB = 0;
	BYTE HB = 0;

	int destReg =0 ;
	int sourceReg = 0;

	WORD address = 0;
	WORD data = 0;
	

	switch (destination)
	{
		case 0x06:
			destReg = REGISTER_A;
			break;

		case 0x07:
			destReg = REGISTER_B;
			break;

		case 0x08:
			destReg = REGISTER_L;
			break;

		case 0x09:
			destReg = REGISTER_H;
			break;

		case 0x0A:
			destReg = REGISTER_M;
			break;
	}



	switch (source)
	{
		case 0x0B:
			sourceReg = REGISTER_A;
			break;

		case 0x0C:
			sourceReg = REGISTER_B;
			break;

		case 0x0D:
			sourceReg = REGISTER_L;
			break;

		case 0x0E:
			sourceReg = REGISTER_H;
			break;

		case 0x0F:
			sourceReg = REGISTER_M;
			break;
	}
	
	Registers[destReg] = Registers[sourceReg];

}


void execute(BYTE opcode)
{	

	if(((opcode >= 0x6B) && (opcode <= 0x6F))
	|| ((opcode >= 0x7B) && (opcode <= 0x7F))
	|| ((opcode >= 0x8B) && (opcode <= 0x8F))
	|| ((opcode >= 0x9B) && (opcode <= 0x9F))
	|| ((opcode >= 0xAB) && (opcode <= 0xAF)))
	{
		Group_2_Move(opcode);
	}
	else
	{
		Group_1(opcode);
	}
}

void emulate()
{
	BYTE opcode;
	int sanity;

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;
	sanity = 0;

	printf("                    A  B  L  H  X  Y  SP\n");

	while ((!halt) && (memory_in_range) && (sanity < 200)) 
	{
		printf("%04X ", ProgramCounter);           // Print current address
		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);
		printf("%02X ", Registers[REGISTER_B]);
		printf("%02X ", Registers[REGISTER_L]);
		printf("%02X ", Registers[REGISTER_H]);
		printf("%02X ", Index_Registers[REGISTER_X]);
		printf("%02X ", Index_Registers[REGISTER_Y]);
		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_I) == FLAG_I)	
		{
			printf("I=1 ");
		}
		else
		{
			printf("I=0 ");
		}
		if ((Flags & FLAG_Z) == FLAG_Z)	
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}
		if ((Flags & FLAG_N) == FLAG_N)	
		{
			printf("N=1 ");
		}
		else
		{
			printf("N=0 ");
		}
		if ((Flags & FLAG_C) == FLAG_C)	
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
		sanity++;
	}

	printf("\n");  // New line
}


////////////////////////////////////////////////////////////////////////////////
//                            Simulator/Emulator (End)                        //
////////////////////////////////////////////////////////////////////////////////


void initialise_filenames() {
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++) {
		hex_file [i] = '\0';
		trc_file [i] = '\0';
	}
}




int find_dot_position(char *filename) {
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		if (chr == '.') {
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename) {
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename) {
	bool exists;
	FILE *ifp;

	exists = false;

	if ( ( ifp = fopen( filename, "r" ) ) != NULL ) 
	{
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename) {
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL ) {
		fclose(ofp);
	}
}



bool getline(FILE *fp, char *buffer) {
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect) {
		c = getc(fp);

		switch (c) {
		case EOF:
			if (i > 0) {
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0) {
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}






void load_and_run(int args,_TCHAR** argv) {
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	if(args == 2){
		ln = 0;
		chr = argv[1][ln];
		while (chr != '\0')
		{
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file [ln] = chr;
				trc_file [ln] = chr;
				ln++;
			}
			chr = argv[1][ln];
		}
	} else {
		ln = 0;
		chr = '\0';
		while (chr != '\n') {
			chr = getchar();

			switch(chr) {
			case '\n':
				break;
			default:
				if (ln < MAX_FILENAME_SIZE)	{
					hex_file [ln] = chr;
					trc_file [ln] = chr;
					ln++;
				}
				break;
			}
		}

	}
	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0) {
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	} else {
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0) {
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	} else {
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file)) {
		// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		Index_Registers[REGISTER_X] = 0;
		Index_Registers[REGISTER_Y] = 0;
		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;

		for (i=0; i<MEMORY_SIZE; i++) {
			Memory[i] = 0x00;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL ) {
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer)) {
				if (sscanf(InputBuffer, "L=%x", &address) == 1) {
					load_at = address;
				} else if (sscanf(InputBuffer, "%x", &code) == 1) {
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				} else {
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate

		emulate();
	} else {
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}

void building(int args,_TCHAR** argv)
{
	char buffer[1024];
	load_and_run(args,argv);
	sprintf(buffer, "0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X", 
		Memory[TEST_ADDRESS_1],
		Memory[TEST_ADDRESS_2],
		Memory[TEST_ADDRESS_3],
		Memory[TEST_ADDRESS_4], 
		Memory[TEST_ADDRESS_5],
		Memory[TEST_ADDRESS_6], 
		Memory[TEST_ADDRESS_7],
		Memory[TEST_ADDRESS_8], 
		Memory[TEST_ADDRESS_9], 
		Memory[TEST_ADDRESS_10],
		Memory[TEST_ADDRESS_11],
		Memory[TEST_ADDRESS_12]
		);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
}



void test_and_mark() {
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;
	int  passed;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete) {
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR) {
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 1)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1) {
				testing_complete = true;
				printf("Current mark = %d\n", mark);

			}else if (sscanf(buffer, "Tests passed %d", &passed) == 1) {
				//testing_complete = true;
				printf("Passed = %d\n", passed);

			} else if (strcmp(buffer, "Error") == 0) {
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			} else {
				// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		Index_Registers[REGISTER_X] = 0;
		Index_Registers[REGISTER_Y] = 0;
				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<MEMORY_SIZE; i++) {
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				FILE *ofp;
				fopen_s(&ofp ,"branch.txt", "a");

				while (!end_of_program) {
					chr = buffer[i];
					switch (chr) {
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1) {
							load_at = address;
						} else if (sscanf(InputBuffer, "%x", &code) == 1) {
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
								Memory[load_at] = (BYTE)code;
								fprintf(ofp, "%02X\n", (BYTE)code);
							}
							load_at++;
						} else {
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}
				fclose(ofp);
				// Emulate

				if (load_at > 1) {
					emulate();
					// Send and store results
					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X", 
						Memory[TEST_ADDRESS_1],
						Memory[TEST_ADDRESS_2],
						Memory[TEST_ADDRESS_3],
						Memory[TEST_ADDRESS_4], 
						Memory[TEST_ADDRESS_5],
						Memory[TEST_ADDRESS_6], 
						Memory[TEST_ADDRESS_7],
						Memory[TEST_ADDRESS_8], 
						Memory[TEST_ADDRESS_9], 
						Memory[TEST_ADDRESS_10],
						Memory[TEST_ADDRESS_11],
						Memory[TEST_ADDRESS_12]
						);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock) {	
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		if(argc == 2){ building(argc,argv); exit(0);}
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run(argc,argv);
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}




