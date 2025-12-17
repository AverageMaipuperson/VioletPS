#include <string>
#include <cstring>

class VCustomEncrypt {
public:
static std::string ce_encode(std::string str);
static std::string ce_decode(const std::string& shifted_hex_input);
};
