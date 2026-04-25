Installation
============

``polycpp-iconv-lite`` targets C++20 and requires ICU because the port uses
ICU converter APIs for legacy character sets.

Requirements
------------

- CMake 3.20 or newer
- A C++20 compiler
- ICU development libraries
- A polycpp checkout, or network access so CMake can fetch polycpp

CMake FetchContent
------------------

Add the library to your ``CMakeLists.txt``:

.. code-block:: cmake

   include(FetchContent)

   FetchContent_Declare(
       polycpp_iconv_lite
       GIT_REPOSITORY https://github.com/polycpp/iconv-lite.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_iconv_lite)

   add_executable(my_app main.cpp)
   target_link_libraries(my_app PRIVATE polycpp::iconv_lite)

The companion configures polycpp with ``POLYCPP_UNICODE=icu`` and links ICU's
``uc`` library. Pin ``GIT_TAG`` to a release commit for reproducible builds.

Using a local polycpp checkout
------------------------------

.. code-block:: bash

   cmake -B build -G Ninja \
       -DCMAKE_BUILD_TYPE=Debug \
       -DPOLYCPP_SOURCE_DIR=/path/to/polycpp

When ``POLYCPP_SOURCE_DIR`` is not set, CMake fetches polycpp from GitHub.

Build options
-------------

``POLYCPP_ICONV_LITE_BUILD_TESTS``
    Build the GoogleTest suite. Defaults to ``ON`` for standalone builds and
    ``OFF`` when consumed through FetchContent.

``POLYCPP_ICONV_LITE_BUILD_EXAMPLES``
    Build examples under ``examples/``. Defaults to ``OFF``.

``POLYCPP_SOURCE_DIR``
    Path to a local polycpp checkout.

Verifying the install
---------------------

.. code-block:: bash

   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_ICONV_LITE_BUILD_EXAMPLES=ON
   cmake --build build -j$(nproc)
   ctest --test-dir build --output-on-failure
   ./build/examples/convert

The example prints Windows-1251 bytes for ``Привет`` and then decodes them
back to UTF-8 text.
