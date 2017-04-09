#include "instruction.h"

unsigned int Pipeline::changeEndian(unsigned int hex)
{
	return (hex<<24 & 0xff000000) | (hex << 8 & 0x00ff0000) | (hex >> 8 & 0x0000ff00) | (hex >> 24 & 0x000000ff); 
}

void Pipeline::IF(unsigned int input)
{
    haltIF = false;
    if((input >> 26) == HALT)   haltIF = true;
    /*if(zeroWrittenFlag == true){
        return;
    }*/
    
    readWB = readMEM;
    readMEM = readEX;
    
    if(stallSignal == true){
        //cout<<"??"<<endl;
        readEX.nopSignal = true;
        stallSignal = false;
    }
    else if(toFlushSignal == true){
        //cout<<"!!"<<endl;
        readEX = readID;
        PC = offset;
    }
    else{
        //cout<<"@@"<<endl;
        readEX = readID;
        PC = PC + 4;
    }
    
    return;
    
}

void Pipeline::ID(unsigned input)
{
    //Control unit decode
    //Hazard detect, Branch/Jump detect, Branch
    //flush
    
    readID.opcode = input >> 26;
    readID.funct = input << 26 >> 26;
    readID.rt = input << 11 >> 27;
    readID.rs = input << 6 >> 27;
    readID.rd = input << 16 >> 27;
    readID.shamt = input << 21 >> 27;
    
    if(toFlushSignal == true){
        readID.nopSignal = true;
        toFlushSignal = false;
        return;
    }
    
    if(readID.opcode == 0 && readID.funct == 0 && readID.rt == 0 && readID.rd == 0 && readID.shamt == 0){
        readID.nopSignal = true;
        return;
    }
    else {
        readID.nopSignal = false;
    }
    
    if(readID.opcode == HALT){
        haltID = true;
        return;
    }
    else {
        haltID = false;
    }
    
    
    //Stall IF/ID and ID/EX [branch]
    if(readID.opcode == BEQ || readID.opcode == BNE || readID.opcode == BGTZ || (readID.opcode == RTYPE && readID.funct == JR)){
        if((readEX.opcode == RTYPE && readEX.funct != JR)
           && readEX.nopSignal == false
           && readEX.rd != 0)
        {
            if(readID.rs == readEX.rd){
                stallSignal = true;
                return;
            }
            else if(readID.rt == readEX.rd && readID.opcode != BGTZ){
                stallSignal = true;
                return;
            }
 
        }
        else if((readEX.opcode == ADDI || readEX.opcode == ADDIU || readEX.opcode == LUI || readEX.opcode == ANDI || readEX.opcode == ORI
                || readEX.opcode == NORI || readEX.opcode == SLTI)
                && readEX.nopSignal == false
                && readEX.rt != 0)
        {
            if(readID.rs == readEX.rt){
                stallSignal = true;
                return;
            }
            else if(readID.rt == readEX.rt && readID.opcode != BGTZ){
                stallSignal = true;
                return;
            }
        }
    }
    
    //Stall IF/ID and EX/DM [non branch]
    if(readMEM.opcode == JAL && readMEM.nopSignal == false){
        if(readID.opcode == RTYPE && readID.funct != JR){
            if(readID.rs == 31 || readID.rt == 31){
                stallSignal = true;
                
                if(readEX.opcode == RTYPE && readEX.nopSignal == false && readEX.rd == 31){
                    stallSignal = false;
                }
                else if((readEX.opcode != LW && readEX.opcode != LH && readEX.opcode != LHU && readEX.opcode != LB && readEX.opcode != LBU
                         && readEX.opcode != SW && readEX.opcode != SB && readEX.opcode != SH
                         && readEX.opcode != BEQ && readEX.opcode != BNE && readEX.opcode != BGTZ
                         && !(readID.opcode == RTYPE && readID.funct == JR) && readID.opcode != J && readID.opcode != JAL)
                        && readEX.rt == 31
                        && readEX.nopSignal == false){
                    stallSignal = false;
                }
                
                if(stallSignal == true){
                    return;
                }
                
            }
        }
        else if(readID.opcode != RTYPE && readID.opcode != BNE && readID.opcode != BEQ && readID.opcode != BGTZ){
            if(readID.rs == 31){
                stallSignal = true;
                
                if(readEX.opcode == RTYPE && readEX.nopSignal == false && readEX.rd == 31){
                    stallSignal = false;
                }
                else if((readEX.opcode != LW && readEX.opcode != LH && readEX.opcode != LHU && readEX.opcode != LB && readEX.opcode != LBU
                         && readEX.opcode != SW && readEX.opcode != SB && readEX.opcode != SH
                         && readEX.opcode != BEQ && readEX.opcode != BNE && readEX.opcode != BGTZ
                         && !(readID.opcode == RTYPE && readID.funct == JR) && readID.opcode != J && readID.opcode != JAL)
                        && readEX.rt == 31
                        && readEX.nopSignal == false){
                    stallSignal = false;
                }
                
                if(stallSignal == true){
                    return;
                }
                
            }
            else if(readID.rt == 31 && (readID.opcode == SW || readID.opcode == SH || readID.opcode == SB)){
                stallSignal = true;
                if(readEX.opcode == RTYPE && readEX.nopSignal == false && readEX.rd == 31){
                    stallSignal = true;
                }
                else if((readEX.opcode != LW && readEX.opcode != LH && readEX.opcode != LHU && readEX.opcode != LB && readEX.opcode != LBU
                         && readEX.opcode != SW && readEX.opcode != SB && readEX.opcode != SH
                         && readEX.opcode != BEQ && readEX.opcode != BNE && readEX.opcode != BGTZ
                         && !(readID.opcode == RTYPE && readID.funct == JR) && readID.opcode != J && readID.opcode != JAL)
                        && readEX.rt == 31
                        && readEX.nopSignal == false){
                    stallSignal = false;
                }
                
                if(stallSignal == true){
                    return;
                }
                
            }
        }
    }


    
    if(readMEM.opcode == RTYPE && readMEM.funct != JR
       && readID.opcode != BEQ && readID.opcode != BNE && readID.opcode != BGTZ && !(readID.opcode == RTYPE && readID.funct == JR)
       && readID.opcode != J && readID.opcode != JAL
       && readMEM.nopSignal == false && readID.nopSignal == false
       && readMEM.rd != 0)
    {
        if(readMEM.rd == readID.rs
           && !(forwardingEXDMtoIDEX_Rs_Signal == true && readEX.rs == readID.rs && readEX.rd == readEX.rs && readEX.opcode == RTYPE)
           && !(forwardingEXDMtoIDEX_Rs_Signal == true && readEX.rs == readID.rs && readEX.rt == readEX.rs && readEX.opcode != RTYPE
                && readEX.opcode != SW && readEX.opcode != SH && readEX.opcode != SB)
           && !(forwardingEXDMtoIDEX_Rt_Signal == true && readEX.rt == readID.rs && readEX.rd == readEX.rt && readEX.opcode == RTYPE)
           && !(readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRL || readID.funct == SRA))
           && readID.opcode != LUI)
        {
            stallSignal = true;
            if(((readEX.opcode == RTYPE && readEX.funct != JR))
               && ((readID.opcode == RTYPE && readID.funct != JR)
                   || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                   || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                   || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SB || readID.opcode == SH)
               && (readID.nopSignal == false && readEX.nopSignal == false))
            {
                if(readEX.rd == readID.rs && readEX.rd == readMEM.rd
                   && !((readID.opcode == RTYPE && readID.funct == SLL && readID.funct == SRA && readID.funct == SRL) || readID.opcode == LUI)
                   && readEX.rd != 0){
                        stallSignal = false;
                }
                
                if(readEX.rd == readID.rt && readEX.rd == readMEM.rd && readEX.rd != 0){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        stallSignal = false;
                    }
                }
            }
            else if((readEX.opcode == ADDI || readEX.opcode == ADDIU || readEX.opcode == LUI || readEX.opcode == ANDI || readEX.opcode == ORI
                     || readEX.opcode == NORI || readEX.opcode == SLTI || readEX.opcode == JAL)
                    && ((readID.opcode == RTYPE && readID.funct != JR )
                        || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                        || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                        || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB)
                    && (readID.nopSignal == false && readEX.nopSignal == false))
                
            {
                if(readEX.opcode == JAL && readID.rs == 31 && readMEM.rd == 31){
                    stallSignal = false;
                }
                else if(readEX.opcode == JAL && readID.rt == 31 && readMEM.rd == 31){
                    stallSignal = false;
                }
                
                if(readEX.rt == readID.rs && readMEM.rd == readEX.rt
                   && !((readID.opcode == RTYPE && readID.funct == SLL && readID.funct == SRA && readID.funct == SRL) || readID.opcode == LUI)
                   && readEX.rt != 0){
                    stallSignal = false;
                }
                
                if(readEX.rt == readID.rt && readMEM.rd == readEX.rt && readEX.rt != 0){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        stallSignal = false;
                    }
                }
                
            }
            
            
            if(stallSignal == true){
                return;
            }
            
        }
        else if(readMEM.rd == readID.rt
                && (readID.opcode == SW || readID.opcode == SH || readID.opcode == SB || (readID.opcode == RTYPE))
                && !(forwardingEXDMtoIDEX_Rs_Signal == true && readEX.rs == readID.rt && readEX.rd == readEX.rs && readEX.opcode == RTYPE)
                && !(forwardingEXDMtoIDEX_Rs_Signal == true && readEX.rs == readID.rt && readEX.rt == readEX.rs && readEX.opcode != RTYPE
                     && readEX.opcode != SW && readEX.opcode != SH && readEX.opcode != SB)
                && !(forwardingEXDMtoIDEX_Rt_Signal == true && readEX.rt == readID.rt && readEX.rd == readEX.rt && readEX.opcode == RTYPE))
        {
            stallSignal = true;
            if(((readEX.opcode == RTYPE && readEX.funct != JR))
               && ((readID.opcode == RTYPE && readID.funct != JR )
                   || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                   || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                   || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SB || readID.opcode == SH)
               && (readID.nopSignal == false && readEX.nopSignal == false))
            {
                if(readEX.rd == readID.rs && readMEM.rd == readEX.rd
                   && !((readID.opcode == RTYPE && readID.funct == SLL && readID.funct == SRA && readID.funct == SRL) || readID.opcode == LUI)
                   && readEX.rd != 0){
                    stallSignal = false;
                }
                
                if(readEX.rd == readID.rt && readEX.rd != 0 && readMEM.rd == readEX.rd){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        stallSignal = false;
                    }
                }
            }
            else if((readEX.opcode == ADDI || readEX.opcode == ADDIU || readEX.opcode == LUI || readEX.opcode == ANDI || readEX.opcode == ORI
                     || readEX.opcode == NORI || readEX.opcode == SLTI || readEX.opcode == JAL)
                    && ((readID.opcode == RTYPE && readID.funct != JR )
                        || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                        || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                        || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB)
                    && (readID.nopSignal == false && readEX.nopSignal == false))
                
            {
                if(readEX.opcode == JAL && readID.rs == 31 && readMEM.rd == 31){
                    stallSignal = false;
                }
                else if(readEX.opcode == JAL && readID.rt == 31 && readMEM.rd == 31){
                    stallSignal = false;
                }
                
                if(readEX.rt == readID.rs && readEX.rt == readMEM.rd
                   && !((readID.opcode == RTYPE && readID.funct == SLL && readID.funct == SRA && readID.funct == SRL) || readID.opcode == LUI)
                   && readEX.rt != 0){
                    stallSignal = false;
                }
                
                if(readEX.rt == readID.rt && readEX.rt == readMEM.rd && readEX.rt != 0){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        stallSignal = false;
                    }
                }
                
            }
            
            if(stallSignal == true){
                return;
            }
            
        }
    }
    else if(readMEM.opcode != LW && readMEM.opcode != LH && readMEM.opcode != LHU && readMEM.opcode != LB && readMEM.opcode != LBU
            && readMEM.opcode != SW && readMEM.opcode != SB && readMEM.opcode != SH
            && readMEM.opcode != BEQ && readMEM.opcode != BNE && readMEM.opcode != BGTZ
            && readMEM.opcode != RTYPE
            && readID.opcode != BNE && readID.opcode != BEQ && readID.opcode != BGTZ && !(readID.opcode == RTYPE && readID.funct == JR)
            && readID.opcode != J && readID.opcode != JAL
            && readMEM.nopSignal == false && readID.nopSignal == false
            && readMEM.rt != 0)
    {

        //cout<<"1"<<endl;
        if(readMEM.rt == readID.rs
            && !(forwardingEXDMtoIDEX_Rs_Signal == true && readEX.rs == readID.rs && readEX.rd == readEX.rs && readEX.opcode == RTYPE)
            && !(forwardingEXDMtoIDEX_Rs_Signal == true && readEX.rs == readID.rs && readEX.rt == readEX.rs && readEX.opcode != RTYPE
                 && readEX.opcode != SW && readEX.opcode != SH && readEX.opcode != SB)
            && !(forwardingEXDMtoIDEX_Rt_Signal == true && readEX.rt == readID.rs && readEX.rd == readEX.rt && readEX.opcode == RTYPE)
            && !(readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRL || readID.funct == SRA))
            && readID.opcode != LUI)
        {
            //cout<<"2"<<endl;
            stallSignal = true;
            if(((readEX.opcode == RTYPE && readEX.funct != JR))
               && ((readID.opcode == RTYPE && readID.funct != JR )
                   || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                   || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                   || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SB || readID.opcode == SH)
               && (readID.nopSignal == false && readEX.nopSignal == false))
            {
                //cout<<"3"<<endl;
                if(readEX.rd == readID.rs && readEX.rd == readMEM.rt
                   && !((readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRL || readID.funct == SRA)) || readID.opcode == LUI)
                   && readEX.rd != 0){
                    stallSignal = false;
                }
                
                if(readEX.rd == readID.rt && readEX.rd == readMEM.rt && readEX.rd != 0){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        stallSignal = false;
                    }
                }
            }
            else if((readEX.opcode == ADDI || readEX.opcode == ADDIU || readEX.opcode == LUI || readEX.opcode == ANDI || readEX.opcode == ORI
                     || readEX.opcode == NORI || readEX.opcode == SLTI || readEX.opcode == JAL)
                    && ((readID.opcode == RTYPE && readID.funct != JR )
                        || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                        || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                        || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB)
                    && (readID.nopSignal == false && readEX.nopSignal == false))
                
            {
                if(readEX.opcode == JAL && readID.rs == 31 && readMEM.rt == 31){
                    stallSignal = false;
                }
                else if(readEX.opcode == JAL && readID.rt == 31 && readMEM.rt == 31){
                    stallSignal = false;
                }
                
                if(readEX.rt == readID.rs && readEX.rt == readMEM.rt
                   && !((readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRL || readID.funct == SRA)) || readID.opcode == LUI)
                   && readEX.rt != 0){
                    stallSignal = false;
                }
                
                if(readEX.rt == readID.rt && readEX.rt == readMEM.rt && readEX.rt != 0){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        stallSignal = false;
                    }
                }
                
            }
            
            if(stallSignal == true){
                return;
            }
            
        }
        else if(readMEM.rt == readID.rt
                && (readID.opcode == SW || readID.opcode == SH || readID.opcode == SB || (readID.opcode == RTYPE))
                && !(forwardingEXDMtoIDEX_Rs_Signal == true && readEX.rs == readID.rt && readEX.rd == readEX.rs && readEX.opcode == RTYPE)
                && !(forwardingEXDMtoIDEX_Rs_Signal == true && readEX.rs == readID.rt && readEX.rt == readEX.rs && readEX.opcode != RTYPE
                     && readEX.opcode != SW && readEX.opcode != SH && readEX.opcode != SB)
                && !(forwardingEXDMtoIDEX_Rt_Signal == true && readEX.rt == readID.rt && readEX.rd == readEX.rt && readEX.opcode == RTYPE))
        {
            stallSignal = true;
            //cout<<"2"<<endl;
            if(((readEX.opcode == RTYPE && readEX.funct != JR))
               && ((readID.opcode == RTYPE && readID.funct != JR )
                   || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                   || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                   || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SB || readID.opcode == SH)
               && (readID.nopSignal == false && readEX.nopSignal == false))
            {
                //cout<<"3"<<endl;
                if(readEX.rd == readID.rs && readEX.rd == readMEM.rt
                   && !((readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRL || readID.funct == SRA)) || readID.opcode == LUI)
                   && readEX.rd != 0){
                    //cout<<"4"<<endl;
                    stallSignal = false;
                }
                
                if(readEX.rd == readID.rt && readEX.rd == readMEM.rt && readEX.rd != 0){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        //cout<<"5"<<endl;
                        stallSignal = false;
                    }
                }
            }
            else if((readEX.opcode == ADDI || readEX.opcode == ADDIU || readEX.opcode == LUI || readEX.opcode == ANDI || readEX.opcode == ORI
                     || readEX.opcode == NORI || readEX.opcode == SLTI || readEX.opcode == JAL)
                    && ((readID.opcode == RTYPE && readID.funct != JR )
                        || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                        || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                        || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB)
                    && (readID.nopSignal == false && readEX.nopSignal == false))
                
            {
                if(readEX.opcode == JAL && readID.rs == 31 && readMEM.rt == 31){
                    stallSignal = false;
                }
                else if(readEX.opcode == JAL && readID.rt == 31 && readMEM.rt == 31){
                    stallSignal = false;
                }
                
                if(readEX.rt == readID.rs && readMEM.rt == readEX.rt
                   && !((readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRL || readID.funct == SRA)) || readID.opcode == LUI)
                   && readEX.rt != 0){
                    stallSignal = false;
                }
                
                if(readEX.rt == readID.rt && readEX.rt != 0 && readMEM.rt == readEX.rt){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        stallSignal = false;
                    }
                }
                
            }
            
            
            if(stallSignal == true){
                return;
            }
            
        }
    }
    
    //cout<<"0"<<endl;
    //LoadUseStall IF/ID and ID/EX
    if((readEX.opcode == LW || readEX.opcode == LH || readEX.opcode == LHU || readEX.opcode == LB || readEX.opcode == LBU)
            && readEX.rt == readID.rs && (readID.nopSignal == false && readEX.nopSignal == false)
            && readEX.rt != 0 && readID.rs != 0)
    {
        //cout<<"0.1"<<endl;
        if(!((readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRA || readID.funct == SRL)) || readID.opcode == LUI
             || readID.opcode == J || readID.opcode == JAL))
        {
            //cout<<"0.2"<<endl;
            stallSignal = true;
            return;
        }
    }
    if((readEX.opcode == LW || readEX.opcode == LH || readEX.opcode == LHU || readEX.opcode == LB || readEX.opcode == LBU)
                && readEX.rt == readID.rt && (readID.nopSignal == false && readEX.nopSignal == false)
                && readEX.rt != 0 && readID.rt != 0)
    {
        //cout<<"1"<<endl;
            if((readID.opcode == RTYPE && readID.funct != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB
               || readID.opcode == BEQ || readID.opcode == BNE)
            {
                //cout<<"2"<<endl;
                stallSignal = true;
                return;
            }
    }
    
    //LoadUseStall IF/ID and EX/DM
    if((readMEM.opcode == LW || readMEM.opcode == LH || readMEM.opcode == LHU || readMEM.opcode == LB || readMEM.opcode == LBU)
            && readMEM.rt == readID.rs && (readID.nopSignal == false && readMEM.nopSignal == false)
            && readMEM.rt != 0 && readID.rs != 0)
    {
        if(!((readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRA || readID.funct == SRL)) || readID.opcode == LUI
             || readID.opcode == J || readID.opcode == JAL))
        {
            stallSignal = true;
            
            if(((readEX.opcode == RTYPE && readEX.funct != JR))
               && ((readID.opcode == RTYPE && readID.funct != JR )
                   || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                   || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                   || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SB || readID.opcode == SH)
               && (readID.nopSignal == false && readEX.nopSignal == false))
            {
                if(readEX.rd == readID.rs && readEX.rd == readMEM.rt
                   && !((readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRL || readID.funct == SRA)) || readID.opcode == LUI)
                   && readEX.rd != 0){
                    stallSignal = false;
                }
                
                /*if(readEX.rd == readID.rt && readEX.rd == readMEM.rt && readEX.rd != 0){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        stallSignal = false;
                    }
                }*/
            }
            else if((readEX.opcode == ADDI || readEX.opcode == ADDIU || readEX.opcode == LUI || readEX.opcode == ANDI || readEX.opcode == ORI
                     || readEX.opcode == NORI || readEX.opcode == SLTI || readEX.opcode == JAL)
                    && ((readID.opcode == RTYPE && readID.funct != JR )
                        || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                        || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                        || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB)
                    && (readID.nopSignal == false && readEX.nopSignal == false))
                
            {
                if(readEX.opcode == JAL && readID.rs == 31 && readMEM.rt == 31){
                    stallSignal = false;
                }
                else if(readEX.opcode == JAL && readID.rt == 31 && readMEM.rt == 31){
                    stallSignal = false;
                }
                
                if(readEX.rt == readID.rs && readEX.rt == readMEM.rt
                   && !((readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRL || readID.funct == SRA)) || readID.opcode == LUI)
                   && readEX.rt != 0){
                    stallSignal = false;
                }
                
                /*if(readEX.rt == readID.rt && readEX.rt == readMEM.rt && readEX.rt != 0){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        stallSignal = false;
                    }
                }*/
                
            }

            if(stallSignal == true)     return;
            
        }

    }
    if((readMEM.opcode == LW || readMEM.opcode == LH || readMEM.opcode == LHU || readMEM.opcode == LB || readMEM.opcode == LBU)
            && readMEM.rt == readID.rt && (readID.nopSignal == false && readMEM.nopSignal == false)
            && readMEM.rt != 0 && readID.rt != 0)
    {
        if((readID.opcode == RTYPE && readID.funct != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB
            || readID.opcode == BEQ || readID.opcode == BNE)
        {
            stallSignal = true;
            
            if(((readEX.opcode == RTYPE && readEX.funct != JR))
               && ((readID.opcode == RTYPE && readID.funct != JR )
                   || readID.opcode == ADDI || readID.opcode == ADDIU || readID.opcode == LUI || readID.opcode == ANDI || readID.opcode == ORI
                   || readID.opcode == NORI || readID.opcode == SLTI || readID.opcode == LW || readID.opcode == LH || readID.opcode == LHU
                   || readID.opcode == LB || readID.opcode == LBU || readID.opcode == SW || readID.opcode == SB || readID.opcode == SH)
               && (readID.nopSignal == false && readEX.nopSignal == false))
            {
                /*if(readEX.rd == readID.rs && readEX.rd == readMEM.rt
                   && !((readID.opcode == RTYPE && (readID.funct == SLL || readID.funct == SRL || readID.funct == SRA)) || readID.opcode == LUI)
                   && readEX.rd != 0){
                    stallSignal = false;
                }*/
                
                if(readEX.rd == readID.rt && readEX.rd == readMEM.rt && readEX.rd != 0){
                    if((readID.opcode == RTYPE && readID.opcode != JR) || readID.opcode == SW || readID.opcode == SH || readID.opcode == SB){
                        stallSignal = false;
                    }
                }
            }
            
            if(stallSignal == true)     return;
        }

    }

    //forwarding IF/ID from EX/DM [branch]
    if((readMEM.opcode == RTYPE && readMEM.funct != JR)
       && (readID.opcode == BEQ || readID.opcode == BNE || readID.opcode == BGTZ || (readID.opcode == RTYPE && readID.funct == JR))
       && (readID.nopSignal == false && readMEM.nopSignal == false))
    {
        if(readMEM.rd == readID.rs && readMEM.rd != 0 && readID.rs != 0){
            forwardingEXDMtoIFID_Rs_Signal = true;
        }
        
        if(readMEM.rd == readID.rt && readID.opcode != BGTZ && !(readID.opcode == RTYPE && readID.funct == JR)
                && readMEM.rd!= 0 && readID.rt != 0){
            forwardingEXDMtoIFID_Rt_Signal = true;
        }
        
    }
    else if((readMEM.opcode == ADDI || readMEM.opcode == ADDIU || readMEM.opcode == LUI || readMEM.opcode == ANDI || readMEM.opcode == ORI
             || readMEM.opcode == NORI || readMEM.opcode == SLTI || readMEM.opcode == JAL)
            && (readID.opcode == BEQ || readID.opcode == BNE || readID.opcode == BGTZ || (readID.opcode == RTYPE && readID.funct == JR))
            && (readID.nopSignal == false && readMEM.nopSignal == false))
    {
        if(readID.rs == 31 && readMEM.opcode == JAL){
            forwardingEXDMtoIFID_Rs_Signal = true;
        }
        if(readID.rt == 31 && readMEM.opcode == JAL && readID.opcode != BGTZ && !(readID.opcode == RTYPE && readID.funct == JR)){
            forwardingEXDMtoIFID_Rt_Signal = true;
        }
        
        if(readMEM.rt == readID.rs && readMEM.rt != 0 && readID.rs != 0){
            forwardingEXDMtoIFID_Rs_Signal = true;
        }
        
        if(readMEM.rt == readID.rt && readID.opcode != BGTZ && !(readID.opcode == RTYPE && readID.funct == JR)
                && readMEM.rt != 0 && readID.rt != 0){
            forwardingEXDMtoIFID_Rt_Signal = true;
        }
        
    }
    
    switch(readID.opcode){
        //R type
        case RTYPE:
            readID.funct = input << 26 >> 26;
            switch(readID.funct){
                case SLL:
                    readID.B = reg[readID.rt];
                    break;
                    
                case SRL:
                    readID.B = reg[readID.rt];
                    break;
                    
                case SRA:
                    readID.B = reg[readID.rt];
                    break;
                    
                case JR:
                    if(forwardingEXDMtoIFID_Rs_Signal == true){
                        offset = readMEM.ALUOut;
                        toFlushSignal = true;
                    }
                    else{
                        offset = reg[readID.rs];
                        toFlushSignal = true;
                    }
        
                    break;
                    
                default:
                    readID.A = reg[readID.rs];
                    readID.B = reg[readID.rt];
                    break;
            }
            break;

        case BEQ:
            offset = ((int)(input << 16) >> 16);
            
            if(forwardingEXDMtoIFID_Rs_Signal == true && forwardingEXDMtoIFID_Rt_Signal == true){
                toFlushSignal = true;
                offset = PC + 4 * offset;
            }
            else if(forwardingEXDMtoIFID_Rs_Signal == true){
                if(readMEM.ALUOut == reg[readID.rt]){
                    toFlushSignal = true;
                    offset = PC + 4 * offset;
                }
            }
            else if(forwardingEXDMtoIFID_Rt_Signal == true){
                if(reg[readID.rs] == readMEM.ALUOut){
                    toFlushSignal = true;
                    offset = PC + 4 * offset;
                }
            }
            else{
                if(reg[readID.rs] == reg[readID.rt]){
                    toFlushSignal = true;
                    offset = PC + 4 * offset;
                }
            }
            //flush
            /*if(offset == PC + 4){
                toFlushSignal = false;
            }*/
            break;
            
        case BNE:
            offset = ((int)(input << 16) >> 16);
            if(forwardingEXDMtoIFID_Rs_Signal == true && forwardingEXDMtoIFID_Rt_Signal == true){
                toFlushSignal = false;
            }
            else if(forwardingEXDMtoIFID_Rs_Signal == true){
                if(readMEM.ALUOut != reg[readID.rt]){
                    toFlushSignal = true;
                    offset = PC + 4 * offset;
                }
            }
            else if(forwardingEXDMtoIFID_Rt_Signal == true){
                if(reg[readID.rs] != readMEM.ALUOut){
                    toFlushSignal = true;
                    offset = PC + 4 * offset;
                }
            }
            else{
                if(reg[readID.rs] != reg[readID.rt]){
                    toFlushSignal = true;
                    offset = PC + 4 * offset;
                }
            }
            //flush
            /*if(offset  == PC + 4){
                toFlushSignal = false;
            }*/
            break;
            
        case BGTZ:
            offset = ((int)(input << 16) >> 16);
            if(forwardingEXDMtoIFID_Rs_Signal == true){
                if((int)readMEM.ALUOut > 0){
                    toFlushSignal = true;
                    offset = PC + 4 * offset;
                }
            }
            else{
                if((int)reg[readID.rs] > 0){
                    toFlushSignal = true;
                    offset = PC + 4 * offset;
                }
            }
            //flush
            /*if(offset == PC + 4){
                toFlushSignal = false;
            }*/
            break;
            
        //J type
        case J:
            offset = input << 6 >> 6;
            offset = ((PC + 4) >> 28) | 4 * offset;
            toFlushSignal = true;
            //PC = PC - 4;
            //flush
            /*if(offset == PC + 4){
                toFlushSignal = false;
            }*/
            break;
            
        case JAL:
            offset = input << 6 >> 6;
            offset = ((PC + 4) >> 28) | 4 * offset;
            toFlushSignal = true;
            readID.A = PC;
            //PC = PC - 4;
            //flush
            /*if(offset == PC + 4){
                toFlushSignal = false;
            }*/
            break;
            
        //I type
        case ANDI:
            readID.signExtend = input << 16 >> 16;
            readID.A = reg[readID.rs];
            break;
            
        case ORI:
            readID.signExtend = input << 16 >> 16;
            readID.A = reg[readID.rs];
            break;
            
        case NORI:
            readID.signExtend = input << 16 >> 16;
            readID.A = reg[readID.rs];
            break;
            
        default:
            readID.signExtend = ((int)(input << 16) >> 16);
            readID.A = reg[readID.rs];
            readID.B = reg[readID.rt];
            //printf("A: %d, sign: 0x%X\n", readID.A, readID.signExtend);
            break;
            
    }
    
    return;

}

