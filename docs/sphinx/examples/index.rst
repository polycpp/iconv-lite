Examples
========

Self-contained programs exercising the public API. Each example builds against
``polycpp::iconv_lite`` without private headers.

.. toctree::
   :maxdepth: 1

   convert
   bom
   stream

Running an example
------------------

From the repository root:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_ICONV_LITE_BUILD_EXAMPLES=ON
   cmake --build build --target polycpp_iconv_lite_example_convert
   ./build/examples/convert
