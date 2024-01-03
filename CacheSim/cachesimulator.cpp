/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size in bytes)  t=32-s-b
32 bit address (MSB -> LSB): TAG || SET || OFFSET

Tag Bits   : the tag field along with the valid bit is used to determine whether the block in the cache is valid or not.
Index Bits : the set index field is used to determine which set in the cache the block is stored in.
Offset Bits: the offset field is used to determine which byte in the block is being accessed.
*/

#include <algorithm>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss
#define NOWRITEMEM 5 // no write to memory
#define WRITEMEM 6   // write to memory


int ans[2];
int instruction = 0;


struct config
{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
// You need to define your cache class here, or design your own data structure for L1 and L2 cache

/*
A single cache block:
    - valid bit (is the data in the block valid?)
    - dirty bit (has the data in the block been modified by means of a write?)
    - tag (the tag bits of the address)
    - data (the actual data stored in the block, in our case, we don't need to store the data)
*/
struct CacheBlock
{
    // we don't actually need to allocate space for data, because we only need to simulate the cache action
    // or else it would have looked something like this: vector<number of bytes> Data; 
    int valid_bit = 0;
    int dirty_bit = 0;
    int tag = 0;
    int set_b=0;
    int tag_b=0;

};

/*
A CacheSet:
    - a vector of CacheBlocks
    - a counter to keep track of which block to evict next
*/
struct set
{
    // tips: 
    // Associativity: eg. resize to 4-ways set associative cache
    int counter = 0;
    vector<CacheBlock*> cache_Block;
};

// You can design your own data structure for L1 and L2 cache; just an example here
class cache
{
    // some cache configuration parameters.
    // cache L1 or L2
    vector<set*> L1;
    vector<set*> L2;
    //config* cfg = new config;
    int L1_block_size;
    int L2_block_size;
    int L1_ways;
    int L2_ways;
    int L1_sets;
    int L2_sets;

    int s_L1;
    int s_L2;
    int b_L1;
    int b_L2;
    int t_L1;
    int t_L2;


public:
    cache(int L1blocksize, int L1setsize, int L1size, int L2blocksize, int L2setsize, int L2size){
        // initialize the cache according to cache parameters
        //vector<set*> L1;
        //vector<set*> L2;
        //config* cfg = new config;
        L1_block_size = L1blocksize;
        L2_block_size = L2blocksize;
        L1_ways = L1setsize;
        L2_ways = L2setsize;
        L1_sets = (L1size) * 1024 / (L1_ways*L1_block_size);
        L2_sets = (L2size) * 1024 / (L2_ways*L2_block_size);

        s_L1 = log2(L1_sets); //11
        s_L2 = log2(L2_sets); //12
        b_L1 = log2(L1_block_size); //3
        b_L2 = log2(L2_block_size); //3
        t_L1 = 32-s_L1-b_L1; //18
        t_L2 = 32-s_L2-b_L2; //17


        //Initialize L1
        int i, j;
        for(i=0; i<L1_sets; i++){
            set* new_set = new set;
            for(j=0; j<L1_ways;j++){
                CacheBlock* new_block = new CacheBlock;
                (new_set->cache_Block).push_back(new_block);
            }
            L1.push_back(new_set);
        }
        

        //Initialize L2
        for(i=0; i<L2_sets; i++){
            set* new_set = new set;
            for(j=0; j<L2_ways;j++){
                CacheBlock* new_block = new CacheBlock;
                (new_set->cache_Block).push_back(new_block);
            }
            L2.push_back(new_set);
        }
        

    }

