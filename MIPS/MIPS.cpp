#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;

#define ADDU (1)
#define SUBU (3)
#define AND (4)
#define OR  (5)
#define NOR (7)

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.
#define MemSize (65536)


class RF
{
  public:
    bitset<32> ReadData1, ReadData2; 
    RF()
    { 
      Registers.resize(32);  
      Registers[0] = bitset<32> (0);  /*Register 0 is always 0*/
    }

    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
    {   
      /**
       * @brief Reads or writes data from/to the Register.
       *
       * This function is used to read or write data from/to the register, depending on the value of WrtEnable.
       * Put the read results to the ReadData1 and ReadData2.
       */
      // TODO: implement!
      ReadData1 = Registers[(RdReg1.to_ulong())];
      /**(printf("%d", RdReg1.to_ulong());*/
      ReadData2 = Registers[(RdReg2.to_ulong())];
      if(WrtEnable.to_ulong() == 1){
        Registers[(WrtReg.to_ulong())] = WrtData;
      }         
    }

    void OutputRF()
    {
      ofstream rfout;
      rfout.open("RFresult.txt",std::ios_base::app);
      if (rfout.is_open())
      {
        rfout<<"A state of RF:"<<endl;
        for (int j = 0; j<32; j++)
        {        
          rfout << Registers[j]<<endl;
        }

      }
      else cout<<"Unable to open file";
      rfout.close();

    }     
  private:
    vector<bitset<32> >Registers;
};


class ALU
{
  public:
    bitset<32> ALUresult;
    bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
    {   
      /**
       * @brief Implement the ALU operation here.
       *
       * ALU operation depends on the ALUOP, which are definded as ADDU, SUBU, etc. 
       */
      // TODO: implement!
      if(ALUOP.to_ulong()==1){
        ALUresult = oprand1.to_ulong() + oprand2.to_ulong();
      }
      else if(ALUOP.to_ulong()==3){
        ALUresult = oprand1.to_ulong() - oprand2.to_ulong();
      }
      else if(ALUOP.to_ulong()==4){
        ALUresult = oprand1 & oprand2;
      }
      else if(ALUOP.to_ulong()==5){
        ALUresult = oprand1 |oprand2;
      }
      else if(ALUOP.to_ulong()==7){ /**NOR*/
        ALUresult = ~(oprand1 |oprand2);
      }


      return ALUresult;
    }            
};


class INSMem
{
  public:
    bitset<32> Instruction;
    INSMem()
    {       
      IMem.resize(MemSize); 
      ifstream imem;
      string line;
      int i=0;
      imem.open("imem.txt");
      if (imem.is_open())
      {
        while (getline(imem,line))
        {      
          IMem[i] = bitset<8>(line);
          i++;
        }

      }
      else cout<<"Unable to open file";
      imem.close();

    }

    bitset<32> ReadMemory (bitset<32> ReadAddress) 
    {    
      // TODO: implement!
      /**
       * @brief Read Instruction Memory (IMem).
       *
       * Read the byte at the ReadAddress and the following three byte,
       * and return the read result. 
       */

      Instruction = 0;
      int counter;
      for(counter=0; counter<4; counter++){
          Instruction <<= 8; /**shift 8 bits for next cycle*/
          Instruction = Instruction.to_ulong() + IMem[ReadAddress.to_ulong() + counter].to_ulong();
 
      }



      return Instruction;     
    }     

  private:
    vector<bitset<8> > IMem;

};

class DataMem    
{
  public:
    bitset<32> readdata;  
    DataMem()
    {
      DMem.resize(MemSize); 
      ifstream dmem;
      string line;
      int i=0;
      dmem.open("dmem.txt");
      if (dmem.is_open())
      {
        while (getline(dmem,line))
        {      
          DMem[i] = bitset<8>(line);
          i++;
        }
      }
      else cout<<"Unable to open file";
      dmem.close();

    }  
    bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem) 
    {    
      /**
       * @brief Reads/writes data from/to the Data Memory.
       *
       * This function is used to read/write data from/to the DataMem, depending on the readmem and writemem.
       * First, if writemem enabled, WriteData should be written to DMem, clear or ignore the return value readdata,
       * and note that 32-bit WriteData will occupy 4 continious Bytes in DMem. 
       * If readmem enabled, return the DMem read result as readdata.
       */
      // TODO: implement!
      if( readmem.to_ulong() == 1){
          readdata = 0;
          int counter;
          for(counter=0; counter<4; counter++){
          readdata <<= 8; /**shift 8 bits for next cycle*/
          readdata = readdata.to_ulong() + DMem[Address.to_ulong() + counter].to_ulong();
          /**readdata <<= 8; /**shift 8 bits for next cycle*/
        }
      }
      if(writemem.to_ulong() ==1){
          int counter = 31;
          for (int i = 0; i < 4; i++) 
             for (int j = 7; j >= 0; j--){
							DMem[Address.to_ulong() + i][j] = WriteData[counter];
							counter--;
						} 
          readdata = 0;
      }
      return readdata;   
      }


    
    void OutputDataMem()
    {
      ofstream dmemout;
      dmemout.open("dmemresult.txt");
      if (dmemout.is_open())
      {
        for (int j = 0; j< 1000; j++)
        {     
          dmemout << DMem[j]<<endl;
        }

      }
      else cout<<"Unable to open file";
      dmemout.close();

    }             

  private:
    vector<bitset<8> > DMem;

};  


