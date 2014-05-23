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

template <typename TOptions>
inline void setupArgumentParser(ArgumentParser & parser, TOptions const & options);

template <typename TOptions>
inline ArgumentParser::ParseResult
parseCommandLine(TOptions & options, ArgumentParser & parser, int argc, char const ** argv);

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function run()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TLimits, typename TSetLimits, typename TOptions>
inline void run(TOptions & options)
{
    switch (options.textIndexType)
    {
    case TOptions::INDEX_ESA:
        run<TAlphabet, TLimits, TSetLimits, IndexEsa<void> >(options);

    case TOptions::INDEX_SA:
        run<TAlphabet, TLimits, TSetLimits, IndexSa<void> >(options);

    case TOptions::INDEX_QGRAM:
        run<TAlphabet, TLimits, TSetLimits, IndexQGram<typename QGramShape<TAlphabet>::Type, BucketRefinement> >(options);

    case TOptions::INDEX_FMTL:
        run<TAlphabet, TLimits, TSetLimits, FMIndex<void, TLFMIndexConfig<TSetLimits> > >(options);

    case TOptions::INDEX_FMWT:
        run<TAlphabet, TLimits, TSetLimits, FMIndex<void, WTFMIndexConfig<TSetLimits> > >(options);

    default:
        throw RuntimeError("Unsupported index type");
    }
}

template <typename TAlphabet, typename TLimits, typename TOptions>
inline void run(TOptions & options)
{
    switch (options.textCount)
    {
    case 8:
        if (options.textSum == 32)
            run<TAlphabet, TLimits, Limits<__uint8, __uint32> >(options);
//        if (options.textSum == 64)
//            run<TAlphabet, TLimits, Limits<__uint8, __uint64> >(options);

//    case 16:
//        if (options.textSum == 32)
//            run<TAlphabet, TLimits, Limits<__uint16, __uint32> >(options);
//        if (options.textSum == 64)
//            run<TAlphabet, TLimits, Limits<__uint16, __uint64> >(options);

    case 32:
        if (options.textSum == 32)
            run<TAlphabet, TLimits, Limits<__uint32, __uint32> >(options);
//        if (options.textSum == 64)
//            run<TAlphabet, TLimits, Limits<__uint32, __uint64> >(options);

    default:
        throw RuntimeError("Unsupported text collection limit");
    }
}

template <typename TAlphabet, typename TOptions>
inline void run(TOptions & options)
{
    switch (options.textLen)
    {
    case 8:
        run<TAlphabet, Limits<__uint8> >(options);

    case 16:
        run<TAlphabet, Limits<__uint16> >(options);

    case 32:
        run<TAlphabet, Limits<__uint32> >(options);

//    case 64:
//        run<TAlphabet, Limits<__uint64> >(options);

    default:
        throw RuntimeError("Unsupported text limit");
    }
}

template <typename TOptions>
inline void run(TOptions & options)
{
    switch (options.alphabetType)
    {
    case TOptions::ALPHABET_DNA:
        run<Dna>(options);

    case TOptions::ALPHABET_PROTEIN:
        run<AminoAcid>(options);

//    case TOptions::ALPHABET_CHAR:
//        run<char>(options);

    default:
        throw RuntimeError("Unsupported alphabet type");
    }
}

template <typename TOptions>
inline int run(int argc, char const ** argv)
{
    ArgumentParser parser;
    TOptions options;
    setupArgumentParser(parser, options);

    ArgumentParser::ParseResult res = parseCommandLine(options, parser, argc, argv);

    if (res != ArgumentParser::PARSE_OK)
        return res;

    try
    {
        run(options);
    }
    catch (Exception const & e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#endif  // #ifndef APP_IBENCH_RUN_H_
