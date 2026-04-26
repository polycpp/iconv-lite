Core API
========

Namespace
---------

.. doxygennamespace:: polycpp::iconv_lite
   :desc-only:

Options and diagnostics
-----------------------

.. doxygenstruct:: polycpp::iconv_lite::EncodeOptions
   :members:

.. doxygenstruct:: polycpp::iconv_lite::DecodeOptions
   :members:

.. doxygenstruct:: polycpp::iconv_lite::EncodingInfo
   :members:

.. doxygenstruct:: polycpp::iconv_lite::Codec
   :members:

.. doxygenclass:: polycpp::iconv_lite::Encoder
   :members:

.. doxygenclass:: polycpp::iconv_lite::Decoder
   :members:

.. doxygenstruct:: polycpp::iconv_lite::IconvStreamOptions
   :members:

.. doxygenclass:: polycpp::iconv_lite::EncodeStream
   :members:

.. doxygenclass:: polycpp::iconv_lite::DecodeStream
   :members:

Functions
---------

.. doxygenfunction:: polycpp::iconv_lite::canonicalizeEncoding

.. doxygenfunction:: polycpp::iconv_lite::encodingExists

.. doxygenfunction:: polycpp::iconv_lite::inspectEncoding

.. doxygenfunction:: polycpp::iconv_lite::getCodec

.. doxygenfunction:: polycpp::iconv_lite::getEncoder

.. doxygenfunction:: polycpp::iconv_lite::getDecoder

.. doxygenfunction:: polycpp::iconv_lite::defaultCharUnicode

.. doxygenfunction:: polycpp::iconv_lite::setDefaultCharUnicode

.. doxygenfunction:: polycpp::iconv_lite::defaultCharSingleByte

.. doxygenfunction:: polycpp::iconv_lite::setDefaultCharSingleByte

.. doxygenfunction:: polycpp::iconv_lite::encode

.. doxygenfunction:: polycpp::iconv_lite::decode

.. doxygenfunction:: polycpp::iconv_lite::supportsStreams

.. doxygenfunction:: polycpp::iconv_lite::enableStreamingAPI

.. doxygenfunction:: polycpp::iconv_lite::encodeStream

.. doxygenfunction:: polycpp::iconv_lite::decodeStream
