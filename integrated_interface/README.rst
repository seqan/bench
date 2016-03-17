Seqan Benchmarks Integrated Interface
=====================================
Introduction
------------
This application provides a integrated interface for Seqan benchmarks.  

Installation
-------------
required libraries:
    SeqAn
    NW.js

On Linux/OSX:

1. Download SeqAn 

    ``mkdir seqan``

    ``cd seqan``

    ``git clone https://github.com/seqan/seqan.git``
    
2. Download bench

    ``cd seqan/apps``

    ``git clone https://github.com/xp3i4/bench.git seqan/apps/bench seqan/apps/bench``

    ``cd bench`` 
    
    ``git checkout integrated-interface``

3. Create a Makefile project

    ``mkdir -p seqan_build/release``
    
    ``cd seqan-build/release``
    
    ``cmake ../../seqan  -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++``
    
4. Download NW.js http://nwjs.io/  and extract the folders to somewhere

5. run install.py

    ``python install.python -n [path to nw.js folder] -s [path to seqan build directory] ...seqan_build/release``

Done

Download binary
---------------
Linux 
Windows
Mac OSX

Usage
-----
linux start the program from seqan_build/release/bin/SeqanBench

osx start from the /Applications or launchpad

Contact
-------
For questions or comments, feel free to contact: Chenxu Pan <chenxu.pan@fu-berlin.de>