void Pipeline::EX()
{
    if(readEX.nopSignal == true)    return;
    
    if(readEX.opcode == HALT){
        haltEX = true;
        return;
    }
    else    haltEX = false;
    
    /*if(zeroWrittenFlag == true){
        return;
    }*/
    
    //forwarding EX/DM tp ID/EX
    if(((readMEM.opcode == RTYPE && readMEM.funct != JR))
       && ((readEX.opcode == RTYPE && readEX.funct != JR )
           || readEX.opcode == ADDI || readEX.opcode == ADDIU || readEX.opcode == LUI || readEX.opcode == ANDI || readEX.opcode == ORI
           || readEX.opcode == NORI || readEX.opcode == SLTI || readEX.opcode == LW || readEX.opcode == LH || readEX.opcode == LHU
           || readEX.opcode == LB || readEX.opcode == LBU || readEX.opcode == SW || readEX.opcode == SB || readEX.opcode == SH)
       && (readEX.nopSignal == false && readMEM.nopSignal == false))
    {
           if(readMEM.rd == readEX.rs
              && !((readEX.opcode == RTYPE && (readEX.funct == SLL || readEX.funct == SRA || readEX.funct == SRL)) || readEX.opcode == LUI)
              && readMEM.rd != 0)
           {
               forwardingEXDMtoIDEX_Rs_Signal = true;
               readEX.A = readMEM.ALUOut;
           }
        
           if(readMEM.rd == readEX.rt && readMEM.rd != 0)
           {
               
               if((readEX.opcode == RTYPE && readEX.opcode != JR) || readEX.opcode == SW || readEX.opcode == SH || readEX.opcode == SB)
               {
                   forwardingEXDMtoIDEX_Rt_Signal = true;
                   readEX.B = readMEM.ALUOut;
               }
           }
    }
    else if((readMEM.opcode == ADDI || readMEM.opcode == ADDIU || readMEM.opcode == LUI || readMEM.opcode == ANDI || readMEM.opcode == ORI
            || readMEM.opcode == NORI || readMEM.opcode == SLTI || readMEM.opcode == JAL)
            && ((readEX.opcode == RTYPE && readEX.funct != JR )
                || readEX.opcode == ADDI || readEX.opcode == ADDIU || readEX.opcode == LUI || readEX.opcode == ANDI || readEX.opcode == ORI
                || readEX.opcode == NORI || readEX.opcode == SLTI || readEX.opcode == LW || readEX.opcode == LH || readEX.opcode == LHU
                || readEX.opcode == LB || readEX.opcode == LBU || readEX.opcode == SW || readEX.opcode == SH || readEX.opcode == SB)
            && (readEX.nopSignal == false && readMEM.nopSignal == false))

    {
        if(readMEM.opcode == JAL && readEX.rs == 31){
            forwardingEXDMtoIDEX_Rs_Signal = true;
            readEX.A = readMEM.ALUOut;
        }
        else if(readMEM.opcode == JAL && readEX.rt == 31){
            forwardingEXDMtoIDEX_Rt_Signal = true;
            readEX.B = readMEM.ALUOut;
        }
        
        if(readMEM.rt == readEX.rs
           && !((readEX.opcode == RTYPE && (readEX.funct == SLL || readEX.funct == SRA || readEX.funct == SRL)) || readEX.opcode == LUI)
           && readMEM.rt != 0)
        {
            forwardingEXDMtoIDEX_Rs_Signal = true;
            readEX.A = readMEM.ALUOut;
        }
        
        if(readMEM.rt == readEX.rt && readMEM.rt != 0)
        {
            
            if((readEX.opcode == RTYPE && readEX.opcode != JR) || readEX.opcode == SW || readEX.opcode == SH || readEX.opcode == SB)
            {
                forwardingEXDMtoIDEX_Rt_Signal = true;
                readEX.B = readMEM.ALUOut;
            }
        }

    }
    
    numberOverflowDetection();
    
    switch(readEX.opcode){
		//R type
		case RTYPE:
			switch(readEX.funct){
				case ADD:
					readEX.ALUOut = readEX.A + readEX.B;
                    break;
				
                case ADDU:
					readEX.ALUOut = readEX.A + readEX.B;
					break;
				
                case SUB:
					readEX.ALUOut = readEX.A - readEX.B;
					break;
				
                case AND:
					readEX.ALUOut = readEX.A & readEX.B;
					break;
				
                case OR:
					readEX.ALUOut = readEX.A | readEX.B;
					break;
				
                case XOR:
					readEX.ALUOut = readEX.A ^ readEX.B;
					break;
				
                case NOR:
					readEX.ALUOut = ~(readEX.A | readEX.B);
					break;
				
                case NAND:
					readEX.ALUOut = ~(readEX.A & readEX.B);
					break;
				
                case SLT:
                    if((readEX.A >> 31) == 0){
                        if((readEX.B >> 31) == 0)    readEX.ALUOut = readEX.A < readEX.B;
                        else    readEX.ALUOut = readEX.A > readEX.B;
                    }
                    else{
                        if((readEX.B >> 31) == 0)    readEX.ALUOut = readEX.A > readEX.B;
                        else    readEX.ALUOut = readEX.A < readEX.B;
                    }
                    break;
				
                case SLL:
					readEX.ALUOut = (readEX.B << (readEX.shamt));
                    //fprintf(snapshot, "=====================> %X, %X\n", readEX.A, readEX.shamt);
					break;
				
                case SRL:
					readEX.ALUOut = (readEX.B >> (readEX.shamt));
					break; 
				
                case SRA:
                    readEX.ALUOut = ((int)readEX.B >> (readEX.shamt));
					break;
				
                default:
					break;
			} 
			break;
		//I type
		case ADDI:
			readEX.ALUOut = readEX.A + readEX.signExtend;
            //cout<<"EX:   A: "<<readEX.A<<" ALUOut: "<<readEX.ALUOut<<" extend: ";
            //printf("%X\n", readEX.signExtend);
			break;
		
        case ADDIU:
			readEX.ALUOut = readEX.A + readEX.signExtend;
			break;

        case LUI:
			readEX.ALUOut = (readEX.signExtend << 16);
			break;
		
        case ANDI:
			readEX.ALUOut = readEX.A & readEX.signExtend;
			break;
		
        case ORI:
			readEX.ALUOut = readEX.A | readEX.signExtend;
			break;
		
        case NORI:
			readEX.ALUOut = ~(readEX.A | readEX.signExtend);
			break;
		
        case SLTI:
            if((readEX.A >> 31) == 0){
                if(readEX.signExtend >> 31 == 0)  readEX.ALUOut = readEX.A < readEX.signExtend;
                else    readEX.ALUOut = readEX.A > readEX.signExtend;
            }
            else{
                if((readEX.signExtend >> 31) == 0)    readEX.ALUOut = readEX.A > readEX.signExtend;
                else    readEX.ALUOut = readEX.A < readEX.signExtend;
            }
			break;
       
        case LW:
            readEX.ALUOut = readEX.A + readEX.signExtend;
            break;
        
        case LH:
            readEX.ALUOut = readEX.A + readEX.signExtend;
            break;
            
        case LHU:
            readEX.ALUOut = readEX.A +readEX.signExtend;
            break;
            
        case LB:
            readEX.ALUOut = readEX.A + readEX.signExtend;
            break;
        
        case LBU:
            readEX.ALUOut = readEX.A + readEX.signExtend;
            break;
            
        case SW:
            readEX.ALUOut = readEX.A + readEX.signExtend;
            //printf("A: %d, sign: 0x%X, ALU: 0x%X\n", readEX.A, readEX.signExtend, readEX.ALUOut);
            break;
            
        case SH:
            readEX.ALUOut = readEX.A + readEX.signExtend;
            break;
            
        case SB:
            readEX.ALUOut = readEX.A + readEX.signExtend;
            break;
        
        case JAL:
            readEX.ALUOut = readEX.A;
            break;
            
        case HALT:
            haltEX = true;
            break;

        default:
           break;
				
    }
    
    return;
    
}


