#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "instruction.h"

using namespace std;

int main()
{
    //file variable
    FILE *iimage, *dimage;
    int iSize, dSize;
    unsigned int *iInstruction, *dInstruction;
	size_t iresult, dresult;
    
    //pipeline initiates
	Pipeline pipeline;
    pipeline.cycle = 0;
    pipeline.PC = 0;
	pipeline.haltIF = false;
    pipeline.haltID = false;
    pipeline.haltEX = false;
    pipeline.haltMEM = false;
    pipeline.haltWB = false;
    pipeline.shutDownFlag = false;
    pipeline.zeroWrittenFlag = false;
    pipeline.stallSignal = false;
    pipeline.toFlushSignal = false;
    pipeline.forwardingEXDMtoIDEX_Rs_Signal = false;
    pipeline.forwardingEXDMtoIDEX_Rt_Signal = false;
    pipeline.forwardingEXDMtoIFID_Rs_Signal = false;
    pipeline.forwardingEXDMtoIFID_Rt_Signal = false;
	for(int i = 0 ; i < 32 ; i++)	pipeline.reg[i] = 0;
	for(int i = 0 ; i < 256 ; i++)	pipeline.dMemory[i] = 0;
    for(int i = 0 ; i < 256 ; i++)  pipeline.iMemory[i] = 0;

    //readfile
    iimage = fopen ("iimage.bin" ,"rb" );
    dimage = fopen ("dimage.bin", "rb");

    fseek(iimage, 0, SEEK_END);
    fseek(dimage, 0, SEEK_END);
    iSize = ftell(iimage);
    dSize = ftell(dimage);
    rewind (iimage);
    rewind (dimage);

    iInstruction = (unsigned int*) malloc (sizeof(unsigned int) * iSize);
    dInstruction = (unsigned int*) malloc (sizeof(unsigned int) * dSize);

    iresult = fread (iInstruction, sizeof(unsigned int), iSize, iimage);
    dresult = fread (dInstruction, sizeof(unsigned int), dSize, dimage);
	
	pipeline.errorDump = fopen("error_dump.rpt", "w");
	pipeline.snapshot = fopen("snapshot.rpt", "w");	

	//data access
	//iMemory
    for(int i = 0 ; i < iInstruction[1] + 2; i++){
		iInstruction[i] = pipeline.changeEndian(iInstruction[i]);
		//printf("%X\n", iInstruction[i]);
		pipeline.iMemory[i] = iInstruction[i];
	}
    //cout<<endl;
    
    //dMemory
	for(int i = 0 ; i < dInstruction[1] + 2 ; i++){
		dInstruction[i] = pipeline.changeEndian(dInstruction[i]);
	}
	
	pipeline.reg[29] = dInstruction[0];
	for(int i = 2 ; i < 2 + dInstruction[1] ; i++){
		pipeline.dMemory[i - 2 ] = dInstruction[i];
		//printf("%X\n", pipeline.dMemory[i - 2]);
	}
	
	//simulation starts
	pipeline.PC = pipeline.iMemory[0];
    pipeline.readEX.nopSignal = true;
    pipeline.readMEM.nopSignal = true;
    pipeline.readWB.nopSignal = true;
    pipeline.readID.nopSignal = true;
    
	while((!pipeline.haltIF || !pipeline.haltID || !pipeline.haltEX || !pipeline.haltMEM || !pipeline.haltWB) && !pipeline.shutDownFlag){
        pipeline.printreg();
        
        //WB
        pipeline.WB();
        //cout<<endl<<"WB done"<<endl;
        
        //MEM
        pipeline.MEM();
        //cout<<"MEM done"<<endl;
        
        //EX
        pipeline.EX();
        //cout<<"EX done"<<endl;
        
        //ID
        if(pipeline.PC <= pipeline.iMemory[0]){
            pipeline.ID(0);
        }
        else{
            pipeline.ID(pipeline.iMemory[(pipeline.PC - 4 - pipeline.iMemory[0]) / 4 + 2]);
        }
        //cout<<"ID done"<<endl;
    
        pipeline.printStage();
        
        //IF
        if(pipeline.PC < pipeline.iMemory[0])
            pipeline.IF(0);
        else
            pipeline.IF(pipeline.iMemory[(pipeline.PC - pipeline.iMemory[0]) / 4 + 2]);
        //cout<<"IF done"<<endl;
      
        pipeline.cycle++;
        if(pipeline.cycle > 500000) break;
    }

	return 0;
}


