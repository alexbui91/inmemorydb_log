/*
 * TransactionHandle.h
 *
 *  Created on: Dec 4, 2017
 *      Author: alex
 */

#ifndef TRANSACTIONHANDLE_H_
#define TRANSACTIONHANDLE_H_

#include <map>
#include <mutex>
#include <iostream>
#include <fstream>
#include <condition_variable>
#include <set>

#include <boost/gil/extension/io/png_dynamic_io.hpp>
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>

using namespace std;
using namespace boost::gil;

struct GrayImage{
	vector<int> data;
	int height = 0;
	int width= 0;
	string name = "";
};

class TransactionHandle{
private:
	map<size_t, GrayImage*>* transaction_data;
	string log_buffer;
	bool pause_flag;
	condition_variable cv;
	mutex m;
	const string log_files = "logs.txt";
	const string latest_trans = "latest_transaction.txt";
	size_t counter;
public:
	TransactionHandle(){
		transaction_data = new map<size_t, GrayImage*>();
		pause_flag = false;
		read_counter();
	}
	virtual ~TransactionHandle() {
		delete transaction_data;
	}
	bool is_pause(){
		return pause_flag;
	}
	size_t read_counter(){
		ifstream file;
		try{
			file.open(latest_trans, std::ifstream::in);
			string line;
			if(file.good()){
				getline(file, line);
				counter = stoull(line);
			}
			file.close();
		}catch(exception& e){
			cerr << e.what() << endl;
			file.close();
		}
		return 0;
	}
	size_t get_counter(){
		counter++;
		ofstream file;
		try{
			file.open(latest_trans, ofstream::out);
			if(file.good()){
				file << to_string(counter);
			}
			file.close();
		}catch(exception& e){
			cerr << e.what() << endl;
			if(file.is_open())
				file.close();
		}
		return counter;
	}
	GrayImage* get_transaction_data(size_t& tx){
		map<size_t, GrayImage*>::iterator it;
		it = transaction_data->find(tx);
		if (it != transaction_data->end()) {
			return it->second;
		}
		return NULL;
	}
	void add_transaction_data(size_t& tx, GrayImage* view){
		map<size_t, GrayImage*>::iterator it;
		it = transaction_data->find(tx);
		if (it == transaction_data->end()) {
			transaction_data->insert(pair<size_t, GrayImage*>(tx, view));
		}
	}
	void remove_transaction_data(size_t& tx){
		map<size_t, GrayImage*>::iterator it;
		it = transaction_data->find(tx);
		if (it != transaction_data->end()) {
			transaction_data->erase(tx);
		}
	}
	// return any transaction exists
	bool is_exist_transaction(){
		return !transaction_data->empty();
	}
	void append_log(string log){
		log_buffer += log + "\n";
	}
	void pause(){
//		 lock_guard<std::mutex> lk(m);
		 pause_flag = true;
	}
	void resume(){
//		lock_guard<std::mutex> lk(m);
		pause_flag = false;
//		cv.notify_one();
	}
	void wait(){
		while(pause_flag){
		   unique_lock<std::mutex> lk(m);
		   cv.wait(lk);
		   lk.unlock();
		}
	}
	/*
	 * type of log
	 * */
	string get_init_log(size_t& tx) {
		string log = "<Start " + to_string(tx) + ">";
		return log;
	}
	string get_convert_log(size_t& tx, string& name) {
		string log = "<Convert " + name + ">";
		return log;
	}
	string get_convert_log(size_t& tx, string& name, int width,  int height) {
		string log = "<Convert " + to_string(tx) + "," + name + "," + to_string(width) + "," + to_string(height) + ">";
		return log;
	}
	string get_commit_log(size_t& tx) {
		string log = "<Commit " + to_string(tx) + ">";
		return log;
	}
	string get_abort_log(size_t& tx) {
		string log = "<Abort " + to_string(tx) + ">";
		return log;
	}
	string get_action_log(size_t& tx, int pix, string o_val, int n_val) {
		string log = "<" + to_string(tx) + "," + to_string(pix) + "," + o_val + ","
				+ to_string(n_val) + ">";
		return log;
	}
	/*
	 * check whether met checkpoint or not during flushing log
	 * */
	void check_before_flush(string log){
//		if(is_pause()){
//			append_log(log);
//		}else{
			// not checkpoint flush log in every action
		flush_log(log);
//		}
	}
	/*
	 * flush log immediately
	 * */
	void flush_log(string log){
		ofstream file;
		file.open(log_files, ios_base::app);
		file << log << "\n";
		file.close();
	}
	/*
	 * A. <T,X,v,w> (T: Transaction ID, X: Pixel ID, v: Old value of pixel X, w: New value of pixel X)
		B. <Start T>
		C. <Commit T>
		D. <Abort T>
	 * */

