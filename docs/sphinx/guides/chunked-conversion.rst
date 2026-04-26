Chunked conversion
==================

Never split a legacy byte stream into chunks and decode each chunk with
one-shot ``decode`` independently. Some encodings use multiple bytes per
character, and a chunk boundary can appear in the middle of a sequence.

Incorrect shape
---------------

.. code-block:: cpp

   // Avoid this for arbitrary chunks.
   auto a = polycpp::iconv_lite::decode(firstChunk, "gbk");
   auto b = polycpp::iconv_lite::decode(secondChunk, "gbk");

Each call creates and flushes a new decoder. If ``firstChunk`` ends with a lead
byte, the decoder has no chance to combine it with ``secondChunk``.

Correct shape with Decoder
--------------------------

.. code-block:: cpp

   auto decoder = polycpp::iconv_lite::getDecoder("gbk");
   std::string text;
   text += decoder.write(firstChunk);
   text += decoder.write(secondChunk);
   text += decoder.end();

Keep one decoder for the lifetime of the byte stream. Call ``end`` exactly once
when no more bytes are expected, so incomplete trailing sequences can be handled
according to iconv-lite behavior.

Correct shape with DecodeStream
-------------------------------

.. code-block:: cpp

   auto decoder = polycpp::iconv_lite::decodeStream("utf8");
   decoder.write(firstChunk);
   decoder.write(secondChunk);
   decoder.end();

``DecodeStream`` uses the same stateful decoder internally and fits stream
pipelines. The readable side emits UTF-8 buffers because polycpp streams carry
bytes.

Chunk-sensitive encodings
-------------------------

Use stateful conversion for arbitrary chunks of:

- UTF-8, UTF-16, and UTF-32
- UTF-7 and UTF-7-IMAP
- Shift_JIS, GBK, GB18030, Big5, EUC-JP, and EUC-KR
- base64 and other codecs with buffered output

Single-byte encodings such as latin1 or Windows-1251 are less sensitive to
split character sequences, but using one stateful converter still keeps code
uniform and makes later encoding changes safer.

For the ``base64`` label, follow iconv-lite's direction: encoding consumes
base64 text and produces bytes, while decoding consumes bytes and produces
base64 text.
