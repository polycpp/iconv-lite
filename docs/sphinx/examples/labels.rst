Validating labels
=================

``examples/labels.cpp`` shows how to accept an encoding label from metadata,
validate it, inspect how it resolves, and then use it for conversion.

Use this pattern when the label comes from:

- a configuration file
- a database column
- a content-type charset parameter
- an import job setting
- user-provided metadata

.. literalinclude:: ../../../examples/labels.cpp
   :language: cpp

Expected output:

.. code-block:: text

   iso88595
   iso88595
   bfe0d8d2d5e2
   Привет

``encodingExists`` is the cheap guard for untrusted labels. ``inspectEncoding``
is for diagnostics, logs, and tests where you want to see the canonical label
and resolved converter.
