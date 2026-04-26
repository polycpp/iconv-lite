BOM handling
============

The byte order mark is a leading marker used by some UTF formats. In practice,
applications often see it in CSV exports, XML files, UTF-16 text files, and
partner feeds. The library follows iconv-lite behavior for stripping and
prepending BOMs.

Decoding with the default policy
--------------------------------

BOM-aware decoders strip an initial decoded U+FEFF by default:

.. code-block:: cpp

   auto text = polycpp::iconv_lite::decode(bytes, "utf8");

Only an initial BOM is affected. U+FEFF elsewhere in the decoded text is normal
content.

Keeping a BOM as content
------------------------

Disable stripping when the BOM is meaningful data:

.. code-block:: cpp

   polycpp::iconv_lite::DecodeOptions options;
   options.stripBOM = false;
   auto text = polycpp::iconv_lite::decode(bytes, "utf8", options);

When ``stripBOM`` is ``false``, ``onBOMStripped`` is not called because no BOM
is removed.

Observing BOM removal
---------------------

Observe actual BOM removal with ``onBOMStripped``:

.. code-block:: cpp

   bool removed = false;
   polycpp::iconv_lite::DecodeOptions options;
   options.onBOMStripped = [&] { removed = true; };
   auto text = polycpp::iconv_lite::decode(bytes, "utf8", options);

The callback is useful for telemetry and compatibility checks: it tells you
that the payload included an initial BOM without requiring the application to
keep U+FEFF in the returned string.

Encoding with a BOM
-------------------

For encoding, ``utf16`` and ``utf32`` auto encoders add a BOM by default.
Other BOM-aware encodings add one only when requested:

.. code-block:: cpp

   polycpp::iconv_lite::EncodeOptions options;
   options.addBOM = true;
   auto bytes = polycpp::iconv_lite::encode("hello", "utf8", options);

Set ``addBOM=false`` to suppress the default BOM on ``utf16`` or ``utf32``.

Choosing a policy
-----------------

.. list-table::
   :header-rows: 1

   * - Situation
     - Suggested policy
   * - Reading modern UTF-8 text
     - Keep the default ``stripBOM=true``.
   * - Preserving exact text content for a diff or editor
     - Use ``stripBOM=false``.
   * - Exporting for a system that requires UTF-8 BOM
     - Set ``EncodeOptions::addBOM=true``.
   * - Exporting UTF-16 or UTF-32 with auto endianness
     - Use the default BOM unless the target explicitly forbids it.
