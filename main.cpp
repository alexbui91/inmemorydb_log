#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <algorithm>
#include <ctime>
#include <map>

#include "string.h"
#include "utils.h"
#include "server.h"
#include "TransactionHandle.h"
#include "ls.h"

using namespace std;

string prefix = "dataset";
string gray = "gray";
bool debug = false;

const int checkpoint_interval = 10;


TransactionHandle* handle = new TransactionHandle();

/*
 * conversion function
 * */
template<typename SrcView>
void to_gray(size_t& tx, const SrcView& src, GrayImage* dst) {
	int height = src.height();
	int width = src.width();
	dst->data.resize(height * width);
	handle->add_transaction_data(tx, dst);
	unsigned short int tmp = 0;
	string o_val = "";
	int px = 0;
	int pid = 0;
	utils::print("start transaction " + to_string(tx) + " convert " + dst->name);
	for (int y = 0; y < height; ++y) {
		typename SrcView::x_iterator src_it = src.row_begin(y);
		for (int x = 0; x < width; ++x) {
			pid = y * width + x;
			tmp = 0;
			px = 0;
			o_val = "";
			for (int c = 0; c < num_channels<SrcView>::value; ++c) {
				px = (int) src_it[x][c];
				tmp += px;
				// a|b|c
				o_val += to_string(px);
				if (c < 2)
					o_val += "|";
			}
			tmp /= 3;
			dst->data[pid] = tmp;
			handle->check_before_flush(handle->get_action_log(tx, pid, o_val, tmp));
		}
	}
}

void start_transaction(string& name, Server* server, int& inc) {
	rgb8_image_t img;
	size_t tx = handle->get_counter();
	handle->check_before_flush(handle->get_init_log(tx));
	string res = "";
	try{
		string url = prefix + "/" + name;
		png_read_image(url, img);
		handle->check_before_flush(handle->get_convert_log(tx, name, img.width(), img.height()));
		gray8_image_t img_out(img.dimensions());
		fill_pixels(view(img_out), 0);
		GrayImage* dst = new GrayImage();
		dst->name = name;
		dst->height = img.height();
		dst->width = img.width();
		handle->add_transaction_data(tx, dst);
		to_gray(tx, const_view(img), dst);
		// commit
		handle->check_before_flush(handle->get_commit_log(tx));
		res = "convert successfully";
		server->sendMessage(inc, res.c_str());
		server->closeConnection(inc);
//		to_image(name, dst->data, const_view(img), view(img_out));
		handle->to_image(gray, name, dst->data, view(img_out));
		utils::print("save gray image " + name);
		handle->remove_transaction_data(tx);
	}catch(exception& e){
		cerr << "Exception: " << e.what() << endl;
		res = "File not exists";
		server->sendMessage(inc, res.c_str());
		server->closeConnection(inc);
		handle->remove_transaction_data(tx);
	}


}

void process_incomming_client(Server* server, int &inc, string& mes) {
	string res = "";
	vector<string> mes_parse = utils::split(mes, " ");
	if (!mes.empty()) {
		string com = mes_parse.at(0);
		if (com == "convert") {
			mes_parse.erase(mes_parse.begin());
			mes = utils::merge(mes_parse, " ");
			thread t1 = thread(start_transaction, ref(mes), ref(server),
					ref(inc));
			t1.join();
		} else if (com == "ls") {
			res = dataset::ls;
			server->sendMessage(inc, res.c_str());
			server->closeConnection(inc);
		} else if (com == "recover") {
			handle->recover(gray);
			res = "Finish recovery";
			server->sendMessage(inc, res.c_str());
			server->closeConnection(inc);
		}
	}

}

void checkpoint(float& interval) {
	int t = 0;
	float t_ = 0.0;
	clock_t t1 = clock();
	while(true){
		t_ = ((float) clock() - (float) t1) / CLOCKS_PER_SEC;
		t = static_cast<int>((t_ / interval));
		if(t == 1){
			t1 = clock();
			if(handle->is_exist_transaction()){
				//	utils::print("checkpoint");
				// pause all current transaction threads
				handle->pause();
				size_t ck_time = utils::get_timestamp();
				string ck_log = "<SCKPT " + to_string(ck_time) + ">";
				handle->flush_log(ck_log);
				// make snapshots
				handle->make_snapshots();
				// flush remaining log during blocking phase
//				handle->flush_log();
				ck_log = "<ECKPT " + to_string(ck_time) + ">";
				handle->flush_log(ck_log);
				// resume processing
				handle->resume();
			}
		}
	}
}

void auto_die() {
	int t = 0;
	float t_ = 0.0;
	clock_t t1 = clock();
	bool random_die = false;
	while(true){
		t_ = ((float) clock() - (float) t1) / CLOCKS_PER_SEC;
		t = static_cast<int>((t_ / 5.0));
		if(t == 1){
			t1 = clock();
			if(handle->is_exist_transaction()){
				random_die = (rand() % 10) == 0;
				if(random_die){
					exit(EXIT_SUCCESS);
					break;
				}
			}
		}
//		if(t == 1){
//			t1 = clock();
//			if(handle->is_exist_transaction()){
//				random_die = (rand() % 2) != 0;
//				if(random_die){
//					exit(EXIT_SUCCESS);
//					break;
//				}
//			}
//		}
	}
}

void start_server(int port) {
	utils::print("Start server");
	Server* server = new Server(port);
	if (!debug)
		server->initServer();
	int inc = 0;
	try {
		if (debug) {
			string name = "convert 201109260002 (1).png";
			process_incomming_client(server, inc, name);
		} else {
			string mes = "";
			char buffer[256];
			int activity = 0;
			while (true) {
				//clear the socket set
				server->clear_set();
				server->add_connection(server->get_master_socket());
				server->check_valid_connection();
				activity = server->wait_connection();
				if ((activity < 0) && (errno != EINTR)) {
					printf("select error");
				}
				if (server->is_valid_master()) {
					inc = server->openConnection(buffer);
					string bu(buffer);
					mes = bu;
					process_incomming_client(server, inc, mes);
				}
			}
		}
	} catch (exception& e) {
		server->closeServer();
		cerr << "Exception in server runtime: " << e.what() << endl;
	}
}

int main(int argc, char *argv[]) {
	srand(time(NULL));
	int port = 8888;
	if (argc > 1) {
		port = atoi(argv[1]);
	}
	// checkpoint interval calculate by ms
	float ckpt_interval = 0.5;
	if (argc > 2) {
		ckpt_interval = atof(argv[2]);
	}
	if (argc > 3) {
		prefix = argv[3];
	}
	if (argc > 4) {
		gray = argv[4];
	}
	if (argc > 5) {
		debug = (bool) atoi(argv[5]);
	}
	// recover or not
	bool r_fl = false;
	if (argc > 6) {
		r_fl = (bool) atoi(argv[6]);
	}
	// using auto die
	bool utd = false;
	if (argc > 7) {
		utd = (bool) atoi(argv[7]);
	}
	if(!r_fl){
		thread cp = thread(checkpoint, ref(ckpt_interval));
		thread server = thread(start_server, ref(port));
		if(utd){
			thread at = thread(auto_die);
			at.join();
		}
		cp.join();
		server.join();
	}else{
		handle->recover(gray);
	}
	return 0;
}
