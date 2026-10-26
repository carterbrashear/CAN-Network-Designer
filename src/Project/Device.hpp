#include<stdint.h>
#include<string>
#include<vector>

using namespace std;
class Device{
public:
    int64_t id;
    string name;
    float pos_x, pos_y;
    string comment;
    vector<int64_t> connected_bus_ids;
    //stored color value. must be stored as a hexidecimal (int= 0x...)
    int hexColor;
};