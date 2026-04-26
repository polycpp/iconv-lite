Streaming conversion
====================

``examples/stream.cpp`` uses the transform stream wrappers. They share the same
stateful encoder and decoder implementations as ``getEncoder`` and
``getDecoder``, but expose them through ``polycpp::stream::Transform``.

Use streams when the surrounding code already uses polycpp stream plumbing, or
when conversion should be one stage in a larger byte-processing pipeline.

.. literalinclude:: ../../../examples/stream.cpp
   :language: cpp

Expected output:

.. code-block:: text

   e0e1e2e3e4e5
   a丂a

For one-shot payloads, prefer ``encode`` and ``decode``. For custom chunk loops,
prefer ``getEncoder`` and ``getDecoder``. Use ``encodeStream`` and
``decodeStream`` when a transform stream fits the rest of the application.
