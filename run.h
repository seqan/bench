// ==========================================================================
//                             SeqAn Benchmark
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

#ifndef APP_BENCH_RUN_H_
#define APP_BENCH_RUN_H_

using namespace seqan;

// ============================================================================
// Forwards
// ============================================================================

//template <typename Type1, typename Type2>
//struct IsBaseType : IsSameType<typename std::is_base_of<Type1, Type2>::type, std::true_type> {};

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

#if defined(APP_BENCH_QGRAMS_CONSTRUCT_CPP_) || defined(APP_BENCH_QGRAMS_FIND_CPP_)
template <typename TAlphabet, typename TLimits, typename TSetLimits, typename TOptions>
inline void run(TOptions & options)
{
    switch (options.textIndexType)
    {
    case TOptions::INDEX_QGRAM_DIRECT:
        switch (options.q)
        {
        case 5:
            run<TAlphabet, TLimits, TSetLimits, IndexQGram<UngappedShape<5> > >(options);
            break;
        case 10:
            run<TAlphabet, TLimits, TSetLimits, IndexQGram<UngappedShape<10> > >(options);
            break;
        case 15:
            run<TAlphabet, TLimits, TSetLimits, IndexQGram<UngappedShape<15> > >(options);
            break;
        default:
            throw RuntimeError("Unsupported q-gram weight");
        }
        break;
    case TOptions::INDEX_QGRAM_OPEN:
        switch (options.q)
        {
        case 10:
            run<TAlphabet, TLimits, TSetLimits, IndexQGram<UngappedShape<10>, OpenAddressing> >(options);
            break;
        case 15:
            run<TAlphabet, TLimits, TSetLimits, IndexQGram<UngappedShape<15>, OpenAddressing> >(options);
            break;
        case 20:
            run<TAlphabet, TLimits, TSetLimits, IndexQGram<UngappedShape<20>, OpenAddressing> >(options);
            break;
        case 25:
            run<TAlphabet, TLimits, TSetLimits, IndexQGram<UngappedShape<25>, OpenAddressing> >(options);
            break;
        case 30:
            run<TAlphabet, TLimits, TSetLimits, IndexQGram<UngappedShape<30>, OpenAddressing> >(options);
            break;
        default:
            throw RuntimeError("Unsupported q-gram weight");
        }
        break;
    default:
        throw RuntimeError("Unsupported index type");
    }
}
#else
template <typename TAlphabet, typename TLimits, typename TSetLimits, typename TOptions>
inline void run(TOptions & options)
{
    switch (options.textIndexType)
    {
    case TOptions::INDEX_ESA:
        run<TAlphabet, TLimits, TSetLimits, IndexEsa<void> >(options);
        break;
    case TOptions::INDEX_SA:
        run<TAlphabet, TLimits, TSetLimits, IndexSa<void> >(options);
        break;
    case TOptions::INDEX_LST:
        run<TAlphabet, TLimits, TSetLimits, IndexWotd<void> >(options);
        break;
    case TOptions::INDEX_QGRAM:
        run<TAlphabet, TLimits, TSetLimits, IndexQGram<typename DefaultShape<TAlphabet>::Type, BucketRefinement> >(options);
        break;
    case TOptions::INDEX_FMTL:
        run<TAlphabet, TLimits, TSetLimits, FMIndex<void, TLFMIndexConfig<TSetLimits> > >(options);
        break;
    case TOptions::INDEX_FMWT:
        run<TAlphabet, TLimits, TSetLimits, FMIndex<void, WTFMIndexConfig<TSetLimits> > >(options);
        break;
    default:
        throw RuntimeError("Unsupported index type");
    }
}
#endif

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
        break;

//    case 16:
//        if (options.textSum == 32)
//            run<TAlphabet, TLimits, Limits<__uint16, __uint32> >(options);
//        if (options.textSum == 64)
//            run<TAlphabet, TLimits, Limits<__uint16, __uint64> >(options);
//        break;

    case 32:
        if (options.textSum == 32)
            run<TAlphabet, TLimits, Limits<__uint32, __uint32> >(options);
//        if (options.textSum == 64)
//            run<TAlphabet, TLimits, Limits<__uint32, __uint64> >(options);
        break;

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
        break;
    case 16:
        run<TAlphabet, Limits<__uint16> >(options);
        break;
    case 32:
        run<TAlphabet, Limits<__uint32> >(options);
        break;
//    case 64:
//        run<TAlphabet, Limits<__uint64> >(options);
        break;
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
        break;
    case TOptions::ALPHABET_PROTEIN:
        run<AminoAcid>(options);
        break;
//    case TOptions::ALPHABET_CHAR:
//        run<char>(options);
//        break;
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

#endif  // #ifndef APP_BENCH_RUN_H_
