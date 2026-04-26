#include <iostream>
#include <string>

#include <polycpp/iconv_lite/iconv_lite.hpp>

int main() {
    namespace iconv = polycpp::iconv_lite;

    const std::string label_from_config = "ISO_8859-5:1988";
    if (!iconv::encodingExists(label_from_config)) {
        std::cerr << "unsupported encoding: " << label_from_config << "\n";
        return 1;
    }

    auto info = iconv::inspectEncoding(label_from_config);
    std::cout << info.canonical << "\n";
    std::cout << info.converter << "\n";

    auto bytes = iconv::encode("Привет", label_from_config);
    std::cout << bytes.toString("hex") << "\n";
    std::cout << iconv::decode(bytes, "iso-8859-5") << "\n";
    return 0;
}
