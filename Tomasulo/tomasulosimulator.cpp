#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <limits>

using std::cout;
using std::endl;
using std::string;
using std::vector;

string inputtracename = "trace.txt";
// remove the ".txt" and add ".out.txt" to the end as output name
string outputtracename = inputtracename.substr(0, inputtracename.length() - 4) + ".out.txt";
string hardwareconfigname = "config.txt";

enum Operation
{
	ADD,
	SUB,
	MULT,
	DIV,
	LOAD,
	STORE
};
// The execute cycle of each operation: ADD, SUB, MULT, DIV, LOAD, STORE
const int OperationCycle[6] = {2, 2, 10, 40, 2, 2};

struct HardwareConfig
{
	int LoadRSsize;	 // number of load reservation stations
	int StoreRSsize; // number of store reservation stations
	int AddRSsize;	 // number of add reservation stations
	int MultRSsize;	 // number of multiply reservation stations
	int FRegSize;	 // number of fp registers
};

// We use the following structure to record the time of each instruction
struct InstructionStatus
{
	int cycleIssued;
	int cycleExecuted; // execution completed
	int cycleWriteResult;
};

// Register Result Status structure
struct RegisterResultStatus
{	
	string RegisterName;
	string ReservationStationName;
	bool dataReady;
};

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
struct Instruction{
	string operation;
	string destination;
	string oprand_1;
	string oprand_2;
};


struct InstructionTable
{
	vector<InstructionStatus> instruction_status;
	vector<Instruction> instruction;
};




class RegisterResultStatuses //RRS
{
public:
	// ...
	void ini_RRS(int regsize){
		 _registers.resize(regsize, RegisterResultStatus{"","", 0});
		for(int i =0; i<regsize; i++){
			_registers[i].RegisterName = "F" + std::to_string(i);
			_registers[i].dataReady = false;
		}
	}

	vector<RegisterResultStatus> ReturnRegisterList(){
		return _registers;
	}

	void update_RRS(string result){
		for(int i=0; i<_registers.size(); i++){
			if(_registers[i].ReservationStationName == result){
				_registers[i].dataReady = true;
			}
		}
	}

	void mark_station(string register_name, string station_name){
		for(int i=0; i<_registers.size(); i++){
			//printf("!!register name is %s\n\n",_registers[i].RegisterName.c_str());
			//_registers[i].ReservationStationName = "OK";
			if(_registers[i].RegisterName == register_name && station_name.substr(0, size(station_name)-1) != "Store"){
				//printf("!!register name is %s\n\n",_registers[i].RegisterName.c_str());
				_registers[i].ReservationStationName = station_name;
				//printf("!!station name is %s\n\n",_registers[i].ReservationStationName.c_str());
				_registers[i].dataReady = false;
			}
		}
	

	}

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/
	/*
	Print all register result status. It is called by PrintRegisterResultStatus4Grade() for grading.
	If you don't want to write such a class, then make sure you call the same function and print the register
	result status in the same format.
	*/
	string _printRegisterResultStatus() const
	{
		std::ostringstream result;
		for (int idx = 0; idx < _registers.size(); idx++)
		{
			result << "F" + std::to_string(idx) << ": ";
			result << _registers[idx].ReservationStationName << ", ";
			result << "dataRdy: " << (_registers[idx].dataReady ? "Y" : "N") << ", ";
			result << "\n";
		}
		return result.str();
	}
/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
private:
	vector<RegisterResultStatus> _registers;
};

// Define your Reservation Station structure
struct ReservationStation
{
	string name ="";
	bool busy = false;
	string op = "";
	string vj = "";
	string vk = "";
	string qj = "";
	string qk = "";
	int timer = -1;
	int issue_time = 0;

};

