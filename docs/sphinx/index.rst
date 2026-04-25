iconv-lite
==========

**C++ companion port of iconv-lite**

Generated planning documentation for the iconv-lite companion library. Replace placeholder user pages as APIs, examples, and tests become real.

.. code-block:: cpp

   #include <polycpp/iconv_lite/iconv_lite.hpp>

.. grid:: 2

   .. grid-item-card:: Drop-in familiarity
      :margin: 1

      Keep the C++ API close to the npm package where that improves migration, and record deliberate C++ adaptations in docs/divergences.md.

   .. grid-item-card:: C++20 native
      :margin: 1

      Header-only where possible, zero-overhead abstractions, ``constexpr``
      and ``std::string_view`` throughout.

   .. grid-item-card:: Tested
      :margin: 1

      The test plan starts from upstream tests and fixtures, then adds C++ integration and regression coverage before release.

   .. grid-item-card:: Plays well with polycpp
      :margin: 1

      Uses the same JSON value, error, and typed-event types as the rest of
      the polycpp ecosystem - no impedance mismatch.

Getting started
---------------

.. code-block:: bash

   # With FetchContent (recommended)
   FetchContent_Declare(
       polycpp_iconv_lite
       GIT_REPOSITORY https://github.com/polycpp/iconv-lite.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_iconv_lite)
   target_link_libraries(my_app PRIVATE polycpp::iconv_lite)

:doc:`Installation <getting-started/installation>` | :doc:`Quickstart <getting-started/quickstart>` | :doc:`Tutorials <tutorials/index>` | :doc:`API reference <api/index>`

.. toctree::
   :hidden:
   :caption: Getting started

   getting-started/installation
   getting-started/quickstart

.. toctree::
   :hidden:
   :caption: Tutorials

   tutorials/index

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