    auto write(auto addr){
        /*
        step 1: select the set in our L1 cache using set index bits
        step 2: iterate through each way in the current set
            - If Matching tag and Valid Bit High -> WriteHit!
                                                    -> Dirty Bit High
        step 3: Otherwise? -> WriteMiss!

        return WH or WM
        */
        int i,j;
        int set = bitset<32>(addr.to_string().substr(t_L1,s_L1)).to_ulong();
        int addr_tag = bitset<32>(addr.to_string().substr(0,t_L1)).to_ulong();
        int set_l2 = bitset<32>(addr.to_string().substr(t_L2,s_L2)).to_ulong();
        int addr_tag_l2 = bitset<32>(addr.to_string().substr(0,t_L2)).to_ulong();
        

         /* if(set == 1979){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("Try writing SET 1979 in L1\n");
                    printf("L1 dirty bit is %d\n", L1[set]->cache_Block[0]->dirty_bit);
                    printf("L1 valid bit is %d\n", L1[set]->cache_Block[0]->valid_bit);
                    printf("Target tag is %d\n", addr_tag);
                    printf("L1 tag is %d\n", L1[set]->cache_Block[0]->tag);     
                }*/ 


        for (i=0; i<L1[set]->cache_Block.size(); i++){
            if(L1[set]->cache_Block[i]->tag == addr_tag && L1[set]->cache_Block[i]->valid_bit==1){
                L1[set]->cache_Block[i]->dirty_bit = 1;
                L1[set]->cache_Block[i]->tag_b = addr_tag_l2;
                L1[set]->cache_Block[i]->set_b = set_l2;

               /* if(set == 1979){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("Write hit in SET 1979 in L1\n");
                    printf("L1 dirty bit is %d\n", L1[set]->cache_Block[0]->dirty_bit);
                    printf("L1 valid bit is %d\n", L1[set]->cache_Block[0]->valid_bit);
                    printf("Target tag is %d\n", addr_tag);
                    printf("L1 tag is %d\n", L1[set]->cache_Block[0]->tag);     
                }*/ 

                return WH;
            }      
        }

    /* if(set == 1979){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("Write miss SET 1979 in L1\n");
                } */ 

     return WM;
    }

    auto writeL2(auto addr){
        /*
        step 1: select the set in our L2 cache using set index bits
        step 2: iterate through each way in the current set
            - If Matching tag and Valid Bit High -> WriteHit!
                                                 -> Dirty Bit High
        step 3: Otherwise? -> WriteMiss!

        return {WM or WH, WRITEMEM or NOWRITEMEM}
        */
        int i,j;
        int set = bitset<32>(addr.to_string().substr(t_L2,s_L2)).to_ulong();
        int addr_tag = bitset<32>(addr.to_string().substr(0,t_L2)).to_ulong();
        int set_l1 = bitset<32>(addr.to_string().substr(t_L1,s_L1)).to_ulong();
        int addr_tag_l1 = bitset<32>(addr.to_string().substr(0,t_L1)).to_ulong();

       /* if(set==4027){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("Try writing SET 4027 in L2\n");
                    printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                    printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                    printf("Target tag is %d\n", addr_tag);
                    printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);  
                } */ 


        for (i=0; i<L2[set]->cache_Block.size(); i++){
            if(L2[set]->cache_Block[i]->tag == addr_tag && L2[set]->cache_Block[i]->valid_bit == 1){
                L2[set]->cache_Block[i]->dirty_bit = 1;
                L2[set]->cache_Block[i]->tag_b = addr_tag_l1;
                L2[set]->cache_Block[i]->set_b = set_l1;

                ans[0] = WH;
                ans[1] = NOWRITEMEM;

               /* if(set == 4027){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("Write hit in SET 4027 in L2\n");
                    printf("L2 dirty bit is %d\n", L1[set]->cache_Block[0]->dirty_bit);
                    printf("L2 valid bit is %d\n", L1[set]->cache_Block[0]->valid_bit);
                    printf("Target tag is %d\n", addr_tag);
                    printf("L2 tag is %d\n", L1[set]->cache_Block[0]->tag);     
                } */ 




                //int ans[2] = {WH, NOWRITEMEM};
                return ans;
            }      
        }



     /*  if(set == 4027){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("Write miss SET 4027 in L2\n");
                } */ 

     ans[0] = WM;
     ans[1] = WRITEMEM;
     return ans;

    }

