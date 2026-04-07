# dmalloc API manual

> [!NOTE]
> This manual structure was heavily inspired by the [man pages](https://man7.org/linux/man-pages/index.html). For this reason, some paragraphs link directly to them when referencing some of their original terms.

## Index
1. [**darenainit()**](#darenainit)
    - [Synopsis](#synopsis)
    - [Description](#description)
    - [Return value](#return-value)
    - [Errors](#errors)
    - [Attributes](#attributes)
    - [Standars](#standards)
    - [Notes](#notes)
    - [Examples](#examples)

## **darenainit()**

### Synopsis
&nbsp; &nbsp; &nbsp; &nbsp; <b>#include "dmalloc.h"</b>
 
&nbsp; &nbsp; &nbsp; &nbsp; <b>int \*darenainit(void\*</b> <i>backing_memory</i><b>, size_t</b><i>capacity</i><b>);</b>

### Description

**darenainit**() initializes the memory region pointed to by *backing_memory* and of size *capacity* to an arena. If *backing_memory* is NULL, the function just returns 0.

### Return value

On success, **darenainit**() returns 0. On failure, it returns -1 and sets [errno](https://man7.org/linux/man-pages/man3/errno.3.html).

### Errors

**ENOMEM** Out of memory. \
**darenainit**() failed because *capacity* was not big enough for a properly initialized arena.

### Attributes

For an explanation of the terms used in this section, see [attributes(7)](https://man7.org/linux/man-pages/man7/attributes.7.html).

| **Interface** | **Attribute** | **Value** |
|:--------------|:--------------|:----------|
| **dmalloc**() | Thread safety | MT-Unsafe |

### Standards

None.

### Notes

Trying to initialize an arena more than once causes undefined behaviour, especially dangerous in multi-threaded applications. However, note that after initialization, any arena is safe to be used in a multi-threaded context with all of the other allocator functions.

### Examples

None.

## **dmalloc()**

### Synopsis
&nbsp; &nbsp; &nbsp; &nbsp; <b>#include "dmalloc.h"</b>
 
&nbsp; &nbsp; &nbsp; &nbsp; <b>void \*dmalloc(size_t</b> <i>size</i><b>, void \*</b><i>arena</i><b>);</b>
### Description

**dmalloc**() allocates *size* bytes of memory in the arena pointed to by *arena*. If *arena* is NULL, the memory is allocated on the heap. The memory is not initialized.  If *size* is 0, then **dmalloc**() returns a unique pointer value that can later be successfully passed to **dfree**().

### Return value

On success, **dmalloc**() returns a pointer to the allocated memory. On failure, it returns NULL and sets [errno](https://man7.org/linux/man-pages/man3/errno.3.html).

### Errors

**ENOMEM** Out of memory. \
**dmalloc**() may have failed for a handful of reasons, depending on where the memory was supposed to be allocated. It possibly failed because an arena allocation in an arena with not enough available space was attempted. Otherwise, for the case of a heap allocation, the function failed because the heap is full or because an error occurred while the heap was being initialized.

### Attributes

For an explanation of the terms used in this section, see [attributes(7)](https://man7.org/linux/man-pages/man7/attributes.7.html).

| **Interface** | **Attribute** | **Value** |
|:--------------|:--------------|:----------|
| **dmalloc**() | Thread safety | MT-Safe   |

### Standards

**dmalloc**() is supposed to behave like **malloc**() when dealing with the heap. For this reason, it follows the [POSIX standard](https://man7.org/linux/man-pages/man3/malloc.3p.html) on top of the [ISO/IEC 9899:2024 §7.24.3.6](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3220.pdf#subsubsection.7.24.3.6) C standard.

[### Notes]: # 

### Examples
The following code shows a minimal complete example of how **dmalloc**() can be used to allocate memory in an arena on the stack.
```
#include "dmalloc.h"
#include <stdio.h> // For perror() and printf().
#include <stdlib.h> // For exit() and the EXIT_* macros.

#define CAPACITY 1024

int main()
{
    char arena[CAPACITY];

    // The arena must be initialized first.
    if(darenainit(arena, CAPACITY) < 0)
    {
        perror("darenainit");
        exit(EXIT_FAILURE);
    }

    void *p = dmalloc(512, arena);

    if(p == NULL)
    {
        perror("dmalloc");
        exit(EXIT_FAILURE);
    }

    printf("Allocated memory starts at address: %p.\n", p);    

    dfree(p, arena);

    /*  It is not mandatory to destroy an arena at the end of your application,
        especially in a single-threaded one. Yet, it is good practice along with
        memory deallocation. */
    if(darenadestroy(arena) < 0)
    {
        perror("darenadestroy");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
```