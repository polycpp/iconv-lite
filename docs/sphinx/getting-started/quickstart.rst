Quickstart
==========

This program encodes UTF-8 text to Windows-1251 bytes and decodes those bytes
back to UTF-8.

Full example
------------

.. code-block:: cpp

   #include <iostream>

   #include <polycpp/iconv_lite/iconv_lite.hpp>

   int main() {
       auto bytes = polycpp::iconv_lite::encode("Привет", "win1251");
       std::cout << bytes.toString("hex") << "\n";
       std::cout << polycpp::iconv_lite::decode(bytes, "cp1251") << "\n";
   }

Build and run it with the CMake wiring from :doc:`installation`.

Expected output
---------------

.. code-block:: text

   cff0e8e2e5f2
   Привет

What happened
-------------

``encode`` accepted UTF-8 text and returned a ``polycpp::buffer::Buffer``.
``decode`` accepted that buffer and returned a UTF-8 ``std::string``. The
``win1251`` and ``cp1251`` labels resolve to the same Windows-1251 codec.

Next steps
----------

- :doc:`../tutorials/converting-text` for more encoding examples.
- :doc:`../guides/bom-handling` for BOM options.
- :doc:`../api/index` for the full public API.
