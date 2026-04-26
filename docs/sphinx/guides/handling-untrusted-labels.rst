Handling untrusted labels
=========================

Encoding labels often come from outside the program: HTTP headers, import job
configuration, database metadata, or user selection. Treat those labels as
untrusted input.

Validate before converting
--------------------------

.. code-block:: cpp

   if (!polycpp::iconv_lite::encodingExists(label)) {
       throw polycpp::TypeError("unsupported encoding");
   }

   auto text = polycpp::iconv_lite::decode(bytes, label);

``encode`` and ``decode`` also throw ``polycpp::TypeError`` for unsupported
labels, so explicit validation is mainly useful when you want to produce a
domain-specific error message before conversion starts.

Inspect resolution for logs and tests
-------------------------------------

.. code-block:: cpp

   auto info = polycpp::iconv_lite::inspectEncoding("ISO_8859-5:1988");
   // info.canonical == "iso88595"
   // info.converter == "iso88595"

Use ``inspectEncoding`` for diagnostics, not as a security boundary. The
supported/unsupported decision should use ``encodingExists`` or the conversion
exception.

Choose a policy for missing labels
----------------------------------

When metadata omits the charset, pick an application-level default rather than
guessing blindly. Common choices are:

- default to UTF-8 for new protocols you control
- require the import job to specify a label
- use a per-partner or per-file-format default
- reject the payload and ask for explicit metadata

Replacement characters
----------------------

Invalid byte sequences and unrepresentable characters are converted with the
library's current replacement defaults. For tests or compatibility with a
specific integration, the defaults can be changed for future converters:

.. code-block:: cpp

   auto old = polycpp::iconv_lite::defaultCharUnicode();
   polycpp::iconv_lite::setDefaultCharUnicode("?");
   auto text = polycpp::iconv_lite::decode(bytes, "gbk");
   polycpp::iconv_lite::setDefaultCharUnicode(old);

Keep replacement changes tightly scoped. Existing encoder and decoder objects
keep their own state; create new converters after changing defaults.