void Pipeline::MEM()
{
    if(readMEM.nopSignal == true)   return;
    if(readMEM.opcode == HALT){
        haltMEM = true;
        return;
    }
    
    /*if(zeroWrittenFlag == true){
        return;
    }*/
    
    shutDownFlag = addressOverflowDetection() | dataMisalignedDetection();
    if(shutDownFlag == true)  return;
    
    switch (readMEM.opcode) {
        case LW:
            readMEM.MDR = dMemory[(readMEM.ALUOut) / 4];
            break;
            
        case LH:
            if((readMEM.ALUOut) % 4 == 0)
                readMEM.MDR = (int)dMemory[(readMEM.ALUOut) / 4] >> 16;
            
            else
                readMEM.MDR = (int)(dMemory[(readMEM.ALUOut) / 4] << 16) >> 16;
            
            break;
            
        case LHU:
            if((readMEM.ALUOut) % 4 == 0)
                readMEM.MDR = dMemory[(readMEM.ALUOut) / 4] >> 16;
            
            else
                readMEM.MDR = dMemory[(readMEM.ALUOut) / 4] << 16 >> 16;
            
            break;
            
        case LB:
            if((readMEM.ALUOut) % 4 == 0)
                readMEM.MDR = (int)dMemory[(readMEM.ALUOut) / 4] >> 24;
            
            else if((readMEM.ALUOut) % 4 == 1)
                readMEM.MDR = (int)(dMemory[(readMEM.ALUOut) / 4] << 8) >> 24;
            
            else if((readMEM.ALUOut) % 4 == 2)
                readMEM.MDR = (int)(dMemory[(readMEM.ALUOut) / 4] << 16) >> 24;
            
            else
                readMEM.MDR = (int)(dMemory[(readMEM.ALUOut) / 4] << 24) >> 24;
            
            break;
            
        case LBU:
            if((readMEM.ALUOut) % 4 == 0)
                readMEM.MDR = dMemory[(readMEM.ALUOut) / 4 ] >> 24;
            
            else if((readMEM.ALUOut) % 4 == 1)
                readMEM.MDR = dMemory[(readMEM.ALUOut) / 4 ] << 8 >> 24;
            
            else if((readMEM.ALUOut) % 4 == 2)
                readMEM.MDR = dMemory[(readMEM.ALUOut) / 4 ] << 16 >> 24;
            
            else
                readMEM.MDR = dMemory[(readMEM.ALUOut) / 4 ] << 24 >> 24;
            
            break;
            
        //store directly
        case SW:
            dMemory[readMEM.ALUOut / 4] = readMEM.B;
            //cout<<"========"<<readMEM.B<<"========"<<endl;
            break;
            
        case SH:
            if((readMEM.ALUOut) % 4 == 0)
                dMemory[readMEM.ALUOut / 4] = (dMemory[(readMEM.ALUOut) / 4] & 0x0000FFFF) + ((readMEM.B & 0x0000FFFF) << 16);
            
            else
                dMemory[readMEM.ALUOut / 4] = (dMemory[(readMEM.ALUOut) / 4] & 0xFFFF0000) + (readMEM.B & 0x0000FFFF);
            
            break;
            
        case SB:
            if((readMEM.ALUOut) % 4 == 0)
                dMemory[readMEM.ALUOut / 4] = (dMemory[(readMEM.ALUOut) / 4] & 0x00FFFFFF) + ((readMEM.B & 0x000000FF) << 24);
            
            else if((readMEM.ALUOut) % 4 == 1)
                dMemory[readMEM.ALUOut / 4] = (dMemory[(readMEM.ALUOut) / 4] & 0xFF00FFFF) + ((readMEM.B & 0x000000FF) << 16);
            
            else if((readMEM.ALUOut) % 4 == 2)
                dMemory[readMEM.ALUOut / 4] = (dMemory[(readMEM.ALUOut) / 4] & 0xFFFF00FF) + ((readMEM.B & 0x000000FF) << 8);
            
            else
                dMemory[readMEM.ALUOut / 4] = (dMemory[(readMEM.ALUOut) / 4] & 0xFFFFFF00) + (readMEM.B & 0x000000FF);
            
            break;
        
        case HALT:
            haltMEM = true;
            break;
            
        default:
            break;

    }
    
    return;

}

