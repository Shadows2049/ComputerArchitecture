#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <bitset>
#include <sstream>
#include <string>
using namespace std;



int bin_to_int(string bin) {
  int val = 0;
  int pow = 1;
  for (int i = bin.length() - 1; i >= 0; i--) {
    if (bin[i] == '1') {
      val += pow;
    }
    pow *= 2;
  }
  return val;
}


char update_counter(char counter, char outcome) {
  if (outcome == '1') { // branch taken
    if (counter == '0') { // strong not taken
      return '1'; // weak not taken
    } else if (counter == '1') { // weak not taken
      return '2'; // weak taken
    } else if (counter == '2') { // weak taken
      return '3'; // strong taken
    } else { // strong taken
      return '3'; // strong taken
    }
  } else { // branch not taken
    if (counter == '0') { // strong not taken
      return '0'; // strong not taken
    } else if (counter == '1') { // weak not taken
      return '0'; // strong not taken
    } else if (counter == '2') { // weak taken
      return '1'; // weak not taken
    } else { // strong taken
      return '2'; // weak taken
    }
  }
}


string update_history(string history, char outcome) {
  history = history.substr(1) + outcome; // left-shift and insert outcome
  return history;
}

char predict_branch(char counter) {
  if (counter == '0' || counter == '1') { // strong or weak not taken
    return '0'; // predict not taken
  } else { // strong or weak taken
    return '1'; // predict taken
  }
}

int main (int argc, char** argv) {
	ifstream config;
	config.open(argv[1]);

	int m, w, h;
	config >> m;
	config >> h;
	config >> w;
 
	config.close();

	ofstream out;
	string out_file_name = string(argv[2]) + ".out";
	out.open(out_file_name.c_str());
	
	ifstream trace;
	trace.open(argv[2]);

	vector<char> PHT(pow(2, m), '2');
	vector<string> BHT(pow(2, h), string(w, '0'));

	string line;
	while (getline(trace, line)) {
		string PC = line.substr(0, line.length() - 2);
    //cout<<PC<<"\n";
		char outcome = line.back();
    stringstream ss;
    ss << hex << PC;
    unsigned n;
    ss >> n;
    bitset<32> b(n);
    PC = b.to_string();


		int BHT_index = bin_to_int(PC.substr(30-h, h));
		string history = BHT[BHT_index];

		string PHT_index = PC.substr(30-m+w, m - w) + history;

		int PHT_index_int = bin_to_int(PHT_index);
		char counter = PHT[PHT_index_int];
		char prediction = predict_branch(counter); // predict the branch as taken or not taken

		// Write the prediction to the output file
		out << prediction << endl;

		counter = update_counter(counter, outcome);
		PHT[PHT_index_int] = counter;

		// Update the BHT entry
		history = update_history(history, outcome);
		BHT[BHT_index] = history;
	}
	 
	trace.close();	
	out.close();
}
