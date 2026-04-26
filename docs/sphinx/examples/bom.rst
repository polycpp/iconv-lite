BOM-sensitive payloads
======================

``examples/bom.cpp`` shows both sides of BOM handling: observing a stripped
BOM while decoding and requesting a BOM while encoding.

Use this when your integration touches text formats where a leading BOM may
appear or be required: CSV exports, XML files, office-system imports, UTF-16
files, or partner feeds that use BOMs for encoding detection.

.. literalinclude:: ../../../examples/bom.cpp
   :language: cpp

Expected output:

.. code-block:: text

   hello
   BOM stripped
   efbbbf68656c6c6f

The callback is an observer: it runs only when a leading BOM is actually
removed. Set ``DecodeOptions::stripBOM`` to ``false`` if the U+FEFF character
must remain part of the decoded text.
