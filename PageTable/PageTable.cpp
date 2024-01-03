#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

#define MemSize (65536)


class PhyMem    
{
  public:
    bitset<32> readdata;  
    PhyMem()
    {
      DMem.resize(MemSize); 
      ifstream dmem;
      string line;
      int i=0;
      dmem.open("pt_initialize.txt");
      if (dmem.is_open())
      {
        while (getline(dmem,line))
        {      
          DMem[i] = bitset<8>(line);
          i++;
        }
      }
      else cout<<"Unable to open page table init file";
      dmem.close();

    }  
    bitset<32> outputMemValue (bitset<12> Address) 
    {    
      bitset<32> readdata;
      /**TODO: implement!
       * Returns the value stored in the physical address 
       */
      string res;
      for(int i=0; i<4; i++){
        res = res + DMem[Address.to_ullong()+i].to_string();
      }
      std::bitset<32> b3(res);
      readdata = b3;
      return readdata;     
    }              

  private:
    vector<bitset<8>>DMem;

};  

int main(int argc, char *argv[])
{
    PhyMem myPhyMem;

    ifstream traces;
    ifstream PTB_file;
    ofstream tracesout;

    string outname;
    outname = "pt_results.txt";

    traces.open(argv[1]);
    PTB_file.open(argv[2]);
    tracesout.open(outname.c_str());

    //Initialize the PTBR
    bitset<12> PTBR;
    PTB_file >> PTBR;

    string line;
    bitset<14> virtualAddr;


    int v1, v2 = 0;
    //int address, data = 0;
    int address;
    int data;
    std::string ans;
    std::string ans2;



    /*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
    // Read a virtual address form the PageTable and convert it to the physical address - CSA23
    if(traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        {
            v1 = v2 = 0;
            address = 0;
            data = 0;
            ans = "0x000";
            ans2 = "0x00000000";
            //TODO: Implement!
            // Access the outer page table 
            int x;
            bitset<32> PA;
            bitset<12> PTBR_new;
            std::bitset<14>addr(line);
            virtualAddr = addr;
            PTBR_new = PTBR;
            x = PTBR_new.to_ullong() + bitset<4>(virtualAddr.to_string().substr(0,4)).to_ullong()*4;
            PTBR_new = bitset<12>(x);
            PA = myPhyMem.outputMemValue(PTBR_new); //outer table content 


            // If outer page table valid bit is 1, access the inner page table 
            if(bitset<1>(PA.to_string().substr(31,1)).to_ullong()==1){
                bitset<12> PA2;
                bitset<32> inner;
                PA2 = bitset<12>(PA.to_string().substr(0,12)); //Physical address for start of inner table
                x = PA2.to_ullong() + bitset<4>(virtualAddr.to_string().substr(4,4)).to_ullong()*4;
                PA2 = bitset<12>(x);
                inner = myPhyMem.outputMemValue(PA2);  //inner table content 
                v1 = 1;
                if(bitset<1>(inner.to_string().substr(31,1)).to_ullong()==1){ //if inner table valid bit is 1
                      bitset<6> PA3;
                      bitset<6> offset;
                      bitset<12> mem_addr;
                      bitset<32> res;
                      string temp;
                      PA3 = bitset<6>(inner.to_string().substr(0,6)); 
                      offset = bitset<6>(virtualAddr.to_string().substr(8,6));
                      temp = PA3.to_string() + offset.to_string();
                      std::bitset<12>addr1(temp);
                      mem_addr = addr1;
                      res = myPhyMem.outputMemValue(mem_addr); //memory content
                      v2 = 1;
                      address = mem_addr.to_ullong();
                      data = res.to_ullong();


                      std::ostringstream ss;
                      ss << "0x" << std::setfill('0') << std::setw(3) << std::hex << address;
                      ans = ss.str();

                      std::ostringstream ss2;
                      ss2 << "0x" << std::setfill('0') << std::setw(8) << std::hex << data;
                      ans2 = ss2.str();
                      
                }

            }

            //Return valid bit in outer and inner page table, physical address, and value stored in the physical memory.
            // Each line in the output file for example should be: 1, 0, 0x000, 0x00000000
            tracesout << v1 << ", " << v2 << ", " << ans << ", " << ans2 << endl; 
        }
        traces.close();
        tracesout.close();
    }

    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
