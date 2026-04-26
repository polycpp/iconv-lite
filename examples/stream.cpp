#include <iostream>

#include <polycpp/iconv_lite/iconv_lite.hpp>

int main() {
    namespace iconv = polycpp::iconv_lite;

    auto encoder = iconv::encodeStream("windows-1251");
    encoder.write("абв");
    encoder.write("где");
    encoder.end();
    std::cout << encoder.read().toString("hex") << "\n";

    auto decoder = iconv::decodeStream("gbk");
    decoder.write(iconv::Buffer::from({0x61, 0x81}));
    decoder.write(iconv::Buffer::from({0x40, 0x61}));
    decoder.end();
    std::cout << decoder.read().toString("utf8") << "\n";
    return 0;
}
