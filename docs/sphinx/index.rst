iconv-lite
==========

C++ companion port of npm ``iconv-lite`` for polycpp applications that need
batch conversion between UTF-8 text and legacy byte encodings.

.. code-block:: cpp

   #include <polycpp/iconv_lite/iconv_lite.hpp>

.. grid:: 2

   .. grid-item-card:: Batch conversion
      :margin: 1

      Encode UTF-8 text into ``polycpp::buffer::Buffer`` and decode buffers
      back into UTF-8 strings.

   .. grid-item-card:: Polycpp-native bytes
      :margin: 1

      The public byte boundary is ``polycpp::buffer::Buffer``. No local byte
      container or safer-buffer clone is introduced.

   .. grid-item-card:: Upstream table codecs
      :margin: 1

      Legacy encoding tables are generated from the upstream iconv-lite
      JavaScript data files and used by local C++ codecs.

   .. grid-item-card:: Clear scope
      :margin: 1

      Stream APIs and dynamic codec registry internals are deferred and
      documented as unsupported in this port version.

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
