# Parallel Zip (wzip)

A multithreaded run-length-encoding (RLE) compression tool written in C. It compresses files by collapsing runs of repeated characters, and parallelizes large inputs across worker threads using `pthread` and memory-mapped I/O.

> Course project: the `wzip` assignment from *Operating Systems: Three Easy Pieces* (Arpaci-Dusseau). The implementation is entirely my own (no starter code).

## Features
- **Run-length encoding** — compresses runs of repeated characters into a compact format: a 4-byte integer count followed by the single repeated character.
- **Multithreaded compression** — files larger than 4 KB are split into chunks and encoded in parallel by worker threads, then merged.
- **Boundary stitching** — when a run of identical characters spans two chunks, the per-thread results are combined so the run is counted correctly across the split.
- **Memory-mapped I/O** — input files are read via `mmap` for efficient access.
- **Multiple files as one stream** — several input files are treated as a single continuous stream, so a run spanning a file boundary compresses as one run.

## Build
```
make
```

## Run
```
./wzip file1 [file2 ...] > output.z
```
Compresses the input file(s) and writes the RLE-encoded result to standard output (redirect it to a file to save).

## Output format
Each run is written as a 4-byte integer count (in native byte order) followed by the 1-byte repeated character — the standard `wzip` format.

## Concepts
POSIX threads (`pthread_create`, `pthread_join`), memory-mapped files (`mmap`), parallel data processing with cross-boundary merging, and binary file I/O in C.

## Files
- `wzip.c` — the full implementation: file mapping, threaded RLE, merge, and output
- `Makefile` — build configuration
