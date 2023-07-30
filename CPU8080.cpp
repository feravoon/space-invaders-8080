#include "CPU8080.h"
#include <iostream>

// Accessing pseudo-register M
uint8_t CPU8080::getM()
{
	return memory.read(getHL());
}
void CPU8080::setM(uint8_t value)
{
	memory.write(getHL(), value);
}

// Accessing register as two byte pairs
uint16_t CPU8080::getBC() { return (B << 8) | C; }
uint16_t CPU8080::getDE() { return (D << 8) | E; }
uint16_t CPU8080::getHL() { return (H << 8) | L; }
void CPU8080::setBC(uint16_t BC) { B = BC >> 8; C = BC & 0x00ff; }
void CPU8080::setDE(uint16_t DE) { D = DE >> 8; E = DE & 0x00ff; }
void CPU8080::setHL(uint16_t HL) { H = HL >> 8; L = HL & 0x00ff; }

uint8_t CPU8080::getFlags()
{
	return ((S ? 1 : 0) << 7) | ((Z ? 1 : 0) << 6) | ((AC ? 1 : 0) << 4) | ((P ? 1 : 0) << 2) | (CY ? 1 : 0) | 0x02;
}

void CPU8080::setFlags(uint8_t value)
{
	S = (value & 0x80) == 0x80;
	Z = (value & 0x40) == 0x40;
	AC = (value & 0x10) == 0x10;
	P = (value & 0x02) == 0x02;
	CY = (value & 0x01) == 0x01;
}

bool CPU8080::parity(int x, int size)
{
	int i;
	int p = 0;
	x = (x & ((1 << size) - 1));
	for (i = 0; i < size; i++)
	{
		if ((x & 0x1) == 0x1) p++;
		x >>= 1;
	}
	return (0 == (p & 0x1));
}

void CPU8080::updateFlagsArithmetic(int res)
{
	CY = (res > 0xff);
	Z = ((res & 0xff) == 0);
	S = (0x80 == (res & 0x80));
	P = parity(res & 0xff, 8);
}

void CPU8080::updateFlagsLogic()
{
	CY = false;
	Z = (A == 0);
	S = (0x80 == (A & 0x80));
	P = parity(A, 8);
}

void CPU8080::updateFlagsZSP(uint8_t val)
{
	Z = (val == 0);
	S = (0x80 == (val & 0x80));
	P = parity(val, 8);
}

void CPU8080::generateInterrupt(int intID)
{
	uint16_t ret = PC;
	memory[SP - 1] = (ret >> 8) & 0xff;
	memory[SP - 2] = ret & 0xff;
	SP -= 2;
	PC = (uint16_t)(8 * intID);
	IE = false;
}

const int CPU8080::cycles[] = {
			4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4, //0x00..0x0f
			4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4,
			4, 10, 16, 5, 5, 5, 7, 4, 4, 10, 16, 5, 5, 5, 7, 4,
			4, 10, 13, 5, 10, 10, 10, 4, 4, 10, 13, 5, 5, 5, 7, 4,

			5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5, //0x40..0x4f
			5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
			5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
			7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,

			4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, //0x80..0x8f
			4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
			4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
			4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,

			11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11, //0xc0..0xcf
			11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11,
			11, 10, 10, 18, 17, 11, 7, 11, 11, 5, 10, 5, 17, 17, 7, 11,
			11, 10, 10, 4, 17, 11, 7, 11, 11, 5, 10, 4, 17, 17, 7, 11,
			}; // Cycle lengths for each opcode

const int CPU8080::byteLengths[] = {
			1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, //0x00..0x0f
			1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1,
			1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1,

			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0x40..0x4f
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0x80..0x8f
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

			1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 3, 3, 3, 2, 1, //0xc0..0xcf
			1, 1, 3, 2, 3, 1, 2, 1, 1, 1, 3, 2, 3, 3, 2, 1,
			1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
			1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
			}; // Instruction byte lengths for each opcode 

CPU8080::CPU8080()
{
	A = 0;
	B = 0;
	C = 0;
	D = 0;
	E = 0;
	H = 0;
	L = 0;
	PC = 0;
	SP = 0;

	Z = false;
	S = false;
	P = false;
	CY = false;
	AC = false;
	IE = true;

	memory = Memory();
	IO = IOcontroller();
}

