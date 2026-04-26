iconv-lite
==========

C++ companion port of npm ``iconv-lite`` for polycpp applications that need to
accept, produce, or relay text in legacy byte encodings while keeping their C++
domain model in UTF-8.

.. code-block:: cpp

   #include <polycpp/iconv_lite/iconv_lite.hpp>

Use this library when a protocol, file, database export, mail body, terminal
integration, or partner system says "this text is Windows-1251", "Shift_JIS",
``gb18030``, ``latin1``, ``utf16le``, or another iconv-lite-style label, but
your application code wants normal UTF-8 ``std::string`` values.

Common workflows
----------------

.. grid:: 2

   .. grid-item-card:: Decode an incoming payload
      :margin: 1

      Convert bytes from files, HTTP bodies, message queues, archives, or
      database exports into UTF-8 text with ``decode``.

   .. grid-item-card:: Encode for a legacy system
      :margin: 1

      Convert UTF-8 application text to a required legacy byte encoding with
      ``encode`` before writing or sending it.

   .. grid-item-card:: Process chunked data
      :margin: 1

      Use ``getDecoder`` / ``getEncoder`` or transform streams when a byte
      sequence may be split across network or file chunks.

   .. grid-item-card:: Normalize labels and BOMs
      :margin: 1

      Accept user-provided labels safely, inspect how they resolve, and control
      BOM stripping or prepending for UTF encodings.

Where to go next
----------------

.. list-table::
   :header-rows: 1

   * - If you need to...
     - Read...
   * - Add the dependency to a CMake project
     - :doc:`getting-started/installation`
   * - Convert a single payload
     - :doc:`getting-started/quickstart` and :doc:`examples/convert`
   * - Choose between batch, stateful, and stream APIs
     - :doc:`guides/conversion-workflows`
   * - Handle split byte sequences or stream chunks
     - :doc:`guides/chunked-conversion`, :doc:`examples/stateful`, and :doc:`examples/stream`
   * - Accept encoding names from config, headers, or users
     - :doc:`guides/encoding-labels` and :doc:`guides/handling-untrusted-labels`
   * - Work with UTF BOMs
     - :doc:`guides/bom-handling` and :doc:`examples/bom`

Getting started
---------------

.. code-block:: cmake

   FetchContent_Declare(
       polycpp_iconv_lite
       GIT_REPOSITORY https://github.com/polycpp/iconv-lite.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_iconv_lite)
   target_link_libraries(my_app PRIVATE polycpp::iconv_lite)

:doc:`Installation <getting-started/installation>` | :doc:`Quickstart <getting-started/quickstart>` | :doc:`Examples <examples/index>` | :doc:`How-to guides <guides/index>` | :doc:`API reference <api/index>`

.. toctree::
   :hidden:
   :caption: Getting started

   getting-started/installation
   getting-started/quickstart

.. toctree::
   :hidden:
   :caption: How-to guides

   guides/index

.. toctree::
   :hidden:
   :caption: API reference

   api/index

.. toctree::
   :hidden:
   :caption: Examples

   examples/index
