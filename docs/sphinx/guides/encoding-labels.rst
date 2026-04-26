Encoding labels
===============

Labels follow iconv-lite canonicalization: lowercase the input, remove a
trailing ``:YYYY`` suffix, and strip non-alphanumeric characters. This lets
applications accept common spellings such as ``win1251``, ``cp1251``,
``windows-1251``, or historical labels like ``ISO_8859-5:1988``.

Canonicalization
----------------

.. code-block:: cpp

   auto canonical = polycpp::iconv_lite::canonicalizeEncoding("ISO_8859-5:1988");
   // canonical == "iso88595"

Checking support
----------------

Common aliases such as ``win1251``, ``1251``, ``cp1251``, ``sjis``, ``gbk``,
``gb18030``, ``big5hkscs``, ``latin1``, ``binary``, ``utf16le``, and
``utf32be`` are handled through explicit aliases and generated upstream table entries.

Use ``encodingExists`` for labels that come from configuration or metadata:

.. code-block:: cpp

   if (!polycpp::iconv_lite::encodingExists(label)) {
       throw polycpp::TypeError("unsupported encoding");
   }

Inspecting resolution
---------------------

Use ``inspectEncoding`` when debugging resolution:

.. code-block:: cpp

   auto info = polycpp::iconv_lite::inspectEncoding("win1251");
   // info.canonical == "win1251"
   // info.converter == "windows-1251"

``requested`` preserves the original input label, ``canonical`` shows the
normalized label, and ``converter`` names the resolved generated table or
internal codec. This is helpful in logs and tests when multiple aliases map to
the same converter.

Where labels usually come from
------------------------------

.. list-table::
   :header-rows: 1

   * - Source
     - Recommended handling
   * - Application configuration
     - Validate at startup with ``encodingExists``.
   * - Import job parameter
     - Validate before reading the payload.
   * - Content-Type charset
     - Validate and fall back only if your protocol defines a default.
   * - Database metadata
     - Validate before converting each batch or connection.
   * - User input
     - Validate and return a clear unsupported-encoding error.
