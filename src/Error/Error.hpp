#ifndef ERROR_HPP
#define ERROR_HPP
#include <string>
using namespace std;
class Error {
private:
	int code;
	string ID;
	string message;
public:
	Error();
	Error(string id, int c, string m);
	Error(const Error& rhs);
	string GetError();
	int GetSeverity();
};

#endif