void Pipeline::WB()
{
    if(readWB.nopSignal == true)    return;
    haltWB = false;
    
    //$0 error detection
    zeroWrittenFlag = false;
    zeroWrittenFlag = zeroWrittenDetection();
    if(zeroWrittenFlag == true)    return;
    
    switch(readWB.opcode){
        //R type
        case RTYPE:
            switch (readWB.funct){
                case JR:
                    break;
                    
                default:
                    reg[readWB.rd] = readWB.ALUOut;
                    break;
            }
            break;
            
        //I type
        case ADDI:
            //cout<<"WB:  rt"<<readWB.rt;
            //printf("ALUOut: %X\n", readWB.ALUOut);
            reg[readWB.rt] = readWB.ALUOut;
            break;
            
        case ADDIU:
            reg[readWB.rt] = readWB.ALUOut;
            break;
            
        case LW:
            reg[readWB.rt] = readWB.MDR;
            break;
            
        case LH:
            reg[readWB.rt] = readWB.MDR;
            break;
            
        case LHU:
            reg[readWB.rt] = readWB.MDR;
            break;
            
        case LB:
            reg[readWB.rt] = readWB.MDR;
            break;
            
        case LBU:
            reg[readWB.rt] = readWB.MDR;
            break;
            
        case JAL:
            reg[31] = readWB.ALUOut;
            break;
            
        case LUI:
            reg[readWB.rt] = readWB.ALUOut;
            break;
            
        case ANDI:
            reg[readWB.rt] = readWB.ALUOut;
            break;
            
        case ORI:
            reg[readWB.rt] = readWB.ALUOut;
            break;
            
        case NORI:
            reg[readWB.rt] = readWB.ALUOut;
            break;
            
        case SLTI:
            reg[readWB.rt] = readWB.ALUOut;
            break;
            
            //Specialized:
        case HALT:
            haltWB = true;
            return;		
            break;
            
        default:
            break;
            
    }
}


