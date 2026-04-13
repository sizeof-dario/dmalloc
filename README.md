# dmalloc ![Last commit](https://img.shields.io/github/last-commit/sizeof-dario/dmalloc) [![CMake (Ubuntu)](https://github.com/sizeof-dario/dmalloc/actions/workflows/cmake-ubuntu.yml/badge.svg)](https://github.com/sizeof-dario/dmalloc/actions/workflows/cmake-ubuntu.yml) [![codecov](https://codecov.io/github/sizeof-dario/dmalloc/graph/badge.svg?token=US9NJVUJ50)](https://codecov.io/github/sizeof-dario/dmalloc)

A didactic memory allocator for UNIX systems. 

## How to get the library

### 1. Clone the repository
Open a terminal in the folder you want to clone the repository into, then paste the following command:
```
git clone https://github.com/sizeof-dario/dmalloc.git
```
### 2. Configure the project
dmalloc uses CMake for its build system. You can use CMake to eventually build dmalloc as either a static or a dynamic library. To do so, first make sure to be in project root folder:
```
cd dmalloc
```
Then, if you want to obtain the static version, use:
```
cmake -B build --preset static
```
Otherwise, for the dynamic one:
```
cmake -B build --preset shared
```
Regardless of the chosen option, you'll now have a **build** folder in **dmalloc**.

### 3. Build the library

Without leaving the **dmalloc** folder, paste the following command:
```
cmake --build build
```
After doing so, **build** will either contain **libdmalloc.a** (static library) or **libdmalloc.so** (dynamic library).

### 4. Use the library
If you want to integrate the library into a project, follow this paragraph steps from 1 to 3. If you want to substitute dmalloc to the libc default allocator in preexisting apps, just follow step 3.

The **dmalloc** folder contains the master header **include/dmalloc.h**. That's the (only) header you need to include in your project to let the compiler know the allocator functions exist. A possible series of steps for integration is the following:
1. Add `#include "dmalloc.h"` in the appropriate project file(s).
2. (If you're using GCC) Add `-I<dir_header> -L<dir_library>` and `-ldmalloc` to your compilation command. **<dir_header>** is the path to **dmalloc.h** and **<dir_library>** is the path to either **libdmalloc.a** or **libdmalloc.so**, depending on your configuration choices. Moreover, if you're linking the static library, it's important to add `-ldmalloc` before any other library.
3. (If you're using **libdmalloc.so**) When launching your application, prefix `LD_PRELOAD=<dir_library>` to the app path.

## How to use the library

You can find a detailed explanation of how the library works and the necessary code documentation at https://sizeof-dario.github.io/dmalloc/.