    auto readL1(auto addr){
        /*
        step 1: select the set in our L1 cache using set index bits
        step 2: iterate through each way in the current set
            - If Matching tag and Valid Bit High -> ReadHit!
        step 3: Otherwise? -> ReadMiss!

        return RH or RM
        */
        int i,j;
        int set = bitset<32>(addr.to_string().substr(t_L1,s_L1)).to_ulong();
        int addr_tag = bitset<32>(addr.to_string().substr(0,t_L1)).to_ulong();
        

          /*if(set == 1979){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("try reading SET 1979 in L1\n");
                    printf("L1 dirty bit is %d\n", L1[set]->cache_Block[0]->dirty_bit);
                    printf("L1 valid bit is %d\n", L1[set]->cache_Block[0]->valid_bit);
                    printf("Target tag is %d\n", addr_tag);
                    printf("L1 tag is %d\n", L1[set]->cache_Block[i]->tag);
                }*/ 
        
        for (i=0; i<L1[set]->cache_Block.size(); i++){
            if(L1[set]->cache_Block[i]->tag == addr_tag && L1[set]->cache_Block[i]->valid_bit ==1 ){
                 /* if(set==1979){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("read hit SET 1979 in L1\n");
                    printf("L1 dirty bit is %d\n", L1[set]->cache_Block[0]->dirty_bit);
                    printf("L1 valid bit is %d\n", L1[set]->cache_Block[0]->valid_bit);
                    printf("Target tag is %d\n", addr_tag);
                    printf("L1 tag is %d\n", L1[set]->cache_Block[i]->tag);
                
                }*/ 
                return RH;
            }      
        } 
    
    /* if(set == 1979){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("Read miss SET 1979 in L1\n");
                }*/ 
     return RM;
    }