class ReservationStations
{
public:
	void ini_RS(int ls, int ss, int as, int ms){
		int index = 0;
		_stations.resize(ls+ss+as+ms, ReservationStation{"", false, "", "", "", "", "", -1, 0});
		for(int i=0; i<ls; i++){
			_stations[index].name = "Load" + std::to_string(i);
			++index;
		}
		for(int i=0; i<ss; i++){
			_stations[index].name = "Store" + std::to_string(i);
			++index;
		}
		for(int i=0; i<as; i++){
			_stations[index].name = "Add" + std::to_string(i);
			++index;
		}
		for(int i=0; i<ms; i++){
			_stations[index].name = "Mult" + std::to_string(i);
			++index;
		}
	}

	void RS_update(string result){
		for(int i=0; i<_stations.size(); i++){
			if(_stations[i].qj == result){
				_stations[i].qj = "";
			}
			if(_stations[i].qk == result){
				_stations[i].qk = "";
			}
		}
	}

	vector<ReservationStation> RS_to_CDB(){
		vector<ReservationStation> result;
		for(int i=0; i<_stations.size(); i++){
			if(_stations[i].timer == 0 && _stations[i].busy == true){
				result.push_back(_stations[i]);
				_stations[i].busy = false;
				_stations[i].timer = -1; //reset timer to -1
			}
		}
		return result;
	}

	int RS_add(string station_name, string destination, string operation, string oprand_1, string oprand_2, int issue_time, RegisterResultStatuses &RRS){
		int flag = 0;
		vector<RegisterResultStatus> RRS_temp = RRS.ReturnRegisterList();
		for(int i=0; i<_stations.size(); i++){
			//printf("\nsearching in the station ...... \n");
			//std::cout<<_stations[i].name.substr(0,size(_stations[i].name)-1)<<endl;
			if(_stations[i].name.substr(0,size(_stations[i].name)-1) == station_name && _stations[i].busy == false){
				//printf("flag = 1\n");
				flag = 1;
				_stations[i].op = operation;
				_stations[i].issue_time = issue_time;
				_stations[i].busy = true;

				// check if all data are ready
				//special store
				if(operation == "STORE"){
					for(int j=0; j<RRS_temp.size(); j++){
						if(RRS_temp[j].RegisterName == destination){
							if(RRS_temp[j].dataReady == false && !RRS_temp[j].ReservationStationName.empty()){
								_stations[i].qj = RRS_temp[j].ReservationStationName;
							}
						}
					}
				}
				//1. not load or store
				else if(oprand_1!= "0" && oprand_2!= "0"){
					for(int j=0; j<RRS_temp.size(); j++){
						if(RRS_temp[j].RegisterName == oprand_1){
							if(RRS_temp[j].dataReady == false && !RRS_temp[j].ReservationStationName.empty()){
								_stations[i].qj = RRS_temp[j].ReservationStationName;
							}
						}
					}
					for(int j=0; j<RRS_temp.size(); j++){
						if(RRS_temp[j].RegisterName == oprand_2){
							if(RRS_temp[j].dataReady == false && !RRS_temp[j].ReservationStationName.empty()){
								_stations[i].qk = RRS_temp[j].ReservationStationName;
							}
						}
					}
				}
				//else, load or store
				else{
					if(oprand_1 == "0"){
						for(int j=0; j<RRS_temp.size(); j++){
							if(RRS_temp[j].RegisterName == oprand_2){
								if(RRS_temp[j].dataReady == false && !RRS_temp[j].ReservationStationName.empty()){
									_stations[i].qk = RRS_temp[j].ReservationStationName;
								}
							}
						}
					}
					else{
						for(int j=0; j<RRS_temp.size(); j++){
							if(RRS_temp[j].RegisterName == oprand_1){
								if(RRS_temp[j].dataReady == false && !RRS_temp[j].ReservationStationName.empty()){
									_stations[i].qj = RRS_temp[j].ReservationStationName;
								}
							}
						}
					}
				}
				//printf("\ndestination passed to mark function is %s\n\n", destination.c_str());
				RRS.mark_station(destination, _stations[i].name);
				break;
			}
		}
		return flag;
	}


