Stateful conversion
===================

``examples/stateful.cpp`` uses ``getDecoder`` and ``getEncoder`` directly.
These objects are useful when you are not using a full polycpp stream but still
receive or emit data in chunks.

Use this shape for:

- a network protocol parser that receives partial frames
- a file reader that processes fixed-size blocks
- a decompressor that emits arbitrary byte chunks
- a parser that needs precise control over when conversion state is flushed

.. literalinclude:: ../../../examples/stateful.cpp
   :language: cpp

Expected output:

.. code-block:: text

   a丂b
   hello

The first GBK byte chunk ends with an incomplete lead byte. The decoder keeps it
until the next chunk arrives, then emits the complete character. The base64
encoder follows iconv-lite's convention for that label: it consumes base64 text,
emits bytes, and buffers input until a complete four-character group is
available.
