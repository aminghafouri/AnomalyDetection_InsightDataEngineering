#include<iostream>
#include<unordered_map>
#include<unordered_set>
#include<deque>
#include<fstream>
#include<string>
#include<utility>
#include<list>
#include<vector>
#include<functional>
#include<map>
#include<queue>
#include<set>
#include<cmath>
#include<ctime>
using namespace std;


class anomalyDetection {
public:
	anomalyDetection(string batch_log) {
		this->readFile(batch_log, false);
	}
	
	// Anomaly Detector
	void anomalyDetector(string timestamp, int id, double amount) {
		//  most recent transactions are stored in a mean heap of size T
		priority_queue<pair<string, double>, vector<pair<string, double> >, std::greater<pair<string, double> > > heap;
		
		// BFS up to level D
		unordered_set<int> visited;
		queue<int> users;
		visited.insert(id);
		users.push(id);
		users.push(-1);
		int degree = 1;
		while (!users.empty() && degree <= D) {
			int cur = users.front();  
			users.pop();
			if (cur == -1) {
				degree++;
				users.push(-1);
				if (users.front() == -1) break;
			}
			else {
				auto frnds_cur = network.find(cur);
				if (frnds_cur != network.end() && !frnds_cur->second.empty() ) {
					for (auto n : frnds_cur->second) {
						auto it = visited.find(n);
						if (it == visited.end()) {
							// for an unprocessed node, add its transactions to the heap if it satisfies the below conditions
							auto trans_n = transactions.find(n);
							if (trans_n != transactions.end() && !trans_n->second.empty()) {
								for (auto i : trans_n->second) {
									// add if heap is not full yet
									if (heap.size() < T) {
										heap.push(pair<string, double>(i.first, i.second));
									}
									// otherwise add only if the new entry is more recent than the oldest entry in the heap
									else if (i.first > heap.top().first) {
										heap.pop();
										heap.push(pair<string, double>(i.first, i.second));
									}
								}
							}

							visited.insert(n);
							users.push(n);
						}
					}
				}
			}
		}

		// Deciding if anomalous through computing mean and std		
		double sum = 0;
		vector<double> nums;
		while (!heap.empty()) {
			double amount = heap.top().second;
			sum += amount;
			nums.push_back(amount);
			heap.pop();
		}
		double mean = sum/T;
		double std = 0.00;
	
		for (int i = 0; i < nums.size(); ++i)
			std += pow(nums[i] - mean, 2)/T;

		std = sqrt(std);
		if (mean != 0 && nums.size() >= 2 && amount > mean + 3 * std) {
			vector<string> v;
			v.push_back(timestamp);
			v.push_back(to_string(id));
			v.push_back(to_string(amount));
			v.push_back(to_string(mean));
			v.push_back(to_string(std));
			flagged.push_back(v);
		}
		
	}

	// processes an entry
	void process(string &event, string &timestamp, string &id_id1, string &amount_id2, bool &isStream) {
		
		// Case 1: Purchase
		if (event == "purchase") {
			int id = stoi(id_id1);
			double amount = stod(amount_id2);

			// call anomaly detector on this entry
			if(isStream)
				anomalyDetector(timestamp, id, amount);

			// update transactions of the user
			auto it = transactions.find(id);
			if (it == transactions.end()) {
				multimap<string, double> m;
				m.insert(pair<string, double>(timestamp, amount));
				transactions.insert(pair<int, multimap<string, double> >(id, m));
			}
			else {
				it->second.insert(pair<string, double>(timestamp, amount));
			}
		}

		// Case 2: Befriend
		else if (event == "befriend") {
			int id1 = stoi(id_id1);
			int id2 = stoi(amount_id2);
			// add id2 to id1's list of friends, similarly add id1 to id2's list of friends
			auto it_id1 = network.find(id1);
			if (it_id1 == network.end()) {
				unordered_set<int> st;
				st.insert(id2);
				network.insert(pair<int, unordered_set<int> >(id1, st));
			}
			else {
				it_id1->second.insert(id2);
			}
			auto it_id2 = network.find(id2);
			if (it_id2 == network.end()) {
				unordered_set<int> st;
				st.insert(id1);
				network.insert(pair<int, unordered_set<int> >(id2, st));
			}
			else {
				it_id2->second.insert(id1);			
			}

		}

		// Case 3: Unfriend
		else if (event == "unfriend") {
			int id1 = stoi(id_id1);
			int id2 = stoi(amount_id2);
			// erase id1 and id2 from each other's list of friends
			auto it_id1 = network.find(id1);
			if (!it_id1->second.empty()) it_id1->second.erase(id2);
			auto it_id2 = network.find(id2);
			if (!it_id2->second.empty()) it_id2->second.erase(id1);
		}
	}

