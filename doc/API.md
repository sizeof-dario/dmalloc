# dmalloc API manual

> This manual structure was heavily inspired by the [man pages](https://man7.org/linux/man-pages/index.html). For this reason, some paragraphs link directly to them when referencing some of their original terms.

## Index

## **dmalloc()**

### Synopsis

    <b>#include "dmalloc.h"</b>

    <b>void *dmalloc(size_t</b> <i>size</i><b>, void *</b><i>arena</i><b>);</b>

### Description

`dmalloc()` allocates `size` bytes of msemory in the arena pointed to by `arena`. If `arena` is `NULL`, the memory is allocated on the heap. The memory is not initialized.  If `size` is `0`, then `dmalloc()` returns a unique pointer value that can later be successfully passed to `dfree()`.

### Return value

On success, `dmalloc()` returns a pointer to the allocated memory. On failure, it returns `NULL` and sets `errno`.

### Errors

### Attributes

For an explanation of the terms used in this section, see [attributes(7)](https://man7.org/linux/man-pages/man7/attributes.7.html).

| **Interface** | **Attribute** | **Value** |
|:--------------|:--------------|:----------|
| `dmalloc()`   | Thread safety | MT-Safe   |

### Standards

### Notes

### Examples