	void RS_pre_execute(){
		for(int k=0; k<_stations.size(); k++){
			if(_stations[k].busy == true){
				//std::cout<<_stations[k].name<<endl;
				//1. all data ready, execure next cycle
				if(_stations[k].qj.empty() && _stations[k].qk.empty() &&_stations[k].timer == -1){
					if(_stations[k].op == "LOAD"){
						_stations[k].timer = OperationCycle[4];
					}
					else if(_stations[k].op == "STORE"){
						_stations[k].timer = OperationCycle[5];
					}
					else if(_stations[k].op == "ADD"){
						_stations[k].timer = OperationCycle[0];
					}
					else if(_stations[k].op == "SUB"){
						_stations[k].timer = OperationCycle[1];
					}
					else if(_stations[k].op == "MULT"){
						_stations[k].timer = OperationCycle[2];
					}
					else if(_stations[k].op == "DIV"){
						_stations[k].timer = OperationCycle[3];
					}
				}
			}
		}
	}

	void RS_execute(){
		for(int k=0; k<_stations.size(); k++){
			if(_stations[k].busy == true && _stations[k].timer > 0){
				_stations[k].timer = _stations[k].timer -1;
				//printf("Station %s starting execution\n", _stations[k].name.c_str());
			}
		}
	}

	vector<ReservationStation> returnRS(){
		return _stations;
	}


private:
	vector<ReservationStation> _stations;
};




struct CDB {
	string station_name;
	int result;
	int issue_time;
};






class CommonDataBus
{
public:
	void add_CDB(string station_name, int issue_time){

		CDB common_data_bus;
		if(station_name!="STORE"){
			common_data_bus.station_name = station_name;
			common_data_bus.issue_time = issue_time;
		}
		commonDataBus.push_back(common_data_bus);
	}

	CDB broadcast(){
			string name, result;
			int time = 100000000;
			CDB temp;
			//if CDB has results
			//printf("cdb size is %d\n",commonDataBus.size() );
			if(commonDataBus.size()>0){
				for(int i=0; i<commonDataBus.size(); i++){
					if(commonDataBus[i].issue_time<time){
						time = commonDataBus[i].issue_time;
						name = commonDataBus[i].station_name;
					}
				}
				// free CDB
				for(int i=0; i<commonDataBus.size(); i++){
					if(commonDataBus[i].issue_time == time && commonDataBus[i].station_name == name){
						commonDataBus.erase(commonDataBus.begin()+i);
					}
				}
				temp.issue_time = time;
				temp.station_name = name;
			}
			return temp;
	}

	vector<CDB>  returnCDB(){
		return commonDataBus;
	}
	


private:
	vector<CDB> commonDataBus;
};










/*
print the instruction status, the reservation stations and the register result status
@param filename: output file name
@param instructionStatus: instruction status
*/
void PrintResult4Grade(const string &filename, const vector<InstructionStatus> &instructionStatus)
{
	std::ofstream outfile(filename, std::ios_base::app); // append result to the end of file
	outfile << "Instruction Status:\n";
	for (int idx = 0; idx < instructionStatus.size(); idx++)
	{
		outfile << "Instr" << idx << ": ";
		outfile << "Issued: " << instructionStatus[idx].cycleIssued << ", ";
		outfile << "Completed: " << instructionStatus[idx].cycleExecuted << ", ";
		outfile << "Write Result: " << instructionStatus[idx].cycleWriteResult << ", ";
		outfile << "\n";
	}
	outfile.close();
}

/*
print the register result status each 5 cycles
@param filename: output file name
@param registerResultStatus: register result status
@param thiscycle: current cycle
*/
void PrintRegisterResultStatus4Grade(const string &filename,
									 const RegisterResultStatuses &registerResultStatus,
									 const int thiscycle)
{
	if (thiscycle % 5 != 0)
		return;
	std::ofstream outfile(filename, std::ios_base::app); // append result to the end of file
	outfile << "Cycle " << thiscycle << ":\n";
	outfile << registerResultStatus._printRegisterResultStatus() << "\n";
	outfile.close();
}