	/*
	 * periodically
	 * flush all log records to disk
	 * flush all buffers to disk
	 * write checkpoint record on disk (log)
	 * pause current transaction util flush all logs to disk
	 * resume after finish flushing
	 * */
	void flush_log(bool force=false){
		if((!force && !log_buffer.empty()) || force){
			ofstream file;
			file.open(log_files, ios_base::app);
//			utils::print(log_buffer)
			file << log_buffer;
			file.close();
			log_buffer = "";
		}
	}
	void create_snapshot(size_t tx, GrayImage* dst){
		vector<int> data = dst->data;
//		utils::print("size of snapshot create" + to_string(data.size()));
		if(!data.empty()){
			string filename = "snapshot/" + to_string(tx);
			ofstream fout(filename, ios::out | ios::binary);
			const char* pointer = reinterpret_cast<const char*>(&data[0]);
			fout.write(pointer, data.size() * sizeof(data[0]));
			fout.close();
		}
	}
	vector<int> get_snapshot(size_t& tx){
		vector<int> data;
		ifstream snp("snapshot/" + to_string(tx), ios::binary);
		if(snp.good()){
			// Stop eating new lines in binary mode!!!
			snp.unsetf(std::ios::skipws);
			// get its size:
			std::streampos filesize;
			snp.seekg(0, std::ios::end);
			filesize = snp.tellg();
			snp.seekg(0, std::ios::beg);
			size_t count = filesize / sizeof(int);
			// std::vector<int> vec(count);
			data.resize(count);
//			utils::print("size of snapshot" + to_string(data.size()));
			snp.read(reinterpret_cast<char*>(&data[0]), filesize);
		}
		return data;
	}
	/*
	 * flush snapshot at starting point
	 * now: skip flush dirty buffer pages to make it simple
	 * */
	void make_snapshots(){
		map<size_t, GrayImage*>::iterator it;
		for(it = transaction_data->begin(); it != transaction_data->end(); it++){
			create_snapshot(it->first, it->second);
		}
	}

	/*
	 * start jobs from latest snapshot
	 * */
	string get_line(vector<char>& cc){
		string line = "";
		for(unsigned int j = 0; j < cc.size(); j++){
			line += cc[j];
		}
		return line;
	}
	/*
	 * find the last checkpoint
	 * */

