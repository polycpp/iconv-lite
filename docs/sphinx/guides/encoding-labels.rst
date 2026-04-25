Encoding labels
===============

Labels follow iconv-lite canonicalization: lowercase the input, remove a
trailing ``:YYYY`` suffix, and strip non-alphanumeric characters.

.. code-block:: cpp

   auto canonical = polycpp::iconv_lite::canonicalize_encoding("ISO_8859-5:1988");
   // canonical == "iso88595"

Common aliases such as ``win1251``, ``1251``, ``cp1251``, ``sjis``, ``gbk``,
``gb18030``, ``big5hkscs``, ``latin1``, ``binary``, ``utf16le``, and
``utf32be`` are handled explicitly before ICU probing.

Use ``inspect_encoding`` when debugging resolution:

.. code-block:: cpp

   auto info = polycpp::iconv_lite::inspect_encoding("win1251");
   // info.canonical == "win1251"
   // info.converter == "windows-1251"
