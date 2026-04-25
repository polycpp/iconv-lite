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

.. doxygenfunction:: polycpp::iconv_lite::canonicalize_encoding

.. doxygenfunction:: polycpp::iconv_lite::encoding_exists

.. doxygenfunction:: polycpp::iconv_lite::inspect_encoding

.. doxygenfunction:: polycpp::iconv_lite::get_codec

.. doxygenfunction:: polycpp::iconv_lite::get_encoder

.. doxygenfunction:: polycpp::iconv_lite::get_decoder

.. doxygenfunction:: polycpp::iconv_lite::default_char_unicode

.. doxygenfunction:: polycpp::iconv_lite::set_default_char_unicode

.. doxygenfunction:: polycpp::iconv_lite::default_char_single_byte

.. doxygenfunction:: polycpp::iconv_lite::set_default_char_single_byte

.. doxygenfunction:: polycpp::iconv_lite::encode

.. doxygenfunction:: polycpp::iconv_lite::decode

.. doxygenfunction:: polycpp::iconv_lite::supports_streams

.. doxygenfunction:: polycpp::iconv_lite::enable_streaming_api

.. doxygenfunction:: polycpp::iconv_lite::encode_stream

.. doxygenfunction:: polycpp::iconv_lite::decode_stream