	int get_latest_checkpoint(){
		vector<char> cc;
		char c;
		string line = "";
		ifstream log(log_files, ifstream::ate);
		int i = 0;
		bool is_met_ck = false;
		while(c != '\n'){
			i++;
			log.seekg(-i, ios::end);
			log.get(c);
			if(c == '\n'){
				line = get_line(cc);
				cc.clear();
				is_met_ck = utils::start_with(line, "<SCKPT");
				if(is_met_ck)
					break;
				c = '\0';
			}else if(log.tellg() == -1){
				break;
			}else{
				cc.insert(cc.begin(), c);
			}
		}
		if(!is_met_ck){
			log.clear();
			log.seekg(0, ios::beg);
			getline(log, line);
			if(utils::start_with(line, "<SCKPT")){
				// checkpoint is at the first line
				i = 0;
			}else{
				// done have checkpoint in log
				i = -1;
			}
		}
		log.close();
		// else checkpoint is found in the reverse loop
		return i;
	}
	GrayImage* find_transaction_start_point(int& checkpoint, size_t& tx){
		GrayImage* g = new GrayImage();
		ifstream log(log_files, ifstream::ate);
		char c;
		vector<char> cc;
		string line = "";
		int i = checkpoint;
		bool is_met = false;
		vector<string> tm;
		size_t ctx = 0;
		while(c != '\n'){
			i++;
			log.seekg(-i, ios::end);
			log.get(c);
			if(c == '\n'){
				line = get_line(cc);
				cc.clear();
				is_met = utils::start_with(line, "<Convert");
				if(is_met){
					line.erase(0, 8);
					line.erase(line.length()-1, 1);
					tm = utils::split(line, ",");
					if(tm.size() == 4){
						ctx = stoull(tm.at(0));
						if(ctx == tx){
							g->name = tm.at(1);
							g->width = stoi(tm.at(2));
							g->height = stoi(tm.at(3));
							break;
						}
					}
				}
				c = '\0';
			}else if(log.tellg() == -1){
				break;
			}else{
				cc.insert(cc.begin(), c);
			}
		}
		log.close();
		return g;
	}
	void get_log_transactions(set<size_t>& transactions, set<size_t>& commit_trans, set<size_t>& aborts, string& line){
		utils::removeStringFromString(line, ">");
		size_t tx;
		vector<string> tm;
		if(utils::start_with(line, "<Commit")){
			//found committed trans
			line.erase(0, 8);
			tx = stoull(line);
			if(commit_trans.find(tx) == commit_trans.end()){
				commit_trans.insert(tx);
			}
		}else if(utils::start_with(line, "<Start")){
			//start the transaction
			line.erase(0, 7);
			tx = stoull(line);
			if(transactions.find(tx) == transactions.end()){
				transactions.insert(tx);
			}
		}else if(utils::start_with(line, "<Abort")){
			line.erase(0, 7);
			tx = stoull(line);
			if(aborts.find(tx) == aborts.end()){
				aborts.insert(tx);
			}
		}else if(!line.empty() && isdigit(line.at(1))){
			line.erase(0, 1);
			tm = utils::split(line, ",");
			if(!tm.empty()){
				tx = stoull(tm.at(0));
			}
			if(transactions.find(tx) == transactions.end()){
				transactions.insert(tx);
			}
		}
	}
	void recover(string& gray_prefix){
		int ckpt = get_latest_checkpoint();
		string line;
		set<size_t> transactions;
		set<size_t> commit_trans;
		set<size_t> aborts;
		if(ckpt == -1 || ckpt == 0){
			// start read lock from the beginning of log file
			ifstream log(log_files);
			while(getline(log, line)){
				get_log_transactions(transactions, commit_trans, aborts, line);
			}
			log.close();
		}else{
			char c;
			vector<char> cc;
			ifstream log(log_files, ifstream::ate);
			// iterate from the ending file => checkpoint
			int i = 0;
			while(c != '\n'){
				i++;
				if(i == ckpt)
					break;
				log.seekg(-i, ios::end);
				log.get(c);
				if(c == '\n'){
					line = get_line(cc);
					get_log_transactions(transactions, commit_trans, aborts, line);
					cc.clear();
					c = '\0';
				}else if(log.tellg() == -1){
					break;
				}else{
					cc.insert(cc.begin(), c);
				}
			}
			log.close();
		}
		set<size_t> not_commit;
		set<size_t> not_commit_abort;
		if(!transactions.empty() && !commit_trans.empty()){
			set_difference(transactions.begin(), transactions.end(), commit_trans.begin(), commit_trans.end(), inserter(not_commit, not_commit.end()));
			// find transactions neither commit nor abort => unfinished
			set_difference(not_commit.begin(), not_commit.end(), aborts.begin(), aborts.end(), inserter(not_commit_abort, not_commit_abort.end()));
			// undo not_commit
			undo(not_commit_abort);
			// redo commit
			redo(ckpt, commit_trans, gray_prefix);
		}else if(!transactions.empty()){
			// undo all
			utils::print("No remaining committed transactions to redo");
			undo(transactions);
		}else if(!commit_trans.empty()){
			// redo all
			utils::print("No remaining uncommitted transactions to undo");
			redo(ckpt, transactions, gray_prefix);
		}

	}
	void redo(int& ckpt, set<size_t>& commit, string gray_prefix){
		utils::print("start redo");
		utils::print("total committed transactions are: " + to_string(commit.size()));
		string line;
		size_t tx = 0;
		set<size_t>::iterator iter;
		map<size_t, GrayImage*> snapshots;
		for(iter = commit.begin(); iter != commit.end(); ++iter){
			tx = *iter;
			GrayImage* g = new GrayImage();
			g->data = get_snapshot(tx);
			snapshots.insert(pair<size_t, GrayImage*>(tx, g));
		}
		if(ckpt == -1 || ckpt == 0){
			// start read lock from the beginning of log file
			ifstream log(log_files);
			while(getline(log, line)){
				redo_action(line, snapshots);
			}
			log.close();
		}else{
			char c;
			vector<char> cc;
			ifstream log(log_files, ifstream::ate);
			int i = 0;
			while(c != '\n'){
				i++;
				if(i == ckpt)
					break;
				log.seekg(-i, ios::end);
				log.get(c);
				if(c == '\n'){
					line = get_line(cc);
					redo_action(line, snapshots);
					cc.clear();
					c = '\0';
				}else if(log.tellg() == -1){
					break;
				}else{
					cc.insert(cc.begin(), c);
				}
			}
			log.close();
		}
		//restore from snapshot
		GrayImage* dt;
		map<size_t, GrayImage*>::iterator sn;
		// reconvert
		for(sn = snapshots.begin(); sn != snapshots.end(); ++sn){
			dt = sn->second;
			tx = sn->first;
			if(dt->name.empty() || dt->height == 0 || dt->width == 0){
				//transaction start before checkpoint
				dt = find_transaction_start_point(ckpt, tx);
				dt->data = sn->second->data;
			}
			if(dt->height != 0 && dt->width != 0){
				gray8_image_t img_out(dt->width, dt->height);
				fill_pixels(view(img_out), 0);
				to_image(gray_prefix, dt->name, dt->data, view(img_out));
			}
		}
	}

