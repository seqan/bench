// ==========================================================================
//                 iBench - Full-text indexing benchmarks
// ==========================================================================
// Copyright (c) 2012-2014, Enrico Siragusa, FU Berlin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Knut Reinert or the FU Berlin nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ENRICO SIRAGUSA OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ==========================================================================
// Author: Enrico Siragusa <enrico.siragusa@fu-berlin.de>
// ==========================================================================

#ifndef APP_IBENCH_RUN_H_
#define APP_IBENCH_RUN_H_

using namespace seqan;

// ============================================================================
// Forwards
// ============================================================================

inline void setupArgumentParser(ArgumentParser & parser, Options const & options);
inline ArgumentParser::ParseResult parseCommandLine(Options & options, ArgumentParser & parser, int argc, char const ** argv);

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function run()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TLimits, typename TSetLimits>
inline void run(Options & options)
{
    switch (options.textIndexType)
    {
//    case Options::INDEX_ESA:
//        return run<TAlphabet, TLimits, TSetLimits, IndexEsa<void> >(options);

    case Options::INDEX_SA:
        return run<TAlphabet, TLimits, TSetLimits, IndexSa<void> >(options);

//    case Options::INDEX_QGRAM:
//        return run<TAlphabet, TLimits, TSetLimits, IndexQGram<typename QGramShape<TAlphabet>::Type, BucketRefinement> >(options);

//    case Options::INDEX_FMTL:
//        return run<TAlphabet, TLimits, TSetLimits, FMIndex<void, FMIndexConfig<TSetLimits> > >(options);

//    case Options::INDEX_FMWT:
//        return run<TAlphabet, TLimits, TSetLimits, FMIndex<void, FMIndexConfig<TSetLimits> > >(options);

    default:
        throw RuntimeError("Unsupported index type");
    }
}

template <typename TAlphabet, typename TLimits>
inline void run(Options & options)
{
    switch (options.textCount)
    {
    case 8:
        if (options.textSum == 32)
            return run<TAlphabet, TLimits, Limits<__uint8, __uint32> >(options);
        if (options.textSum == 64)
            return run<TAlphabet, TLimits, Limits<__uint8, __uint64> >(options);

    case 16:
        if (options.textSum == 32)
            return run<TAlphabet, TLimits, Limits<__uint16, __uint32> >(options);
        if (options.textSum == 64)
            return run<TAlphabet, TLimits, Limits<__uint16, __uint64> >(options);

    case 32:
        if (options.textSum == 32)
            return run<TAlphabet, TLimits, Limits<__uint32, __uint32> >(options);
        if (options.textSum == 64)
            return run<TAlphabet, TLimits, Limits<__uint32, __uint64> >(options);

    default:
        throw RuntimeError("Unsupported text collection limit");
    }
}

template <typename TAlphabet>
inline void run(Options & options)
{
    switch (options.textLen)
    {
    case 16:
        return run<TAlphabet, Limits<__uint16> >(options);

    case 32:
        return run<TAlphabet, Limits<__uint32> >(options);

    case 64:
        return run<TAlphabet, Limits<__uint64> >(options);

    default:
        throw RuntimeError("Unsupported text limit");
    }
}

inline void run(Options & options)
{
    switch (options.alphabetType)
    {
    case Options::ALPHABET_DNA:
        return run<Dna>(options);

    case Options::ALPHABET_PROTEIN:
        return run<AminoAcid>(options);

    case Options::ALPHABET_CHAR:
        return run<char>(options);

    default:
        throw Exception();
    }
}

inline int run(int argc, char const ** argv)
{
    ArgumentParser parser;
    Options options;
    setupArgumentParser(parser, options);

    ArgumentParser::ParseResult res = parseCommandLine(options, parser, argc, argv);

    if (res != seqan::ArgumentParser::PARSE_OK)
        return res;

    run(options);

    return 0;
}

#endif  // #ifndef APP_IBENCH_RUN_H_
