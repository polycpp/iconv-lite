BOM options
===========

``examples/bom.cpp`` shows both sides of BOM handling: observing a stripped
BOM while decoding and requesting a BOM while encoding.

.. code-block:: cpp

   #include <iostream>

   #include <polycpp/iconv_lite/iconv_lite.hpp>

   int main() {
       namespace iconv = polycpp::iconv_lite;

       auto bytes = iconv::Buffer::concat({
           iconv::Buffer::from({0xEF, 0xBB, 0xBF}),
           iconv::Buffer::from("hello"),
       });

       bool stripped = false;
       iconv::DecodeOptions decodeOptions;
       decodeOptions.onBOMStripped = [&] {
           stripped = true;
       };

       std::cout << iconv::decode(bytes, "utf8", decodeOptions) << "\n";
       std::cout << (stripped ? "BOM stripped" : "BOM kept") << "\n";

       iconv::EncodeOptions encodeOptions;
       encodeOptions.addBOM = true;
       std::cout << iconv::encode("hello", "utf8", encodeOptions).toString("hex") << "\n";
       return 0;
   }

Expected output:

.. code-block:: text

   hello
   BOM stripped
   efbbbf68656c6c6f