void Pipeline::printreg()
{
    //printf("\n==========\ncycle %d\n", cycle);
    fprintf(snapshot, "cycle %d\n", cycle);
    
	for(int i = 0 ; i < 32 ; i++){
		//printf("%02d %X\n", i,  reg[i]);
		fprintf(snapshot, "$%02d: 0x%08X\n", i, reg[i]);
	}
	//printf("PC = %X\n", PC);
	fprintf(snapshot, "PC: 0x%08X\n", PC);

	return;
}

void Pipeline::printStage()
{
    //printf("IF: 0x%08X", iMemory[(PC - iMemory[0]) / 4 + 2]);
    if(PC < iMemory[0]){
        fprintf(snapshot, "IF: 0x00000000");
    }
    else{
        fprintf(snapshot, "IF: 0x%08X", iMemory[(PC - iMemory[0]) / 4 + 2]);
        //printf("%X, %X", PC, iMemory[0]);
    }
    
    if(stallSignal == true){
        //printf(" to_be_stalled\n");
        fprintf(snapshot, " to_be_stalled\n");
    }
    else if(toFlushSignal == true){
        //printf(" to_be_flushed\n");
        fprintf(snapshot, " to_be_flushed\n");
    }
    else{
        //cout<<endl;
        fprintf(snapshot, "\n");
    }

    //printf("ID: ");
    fprintf(snapshot, "ID: ");

    if(readID.nopSignal == true){
        //printf("NOP");
        fprintf(snapshot, "NOP");
    }
    else{
        switch(readID.opcode){
            //R type
            case RTYPE:
                switch(readID.funct){
                    case ADD:   //cout<<"ADD";
                        fprintf(snapshot, "ADD");   break;
                        
                    case ADDU:  //cout<<"ADDU";
                        fprintf(snapshot, "ADDU");  break;
                        
                    case SUB:   //cout<<"SUB";
                        fprintf(snapshot, "SUB");   break;
                        
                    case AND:   //cout<<"AND";
                        fprintf(snapshot, "AND");   break;
                        
                    case OR:    //cout<<"OR";
                        fprintf(snapshot, "OR");    break;
                        
                    case XOR:   //cout<<"XOR";
                        fprintf(snapshot, "XOR");   break;
                        
                    case NOR:   //cout<<"NOR";
                        fprintf(snapshot, "NOR");   break;
                        
                    case NAND:  //cout<<"NAND";
                        fprintf(snapshot, "NAND");   break;
                        
                    case SLT:   //cout<<"SLT";
                        fprintf(snapshot, "SLT");   break;
                        
                    case SLL:   //cout<<"SLL";
                        fprintf(snapshot, "SLL");   break;
                        
                    case SRL:   //cout<<"SRL";
                        fprintf(snapshot, "SRL");   break;
                        
                    case SRA:   //cout<<"SRA";
                        fprintf(snapshot, "SRA");   break;
                        
                    case JR:   // cout<<"JR";
                        fprintf(snapshot, "JR");   break;
                        
                    default:    break;
                }
                break;
                //I type
            case ADDI:  //cout<<"ADDI";
                fprintf(snapshot, "ADDI");   break;
                
            case ADDIU: //cout<<"ADDIU";
                fprintf(snapshot, "ADDIU");   break;
                
            case LW:    //cout<<"LW";
                fprintf(snapshot, "LW");   break;
                
            case LH:    //cout<<"LH";
                fprintf(snapshot, "LH");   break;
                
            case LHU:   //cout<<"LHU";
                fprintf(snapshot, "LHU");   break;
                
            case LB:    //cout<<"LB";
                fprintf(snapshot, "LB");   break;
                
            case LBU:   //cout<<"LBU";
                fprintf(snapshot, "LBU");   break;
                
            case SW:    //cout<<"SW";
                fprintf(snapshot, "SW");   break;
                
            case SH:    //cout<<"SH";
                fprintf(snapshot, "SH");   break;
                
            case SB:    //cout<<"SB";
                fprintf(snapshot, "SB");   break;
                
            case LUI:   //cout<<"LUI";
                fprintf(snapshot, "LUI");   break;
                
            case ANDI:  //cout<<"ANDI";
                fprintf(snapshot, "ANDI");   break;
                
            case ORI:   //cout<<"ORI";
                fprintf(snapshot, "ORI");   break;
                
            case NORI:  //cout<<"NORI";
                fprintf(snapshot, "NORI");  break;
                
            case SLTI:  //cout<<"SLTI";
                fprintf(snapshot, "SLTI");   break;
                
            case BEQ:   //cout<<"BEQ";
                fprintf(snapshot, "BEQ");   break;
                
            case BNE:   //cout<<"BNE";
                fprintf(snapshot, "BNE");   break;
                
            case BGTZ:  //cout<<"BGTZ";
                fprintf(snapshot, "BGTZ");   break;
                
                //J type
            case J:     //cout<<"J";
                fprintf(snapshot, "J");   break;
                
            case JAL:   //cout<<"JAL";
                fprintf(snapshot, "JAL");   break;
                
            case HALT:  //cout<<"HALT";
                fprintf(snapshot, "HALT");  break;
                
            default:    break;
                
        }
    }

    
    //printf("cycle %d: rd = %d, rs = %d, rt = %d\n", cycle, readID.rd, readID.rs, readID.rt);
    
    if(forwardingEXDMtoIFID_Rs_Signal && forwardingEXDMtoIFID_Rt_Signal){
        fprintf(snapshot, " fwd_EX-DM_rs_$%d fwd_EX-DM_rt_$%d\n", readID.rs, readID.rt);
        forwardingEXDMtoIFID_Rs_Signal = false;
        forwardingEXDMtoIFID_Rt_Signal = false;
    }
    else if(forwardingEXDMtoIFID_Rs_Signal == true){
        //cout<<" fwd_EX-DM_rs_$";
        //printf("%d\n", readID.rs);
        fprintf(snapshot, " fwd_EX-DM_rs_$%d\n", readID.rs);
        forwardingEXDMtoIFID_Rs_Signal = false;
    }
    else if(forwardingEXDMtoIFID_Rt_Signal == true){
        //cout<<" fwd_EX-DM_rt_$";
        //printf("%d\n", readID.rt);
        fprintf(snapshot, " fwd_EX-DM_rt_$%d\n", readID.rt);
        forwardingEXDMtoIFID_Rt_Signal = false;
    }
    else if(stallSignal == true){
        //printf(" to_be_stalled\n");
        fprintf(snapshot, " to_be_stalled\n");
    }
    else{
        //cout<<endl;
        fprintf(snapshot, "\n");
    }
    
    //cout<<"EX: ";
    fprintf(snapshot, "EX: ");
    if(readEX.nopSignal == true){
        //printf("NOP");
        fprintf(snapshot, "NOP");
    }
    else{
        switch(readEX.opcode){
                //R type
            case RTYPE:
                switch(readEX.funct){
                    case ADD:   //cout<<"ADD";
                        fprintf(snapshot, "ADD");   break;
                        
                    case ADDU:  //cout<<"ADDU";
                        fprintf(snapshot, "ADDU");  break;
                        
                    case SUB:   //cout<<"SUB";
                        fprintf(snapshot, "SUB");   break;
                        
                    case AND:   //cout<<"AND";
                        fprintf(snapshot, "AND");   break;
                        
                    case OR:    //cout<<"OR";
                        fprintf(snapshot, "OR");    break;
                        
                    case XOR:   //cout<<"XOR";
                        fprintf(snapshot, "XOR");   break;
                        
                    case NOR:   //cout<<"NOR";
                        fprintf(snapshot, "NOR");   break;
                        
                    case NAND:  //cout<<"NAND";
                        fprintf(snapshot, "NAND");   break;
                        
                    case SLT:   //cout<<"SLT";
                        fprintf(snapshot, "SLT");   break;
                        
                    case SLL:   //cout<<"SLL";
                        fprintf(snapshot, "SLL");   break;
                        
                    case SRL:   //cout<<"SRL";
                        fprintf(snapshot, "SRL");   break;
                        
                    case SRA:   //cout<<"SRA";
                        fprintf(snapshot, "SRA");   break;
                        
                    case JR:   // cout<<"JR";
                        fprintf(snapshot, "JR");   break;
                        
                    default:    break;
                }
                break;
                //I type
            case ADDI:  //cout<<"ADDI";
                fprintf(snapshot, "ADDI");   break;
                
            case ADDIU: //cout<<"ADDIU";
                fprintf(snapshot, "ADDIU");   break;
                
            case LW:    //cout<<"LW";
                fprintf(snapshot, "LW");   break;
                
            case LH:    //cout<<"LH";
                fprintf(snapshot, "LH");   break;
                
            case LHU:   //cout<<"LHU";
                fprintf(snapshot, "LHU");   break;
                
            case LB:    //cout<<"LB";
                fprintf(snapshot, "LB");   break;
                
            case LBU:   //cout<<"LBU";
                fprintf(snapshot, "LBU");   break;
                
            case SW:    //cout<<"SW";
                fprintf(snapshot, "SW");   break;
                
            case SH:    //cout<<"SH";
                fprintf(snapshot, "SH");   break;
                
            case SB:    //cout<<"SB";
                fprintf(snapshot, "SB");   break;
                
            case LUI:   //cout<<"LUI";
                fprintf(snapshot, "LUI");   break;
                
            case ANDI:  //cout<<"ANDI";
                fprintf(snapshot, "ANDI");   break;
                
            case ORI:   //cout<<"ORI";
                fprintf(snapshot, "ORI");   break;
                
            case NORI:  //cout<<"NORI";
                fprintf(snapshot, "NORI");  break;
                
            case SLTI:  //cout<<"SLTI";
                fprintf(snapshot, "SLTI");   break;
                
            case BEQ:   //cout<<"BEQ";
                fprintf(snapshot, "BEQ");   break;
                
            case BNE:   //cout<<"BNE";
                fprintf(snapshot, "BNE");   break;
                
            case BGTZ:  //cout<<"BGTZ";
                fprintf(snapshot, "BGTZ");   break;
                
                //J type
            case J:     //cout<<"J";
                fprintf(snapshot, "J");   break;
                
            case JAL:   //cout<<"JAL";
                fprintf(snapshot, "JAL");   break;
                
            case HALT:  //cout<<"HALT";
                fprintf(snapshot, "HALT");  break;
                
            default:    break;
                
        }
    }
    
    if(forwardingEXDMtoIDEX_Rs_Signal && forwardingEXDMtoIDEX_Rt_Signal){
        fprintf(snapshot, " fwd_EX-DM_rs_$%d fwd_EX-DM_rt_$%d\n", readEX.rs, readEX.rt);
        forwardingEXDMtoIDEX_Rt_Signal = false;
        forwardingEXDMtoIDEX_Rs_Signal = false;
    }
    else if(forwardingEXDMtoIDEX_Rs_Signal){
        //cout<<" fwd_EX-DM_rs_$";
        //printf("%d\n", readEX.rs);
        fprintf(snapshot, " fwd_EX-DM_rs_$%d\n", readEX.rs);
        forwardingEXDMtoIDEX_Rs_Signal = false;
    }
    else if(forwardingEXDMtoIDEX_Rt_Signal){
        //cout<<" fwd_EX-DM_rt_$";
        //printf("%d\n", readEX.rt);
        fprintf(snapshot, " fwd_EX-DM_rt_$%d\n", readEX.rt);
        forwardingEXDMtoIDEX_Rt_Signal = false;
    }
    else{
        //cout<<endl;
        fprintf(snapshot, "\n");
    }
    
    //cout<<"DM: ";
    fprintf(snapshot, "DM: ");
    if(readMEM.nopSignal == true){
        //printf("NOP");
        fprintf(snapshot, "NOP");
    }
    else{
        switch(readMEM.opcode){
                //R type
            case RTYPE:
                switch(readMEM.funct){
                    case ADD:   //cout<<"ADD";
                        fprintf(snapshot, "ADD");   break;
                        
                    case ADDU:  //cout<<"ADDU";
                        fprintf(snapshot, "ADDU");  break;
                        
                    case SUB:   //cout<<"SUB";
                        fprintf(snapshot, "SUB");   break;
                        
                    case AND:   //cout<<"AND";
                        fprintf(snapshot, "AND");   break;
                        
                    case OR:    //cout<<"OR";
                        fprintf(snapshot, "OR");    break;
                        
                    case XOR:   //cout<<"XOR";
                        fprintf(snapshot, "XOR");   break;
                        
                    case NOR:   //cout<<"NOR";
                        fprintf(snapshot, "NOR");   break;
                        
                    case NAND:  //cout<<"NAND";
                        fprintf(snapshot, "NAND");   break;
                        
                    case SLT:   //cout<<"SLT";
                        fprintf(snapshot, "SLT");   break;
                        
                    case SLL:   //cout<<"SLL";
                        fprintf(snapshot, "SLL");   break;
                        
                    case SRL:   //cout<<"SRL";
                        fprintf(snapshot, "SRL");   break;
                        
                    case SRA:   //cout<<"SRA";
                        fprintf(snapshot, "SRA");   break;
                        
                    case JR:   // cout<<"JR";
                        fprintf(snapshot, "JR");   break;
                        
                    default:    break;
                }
                break;
                //I type
            case ADDI:  //cout<<"ADDI";
                fprintf(snapshot, "ADDI");   break;
                
            case ADDIU: //cout<<"ADDIU";
                fprintf(snapshot, "ADDIU");   break;
                
            case LW:    //cout<<"LW";
                fprintf(snapshot, "LW");   break;
                
            case LH:    //cout<<"LH";
                fprintf(snapshot, "LH");   break;
                
            case LHU:   //cout<<"LHU";
                fprintf(snapshot, "LHU");   break;
                
            case LB:    //cout<<"LB";
                fprintf(snapshot, "LB");   break;
                
            case LBU:   //cout<<"LBU";
                fprintf(snapshot, "LBU");   break;
                
            case SW:    //cout<<"SW";
                fprintf(snapshot, "SW");   break;
                
            case SH:    //cout<<"SH";
                fprintf(snapshot, "SH");   break;
                
            case SB:    //cout<<"SB";
                fprintf(snapshot, "SB");   break;
                
            case LUI:   //cout<<"LUI";
                fprintf(snapshot, "LUI");   break;
                
            case ANDI:  //cout<<"ANDI";
                fprintf(snapshot, "ANDI");   break;
                
            case ORI:   //cout<<"ORI";
                fprintf(snapshot, "ORI");   break;
                
            case NORI:  //cout<<"NORI";
                fprintf(snapshot, "NORI");  break;
                
            case SLTI:  //cout<<"SLTI";
                fprintf(snapshot, "SLTI");   break;
                
            case BEQ:   //cout<<"BEQ";
                fprintf(snapshot, "BEQ");   break;
                
            case BNE:   //cout<<"BNE";
                fprintf(snapshot, "BNE");   break;
                
            case BGTZ:  //cout<<"BGTZ";
                fprintf(snapshot, "BGTZ");   break;
                
                //J type
            case J:     //cout<<"J";
                fprintf(snapshot, "J");   break;
                
            case JAL:   //cout<<"JAL";
                fprintf(snapshot, "JAL");   break;
                
            case HALT:  //cout<<"HALT";
                fprintf(snapshot, "HALT");  break;
                
            default:    break;
                
        }
    }

    //cout<<endl;
    fprintf(snapshot, "\n");
    
    //cout<<"WB: ";
    fprintf(snapshot, "WB: ");
    if(readWB.nopSignal == true){
        //printf("NOP");
        fprintf(snapshot, "NOP");
    }
    else{
        switch(readWB.opcode){
                //R type
            case RTYPE:
                switch(readWB.funct){
                    case ADD:   //cout<<"ADD";
                        fprintf(snapshot, "ADD");   break;
                        
                    case ADDU:  //cout<<"ADDU";
                        fprintf(snapshot, "ADDU");  break;
                        
                    case SUB:   //cout<<"SUB";
                        fprintf(snapshot, "SUB");   break;
                        
                    case AND:   //cout<<"AND";
                        fprintf(snapshot, "AND");   break;
                        
                    case OR:    //cout<<"OR";
                        fprintf(snapshot, "OR");    break;
                        
                    case XOR:   //cout<<"XOR";
                        fprintf(snapshot, "XOR");   break;
                        
                    case NOR:   //cout<<"NOR";
                        fprintf(snapshot, "NOR");   break;
                        
                    case NAND:  //cout<<"NAND";
                        fprintf(snapshot, "NAND");   break;
                        
                    case SLT:   //cout<<"SLT";
                        fprintf(snapshot, "SLT");   break;
                        
                    case SLL:   //cout<<"SLL";
                        fprintf(snapshot, "SLL");   break;
                        
                    case SRL:   //cout<<"SRL";
                        fprintf(snapshot, "SRL");   break;
                        
                    case SRA:   //cout<<"SRA";
                        fprintf(snapshot, "SRA");   break;
                        
                    case JR:   // cout<<"JR";
                        fprintf(snapshot, "JR");   break;
                        
                    default:    break;
                }
                break;
                //I type
            case ADDI:  //cout<<"ADDI";
                fprintf(snapshot, "ADDI");   break;
                
            case ADDIU: //cout<<"ADDIU";
                fprintf(snapshot, "ADDIU");   break;
                
            case LW:    //cout<<"LW";
                fprintf(snapshot, "LW");   break;
                
            case LH:    //cout<<"LH";
                fprintf(snapshot, "LH");   break;
                
            case LHU:   //cout<<"LHU";
                fprintf(snapshot, "LHU");   break;
                
            case LB:    //cout<<"LB";
                fprintf(snapshot, "LB");   break;
                
            case LBU:   //cout<<"LBU";
                fprintf(snapshot, "LBU");   break;
                
            case SW:    //cout<<"SW";
                fprintf(snapshot, "SW");   break;
                
            case SH:    //cout<<"SH";
                fprintf(snapshot, "SH");   break;
                
            case SB:    //cout<<"SB";
                fprintf(snapshot, "SB");   break;
                
            case LUI:   //cout<<"LUI";
                fprintf(snapshot, "LUI");   break;
                
            case ANDI:  //cout<<"ANDI";
                fprintf(snapshot, "ANDI");   break;
                
            case ORI:   //cout<<"ORI";
                fprintf(snapshot, "ORI");   break;
                
            case NORI:  //cout<<"NORI";
                fprintf(snapshot, "NORI");  break;
                
            case SLTI:  //cout<<"SLTI";
                fprintf(snapshot, "SLTI");   break;
                
            case BEQ:   //cout<<"BEQ";
                fprintf(snapshot, "BEQ");   break;
                
            case BNE:   //cout<<"BNE";
                fprintf(snapshot, "BNE");   break;
                
            case BGTZ:  //cout<<"BGTZ";
                fprintf(snapshot, "BGTZ");   break;
                
                //J type
            case J:     //cout<<"J";
                fprintf(snapshot, "J");   break;
                
            case JAL:   //cout<<"JAL";
                fprintf(snapshot, "JAL");   break;
                
            case HALT:  //cout<<"HALT";
                fprintf(snapshot, "HALT");  break;
                
            default:    break;
                
        }
    }
    //cout<<endl;
    fprintf(snapshot, "\n\n\n");
    
    return;
    
}