bitset<5> Gf(bitset<32> Instruction, int start, int end) {
	int i, j = 0;
	bitset<5> temp;
	for (i = start; i <= end; i++)
	{
		temp[j] = Instruction[i];         
		j++;
	}
	return temp;
}


bitset<6> Gs(bitset<32> Instruction, int start, int end) {
	int i, j =0;
	bitset<6> temp;
	for (i = start; i <= end; i++)
	{
		temp[j] = Instruction[i];      
		j++;
	}
	return temp;
}

bitset<32> Ext(bitset<16> Immediate){
	int j;
	bitset<32> temp;
	for (j = 0; j < 16; j++){
		temp[j] = Immediate[j];
		temp[j+16] = Immediate[15]; //sign extentioon
	}
	return temp;
}






int main()
{
  RF myRF;
  ALU myALU;
  INSMem myInsMem;
  DataMem myDataMem;

  bitset<32> Instruction, pc_j, result, j_address, extention, pc, r_data, l_data, branch_address =0;
  bitset<6> opcode, function;
  bitset<5> rs,rt,rd,shamt;
  bitset<3> operation;

  bitset<16> immediate;
  bool isEqual = 0;

  while (1)  // TODO: implement!
  {
    // Fetch: fetch an instruction from myInsMem.
    Instruction = myInsMem.ReadMemory(pc.to_ulong());
		cout << "Current Instruction: " << Instruction << endl;


    // If current instruction is "11111111111111111111111111111111", then break; (exit the while loop)
      if (Instruction == 0xFFFFFFFF){
			    cout<<"HALT-!"<<endl;
        	break;
			}


    // decode(Read RF): get opcode and other signals from instruction, decode instruction
    opcode = Gs(Instruction, 26, 31);
		cout << "OPcode is: " << opcode << endl;

    if (opcode == 0x00){

			rs = Gf(Instruction, 21, 25);
			rt = Gf(Instruction, 16, 20);
			rd = Gf(Instruction, 11, 15);
      shamt = Gf(Instruction, 6, 10);
			function = Gs(Instruction, 0, 5);

			cout<<"Function is: "<<function<<endl;

      /**Confirm ALU Operation*/
			for (int i = 0; i < 3; i++) {
				operation[i] = function[i];
			}
			myRF.ReadWrite(rs, rt, rd, 0, 0);
		}

    else if (opcode == 0X02) { // jump

			pc_j = pc.to_ulong()+4;
			for (int i = 2; i < 28; i++){ //address
				j_address[i] = Instruction[i-2];
			}
			for (int i = 28; i < 32; i++){ /**first four bits from PC + 4&*/
				j_address[i] = pc_j[i];
			}
			for (int i = 0; i < 2; i++){ /**shift left*/
				j_address[i] = 0;
			}
		}

    else{
      rs = Gf(Instruction, 21, 25);
			rt = Gf(Instruction, 16, 20);
			for (int i = 0; i < 16; i++){
				immediate[i] = Instruction[i];
			}

			extention = Ext(immediate);                    //SignExtImm
			myRF.ReadWrite(rs, rt, rt, 0, 0);
    }


    // Execute: after decoding, ALU may run and return result
    if (opcode == 0x00){
			result = myALU.ALUOperation(operation, myRF.ReadData1, myRF.ReadData2); //ALU operation
			cout << "Result is:" << result << endl ;
		}
    else if (opcode == 0x04){
			result = myALU.ALUOperation(3, myRF.ReadData1, myRF.ReadData2);
			if (result == 0)
			  isEqual = 1;		
			}
    else if(opcode == 0x23 || opcode == 0x2B || opcode == 0x09){
      result = myALU.ALUOperation(1, myRF.ReadData1, extention);
    }


    // Read/Write Mem: access data memory (myDataMem)
    if (opcode == 0x23){ //lw, load data
      r_data = myDataMem.MemoryAccess(result, myRF.ReadData2, 1, 0);
      }
    else if(opcode == 0x2b)       //sw, store data
		{
		  l_data = myDataMem.MemoryAccess(result, myRF.ReadData2, 0, 1);
		}

    // Write back to RF: some operations may write things to RF
    if(opcode == 0x09){   //addiu
      myRF.ReadWrite(rs, rt, rt, result, 1);
    }
    else if(opcode == 0x00){
      myRF.ReadWrite(rs, rt, rd, result, 1); 
    }
    else if(opcode == 0x23){
      myRF.ReadWrite(rs, rt, rt, r_data, 1);
    }


    if (opcode == 0x02){
      	pc = j_address;	
    }
    
		else if(opcode == 0x04 && isEqual){ //beq

          for (int i = 2; i < 18; i++) {
            branch_address[i] = immediate[i-2];
          }
          for (int i = 18; i < 32; i++) {
            branch_address[i] = branch_address[17];
          }
          for (int i = 0; i < 2; i++) {
            branch_address[i] = 0;
          }
          
        pc = pc.to_ulong() + 4 + branch_address.to_ulong()*4; //jump
        isEqual =0;                  //reset beq
		}
    else
			pc = pc.to_ulong() + 4;        //common instruction, pc=pc+4;

    /**** You don't need to modify the following lines. ****/
    myRF.OutputRF(); // dump RF;    
  }
  myDataMem.OutputDataMem(); // dump data mem
   system("pause");
  return 0;
}
