#include <iostream>
#include <string>
#include <time.h>
#include <boost/algorithm/string.hpp>

#include "client.h"
#include "utils.h"

using namespace std;


void test(int port, string &host){
	Client* client = new Client(port);
	clock_t t1 = clock();
	int t = 0;
	size_t i = 0;
	string mes;
	while(true){
		t = (static_cast<int>(((float) clock() - (float) t1) / CLOCKS_PER_SEC)) / 3;
		if(t == 1){
			t1 = clock();
			mes.clear();
			mes.append("message ").append(to_string(i));
			client->openSocket(host);
			client->sendMessage(mes);
			client->closeSocket();
			i++;
		}
	}
}

int main(int argc, char *argv[]){
	int port = 8888;
	if(argc > 1){
		port = atoi(argv[1]);
	}
	int auto_action = 0;
	if(argc > 2){
		auto_action = atoi(argv[2]);
	}
	string host = "127.0.0.1";
	if(argc > 3){
		host = argv[3];
	}
	int interval = 5;
	if(argc > 4){
		interval = atoi(argv[4]);
	}
	Client* client = new Client(port);
	clock_t t1 = clock();
	int t = 0;
	string mes;
	if(auto_action){
		while(true){
			t = (static_cast<int>(((float) clock() - (float) t1) / CLOCKS_PER_SEC)) / interval;
			if(t == 1){
				mes = "convert 201109260002 (1).png";
				t1 = clock();
				utils::print("=> convert " + mes);
				client->openSocket(host);
				client->sendMessage(mes);
				client->closeSocket();
			}
		}
	}else{
		utils::print("Type 'convert file_name.png' to convert file_name to gray image");
		utils::print("Type 'ls' to list all filenames ");
//		utils::print("Type 'recover' to fix failure");
		string mes = "";
		char input[1024];
		while(true){
			cout << "Enter your command: ";
			cin.getline(input,sizeof(input));
			mes = string(input);
			boost::algorithm::to_lower(mes);
//			cout << mes << endl;
			if(mes.length() && (utils::start_with(mes, "convert")
				|| utils::start_with(mes, "ls"))){
				client->openSocket("127.0.0.1");
				client->sendMessage(mes);
				client->closeSocket();
			}else{
				utils::print("Command error");
			}
		}
	}

	return 0;
}