void PrintStation(ReservationStations RS){
	vector<ReservationStation> result;
	result = RS.returnRS();
	printf("\n**********Reservation Station**********\n");
	for(int i=0; i<result.size(); i++){
		printf("%s\t, %d\t, %s\t, %s\t, %s\t, %s\t, %s\t, %d\t, %d\n", result[i].name.c_str(), result[i].busy, result[i].op.c_str(), result[i].vj.c_str(), 
		result[i].vk.c_str(), result[i].qj.c_str(), result[i].qk.c_str(), result[i].timer, result[i].issue_time);
	}

}

void PrintCDB(CommonDataBus _CDB){
	vector<CDB> result;
	result = _CDB.returnCDB();
	printf("\n**********CDB**********\n");
	for(int i=0; i<result.size(); i++){
		printf("%s\t,%d\n", result[i].station_name.c_str(), result[i].issue_time);
	}

}

void PrintRegister(RegisterResultStatuses registerStatuses){
	vector<RegisterResultStatus> result;
	result = registerStatuses.ReturnRegisterList();
	printf("\n**********RRS TABLE**********\n");
	for(int i=0; i<result.size(); i++){
		printf("%s\t,%s\t,%d\n", result[i].RegisterName.c_str(), result[i].ReservationStationName.c_str(),result[i].dataReady);
	}
}

// Function to simulate the Tomasulo algorithm
void simulateTomasulo(HardwareConfig hardwareConfig, InstructionTable &INStable)
{
	int thiscycle = 1; // start cycle: 1
	//initialize RRS
	RegisterResultStatuses RRS;
	RRS.ini_RRS(hardwareConfig.FRegSize);
	//initialize CDB
	CommonDataBus _CDB;
	//initialize reservation stations
	ReservationStations reservationStation;
	reservationStation.ini_RS(hardwareConfig.LoadRSsize, hardwareConfig.StoreRSsize, hardwareConfig.AddRSsize, hardwareConfig.MultRSsize);


	while (thiscycle < 100000000)
	{
		//PrintStation(reservationStation);
		//PrintCDB(_CDB);
		//PrintRegister(RRS);
		// Reservation Stations should be updated every cycle, and broadcast to Common Data Bus
		// ...
		//1.CDB broadcast to RRS & reservation stations
		CDB cdb_result;
		cdb_result = _CDB.broadcast();

		


		if(!cdb_result.station_name.empty()){
			RRS.update_RRS(cdb_result.station_name);
			reservationStation.RS_update(cdb_result.station_name);
			//update instruction table write result time
			for(int j=0; j<INStable.instruction.size(); j++){
					if(INStable.instruction_status[j].cycleIssued == cdb_result.issue_time){
						INStable.instruction_status[j].cycleWriteResult = thiscycle;
					}
				}

		}

		

		// Issue new instruction in each cycle
		// ...
		for(int i=0; i<INStable.instruction.size(); i++){
			if(INStable.instruction_status[i].cycleIssued == 0){
				//std::cout<<INStable.instruction[i].destination<<endl;
				//this instruction needs to be added to reservation station
				//1. decode
				string op;
				string sub_station_name;
				int flag;
				op= INStable.instruction[i].operation;
				if(op == "LOAD"){
					sub_station_name = "Load";
				}
				else if(op == "STORE"){
					sub_station_name = "Store";
				}
				else if(op == "DIV"){
					sub_station_name = "Mult";
				}
				else if(op == "MULT"){
					sub_station_name = "Mult";
				}
				else if(op == "ADD"){
					sub_station_name = "Add";
				}
				else if(op == "SUB"){
					sub_station_name = "Add";
				}
				//2. try add instruction to station, and update RRS
				flag = reservationStation.RS_add(sub_station_name,INStable.instruction[i].destination,op, INStable.instruction[i].oprand_1, 
				INStable.instruction[i].oprand_2, thiscycle, RRS);

				if(flag==1){
					//printf("issue success\n");
					INStable.instruction_status[i].cycleIssued = thiscycle;
				}
				else{
					INStable.instruction_status[i].cycleIssued = 0;
				}
				break;
			}
		}
		
		//execute stations
		reservationStation.RS_execute();
		reservationStation.RS_pre_execute();


		//2.store executed results to CDB
		vector<ReservationStation> temp_RS;
		temp_RS = reservationStation.RS_to_CDB();
		if(temp_RS.size()>0){
			//printf("Store result to CDB\n");
			for(int i=0; i<temp_RS.size(); i++){
				//update CDB
				_CDB.add_CDB(temp_RS[i].name, temp_RS[i].issue_time);
				//update instruction table executed time
				for(int j=0; j<INStable.instruction.size(); j++){
					if(INStable.instruction_status[j].cycleIssued == temp_RS[i].issue_time){
						INStable.instruction_status[j].cycleExecuted = thiscycle;
					}
				}
			}
		}


		// At the end of this cycle, we need this function to print all registers status for grading
		PrintRegisterResultStatus4Grade(outputtracename, RRS, thiscycle);

		//PrintStation(reservationStation);
		//PrintCDB(_CDB);
		//PrintRegister(RRS);

		//printf("\n\n******************************************cycle %d ends***************************************\n\n", thiscycle);
		++thiscycle;

		// The simulator should stop when all instructions are finished.
		// ...
		int end_flag = 1;
		for(int i=0; i<INStable.instruction.size(); i++){
			if(INStable.instruction_status[i].cycleWriteResult == 0){
				end_flag = 0;
			}
		}
		if(end_flag==1){
			break;
		}
		

		
	}
};

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/



