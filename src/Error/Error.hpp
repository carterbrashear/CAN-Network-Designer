#ifndef ERROR_HPP
#define ERROR_HPP
#include <string>
class Error {
private:
	int code;
	std::string ID;
	std::string message;
public:
	Error();
	Error(std::string id, int c, std::string m);
	Error(const Error& rhs);
	std::string GetError();
	int GetSeverity();
	void setCode(int c);
	std::string getCodeID();
	void setID(std::string s);
	void setMessage(std::string m);
	std::string getMessage();
};

#endif