#include <iostream>
#include <string>

#include <polycpp/iconv_lite/iconv_lite.hpp>

int main() {
    namespace iconv = polycpp::iconv_lite;

    auto decoder = iconv::getDecoder("gbk");
    std::string text;
    text += decoder.write(iconv::Buffer::from({0x61, 0x81}));
    text += decoder.write(iconv::Buffer::from({0x40, 0x62}));
    text += decoder.end();
    std::cout << text << "\n";

    auto encoder = iconv::getEncoder("base64");
    auto binary = iconv::Buffer::concat({
        encoder.write("aGV"),
        encoder.write("sbG8="),
        encoder.end(),
    });
    std::cout << binary.toString("utf8") << "\n";
    return 0;
}