    auto readL2(auto addr){
        /*
        step 1: select the set in our L2 cache using set index bits
        step 2: iterate through each way in the current set
            - If Matching tag and Valid Bit High -> ReadHit!
                                                 -> copy dirty bit
        step 3: otherwise? -> ReadMiss! -> need to pull data from Main Memory
        step 4: find a place in L1 for our requested data
            - case 1: empty way in L1 -> place requested data
            - case 2: no empty way in L1 -> evict from L1 to L2
                    - case 2.1: empty way in L2 -> place evicted L1 data there
                    - case 2.2: no empty way in L2 -> evict from L2 to memory

        return {RM or RH, WRITEMEM or NOWRITEMEM}
        */
        int i,j;
        int dirty_bit_cpy = 0;
        int set = bitset<32>(addr.to_string().substr(t_L2,s_L2)).to_ulong();
        
        int addr_tag = bitset<32>(addr.to_string().substr(0,t_L2)).to_ulong();
        int set_l1 = bitset<32>(addr.to_string().substr(t_L1,s_L1)).to_ulong();
        int addr_tag_l1 = bitset<32>(addr.to_string().substr(0,t_L1)).to_ulong();

        int flag;




        for (i=0; i<L2[set]->cache_Block.size(); i++){
            //hit
            if(L2[set]->cache_Block[i]->tag == addr_tag && L2[set]->cache_Block[i]->valid_bit == 1){
                //printf("L2 read HIT\n");
                
                /*if(set==4027){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("reading hit SET 4027 in L2\n");
                    printf("Set L1 is %d\n",set_l1);
                    printf("L1 dirty bit is %d\n", L1[set_l1]->cache_Block[0]->dirty_bit);
                    printf("L1 valid bit is %d\n", L1[set_l1]->cache_Block[0]->valid_bit);
                    printf("L1 tag is %d\n", L1[set_l1]->cache_Block[0]->tag);
                    printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                    printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                    printf("Target tag is %d\n", addr_tag);
                    printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);
                }*/ 



                dirty_bit_cpy = L2[set]->cache_Block[i]->dirty_bit;
                L2[set]->cache_Block[i]->valid_bit = 0;
                 //case 1, move data from L2 to L1, L1 has room
                
                for (j=0; j<L1[L2[set]->cache_Block[i]->set_b]->cache_Block.size(); j++){
                    if(L1[L2[set]->cache_Block[i]->set_b]->cache_Block[j]->valid_bit == 0){

                        L1[L2[set]->cache_Block[i]->set_b]->cache_Block[j]->tag = addr_tag_l1;
                        L1[L2[set]->cache_Block[i]->set_b]->cache_Block[j]->tag_b = addr_tag;
                        L1[L2[set]->cache_Block[i]->set_b]->cache_Block[j]->set_b = set;
                        L1[L2[set]->cache_Block[i]->set_b]->cache_Block[j]->valid_bit = 1;
                        L1[L2[set]->cache_Block[i]->set_b]->cache_Block[j]->dirty_bit = dirty_bit_cpy;


                       /* if(set==4027){
                        printf("\n****Instruction**** %d\n",instruction);
                        printf("reading hit SET 4027 in L2\n");
                        printf("case1\n");
                        printf("Target tag is %d\n", addr_tag);
                        printf("Set L1 is %d\n",set_l1);
                        printf("L1 dirty bit is %d\n", L1[set_l1]->cache_Block[0]->dirty_bit);
                        printf("L1 valid bit is %d\n", L1[set_l1]->cache_Block[0]->valid_bit);
                        printf("L1 tag is %d\n", L1[set_l1]->cache_Block[0]->tag);
                        printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                        printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                        printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);
                        }*/ 

                        ans[0] = RH;
                        ans[1] = NOWRITEMEM;
                        //int ans[2] ={RH, NOWRITEMEM}
                        return ans;
                    }      
                }
                //case 2, L1 has no room
                //case 2.1, L2 has room, place L1 in L2
                //reset counter if it reaches the highest

                set_l1 = L2[set]->cache_Block[i]->set_b;

                if(L1[set_l1]->counter==L1[set_l1]->cache_Block.size()){
                    L1[set_l1]->counter = 0;
                }
                 if(L1[set_l1]->cache_Block[L1[set_l1]->counter]->set_b != 0){
                    set = L1[set_l1]->cache_Block[0]->set_b;
                }

                for (j=0; j<L2[set]->cache_Block.size(); j++){
                    if(L2[set]->cache_Block[j]->valid_bit == 0){
                        //Store L1 block into L2[i]
                        L2[set]->cache_Block[j]->tag = (L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b);
                        L2[set]->cache_Block[j]->tag_b = L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag;
                        L2[set]->cache_Block[j]->set_b = set_l1;
                        L2[set]->cache_Block[j]->valid_bit = 1;
                        L2[set]->cache_Block[j]->dirty_bit =L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit;

                        //Store data into L1[counter]
                        L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag = addr_tag_l1;
                        L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b = addr_tag;
                        L1[set_l1]->cache_Block[L1[set_l1]->counter]->set_b = set;
                        L1[set_l1]->cache_Block[L1[set_l1]->counter]->valid_bit = 1;
                        L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit = dirty_bit_cpy;
                        L1[set_l1]->counter += 1; //update counter
                        ans[0] = RH;
                        ans[1] = NOWRITEMEM;

                       /* if(set==4027){
                        printf("\n****Instruction**** %d\n",instruction);
                        printf("reading hit SET 4027 in L2\n");
                        printf("case2\n");
                        printf("store L1 into L2\n");
                        printf("Target tag is %d\n", addr_tag);
                        printf("Set L1 is %d\n",set_l1);
                        printf("L1 dirty bit is %d\n", L1[set_l1]->cache_Block[0]->dirty_bit);
                        printf("L1 valid bit is %d\n", L1[set_l1]->cache_Block[0]->valid_bit);
                        printf("L1 tag is %d\n", L1[set_l1]->cache_Block[0]->tag);
                        printf("L1 backup tag is %d\n", L1[set_l1]->cache_Block[0]->tag_b);
                        printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                        printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                        printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);
                        }*/ 

                        //int ans[2] ={RH, NOWRITEMEM};
                        return ans;
                    }
                }

                //case 2.2, L2 has no room, evict a block from L2 to memory, then put L1 into L2
                //reset counter if it reaches the highest
                if(L2[set]->counter==L2[set]->cache_Block.size()){
                    L2[set]->counter = 0;
                }

                if(L2[set]->cache_Block[L2[set]->counter]->dirty_bit == 1 && L2[set]->cache_Block[L2[set]->counter]->valid_bit == 1 ){
                    //move L2[counter] into memory
                    /*
                    no simulation of memory in this project
                    */
                    //Store L1 block into L2[counter]

                    L2[set]->cache_Block[L2[set]->counter]->tag = (L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b);
                    L2[set]->cache_Block[L2[set]->counter]->tag_b = L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag;
                    L2[set]->cache_Block[L2[set]->counter]->set_b = set_l1;
                    L2[set]->cache_Block[L2[set]->counter]->valid_bit = 1;
                    L2[set]->cache_Block[L2[set]->counter]->dirty_bit =L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit;
                    //Store data into L1[counter]
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag = addr_tag_l1;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b = addr_tag;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->set_b = set;


                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->valid_bit = 1;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit = dirty_bit_cpy;
                    L1[set_l1]->counter += 1; //update counter
                    L2[set]->counter += 1;

                       /* if(set==4027){
                        printf("\n****Instruction**** %d\n",instruction);
                        printf("reading hit SET 4027 in L2\n");
                        printf("case3\n");
                        printf("store L1 into L2, L2 eviction\n");
                        printf("Target tag is %d\n", addr_tag);
                        printf("Set L1 is %d\n",set_l1);
                        printf("L1 dirty bit is %d\n", L1[set_l1]->cache_Block[0]->dirty_bit);
                        printf("L1 valid bit is %d\n", L1[set_l1]->cache_Block[0]->valid_bit);
                        printf("L1 tag is %d\n", L1[set_l1]->cache_Block[0]->tag);
                        printf("L1 backup tag is %d\n", L1[set_l1]->cache_Block[0]->tag_b);
                        printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                        printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                        printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);
                        }*/ 



                    ans[0] = RH;
                    ans[1] = WRITEMEM;
                    //int ans[2] = {RH, WRITEMEM};
                    return ans;
                }

                 else{

                    //no action to memory
                    //Store L1 block into L2[counter]
                    L2[set]->cache_Block[L2[set]->counter]->tag = (L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b);
                    L2[set]->cache_Block[L2[set]->counter]->tag_b = L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag;
                    L2[set]->cache_Block[L2[set]->counter]->set_b = set_l1;
                    L2[set]->cache_Block[L2[set]->counter]->valid_bit = 1;
                    L2[set]->cache_Block[L2[set]->counter]->dirty_bit =L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit;
                    //Store data into L1[counter]
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag = addr_tag_l1;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b = addr_tag;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->set_b = set;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->valid_bit = 1;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit = dirty_bit_cpy;
                    L1[set_l1]->counter += 1; //update counter
                    L2[set]->counter += 1;


                        /*if(set==4027){
                        printf("\n****Instruction**** %d\n",instruction);
                        printf("reading hit SET 4027 in L2\n");
                        printf("case4\n");
                        printf("store L1 into L2, L2 overwrite\n");
                        printf("Target tag is %d\n", addr_tag);
                        printf("Set L1 is %d\n",set_l1);
                        printf("L1 dirty bit is %d\n", L1[set_l1]->cache_Block[0]->dirty_bit);
                        printf("L1 valid bit is %d\n", L1[set_l1]->cache_Block[0]->valid_bit);
                        printf("L1 tag is %d\n", L1[set_l1]->cache_Block[0]->tag);
                        printf("L1 backup tag is %d\n", L1[set_l1]->cache_Block[0]->tag_b);
                        printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                        printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                        printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);
                        }*/ 






                    ans[0] = RH;
                    ans[1] = NOWRITEMEM;
                    //int ans[2] = {RH, NOWRITEMEM};
                    return ans;
                }
            } 
      
           
        }

         //miss
               set_l1 = bitset<32>(addr.to_string().substr(t_L1,s_L1)).to_ulong();

               /*if(set==4027){
                    printf("\n****Instruction**** %d\n",instruction);
                    printf("reading miss SET 4027 in L2\n");
                    printf("Set L1 is %d\n",set_l1);
                    printf("L1 dirty bit is %d\n", L1[set_l1]->cache_Block[0]->dirty_bit);
                    printf("L1 valid bit is %d\n", L1[set_l1]->cache_Block[0]->valid_bit);
                    printf("L1 tag is %d\n", L1[set_l1]->cache_Block[0]->tag);
                    printf("L1 set backup is %d\n", L1[set_l1]->cache_Block[0]->set_b);
                    printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                    printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                    printf("Target tag is %d\n", addr_tag);
                    printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);
                }*/ 

                //data from memory put in L1
                //case 1, L1 has room
                 for (i=0; i<L1[set_l1]->cache_Block.size(); i++){
                    if(L1[set_l1]->cache_Block[i]->valid_bit == 0){
                        L1[set_l1]->cache_Block[i]->tag = addr_tag_l1;
                        L1[set_l1]->cache_Block[i]->tag_b = addr_tag;
                        L1[set_l1]->cache_Block[i]->set_b = set;
                        L1[set_l1]->cache_Block[i]->valid_bit = 1;
                        L1[set_l1]->cache_Block[i]->dirty_bit = 0;


                        /*if(set==4027){
                        printf("\n****Instruction**** %d\n",instruction);
                        printf("case1\n");
                        printf("L1 has room, put memory in L1\n");
                        printf("Target tag is %d\n", addr_tag);
                        printf("Set L1 is %d\n",set_l1);
                        printf("L1 dirty bit is %d\n", L1[set_l1]->cache_Block[0]->dirty_bit);
                        printf("L1 valid bit is %d\n", L1[set_l1]->cache_Block[0]->valid_bit);
                        printf("L1 tag is %d\n", L1[set_l1]->cache_Block[0]->tag);
                        printf("L1 backup tag is %d\n", L1[set_l1]->cache_Block[0]->tag_b);
                        printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                        printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                        printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);
                        }*/ 





                        ans[0] = RM;
                        ans[1] = NOWRITEMEM;
                        return ans;
                    } 
                }

                //case 2, L1 has no room
                //case 2.1, L2 has room, place L1 in L2
                //reset counter if it reaches the highest
                if(L1[set_l1]->counter==L1[set_l1]->cache_Block.size()){
                    L1[set_l1]->counter = 0;
                }

                 if(L1[set_l1]->cache_Block[L1[set_l1]->counter]->set_b != 0){
                    set = L1[set_l1]->cache_Block[0]->set_b;
                }

                for (i=0; i<L2[set]->cache_Block.size(); i++){
                    if(L2[set]->cache_Block[i]->valid_bit == 0){
                        //Store L1 block into L2[i]
                        L2[set]->cache_Block[i]->tag = (L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b);
                        L2[set]->cache_Block[i]->tag_b = L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag;
                        L2[set]->cache_Block[i]->set_b = set_l1;
                        L2[set]->cache_Block[i]->valid_bit = 1;
                        L2[set]->cache_Block[i]->dirty_bit =L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit;

                        //Store data into L1[counter]
                        L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag = addr_tag_l1;
                        L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b = addr_tag;
                        L1[set_l1]->cache_Block[L1[set_l1]->counter]->set_b = set;
                        L1[set_l1]->cache_Block[L1[set_l1]->counter]->valid_bit = 1;
                        L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit = 0;
                        L1[set_l1]->counter += 1; //update counter
                        
                        /*if(set==4027){
                        printf("\n****Instruction**** %d\n",instruction);
                        printf("case2\n");
                        printf("L1 has no room put L1 in L2\n");
                        printf("Target tag is %d\n", addr_tag);
                        printf("Set L1 is %d\n",set_l1);
                        printf("L1 dirty bit is %d\n", L1[set_l1]->cache_Block[0]->dirty_bit);
                        printf("L1 valid bit is %d\n", L1[set_l1]->cache_Block[0]->valid_bit);
                        printf("L1 tag is %d\n", L1[set_l1]->cache_Block[0]->tag);
                        printf("L1 backup tag is %d\n", L1[set_l1]->cache_Block[0]->tag_b);
                        printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                        printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                        printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);
                        }*/

                        ans[0] = RM;
                        ans[1] = NOWRITEMEM;
                        //int ans[2] ={RH, NOWRITEMEM};
                        return ans;
                    }
                }

                //case 2.2, L2 has no room, evict a block from L2 to memory, then put L1 into L2
                //reset counter if it reaches the highest
                if(L2[set]->counter==L2[set]->cache_Block.size()){
                    L2[set]->counter = 0;
                }

                if(L2[set]->cache_Block[L2[set]->counter]->dirty_bit == 1 && L2[set]->cache_Block[L2[set]->counter]->valid_bit == 1){
                    //move L2[counter] into memory
                    /*
                    no simulation of memory in this project
                    */
                    //Store L1 block into L2[counter]
                    L2[set]->cache_Block[L2[set]->counter]->tag = (L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b);
                    L2[set]->cache_Block[L2[set]->counter]->tag_b = L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag;
                    L2[set]->cache_Block[L2[set]->counter]->set_b = set_l1;
                    L2[set]->cache_Block[L2[set]->counter]->valid_bit = 1;
                    L2[set]->cache_Block[L2[set]->counter]->dirty_bit =L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit;
                    //Store data into L1[counter]
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag = addr_tag_l1;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b = addr_tag;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->set_b = set;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->valid_bit = 1;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit = 0;
                    L1[set_l1]->counter += 1; //update counter
                    L2[set]->counter += 1;

                    /*if(set==4027){
                        printf("\n****Instruction**** %d\n",instruction);
                        printf("case3\n");
                        printf("L1 has no room, L2 eviction\n");
                        printf("Target tag is %d\n", addr_tag);
                        printf("Set L1 is %d\n",set_l1);
                        printf("L1 dirty bit is %d\n", L1[set_l1]->cache_Block[0]->dirty_bit);
                        printf("L1 valid bit is %d\n", L1[set_l1]->cache_Block[0]->valid_bit);
                        printf("L1 tag is %d\n", L1[set_l1]->cache_Block[0]->tag);
                        printf("L1 backup tag is %d\n", L1[set_l1]->cache_Block[0]->tag_b);
                        printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                        printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                        printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);
                        }*/ 



                    ans[0] = RM;
                    ans[1] = WRITEMEM;
                    //int ans[2] = {RH, WRITEMEM};
                    return ans;
                }

                 else{

                    //printf("load l1 into l2, no evict in l2, store l1\n");
                    //no action to memory
                    //Store L1 block into L2[counter]
                    L2[set]->cache_Block[L2[set]->counter]->tag = (L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b);
                    L2[set]->cache_Block[L2[set]->counter]->tag_b = L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag;
                    L2[set]->cache_Block[L2[set]->counter]->set_b = set_l1;
                    L2[set]->cache_Block[L2[set]->counter]->valid_bit = 1;
                    L2[set]->cache_Block[L2[set]->counter]->dirty_bit =L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit;
                    //Store data into L1[counter]
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag = addr_tag_l1;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->tag_b = addr_tag;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->set_b = set;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->valid_bit = 1;
                    L1[set_l1]->cache_Block[L1[set_l1]->counter]->dirty_bit = 0;
                    L1[set_l1]->counter += 1; //update counter
                    L2[set]->counter += 1;
                    ans[0] = RM;
                    ans[1] = NOWRITEMEM;

                    /*if(set==4027){
                        printf("\n****Instruction**** %d\n",instruction);
                        printf("case4\n");
                        printf("L1 has no room, overwrite L2\n");
                        printf("Target tag is %d\n", addr_tag);
                        printf("Set L1 is %d\n",set_l1);
                        printf("L1 dirty bit is %d\n", L1[set_l1]->cache_Block[0]->dirty_bit);
                        printf("L1 valid bit is %d\n", L1[set_l1]->cache_Block[0]->valid_bit);
                        printf("L1 tag is %d\n", L1[set_l1]->cache_Block[0]->tag);
                        printf("L1 backup tag is %d\n", L1[set_l1]->cache_Block[0]->tag_b);
                        printf("L2 dirty bit is %d\n", L2[set]->cache_Block[0]->dirty_bit);
                        printf("L2 valid bit is %d\n", L2[set]->cache_Block[0]->valid_bit);
                        printf("L2 tag is %d\n", L2[set]->cache_Block[0]->tag);
                        }*/ 




                    //int ans[2] = {RH, NOWRITEMEM};
                    return ans;
                }

        
        
                
                
             
            

    }
};

// Tips: another example:
/*
class CacheSystem
{
    Cache L1;
    Cache L2;
public:
    int L1AcceState, L2AcceState, MemAcceState;
    auto read(auto addr){};
    auto write(auto addr){};
};

class Cache
{
    set * CacheSet;
public:
    auto read_access(auto addr){};
    auto write_access(auto addr){};
    auto check_exist(auto addr){};
    auto evict(auto addr){};
};
**/
/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/





int main(int argc, char *argv[])
{

    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while (!cache_params.eof())                   // read config file
    {
        cache_params >> dummyLine;                // L1:
        cache_params >> cacheconfig.L1blocksize;  // L1 Block size
        cache_params >> cacheconfig.L1setsize;    // L1 Associativity
        cache_params >> cacheconfig.L1size;       // L1 Cache Size
        cache_params >> dummyLine;                // L2:
        cache_params >> cacheconfig.L2blocksize;  // L2 Block size
        cache_params >> cacheconfig.L2setsize;    // L2 Associativity
        cache_params >> cacheconfig.L2size;       // L2 Cache Size
    }
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    string line;
    string accesstype;     // the Read/Write access type from the memory trace;
    string xaddr;          // the address from the memory trace store in hex;
    unsigned int addr;     // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;




/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like
    if (cacheconfig.L1blocksize!=cacheconfig.L2blocksize){
        printf("please test with the same block size\n");
        return 1;
    }
    cache c1(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size, cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);
   // printf("%d %d %d %d %d %d",cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size, cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size );
 

    int L1AcceState = NA; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState = NA; // L2 access state variable, can be one of NA, RH, RM, WH, WM;
    int MemAcceState = NOWRITEMEM; // Main Memory access state variable, can be either NA or WH;

    if (traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        { // read mem access file and access Cache
            instruction += 1;
            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr)){
                break;
            }
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32>(addr);

            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R") == 0)  // a Read request
            {
                // Implement by you:
                //   read access to the L1 Cache,
                //   and then L2 (if required),
                //   update the access state variable;
                //   return: L1AcceState L2AcceState MemAcceState
                
                // For example:
                // L1AcceState = cache.readL1(addr); // read L1
                // if(L1AcceState == RM){
                //     L2AcceState, MemAcceState = cache.readL2(addr); // if L1 read miss, read L2
                // }
                // else{ ... }
                L1AcceState = c1.readL1(accessaddr);
                if(L1AcceState== RM){
                    int* p;
                    p =  c1.readL2(accessaddr);
                    L2AcceState= p[0];// if L1 read miss, read L2
                    MemAcceState = p[1];
                }
                else{
                    //printf("A read hit\n");
                    L2AcceState = NA;
                    MemAcceState = NOWRITEMEM;
                }

            }
            else{ // a Write request
                // Implement by you:
                //   write access to the L1 Cache, or L2 / main MEM,
                //   update the access state variable;
                //   return: L1AcceState L2AcceState

                // For example:
                // L1AcceState = cache.writeL1(addr);
                // if (L1AcceState == WM){
                //     L2AcceState, MemAcceState = cache.writeL2(addr);
                // }
                // else if(){...}
                L1AcceState = c1.write(accessaddr);
                if(L1AcceState == WM){
                    int* p;
                    p = c1.writeL2(accessaddr);
                    L2AcceState = p[0];
                    MemAcceState = p[1];
                }

                else{
                    L2AcceState = NA;
                    MemAcceState =NOWRITEMEM;
                }
            }
/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/




            // Grading: don't change the code below.
            // We will print your access state of each cycle to see if your simulator gives the same result as ours.
            tracesout << L1AcceState << " " << L2AcceState << " " << MemAcceState << endl; // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close();
    }
    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
