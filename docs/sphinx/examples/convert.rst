Windows-1251 round trip
=======================

``examples/convert.cpp`` demonstrates the most common shape of the API:
encode UTF-8 text into a legacy byte buffer, then decode the buffer back.

.. code-block:: cpp

   #include <iostream>

   #include <polycpp/iconv_lite/iconv_lite.hpp>

   int main() {
       auto bytes = polycpp::iconv_lite::encode("Привет", "win1251");
       std::cout << bytes.toString("hex") << "\n";
       std::cout << polycpp::iconv_lite::decode(bytes, "cp1251") << "\n";
       return 0;
   }

Expected output:

.. code-block:: text

   cff0e8e2e5f2
   Привет
