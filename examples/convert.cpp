#include <iostream>

#include <polycpp/iconv_lite/iconv_lite.hpp>

int main() {
    auto bytes = polycpp::iconv_lite::encode("Привет", "win1251");
    std::cout << bytes.toString("hex") << "\n";
    std::cout << polycpp::iconv_lite::decode(bytes, "cp1251") << "\n";
    return 0;
}
