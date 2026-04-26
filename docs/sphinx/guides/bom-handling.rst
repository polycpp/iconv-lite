BOM handling
============

BOM-aware decoders strip an initial decoded U+FEFF by default:

.. code-block:: cpp

   auto text = polycpp::iconv_lite::decode(bytes, "utf8");

Disable stripping when the BOM is meaningful data:

.. code-block:: cpp

   polycpp::iconv_lite::DecodeOptions options;
   options.stripBOM = false;
   auto text = polycpp::iconv_lite::decode(bytes, "utf8", options);

Observe actual BOM removal with ``onBOMStripped``:

.. code-block:: cpp

   bool removed = false;
   polycpp::iconv_lite::DecodeOptions options;
   options.onBOMStripped = [&] { removed = true; };
   auto text = polycpp::iconv_lite::decode(bytes, "utf8", options);

For encoding, ``utf16`` and ``utf32`` auto encoders add a BOM by default.
Other BOM-aware encodings add one only when requested:

.. code-block:: cpp

   polycpp::iconv_lite::EncodeOptions options;
   options.addBOM = true;
   auto bytes = polycpp::iconv_lite::encode("hello", "utf8", options);

Set ``addBOM=false`` to suppress the default BOM on ``utf16`` or ``utf32``.
