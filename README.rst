SeqAn Benchmarks
================

This app contains benchmarks for various core algorithms and data structures implemented in SeqAn.

Benchmarks
~~~~~~~~~~

- Indices

  - Construction
  - Top-down traversal
  - Exact and approximate string matching

- Filters

  - q-Grams
  - Non-overlapping seeds

Installation
------------

1. Download the SeqAn library:

   ``git clone https://github.com/seqan/seqan.git``

2. Download the bench app **into the SeqAn apps folder**:

   ``git clone https://github.com/esiragusa/bench.git seqan/apps/bench``

3. Create a Makefile project:

   ``mkdir seqan-build``
   ``cd seqan-build``
   ``cmake ../seqan -DCMAKE_C_COMPILER=/usr/bin/gcc-4.8 -DCMAKE_CXX_COMPILER=/usr/bin/g++-4.8``

4. List all Makefile targets of the bench app:

   ``make help | grep bench_``

5. Build e.g. the index construction benchmark:

   ``make bench_construct``

Usage
-----

To get a complete usage description, invoke each tool with -h or --help.


Contact
-------

For questions or comments, feel free to contact: Enrico Siragusa <enrico.siragusa@fu-berlin.de>
