Choosing a conversion workflow
==============================

The library has three conversion shapes. Pick the narrowest one that matches
how your application receives data.

Batch conversion
----------------

Use ``encode`` and ``decode`` when the full input is already available.

.. code-block:: cpp

   auto legacyBytes = polycpp::iconv_lite::encode("こんにちは", "shiftjis");
   auto decodedText = polycpp::iconv_lite::decode(legacyBytes, "shiftjis");

Batch conversion is the simplest choice for complete request bodies, files
loaded into memory, message payloads, database values, and test fixtures. It
creates a converter, processes the input, flushes any pending state, and returns
the complete result.

Stateful conversion
-------------------

Use ``getEncoder`` and ``getDecoder`` when data arrives in chunks but you are
not using a polycpp stream pipeline.

.. code-block:: cpp

   auto decoder = polycpp::iconv_lite::getDecoder("gbk");
   auto first = decoder.write(polycpp::iconv_lite::Buffer::from({0x81}));
   auto second = decoder.write(polycpp::iconv_lite::Buffer::from({0x40}));
   auto tail = decoder.end();

Stateful conversion preserves incomplete byte sequences across ``write`` calls.
That matters for DBCS encodings, UTF-8, UTF-16, UTF-32, UTF-7, base64, and
other encodings whose output may depend on bytes from a later chunk.

Transform streams
-----------------

Use ``encodeStream`` and ``decodeStream`` when conversion should be a
``polycpp::stream::Transform`` stage.

.. code-block:: cpp

   auto decoder = polycpp::iconv_lite::decodeStream("gb18030");
   decoder.write(chunk);
   decoder.end();

Polycpp streams are byte-oriented, so ``DecodeStream`` emits UTF-8 buffers on
the readable side. Its ``collect`` helper exposes decoded text directly when you
want an end callback.

Decision guide
--------------

.. list-table::
   :header-rows: 1

   * - Situation
     - API
   * - Complete input, complete output
     - ``encode`` / ``decode``
   * - Custom loop over byte or text chunks
     - ``getEncoder`` / ``getDecoder``
   * - Existing polycpp stream pipeline
     - ``encodeStream`` / ``decodeStream``
   * - Need metadata about a label
     - ``inspectEncoding``
   * - Need to reject unsupported user input
     - ``encodingExists`` before conversion
