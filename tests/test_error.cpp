#include <cassert>
#include <string>
#include "Error/Error.hpp"
int main() {
    // Test normal error creation
    Error test_error("DB", 404, "Database connection failed");

    // Verify GetError() returns the correctly formatted error
    assert(test_error.GetError() == "DB-404: Database connection failed" && "GetError() should return ID-code: message.");

    // Test error code 0
    Error test_success("DB", 0, "Operation successful");

    // Verify GetError() returns only the message when code is 0 meaning no error
    assert(test_success.GetError() == "Operation successful" && "GetError() should return only the message when code is 0.");

    // Test severity 9
    Error test_severity("DB", 910, "Crital error loading database");

    // Verify GetSeverity() returns the correct value
    assert(test_severity.GetSeverity() == 9 && "GetSeverity should only return the left most digit of the error code");

    // Success
    return 0;
}