int main(int argc, char **argv)
{
	if (argc > 1)
	{
		hardwareconfigname = argv[1];
		inputtracename = argv[2];
	}

	HardwareConfig hardwareConfig;
	std::ifstream config;
	config.open(hardwareconfigname);
	config >> hardwareConfig.LoadRSsize;  // number of load reservation stations
	config >> hardwareConfig.StoreRSsize; // number of store reservation stations
	config >> hardwareConfig.AddRSsize;	  // number of add reservation stations
	config >> hardwareConfig.MultRSsize;  // number of multiply reservation stations
	config >> hardwareConfig.FRegSize;	  // number of fp registers
	config.close();
/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/

	// Read instructions from a file (replace 'instructions.txt' with your file name)
	// ...
	vector<Instruction> Instructions;
    vector<InstructionStatus> INSstatus;
    InstructionTable INStable;
    string line;
    std::ifstream myfile;
    myfile.open("trace.txt");
    //printf("ok\n");
    int ins_counter = 0;
    while (std::getline(myfile, line))
    {   
        Instruction instr;
        std::istringstream iss(line);
        string a, b, c, d;
        //std::cout<<line<<endl;
        iss>>a>>b>>c>>d;
        instr.operation = a;
        instr.destination = b;
        instr.oprand_1 = c;
        instr.oprand_2 = d;
        Instructions.push_back(instr);
        ins_counter +=1;
        
    }
	//create instruction table
    INSstatus.resize(ins_counter, InstructionStatus{0, 0, 0});
    INStable.instruction = Instructions;
    INStable.instruction_status = INSstatus;




	

	// Initialize the register result status
	
	// ...

	// Initialize the instruction status table
	//vector<InstructionStatus> instructionStatus;
	// ...

	// Simulate Tomasulo:
	// simulateTomasulo(registerResultStatus, instructionStatus, ...);
	simulateTomasulo(hardwareConfig, INStable);

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

	// At the end of the program, print Instruction Status Table for grading
	PrintResult4Grade(outputtracename, INStable.instruction_status);

	return 0;
}