//#####error detection#####

bool Pipeline::zeroWrittenDetection()
{
    switch(readWB.opcode){
        case RTYPE:
            if((readWB.funct) == SLL){
                if((readWB.rd) == 0){
                    if((readWB.rt != 0) || (readWB.shamt != 0)){
                        fprintf(errorDump, "In cycle %d: Write $0 Error\n", cycle + 1);
                        return true;
                    }
                    else    return false;
                }
            }
            else if((readWB.funct) != JR){
                if((readWB.rd) == 0){
                    //cout<<"Write $0 Error"<<endl;
                    fprintf(errorDump, "In cycle %d: Write $0 Error\n", cycle + 1);
                    return true;
                }
            }
            break;
        case SW:
            break;
        case SH:
            break;
        case SB:
            break;
        case BEQ:
            break;
        case BNE:
            break;
        case BGTZ:
            break;
        case J:
            break;
        case JAL:
            break;
        case HALT:
            break;
        default:
            if(readWB.rt == 0){
                fprintf(errorDump, "In cycle %d: Write $0 Error\n", cycle + 1);
                return true;
            }
            break;
            
    }
    return false;
    
}

void Pipeline::numberOverflowDetection()
{

    switch(readEX.opcode){
        case RTYPE:
            switch (readEX.funct) {
                case ADD:
                    if( ((readEX.A + readEX.B) >> 31) != (readEX.A >> 31) && ((readEX.A + readEX.B)>>31) != (readEX.B >> 31) ){
                        fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                        return;
                    }
                    break;
                    
                case SUB:
                    if(readEX.B == 0x80000000){
                        if( ((readEX.A - readEX.B) >> 31) != (readEX.A >> 31) && ((readEX.A - readEX.B) >> 31) != (readEX.B >> 31) ){
                            fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                            return;
                        }
                    }
                    else{
                        if( (((readEX.A - readEX.B)>>31) != (readEX.A>>31)) && (((readEX.A - readEX.B) >> 31) == (readEX.B>>31)) ){
                            fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                            return;
                        }
                    }
                    break;
                    
                default:
                    break;
            }
            break;
            
        case ADDI:
            if(( ((readEX.A + readEX.signExtend) >> 31)!= (readEX.A >> 31)) &&
               (((readEX.A + readEX.signExtend) >> 31) != (readEX.signExtend >> 31)) ){
                fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                return;
            }
            break;
            
        case LW:
            if( (((readEX.A + readEX.signExtend) >> 31) != (readEX.A >> 31)) &&
               (((readEX.A + readEX.signExtend) >> 31) != (readEX.signExtend>>31)) ){
                fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                return;
            }
            break;
            
        case LH:
            if( (((readEX.A + readEX.signExtend) >> 31) != (readEX.A >> 31)) &&
               (((readEX.A + readEX.signExtend) >> 31) != (readEX.signExtend>>31)) ){
                fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                return;
            }
            break;
            
        case LHU:
            if( (((readEX.A + readEX.signExtend) >> 31) != (readEX.A >> 31)) &&
               (((readEX.A + readEX.signExtend) >> 31) != (readEX.signExtend>>31)) ){
                fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                return;
            }
            break;
            
        case LB:
            if( (((readEX.A + readEX.signExtend) >> 31) != (readEX.A >> 31)) &&
               (((readEX.A + readEX.signExtend) >> 31) != (readEX.signExtend>>31)) ){
                fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                return;
            }
            break;
            
        case LBU:
            if( (((readEX.A + readEX.signExtend) >> 31) != (readEX.A >> 31)) &&
               (((readEX.A + readEX.signExtend) >> 31) != (readEX.signExtend>>31)) ){
                fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                return;
            }
            break;
            
        case SW:
            if( (((readEX.A + readEX.signExtend) >> 31) != (readEX.A >> 31)) &&
               (((readEX.A + readEX.signExtend) >> 31) != (readEX.signExtend>>31)) ){
                fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                return;
            }
            break;
            
        case SH:
            if( (((readEX.A + readEX.signExtend) >> 31) != (readEX.A >> 31)) &&
               (((readEX.A + readEX.signExtend) >> 31) != (readEX.signExtend >> 31)) ){
                fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                return;
            }
            break;
            
        case SB:
            if( (((readEX.A + readEX.signExtend) >> 31) != (readEX.A >> 31)) &&
               (((readEX.A + readEX.signExtend) >> 31) != (readEX.signExtend >> 31)) ){
                fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle + 1);
                return;
            }
            break;
        case BEQ:
            break;
            
        default:
            return;
            break;
    }
}

