#include "Error.hpp"
Error::Error() {
	ID = "UK";
	code = 500;
	message = "An unknown error occured";
}
Error::Error(string id, int c, string m) {
	ID = id;
	code = c;
	message = m;
}
Error::Error(const Error& rhs) {
	ID = rhs.ID;
	code = rhs.code;
	message = rhs.message;
}
string Error::GetError() {
	//if code = 0 no error occured so no need to include ID stuff just print out the success message
	if (code == 0) {
		return message;
	}
	return ID + "-" + to_string(code) + ": " + message;
}
int Error::GetSeverity() {
	//returns the first digit of the cod which determines the severity ranging from 1 not severe to 9 very problematic
	return code / 100;
}