	// read file
	void readFile(string file, bool isStream) {
		ifstream batch_log(file); // Read input file
		
		string s;
		std::string delimiter = "\"";
		
		// Reading D & T
		if (!isStream) {
			getline(batch_log, s);
			size_t pos = 0;
			string token;
			int q_cnt = 0;
			while ((pos = s.find(delimiter)) != string::npos) {
				token = s.substr(0, pos);
				if (q_cnt == 3) { // D
					D = stoi(token);
				}
				else if (q_cnt == 7) { // T
					T = stoi(token);
				}
				q_cnt++;
				s.erase(0, pos + delimiter.length());
			}
		}
		// Reading entries
		while (getline(batch_log, s)) {
			size_t pos = 0;
			string token;
			int q_cnt = 0;
			string event;
			string timestamp;
			string id_id1;
			string amount_id2;
			while ((pos = s.find(delimiter)) != string::npos) {
				token = s.substr(0, pos);
				if (q_cnt == 3) {
					event = token;
				}
				else if (q_cnt == 7) {
					timestamp = token;
				}
				else if (q_cnt == 11) {
					id_id1 = token;
				}
				else if (q_cnt == 15) {
					amount_id2 = token;
				}
				q_cnt++;
				s.erase(0, pos + delimiter.length());
			}

			process(event, timestamp, id_id1, amount_id2, isStream); // processing each entry
		}
		this->outputFlagged();

	}


	// prints list of friends for each user
	void printNetwork() {
		for (auto i : network) {
			for (auto j : i.second) {
				cout << i.first << " " << j << endl;
			}
			cout << endl;
		}
	}

	// prints list of transactions for each user
	void printTransactions() {
		for (auto i : transactions) {
			for (auto j : i.second) {
				cout << i.first << " " << j.first << " " << j.second << endl;
			}
			cout << endl;
		}
	}

	// outputs flagged entries
	void outputFlagged() {
		ofstream myfile("./log_output/flagged_purchases.json");
		if (myfile.is_open())
		{
			for (auto i : flagged) {
				myfile << "{\"event_type\":\"purchase\", \"timestamp\":\"" << i[0] << "\", \"id\": \"" << i[1] << "\", \"amount\": \""
					<< round(stod(i[2]) * 100) / 100 << "\", \"mean\": \"" << round(stod(i[3]) * 100) / 100 << "\", \"sd\": \"" << round(stod(i[4]) * 100) / 100 << "\"}" << endl;
			}
			myfile.close();
		}
		else cout << "Unable to open file";
	}

	int T, D;
	unordered_map<int, unordered_set<int> > network; // hash table where key is person's user id and value is set of the person's friends
	unordered_map<int, multimap<string, double> > transactions; // hashtable where key is person's user id and value is multimap containing all previous transactions of the user
	vector<vector<string> > flagged; // two-dimensional vector where at each row, we store information about the flagged entries

};



int main(int argc, char* argv[]) {
	clock_t begin = clock();
	// address of batch_log and stream_log files
	string batch_log = "./log_input/batch_log.json";
	string stream_log = "./log_input/stream_log.json";
	anomalyDetection* obj = new anomalyDetection(batch_log);
	obj->readFile(stream_log, true);
	clock_t end = clock();
	double elapsed_sec = double(end-begin)/CLOCKS_PER_SEC;
	cout << elapsed_sec << endl;
	delete obj;
	return 0;
}
