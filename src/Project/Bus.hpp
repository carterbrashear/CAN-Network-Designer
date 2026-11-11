#include<string>
#include<vector>
#include <cstdint>

using namespace std;
class Bus{
public:
    int64_t id;
    string name;
    int32_t nominal_bit_rate;
    bool is_fd;
    uint32_t color;
    string comment;

    //public function to calculate load on bus. NOTE: Requires message class.
    //double calculateBusLoad(const vector<Message>& messages);
};