int CPU8080::processInstruction() // Main method for processing an instruction (sould be called repeatedly in a loop)
{
	uint8_t opcode = memory[PC]; // Opcode is read from memory location indicated by PC
	uint8_t opcode1, opcode2; // Two bytes are defined for two instruction operands

	// Instruction uint8_t length
	int byteLength = byteLengths[opcode];

	if (byteLength > 1)
	{
		// If instruction is longer than 1 byte, take next uint8_t as first operand
		opcode1 = memory[PC + 1];
	}

	if (byteLength > 2)
	{
		// If instruction is longer than 2 bytes, take further next uint8_t as second operand
		opcode2 = memory[PC + 2];
	}

	PC += byteLength; // Advance the PC by the uint8_t length of current instruction

	switch (opcode)
	{
	case 0x00: // NOP
	case 0x10:
	case 0x20:
	case 0x30:
	case 0x08:
	case 0x18:
	case 0x28:
	case 0x38:
		// Do nothing
		break;

	case 0x07: // RLC
	{
		uint8_t x = A;
		A = (((x & 0x80) >> 7) | (x << 1));
		CY = (0x80 == (x & 0x80));
	}
	break;

	case 0x0f: // RRC
	{
		uint8_t x = A;
		A = (((x & 0x01) << 7) | (x >> 1));
		CY = (0x01 == (x & 0x01));
	}
	break;

	case 0x17: // RAL
	{
		uint8_t x = A;
		A = ((CY ? 1 : 0) | (x << 1));
		CY = (0x80 == (x & 0x80));
	}
	break;

	case 0x1f: // RAR
	{
		uint8_t x = A;
		A = (((CY ? 1 : 0) << 7) | (x >> 1));
		CY = (0x01 == (x & 0x01));
	}
	break;

	// MVI
	case 0x06: B = opcode1; break; //MVI B, byte
	case 0x0e: C = opcode1; break; //MVI C, byte
	case 0x16: D = opcode1; break; //MVI D, byte
	case 0x1e: E = opcode1; break; //MVI E, byte
	case 0x26: H = opcode1; break; //MVI H, byte
	case 0x2e: L = opcode1; break; //MVI L, byte
	case 0x36: setM(opcode1); break; //MVI M, byte
	case 0x3e: A = opcode1; break; //MVI A, byte

	// LXI
	case 0x01: setBC((opcode2 << 8) | opcode1); break; //LXI B, word
	case 0x11: setDE((opcode2 << 8) | opcode1); break; //LXI D, word
	case 0x21: setHL((opcode2 << 8) | opcode1); break; //LXI H, word
	case 0x31: SP = ((opcode2 << 8) | opcode1); break; //LXI SP, word

	// STAX
	case 0x02: memory[getBC()] = A; break; // STAX B
	case 0x12: memory[getDE()] = A; break; // STAX D

	case 0x22: memory[(opcode2 << 8) | opcode1] = L; memory[((opcode2 << 8) | opcode1) + 1] = H; break; // SHLD
	case 0x32: memory[(opcode2 << 8) | opcode1] = A; break; // STA

	case 0x2f: A = ~A; break; // CMA

	case 0x37: CY = true; break; // STC
	case 0x3f: CY = false; break; // CMC

	// LDAX
	case 0x0a: A = memory[getBC()]; break; // LDAX B
	case 0x1a: A = memory[getDE()]; break; // LDAX D

	case 0x2a: L = memory[(opcode2 << 8) | opcode1]; H = memory[((opcode2 << 8) | opcode1) + 1];  break; // LHLD
	case 0x3a: A = memory[(opcode2 << 8) | opcode1]; break; // LDA

	// DAD
	case 0x09: { int res = getHL() + getBC(); setHL(res); CY = (res & 0xffff0000) != 0; }; break; // DAD B
	case 0x19: { int res = getHL() + getDE(); setHL(res); CY = (res & 0xffff0000) != 0; }; break; // DAD D
	case 0x29: { int res = getHL() + getHL(); setHL(res); CY = (res & 0xffff0000) != 0; }; break; // DAD HL
	case 0x39: { int res = getHL() + SP; setHL(res); CY = (res & 0xffff0000) != 0; }; break; // DAD SP

	case 0x27: // DAA
		if ((A & 0xf) > 9)
			A += 6;
		if ((A & 0xf0) > 0x90)
		{
			int res = A + 0x60;
			A = (res & 0xff);
			updateFlagsArithmetic(res);
		}
		break;

	// INR
	case 0x04: B++; updateFlagsZSP(B); break; // INR B
	case 0x0c: C++; updateFlagsZSP(C); break; // INR C
	case 0x14: D++; updateFlagsZSP(D); break; // INR D
	case 0x1c: E++; updateFlagsZSP(E); break; // INR E
	case 0x24: H++; updateFlagsZSP(H); break; // INR H
	case 0x2c: L++; updateFlagsZSP(L); break; // INR L
	case 0x34: setM(getM()+1); updateFlagsZSP(getM()); break; // INR M
	case 0x3c: A++; updateFlagsZSP(A); break; // INR A

	// DCR
	case 0x05: B--; updateFlagsZSP(B); break; // DCR B
	case 0x0d: C--; updateFlagsZSP(C); break; // DCR C
	case 0x15: D--; updateFlagsZSP(D); break; // DCR D
	case 0x1d: E--; updateFlagsZSP(E); break; // DCR E
	case 0x25: H--; updateFlagsZSP(H); break; // DCR H
	case 0x2d: L--; updateFlagsZSP(L); break; // DCR L
	case 0x35: setM(getM() - 1); updateFlagsZSP(getM()); break; // DCR M
	case 0x3d: A--; updateFlagsZSP(A); break; // DCR A

	// INX
	case 0x03: setBC(getBC() + 1); break; // INX B
	case 0x13: setDE(getDE() + 1); break; // INX D
	case 0x23: setHL(getHL() + 1); break; // INX H
	case 0x33: SP++; break; // INX SP

	// DCX
	case 0x0b: setBC(getBC() - 1); break; // DCX B
	case 0x1b: setDE(getDE() - 1); break; // DCX D
	case 0x2b: setHL(getHL() - 1); break; // DCX H
	case 0x3b: SP--; break; // DCX SP

	// MOV B
	case 0x40: break; // MOV B,B
	case 0x41: B = C; break; // MOV B,C
	case 0x42: B = D; break; // MOV B,D
	case 0x43: B = E; break; // MOV B,E
	case 0x44: B = H; break; // MOV B,H
	case 0x45: B = L; break; // MOV B,L
	case 0x46: B = getM(); break; // MOV B,M
	case 0x47: B = A; break; // MOV B,A

	// MOV C
	case 0x48: C = B; break; // MOV C,B
	case 0x49: break; // MOV C,C
	case 0x4a: C = D; break; // MOV C,D
	case 0x4b: C = E; break; // MOV C,E
	case 0x4c: C = H; break; // MOV C,H
	case 0x4d: C = L; break; // MOV C,L
	case 0x4e: C = getM(); break; // MOV C,M
	case 0x4f: C = A; break; // MOV C,A

	// MOV D
	case 0x50: D = B; break; // MOV D,B
	case 0x51: D = C; break; // MOV D,C
	case 0x52: break; // MOV D,D
	case 0x53: D = E; break; // MOV D,E
	case 0x54: D = H; break; // MOV D,H
	case 0x55: D = L; break; // MOV D,L
	case 0x56: D = getM(); break; // MOV D,M
	case 0x57: D = A; break; // MOV D,A

	// MOV E
	case 0x58: E = B; break; // MOV E,B
	case 0x59: E = C; break; // MOV E,C
	case 0x5a: E = D; break; // MOV E,D
	case 0x5b: break; // MOV E,E
	case 0x5c: E = H; break; // MOV E,H
	case 0x5d: E = L; break; // MOV E,L
	case 0x5e: E = getM(); break; // MOV E,M
	case 0x5f: E = A; break; // MOV E,A

	// MOV H
	case 0x60: H = B; break; // MOV H,B
	case 0x61: H = C; break; // MOV H,C
	case 0x62: H = D; break; // MOV H,D
	case 0x63: H = E; break; // MOV H,E
	case 0x64: break; // MOV H,H
	case 0x65: H = L; break; // MOV H,L
	case 0x66: H = getM(); break; // MOV H,M
	case 0x67: H = A; break; // MOV H,A

	// MOV L
	case 0x68: L = B; break; // MOV L,B
	case 0x69: L = C; break; // MOV L,C
	case 0x6a: L = D; break; // MOV L,D
	case 0x6b: L = E; break; // MOV L,E
	case 0x6c: L = H; break; // MOV L,H
	case 0x6d: break; // MOV L,L
	case 0x6e: L = getM(); break; // MOV L,M
	case 0x6f: L = A; break; // MOV L,A

	// MOV M
	case 0x70: setM (B); break; // MOV M,B
	case 0x71: setM(C); break; // MOV M,C
	case 0x72: setM(D); break; // MOV M,D
	case 0x73: setM(E); break; // MOV M,E
	case 0x74: setM(H); break; // MOV M,H
	case 0x75: setM(L); break; // MOV M,L
	case 0x76: break; // HLT
	case 0x77: setM(A); break; // MOV M,A

	// MOV A
	case 0x78: A = B; break; // MOV A,B
	case 0x79: A = C; break; // MOV A,C
	case 0x7a: A = D; break; // MOV A,D
	case 0x7b: A = E; break; // MOV A,E
	case 0x7c: A = H; break; // MOV A,H
	case 0x7d: A = L; break; // MOV A,L
	case 0x7e: A = getM(); break; // MOV A,M
	case 0x7f: break; // MOV A,A

	// ADD
	case 0x80: { int res = A + B; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD B
	case 0x81: { int res = A + C; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD C
	case 0x82: { int res = A + D; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD D
	case 0x83: { int res = A + E; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD E
	case 0x84: { int res = A + H; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD H
	case 0x85: { int res = A + L; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD L
	case 0x86: { int res = A + getM(); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD M
	case 0x87: { int res = A + A; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD A

	// ADC
	case 0x88: { int res = A + B + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC B
	case 0x89: { int res = A + C + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC C
	case 0x8a: { int res = A + D + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC D
	case 0x8b: { int res = A + E + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC E
	case 0x8c: { int res = A + H + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC H
	case 0x8d: { int res = A + L + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC L
	case 0x8e: { int res = A + getM() + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC M
	case 0x8f: { int res = A + A + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC A

	// SUB
	case 0x90: { uint16_t res = (A - B); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB B
	case 0x91: { uint16_t res = (A - C); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB C
	case 0x92: { uint16_t res = (A - D); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB D
	case 0x93: { uint16_t res = (A - E); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB E
	case 0x94: { uint16_t res = (A - H); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB H
	case 0x95: { uint16_t res = (A - L); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB L
	case 0x96: { uint16_t res = (A - getM()); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB M
	case 0x97: { uint16_t res = (A - A); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB A

	// SBB
	case 0x98: { uint16_t res = (A - B - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB B
	case 0x99: { uint16_t res = (A - C - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB C
	case 0x9a: { uint16_t res = (A - D - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB D
	case 0x9b: { uint16_t res = (A - E - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB E
	case 0x9c: { uint16_t res = (A - H - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB H
	case 0x9d: { uint16_t res = (A - L - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB L
	case 0x9e: { uint16_t res = (A - getM() - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB M
	case 0x9f: { uint16_t res = (A - A - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB A

	// ANA
	case 0xa0: A &= B; updateFlagsLogic(); break; // ANA B
	case 0xa1: A &= C; updateFlagsLogic(); break; // ANA C
	case 0xa2: A &= D; updateFlagsLogic(); break; // ANA D
	case 0xa3: A &= E; updateFlagsLogic(); break; // ANA E
	case 0xa4: A &= H; updateFlagsLogic(); break; // ANA H
	case 0xa5: A &= L; updateFlagsLogic(); break; // ANA L
	case 0xa6: A &= getM(); updateFlagsLogic(); break; // ANA M
	case 0xa7: A &= A; updateFlagsLogic(); break; // ANA A

	// XRA
	case 0xa8: A ^= B; updateFlagsLogic(); break; // XRA B
	case 0xa9: A ^= C; updateFlagsLogic(); break; // XRA C
	case 0xaa: A ^= D; updateFlagsLogic(); break; // XRA D
	case 0xab: A ^= E; updateFlagsLogic(); break; // XRA E
	case 0xac: A ^= H; updateFlagsLogic(); break; // XRA H
	case 0xad: A ^= L; updateFlagsLogic(); break; // XRA L
	case 0xae: A ^= getM(); updateFlagsLogic(); break; // XRA M
	case 0xaf: A ^= A; updateFlagsLogic(); break; // XRA A

	// ORA
	case 0xb0: A |= B; updateFlagsLogic(); break; // ORA B
	case 0xb1: A |= C; updateFlagsLogic(); break; // ORA C
	case 0xb2: A |= D; updateFlagsLogic(); break; // ORA D
	case 0xb3: A |= E; updateFlagsLogic(); break; // ORA E
	case 0xb4: A |= H; updateFlagsLogic(); break; // ORA H
	case 0xb5: A |= L; updateFlagsLogic(); break; // ORA L
	case 0xb6: A |= getM(); updateFlagsLogic(); break; // ORA M
	case 0xb7: A |= A; updateFlagsLogic(); break; // ORA A

	// CMP
	case 0xb8: { uint16_t res = (A - B); updateFlagsArithmetic(res); } break; // CMP B
	case 0xb9: { uint16_t res = (A - C); updateFlagsArithmetic(res); } break; // CMP C
	case 0xba: { uint16_t res = (A - D); updateFlagsArithmetic(res); } break; // CMP D
	case 0xbb: { uint16_t res = (A - E); updateFlagsArithmetic(res); } break; // CMP E
	case 0xbc: { uint16_t res = (A - H); updateFlagsArithmetic(res); } break; // CMP H
	case 0xbd: { uint16_t res = (A - L); updateFlagsArithmetic(res); } break; // CMP L
	case 0xbe: { uint16_t res = (A - getM()); updateFlagsArithmetic(res); } break; // CMP M
	case 0xbf: { uint16_t res = (A - A); updateFlagsArithmetic(res); } break; // CMP A

	case 0xc6: { int res = A + opcode1; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADI 
	case 0xd6: { uint16_t res = (A - opcode1); updateFlagsZSP((res & 0xff)); CY = res > 0xff; A = (res & 0xff); } break; // SUI
	case 0xe6: A &= opcode1; updateFlagsLogic(); break; // ANI
	case 0xf6: A |= opcode1; updateFlagsLogic(); break; // ORI

	case 0xce: { int res = A + opcode1 + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ACI
	case 0xde: { uint16_t res = (A - opcode1 - (CY ? 1 : 0)); updateFlagsZSP((res & 0xff)); CY = res > 0xff; A = (res & 0xff); } break; // SBI
	case 0xee: A ^= opcode1; updateFlagsLogic(); break; // XRI
	case 0xfe: { uint8_t res = (A - opcode1); updateFlagsZSP(res); CY = A < opcode1; } break; // CPI

	case 0xc3: PC = ((opcode2 << 8) | opcode1); break; // JMP adr
	case 0xc2: if (!Z) { PC = ((opcode2 << 8) | opcode1); } break; // JNZ adr
	case 0xca: if (Z) { PC = ((opcode2 << 8) | opcode1); } break; // JZ adr
	case 0xd2: if (!CY) { PC = ((opcode2 << 8) | opcode1); } break; // JNC adr
	case 0xda: if (CY) { PC = ((opcode2 << 8) | opcode1); } break; // JC adr
	case 0xe2: if (!P) { PC = ((opcode2 << 8) | opcode1); } break; // JPO adr
	case 0xea: if (P) { PC = ((opcode2 << 8) | opcode1); } break; // JPE adr
	case 0xf2: if (!S) { PC = ((opcode2 << 8) | opcode1); } break; // JP adr
	case 0xfa: if (S) { PC = ((opcode2 << 8) | opcode1); } break; // JM adr

	case 0xc7: // RST 0
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0000;
	}
	break;

	case 0xcf: // RST 1
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0008;
	}
	break;

	case 0xd7: // RST 2
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0010;
	}
	break;

	case 0xdf: // RST 3
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0018;
	}
	break;

	case 0xe7: // RST 4
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0020;
	}
	break;

	case 0xef: // RST 5
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0028;
	}
	break;

	case 0xf7: // RST 6
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0030;
	}
	break;

	case 0xff: // RST 7
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0038;
	}
	break;



	case 0xc9: // RET
		PC = (memory[SP] | (memory[SP + 1] << 8));
		SP += 2;
		break;

	case 0xc8: // RZ
		if (Z)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xc0: // RNZ
		if (!Z)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xd8: // RC
		if (CY)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xd0: // RNC
		if (!CY)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xe8: // RPE
		if (P)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xe0: // RPO
		if (!P)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xf8: // RM
		if (S)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xf0: // RP
		if (!S)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xcd: // CALL adr
		if (5 == ((opcode2 << 8) | opcode1)) // to simulate message printing routine of CP/M for debugging purposes
		{
			if (C == 9)
			{
				printf("\n\n");
				int offset = (D << 8) | (E);
				char str = (char)memory[offset + 3];  //skip the prefix bytes    
				while (str != '$')
				{
					str = (char)memory[offset + 3];
					printf("%s",&str);
					offset++;
				}
				printf("\n\n");
			}
			else if (C == 2)
			{
				//saw this in the inspected code, never saw it called    
				printf("print char routine called\n");
			}
		}
		else if (0 == ((opcode2 << 8) | opcode1))
		{
			printf("CPU FAIL!");
		}
		else // Actual logic of the CALL insturction
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xc4: // CNZ
		if (!Z)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;
	case 0xcc: // CZ
		if (Z)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xd4: // CNC
		if (!CY)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xdc: // CC
		if (CY)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xe4: // CPO
		if (!P)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xec: // CPE
		if (P)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xf4: // CP
		if (!S)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xfc: // CM
		if (S)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xc5: memory[SP - 1] = B; memory[SP - 2] = C; SP -= 2; break; // PUSH B
	case 0xd5: memory[SP - 1] = D; memory[SP - 2] = E; SP -= 2; break; // PUSH D
	case 0xe5: memory[SP - 1] = H; memory[SP - 2] = L; SP -= 2; break; // PUSH HL
	case 0xf5: memory[SP - 1] = A; memory[SP - 2] = getFlags(); SP -= 2; break; // PUSH PSW

	case 0xc1: B = memory[SP + 1]; C = memory[SP]; SP += 2; break; // POP B
	case 0xd1: D = memory[SP + 1]; E = memory[SP]; SP += 2; break; // POP D
	case 0xe1: H = memory[SP + 1]; L = memory[SP]; SP += 2; break; // POP HL
	case 0xf1: A = memory[SP + 1]; setFlags(memory[SP]); SP += 2; break; // POP PSW

	case 0xeb: { uint16_t tmp; tmp = getHL(); setHL(getDE()); setDE(tmp); } break; // XCHG

	case 0xd3: IO.O[opcode1] = A; IO.newOutput[opcode1] = true; break; // OUT
	case 0xdb: A = IO.I[opcode1]; break; // IN

	case 0xf3: IE = false; break; // DI
	case 0xfb: IE = true; break; // EI

	case 0xe3: // XTHL 
	{
		uint8_t Hval = H;
		uint8_t Lval = L;
		L = memory[SP];
		H = memory[SP + 1];
		memory[SP] = Lval;
		memory[SP + 1] = Hval;
	}
	break;

	case 0xe9: PC = ((H << 8) | L); break; // PCHL
	case 0xf9: SP = ((H << 8) | L); break; // SPHL

	default:
		printf("\n\nUnimplemented instruction:\n");
		PC -= byteLength; // Decrement PC by 1 for disassembly code to work on the right instruction
		disassemble8080Op();
		// throw new Exception("Unimplemented Opcode!");
		return 0;
	}


	if (printOutput)
	{
		printf("\t\t");
		printf("%c", Z ? 'Z' : '.');
		printf("%c", S ? 'S' : '.');
		printf("%c", P ? 'P' : '.');
		printf("%c", CY ? 'C' : '.');
		printf("%c  ", AC ? 'A' : '.');
		printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", A, B, C, D, E, H, L, SP);
	}

	return cycles[opcode]; // return the number of cycles for cycle counting
}

int CPU8080::disassemble8080Op()
{
	uint8_t code = memory.read(PC);
	uint8_t code1 = memory.read(PC+1);
	uint8_t code2 = memory.read(PC+2);

	int opbytes = 1;
	printf("%04x ", PC);
	switch (code)
	{
	case 0x00: printf("NOP"); break;
	case 0x01: printf("LXI    B,#$%02x%02x", code2, code1); opbytes = 3; break;
	case 0x02: printf("STAX   B"); break;
	case 0x03: printf("INX    B"); break;
	case 0x04: printf("INR    B"); break;
	case 0x05: printf("DCR    B"); break;
	case 0x06: printf("MVI    B,#$%02x", code1); opbytes = 2; break;
	case 0x07: printf("RLC"); break;
	case 0x08: printf("NOP"); break;
	case 0x09: printf("DAD    B"); break;
	case 0x0a: printf("LDAX   B"); break;
	case 0x0b: printf("DCX    B"); break;
	case 0x0c: printf("INR    C"); break;
	case 0x0d: printf("DCR    C"); break;
	case 0x0e: printf("MVI    C,#$%02x", code1); opbytes = 2;	break;
	case 0x0f: printf("RRC"); break;

	case 0x10: printf("NOP"); break;
	case 0x11: printf("LXI    D,#$%02x%02x", code2, code1); opbytes = 3; break;
	case 0x12: printf("STAX   D"); break;
	case 0x13: printf("INX    D"); break;
	case 0x14: printf("INR    D"); break;
	case 0x15: printf("DCR    D"); break;
	case 0x16: printf("MVI    D,#$%02x", code1); opbytes = 2; break;
	case 0x17: printf("RAL"); break;
	case 0x18: printf("NOP"); break;
	case 0x19: printf("DAD    D"); break;
	case 0x1a: printf("LDAX   D"); break;
	case 0x1b: printf("DCX    D"); break;
	case 0x1c: printf("INR    E"); break;
	case 0x1d: printf("DCR    E"); break;
	case 0x1e: printf("MVI    E,#$%02x", code1); opbytes = 2; break;
	case 0x1f: printf("RAR"); break;

	case 0x20: printf("NOP"); break;
	case 0x21: printf("LXI    H,#$%02x%02x", code2, code1); opbytes = 3; break;
	case 0x22: printf("SHLD   $%02x%02x", code2, code1); opbytes = 3; break;
	case 0x23: printf("INX    H"); break;
	case 0x24: printf("INR    H"); break;
	case 0x25: printf("DCR    H"); break;
	case 0x26: printf("MVI    H,#$%02x", code1); opbytes = 2; break;
	case 0x27: printf("DAA"); break;
	case 0x28: printf("NOP"); break;
	case 0x29: printf("DAD    H"); break;
	case 0x2a: printf("LHLD   $%02x%02x", code2, code1); opbytes = 3; break;
	case 0x2b: printf("DCX    H"); break;
	case 0x2c: printf("INR    L"); break;
	case 0x2d: printf("DCR    L"); break;
	case 0x2e: printf("MVI    L,#$%02x", code1); opbytes = 2; break;
	case 0x2f: printf("CMA"); break;

	case 0x30: printf("NOP"); break;
	case 0x31: printf("LXI    SP,#$%02x%02x", code2, code1); opbytes = 3; break;
	case 0x32: printf("STA    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0x33: printf("INX    SP"); break;
	case 0x34: printf("INR    M"); break;
	case 0x35: printf("DCR    M"); break;
	case 0x36: printf("MVI    M,#$%02x", code1); opbytes = 2; break;
	case 0x37: printf("STC"); break;
	case 0x38: printf("NOP"); break;
	case 0x39: printf("DAD    SP"); break;
	case 0x3a: printf("LDA    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0x3b: printf("DCX    SP"); break;
	case 0x3c: printf("INR    A"); break;
	case 0x3d: printf("DCR    A"); break;
	case 0x3e: printf("MVI    A,#$%02x", code1); opbytes = 2; break;
	case 0x3f: printf("CMC"); break;

	case 0x40: printf("MOV    B,B"); break;
	case 0x41: printf("MOV    B,C"); break;
	case 0x42: printf("MOV    B,D"); break;
	case 0x43: printf("MOV    B,E"); break;
	case 0x44: printf("MOV    B,H"); break;
	case 0x45: printf("MOV    B,L"); break;
	case 0x46: printf("MOV    B,M"); break;
	case 0x47: printf("MOV    B,A"); break;
	case 0x48: printf("MOV    C,B"); break;
	case 0x49: printf("MOV    C,C"); break;
	case 0x4a: printf("MOV    C,D"); break;
	case 0x4b: printf("MOV    C,E"); break;
	case 0x4c: printf("MOV    C,H"); break;
	case 0x4d: printf("MOV    C,L"); break;
	case 0x4e: printf("MOV    C,M"); break;
	case 0x4f: printf("MOV    C,A"); break;

	case 0x50: printf("MOV    D,B"); break;
	case 0x51: printf("MOV    D,C"); break;
	case 0x52: printf("MOV    D,D"); break;
	case 0x53: printf("MOV    D.E"); break;
	case 0x54: printf("MOV    D,H"); break;
	case 0x55: printf("MOV    D,L"); break;
	case 0x56: printf("MOV    D,M"); break;
	case 0x57: printf("MOV    D,A"); break;
	case 0x58: printf("MOV    E,B"); break;
	case 0x59: printf("MOV    E,C"); break;
	case 0x5a: printf("MOV    E,D"); break;
	case 0x5b: printf("MOV    E,E"); break;
	case 0x5c: printf("MOV    E,H"); break;
	case 0x5d: printf("MOV    E,L"); break;
	case 0x5e: printf("MOV    E,M"); break;
	case 0x5f: printf("MOV    E,A"); break;

	case 0x60: printf("MOV    H,B"); break;
	case 0x61: printf("MOV    H,C"); break;
	case 0x62: printf("MOV    H,D"); break;
	case 0x63: printf("MOV    H.E"); break;
	case 0x64: printf("MOV    H,H"); break;
	case 0x65: printf("MOV    H,L"); break;
	case 0x66: printf("MOV    H,M"); break;
	case 0x67: printf("MOV    H,A"); break;
	case 0x68: printf("MOV    L,B"); break;
	case 0x69: printf("MOV    L,C"); break;
	case 0x6a: printf("MOV    L,D"); break;
	case 0x6b: printf("MOV    L,E"); break;
	case 0x6c: printf("MOV    L,H"); break;
	case 0x6d: printf("MOV    L,L"); break;
	case 0x6e: printf("MOV    L,M"); break;
	case 0x6f: printf("MOV    L,A"); break;

	case 0x70: printf("MOV    M,B"); break;
	case 0x71: printf("MOV    M,C"); break;
	case 0x72: printf("MOV    M,D"); break;
	case 0x73: printf("MOV    M.E"); break;
	case 0x74: printf("MOV    M,H"); break;
	case 0x75: printf("MOV    M,L"); break;
	case 0x76: printf("HLT");        break;
	case 0x77: printf("MOV    M,A"); break;
	case 0x78: printf("MOV    A,B"); break;
	case 0x79: printf("MOV    A,C"); break;
	case 0x7a: printf("MOV    A,D"); break;
	case 0x7b: printf("MOV    A,E"); break;
	case 0x7c: printf("MOV    A,H"); break;
	case 0x7d: printf("MOV    A,L"); break;
	case 0x7e: printf("MOV    A,M"); break;
	case 0x7f: printf("MOV    A,A"); break;

	case 0x80: printf("ADD    B"); break;
	case 0x81: printf("ADD    C"); break;
	case 0x82: printf("ADD    D"); break;
	case 0x83: printf("ADD    E"); break;
	case 0x84: printf("ADD    H"); break;
	case 0x85: printf("ADD    L"); break;
	case 0x86: printf("ADD    M"); break;
	case 0x87: printf("ADD    A"); break;
	case 0x88: printf("ADC    B"); break;
	case 0x89: printf("ADC    C"); break;
	case 0x8a: printf("ADC    D"); break;
	case 0x8b: printf("ADC    E"); break;
	case 0x8c: printf("ADC    H"); break;
	case 0x8d: printf("ADC    L"); break;
	case 0x8e: printf("ADC    M"); break;
	case 0x8f: printf("ADC    A"); break;

	case 0x90: printf("SUB    B"); break;
	case 0x91: printf("SUB    C"); break;
	case 0x92: printf("SUB    D"); break;
	case 0x93: printf("SUB    E"); break;
	case 0x94: printf("SUB    H"); break;
	case 0x95: printf("SUB    L"); break;
	case 0x96: printf("SUB    M"); break;
	case 0x97: printf("SUB    A"); break;
	case 0x98: printf("SBB    B"); break;
	case 0x99: printf("SBB    C"); break;
	case 0x9a: printf("SBB    D"); break;
	case 0x9b: printf("SBB    E"); break;
	case 0x9c: printf("SBB    H"); break;
	case 0x9d: printf("SBB    L"); break;
	case 0x9e: printf("SBB    M"); break;
	case 0x9f: printf("SBB    A"); break;

	case 0xa0: printf("ANA    B"); break;
	case 0xa1: printf("ANA    C"); break;
	case 0xa2: printf("ANA    D"); break;
	case 0xa3: printf("ANA    E"); break;
	case 0xa4: printf("ANA    H"); break;
	case 0xa5: printf("ANA    L"); break;
	case 0xa6: printf("ANA    M"); break;
	case 0xa7: printf("ANA    A"); break;
	case 0xa8: printf("XRA    B"); break;
	case 0xa9: printf("XRA    C"); break;
	case 0xaa: printf("XRA    D"); break;
	case 0xab: printf("XRA    E"); break;
	case 0xac: printf("XRA    H"); break;
	case 0xad: printf("XRA    L"); break;
	case 0xae: printf("XRA    M"); break;
	case 0xaf: printf("XRA    A"); break;

	case 0xb0: printf("ORA    B"); break;
	case 0xb1: printf("ORA    C"); break;
	case 0xb2: printf("ORA    D"); break;
	case 0xb3: printf("ORA    E"); break;
	case 0xb4: printf("ORA    H"); break;
	case 0xb5: printf("ORA    L"); break;
	case 0xb6: printf("ORA    M"); break;
	case 0xb7: printf("ORA    A"); break;
	case 0xb8: printf("CMP    B"); break;
	case 0xb9: printf("CMP    C"); break;
	case 0xba: printf("CMP    D"); break;
	case 0xbb: printf("CMP    E"); break;
	case 0xbc: printf("CMP    H"); break;
	case 0xbd: printf("CMP    L"); break;
	case 0xbe: printf("CMP    M"); break;
	case 0xbf: printf("CMP    A"); break;

	case 0xc0: printf("RNZ"); break;
	case 0xc1: printf("POP    B"); break;
	case 0xc2: printf("JNZ    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xc3: printf("JMP    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xc4: printf("CNZ    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xc5: printf("PUSH   B"); break;
	case 0xc6: printf("ADI    #$%02x", code1); opbytes = 2; break;
	case 0xc7: printf("RST    0"); break;
	case 0xc8: printf("RZ"); break;
	case 0xc9: printf("RET"); break;
	case 0xca: printf("JZ     $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xcb: printf("JMP    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xcc: printf("CZ     $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xcd: printf("CALL   $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xce: printf("ACI    #$%02x", code1); opbytes = 2; break;
	case 0xcf: printf("RST    1"); break;

	case 0xd0: printf("RNC"); break;
	case 0xd1: printf("POP    D"); break;
	case 0xd2: printf("JNC    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xd3: printf("OUT    #$%02x", code1); opbytes = 2; break;
	case 0xd4: printf("CNC    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xd5: printf("PUSH   D"); break;
	case 0xd6: printf("SUI    #$%02x", code1); opbytes = 2; break;
	case 0xd7: printf("RST    2"); break;
	case 0xd8: printf("RC");  break;
	case 0xd9: printf("RET"); break;
	case 0xda: printf("JC     $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xdb: printf("IN     #$%02x", code1); opbytes = 2; break;
	case 0xdc: printf("CC     $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xdd: printf("CALL   $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xde: printf("SBI    #$%02x", code1); opbytes = 2; break;
	case 0xdf: printf("RST    3"); break;

	case 0xe0: printf("RPO"); break;
	case 0xe1: printf("POP    H"); break;
	case 0xe2: printf("JPO    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xe3: printf("XTHL");break;
	case 0xe4: printf("CPO    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xe5: printf("PUSH   H"); break;
	case 0xe6: printf("ANI    #$%02x", code1); opbytes = 2; break;
	case 0xe7: printf("RST    4"); break;
	case 0xe8: printf("RPE"); break;
	case 0xe9: printf("PCHL");break;
	case 0xea: printf("JPE    $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xeb: printf("XCHG"); break;
	case 0xec: printf("CPE     $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xed: printf("CALL   $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xee: printf("XRI    #$%02x", code1); opbytes = 2; break;
	case 0xef: printf("RST    5"); break;

	case 0xf0: printf("RP");  break;
	case 0xf1: printf("POP    PSW"); break;
	case 0xf2: printf("JP     $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xf3: printf("DI");  break;
	case 0xf4: printf("CP     $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xf5: printf("PUSH   PSW"); break;
	case 0xf6: printf("ORI    #$%02x", code1); opbytes = 2; break;
	case 0xf7: printf("RST    6"); break;
	case 0xf8: printf("RM");  break;
	case 0xf9: printf("SPHL");break;
	case 0xfa: printf("JM     $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xfb: printf("EI");  break;
	case 0xfc: printf("CM     $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xfd: printf("CALL   $%02x%02x", code2, code1); opbytes = 3; break;
	case 0xfe: printf("CPI    #$%02x", code1); opbytes = 2; break;
	case 0xff: printf("RST    7"); break;
	}
	
	return opbytes;
}