	void redo_action(string& line, map<size_t, GrayImage*>& snapshots){
		vector<string> tm;
		size_t tx = 0;
		set<size_t>::iterator iter;
		map<size_t, GrayImage*>::iterator sn;
		size_t px = 0;
		int new_v = 0;
		GrayImage* dt;
		if(!line.empty() && isdigit(line.at(1))){
			line.erase(0, 1);
			line.erase(line.length() - 1, 1);
			tm = utils::split(line, ",");
			if(tm.size() == 4){
				tx = stoull(tm.at(0));
				sn = snapshots.find(tx);
				if(sn != snapshots.end()){
					px = stoi(tm.at(1));
					new_v = stoi(tm.at(3));
					dt = sn->second;
					if(dt->data.size() >= px){
						dt->data[px] = new_v;
					}
				}
			}
		}else if(utils::start_with(line, "<Convert")){
			line.erase(0, 8);
			line.erase(line.length() - 1, 1);
			tm = utils::split(line, ",");
			if(tm.size() == 4){
				tx = stoull(tm.at(0));
				sn = snapshots.find(tx);
				if(sn != snapshots.end()){
					dt = sn->second;
					dt->name = tm.at(1);
					dt->width = stoi(tm.at(2));
					dt->height = stoi(tm.at(3));
				}
			}
		}
	}

	void undo(set<size_t>& uncommit){
		utils::print("start undo");
		utils::print("total uncommitted transactions are: " + to_string(uncommit.size()));
		if(uncommit.size()){
			// write abort log for transactions
			size_t tx = 0;
			set<size_t>::iterator iter;
			for(iter = uncommit.begin(); iter != uncommit.end(); ++iter){
				tx = *iter;
				flush_log(get_abort_log(tx));
			}
		}
	}
	template<typename SrcView, typename View>
	void to_image(string& prefix, string& name, vector<int> data, const SrcView& src, const View& dst) {
		int height = dst.height();
		int width = dst.width();
		// do actions
		unsigned short int tmp = 0;
		for (int y = 0; y < height; ++y) {
			typename SrcView::x_iterator src_it = src.row_begin(y);
			typename View::x_iterator imo = dst.row_begin(y);
			for (int x = 0; x < width; ++x) {
				tmp = 0;
				for (int c = 0; c < num_channels<SrcView>::value; ++c) {
					tmp += (int) src_it[x][c];
				}
				imo[x] = tmp / 3;
			}
		}
		png_write_view(prefix + "/" + name, dst);
	}
	template<typename View>
	void to_image(string& prefix, string& name, vector<int> data, const View& dst) {
		int height = dst.height();
		int width = dst.width();
		size_t idx = 0;
		// do actions
		for (int y = 0; y < height; ++y) {
			typename View::x_iterator imo = dst.row_begin(y);
			for (int x = 0; x < width; ++x) {
				idx = width * y + x;
				if(idx < data.size()){
					imo[x] = data.at(idx);
				}
			}
		}
		png_write_view(prefix + "/" + name, dst);
	}
};



#endif /* TRANSACTIONHANDLE_H_ */
