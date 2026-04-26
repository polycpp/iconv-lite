Converting text
===============

Start with UTF-8 text and choose an iconv-lite-style label:

.. code-block:: cpp

   auto bytes = polycpp::iconv_lite::encode("こんにちは", "shiftjis");

The returned value is a ``polycpp::buffer::Buffer``. You can inspect or pass it
to other polycpp APIs that accept buffers:

.. code-block:: cpp

   auto hex = bytes.toString("hex"); // 82b182f182c982bf82cd

Decode uses the reverse direction:

.. code-block:: cpp

   auto text = polycpp::iconv_lite::decode(bytes, "sjis");

Aliases are canonicalized before lookup, so ``shiftjis`` and ``sjis`` both
resolve to the Shift_JIS codec.

Checking support
----------------

Use ``encodingExists`` before accepting user-provided labels:

.. code-block:: cpp

   if (!polycpp::iconv_lite::encodingExists(label)) {
       throw std::runtime_error("unsupported encoding");
   }

Unsupported labels throw ``polycpp::TypeError`` from ``encode`` and ``decode``.
