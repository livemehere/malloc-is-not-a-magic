[English](#malloc-is-not-a-magic) | [한국어](./README.ko.md)

# Malloc is not a Magic

Most languages allocate memory from the OS through `new`, and in many cases they come with a built-in `GC (Garbage Collector)`, so manual deallocation is not something the developer usually has to think about.

C/C++, on the other hand, allow direct control over memory through `malloc` and `free`, or `new` and `delete`. That freedom in manual memory management is one of the main reasons C/C++ can hold such a strong performance advantage over many other languages. After actually testing things like virtual function overhead, memory layout control, and cache efficiency, it became clear that small optimizations stack up, and the accumulated gain can easily become 2.x or more.

The key point was that the cost of communicating with the OS, the system call itself, is the most expensive part. Most performance-oriented memory techniques are really attempts to reduce that cost. From there came the more fundamental question: how does a program actually borrow memory from the OS in the first place?

This is a light attempt to organize that question.

`main.c` contains a very small allocator.

- It requests memory pages from the OS with `mmap`.
- It stores metadata in a `Block` header.
- It reuses free blocks when possible.
- It splits large free blocks into the size that is actually needed.
- It merges adjacent free blocks again during `free`.
- It prints the heap state so the allocation flow can be seen directly.

## Metadata Management

To manage the borrowed memory, a `Block` structure is defined as a doubly linked list.
The actual usable memory returned to the caller is the region right after the `Block` header.

```c
typedef struct Block {
  size_t size;
  bool is_free;
  struct Block *prev;
  struct Block *next;
} Block;
```

## Allocation Flow

`my_malloc(size)` follows this flow.

1. Traverse the list and look for a reusable free block first.
2. If a block is large enough, split it and reuse it.
3. If there is no suitable block, request one or more pages from the OS with `mmap`.
4. Link the new memory region into the list, split it, and return the payload area.

## Free Flow

`my_free(ptr)` follows this flow.

1. Mark the block as free.
2. If the next block is free and physically adjacent in memory, merge it.
3. If the previous block is free and physically adjacent in memory, merge again.

> The important condition for merging is that the actual memory addresses must be adjacent. That is also part of why heap access is more expensive than stack access. Heap memory is requested in page-sized units, usually 4 KB here, and its physical placement is not guaranteed to be contiguous in the same simple way.

## Closing Notes

The goal was not to reproduce the standard library's `malloc` exactly.
The goal was to stop treating memory as a black box.
Once the view drops down to this level, topics that used to feel abstract start connecting in a much more concrete way.

- why heap allocation is not free
- why object memory layout matters
- why indirection affects cache efficiency
- why custom allocators and memory pools exist
- why C/C++ can expose performance characteristics more directly than many managed languages

## Build & Run

```bash
gcc main.c -o out && ./out
```
