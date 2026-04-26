Quickstart
==========

This program shows the core model: application text is UTF-8, external payloads
are ``polycpp::buffer::Buffer`` bytes, and the encoding label tells iconv-lite
how to cross that boundary.

Full example
------------

.. literalinclude:: ../../../examples/convert.cpp
   :language: cpp

Build and run the bundled example with the CMake wiring from
:doc:`installation`:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_ICONV_LITE_BUILD_EXAMPLES=ON
   cmake --build build --target polycpp_iconv_lite_example_convert
   ./build/examples/convert

This mirrors a common "send text to a legacy system, then decode a response or
fixture" flow.

Expected output
---------------

.. code-block:: text

   cff0e8e2e5f2
   Привет

What happened
-------------

``encode`` accepted UTF-8 text and returned a ``polycpp::buffer::Buffer``.
``decode`` accepted that buffer and returned a UTF-8 ``std::string``. The
``win1251`` and ``cp1251`` labels resolve to the same Windows-1251 codec.

Use ``encode`` when your C++ code owns UTF-8 text and the outside world expects
legacy bytes. Use ``decode`` when the outside world gives you bytes and your
application should work with UTF-8 text.

Next steps
----------

- :doc:`../examples/index` for complete programs mapped to common use cases.
- :doc:`../guides/conversion-workflows` for choosing the right API shape.
- :doc:`../guides/bom-handling` for BOM options.
- :doc:`../api/index` for the full public API.
