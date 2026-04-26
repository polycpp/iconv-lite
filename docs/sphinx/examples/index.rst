Examples
========

The examples are small standalone programs, but each one maps to a real class
of integration problem. They all build against ``polycpp::iconv_lite`` without
private headers.

.. list-table::
   :header-rows: 1

   * - Example
     - Use it when...
   * - :doc:`convert`
     - You have a complete payload and need one encode/decode operation.
   * - :doc:`bom`
     - You read or write UTF payloads where an initial BOM matters.
   * - :doc:`stateful`
     - You receive chunks and need to preserve incomplete byte sequences.
   * - :doc:`stream`
     - You want a polycpp transform stream around the stateful converters.
   * - :doc:`labels`
     - Encoding names come from config, headers, metadata, or user input.

.. toctree::
   :maxdepth: 1

   convert
   bom
   stateful
   stream
   labels

Running an example
------------------

From the repository root:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_ICONV_LITE_BUILD_EXAMPLES=ON
   cmake --build build --target polycpp_iconv_lite_example_convert
   ./build/examples/convert

All examples are emitted under ``build/examples/`` when
``POLYCPP_ICONV_LITE_BUILD_EXAMPLES`` is enabled.
