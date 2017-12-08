/*
 * utils.h
 *
 *  Created on: Oct 26, 2017
 *      Author: alex
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <algorithm>
#include <cstring>
#include <iostream>
//#include <sys/time.h>
#include <chrono>

using namespace std;

using bigint = long long int;

namespace utils {

inline void removeStringFromString(string &str, string c) {
	char *c_c = new char[c.size()+1];
	std::strcpy(c_c, c.c_str());
	for (unsigned int i = 0; i < strlen(c_c); ++i) {
		str.erase(remove(str.begin(), str.end(), c_c[i]), str.end());
	}
}
inline void removeCharsFromString(string &str, char* charsToRemove) {
	for (unsigned int i = 0; i < strlen(charsToRemove); ++i) {
		str.erase(remove(str.begin(), str.end(), charsToRemove[i]), str.end());
	}
}
/*
 * https://stackoverflow.com/questions/27677964/print-current-system-time-in-nanoseconds-using-c-chrono
 * http://www.cplusplus.com/reference/chrono/time_point/time_since_epoch/
 * */

inline size_t get_timestamp() {
//	struct timeval tp;
//	gettimeofday(&tp, NULL);
//	long ms = tp.tv_sec * 1000 + tp.tv_usec;
	chrono::system_clock::time_point tp = chrono::system_clock::now();
	chrono::system_clock::duration dtn = tp.time_since_epoch();
	auto nanoseconds = chrono::duration_cast<chrono::nanoseconds>(dtn);
	size_t ms = nanoseconds.count();
	return ms;
}

inline bool start_with(string &s, string pat){
	return s.find(pat) == 0;
}
template <typename T>
inline void print(T mes){
	cout << mes << endl;
}


inline vector<string> split(string& input, string delim){
	vector<string> output;
	size_t last = 0;
	size_t next = 0;
	string temp_col;
	while ((next = input.find(delim, last)) != string::npos) {
		temp_col = input.substr(last, next - last);
		last = next + delim.length();
		output.push_back(temp_col);
	}
	temp_col = input.substr(last);
	output.push_back(temp_col);
	return output;
}

inline string merge(vector<string>& input, string delim=""){
	string output = "";
	unsigned int end = input.size() - 1;
	for(unsigned int i = 0; i < input.size(); i++){
		output += input.at(i);
		if(!delim.empty() && i < end){
			output += delim;
		}
	}
	return output;
}

template <class ReverseIterator>
typename ReverseIterator::iterator_type make_forward(ReverseIterator rit)
{
    return --(rit.base()); // move result of .base() back by one.
    // alternatively
    // return (++rit).base() ;
    // or
    // return (rit+1).base().
}

}
;
#endif /* UTILS_H_ */
