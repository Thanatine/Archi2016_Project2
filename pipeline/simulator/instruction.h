#include <iostream>
#include <stdio.h>
#include <stdlib.h>
//R type
#define RTYPE 0x00 
#define ADD 0x20
#define ADDU 0x21
#define SUB 0x22
#define AND 0x24
#define OR 0x25
#define XOR 0x26
#define NOR 0x27
#define NAND 0x28
#define SLT 0x2A
#define SLL 0x00
#define SRL 0x02
#define SRA 0x03
#define JR 0x08

//I type
#define ADDI 0x08
#define ADDIU 0x09
#define LW 0x23
#define LH 0x21
#define LHU 0x25
#define LB 0x20
#define LBU 0x24
#define SW 0x2B
#define SH 0x29
#define SB 0x28
#define LUI 0x0F
#define ANDI 0x0C
#define ORI 0x0D
#define NORI 0x0E
#define SLTI 0x0A
#define BEQ 0x04
#define BNE 0x05
#define BGTZ 0x07

//J type
#define J 0x02
#define JAL 0x03

//Specialized Instruction
#define HALT 0x3F

using namespace std;

class Instruction
{
public:
    unsigned int opcode, funct, rt, rs, rd;
    unsigned int A, B, ALUOut, signExtend, shamt, MDR;
    bool nopSignal;
    
};

class Pipeline
{
public:
    //fields
    
    ///simulation variables
    FILE *errorDump, *snapshot;
	unsigned int reg[32];
    unsigned int iMemory[256], dMemory[256];
    unsigned int PC, cycle;
    
    ///instruction variables
    Instruction readID, readEX, readMEM, readWB;
    bool haltIF, haltID, haltEX, haltMEM, haltWB;
    unsigned int offset;
    bool forwardingEXDMtoIDEX_Rs_Signal, forwardingEXDMtoIDEX_Rt_Signal, forwardingEXDMtoIFID_Rs_Signal, forwardingEXDMtoIFID_Rt_Signal;
    bool stallSignal;
    bool toFlushSignal;
    bool shutDownFlag, zeroWrittenFlag;
    
    //methods
	unsigned int changeEndian(unsigned int hex);
    void IF(unsigned int input);    void ID(unsigned int input);    void EX();      void MEM();     void WB();
    bool zeroWrittenDetection();    void numberOverflowDetection();     bool addressOverflowDetection();    bool dataMisalignedDetection();
	void printreg();    void printStage();
    
};
