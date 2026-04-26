Batch conversion
================

``examples/convert.cpp`` demonstrates the most common API shape: convert one
complete UTF-8 string to external bytes, or one complete external byte payload
back to UTF-8.

Use this shape when the full payload is already in memory:

- request or response body already buffered by your application
- text file loaded as bytes
- message queue payload
- database export/import field
- test fixture containing known legacy bytes

.. literalinclude:: ../../../examples/convert.cpp
   :language: cpp

The example uses Windows-1251 because it makes the byte-level result obvious:
``Привет`` maps to six single-byte Cyrillic characters. The same API shape
works for Shift_JIS, GBK, GB18030, Big5, EUC-JP, latin1, UTF-16, UTF-32, and
the other supported labels.

Expected output:

.. code-block:: text

   cff0e8e2e5f2
   Привет

The returned ``Buffer`` can be passed to other polycpp APIs that operate on
bytes, written to a file, sent over a socket, or inspected with
``toString("hex")`` in tests.
