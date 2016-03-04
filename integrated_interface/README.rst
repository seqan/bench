Seqan Benchmarks Integrated Interface
=====================================
Introduction
------------
This application provides a integrated interface for Seqan benchmarks.  

Support
-------
Linux
Windows
OSX

Installation
-------------
required libraries:
    SeqAn
    NW.js

On Linux:
1. Download SeqAn
   ``git clone https://github.com/seqan/seqan.git``
2. Download the bench into the SeqAn apps folder
   ``git clone https://github.com/xp3i4/bench.git seqan/apps/bench seqan/apps/bench``
3. Create a Makefile project:

   ``mkdir seqan-build``
   
   ``cd seqan-build``
   
   ``cmake ../seqan -DCMAKE_C_COMPILER=/usr/bin/gcc-4.8 -DCMAKE_CXX_COMPILER=/usr/bin/g++-4.8``
4. Download NW.js http://nwjs.io/ and extract it to the seqan/apps/bench/integerated_interface
5. run pack.py

Download binary
---------------
Linux 
Windows
Mac OSX

Usage
-----
click the executable binary "app" to start

Contact
-------
For questions or comments, feel free to contact: Chenxu Pan <chenxu.pan@fu-berlin.de>