bool Pipeline::addressOverflowDetection()
{
    switch(readMEM.opcode){
        case LW:
            if((readMEM.ALUOut + 3) / 4 >= 256 || (readMEM.ALUOut + 3) < readMEM.ALUOut){
                fprintf(errorDump, "In cycle %d: Address Overflow\n", cycle + 1);
                return true;
            }
            break;
            
        case LH:
            if((readMEM.ALUOut + 1) / 4 >= 256 || (readMEM.ALUOut + 1) < readMEM.ALUOut){
                fprintf(errorDump, "In cycle %d: Address Overflow\n", cycle + 1);
                return true;
            }
            break;
        case LHU:
            if((readMEM.ALUOut + 1) / 4 >= 256 || (readMEM.ALUOut + 1) < readMEM.ALUOut){
                fprintf(errorDump, "In cycle %d: Address Overflow\n", cycle + 1);
                return true;
            }
            break;
        case LB:
            if((readMEM.ALUOut) / 4 >= 256){
                fprintf(errorDump, "In cycle %d: Address Overflow\n", cycle + 1);
                return true;
            }
            break;
        case LBU:
            if(readMEM.ALUOut / 4 >= 256){
                fprintf(errorDump, "In cycle %d: Address Overflow\n", cycle + 1);
                return true;
            }
            break;
        case SW:
            //printf("i'm wrong: 0x%X, 0x%X\n", readMEM.ALUOut, readMEM.ALUOut + 3);
            if((readMEM.ALUOut + 3) / 4 >= 256 || (readMEM.ALUOut + 3) < readMEM.ALUOut){
                fprintf(errorDump, "In cycle %d: Address Overflow\n", cycle + 1);
                return true;
            }
            break;
        case SH:
            if((readMEM.ALUOut + 1) / 4 >= 256 || (readMEM.ALUOut + 1) < readMEM.ALUOut){
                fprintf(errorDump, "In cycle %d: Address Overflow\n", cycle + 1);
                return true;
            }
            break;
        case SB:
            if((readMEM.ALUOut) / 4 >= 256){
                fprintf(errorDump, "In cycle %d: Address Overflow\n", cycle + 1);
                return true;
            }
            break;
        default:
            break;
    }
    
    return false;
    
}

bool Pipeline::dataMisalignedDetection()
{
    switch (readMEM.opcode) {
        case LW:
            if(readMEM.ALUOut % 4 != 0){
                fprintf(errorDump, "In cycle %d: Misalignment Error\n", cycle + 1);
                return true;
            }
            break;
            
        case LH:
            if(readMEM.ALUOut % 2 != 0){
                fprintf(errorDump, "In cycle %d: Misalignment Error\n", cycle + 1);
                return true;
            }
            break;
            
        case LHU:
            if(readMEM.ALUOut % 2 != 0){
                fprintf(errorDump, "In cycle %d: Misalignment Error\n", cycle + 1);
                return true;
            }
            break;
            
        case SW:
            if(readMEM.ALUOut % 4 != 0){
                fprintf(errorDump, "In cycle %d: Misalignment Error\n", cycle + 1);
                return true;
            }
            break;
            
        case SH:
            if(readMEM.ALUOut % 2 != 0){
                fprintf(errorDump, "In cycle %d: Misalignment Error\n", cycle + 1);
                return true;
            }
            break;
            
        default:
            break;
    }
    
    return false;
    
}


