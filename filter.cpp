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

#define APP_IBENCH_FILTER_CPP_

// ============================================================================
// Prerequisites
// ============================================================================

// ----------------------------------------------------------------------------
// SeqAn headers
// ----------------------------------------------------------------------------

#include <seqan/basic.h>
#include <seqan/sequence.h>
#include <seqan/find.h>
#include <seqan/index.h>
#include <seqan/arg_parse.h>

#include "options.h"
#include "types.h"
#include "run.h"
#include "misc.h"
#include "../yara/find_extender.h"

using namespace seqan;

// ============================================================================
// Classes
// ============================================================================

// ----------------------------------------------------------------------------
// Class Stats
// ----------------------------------------------------------------------------

struct Stats
{
    static double           totalTime;
    static String<unsigned> verifications;
    static String<unsigned> matches;
};

double Stats::totalTime = 0;
String<unsigned> Stats::verifications;
String<unsigned> Stats::matches;

// ----------------------------------------------------------------------------
// Class Options
// ----------------------------------------------------------------------------

struct Options : BaseOptions
{
    typedef std::string             TString;
    typedef std::vector<TString>    TList;

    enum AlgorithmType
    {
        ALGO_SEEDS, ALGO_QGRAMS
    };

    CharString      queryFile;

    AlgorithmType   algorithmType;
    TList           algorithmTypeList;

    double          errorRate;
    bool            editDistance;
    bool            verify;

    unsigned        seedsErrors;
    bool            seedsOnline;

    unsigned        qgramsWeight;
    unsigned        qgramsThreshold;
    TString         qgramsShape;

    Options() :
        BaseOptions(),
        algorithmType(ALGO_SEEDS),
        errorRate(0.0),
        editDistance(false),
        verify(false),
        seedsErrors(0),
        seedsOnline(false),
        qgramsWeight(10),
        qgramsThreshold(1),
        qgramsShape()
    {
        algorithmTypeList.push_back("seeds");
        algorithmTypeList.push_back("qgrams");
    }
};

// ============================================================================
// Tags and Metafunctions
// ============================================================================

template <>
struct Pigeonhole<EditDistance>
{
    enum { ONE_PER_DIAGONAL = 0 };  // 1 turns on heuristic duplicate masking.
    enum { HAMMING_ONLY = 0 };
};

template <>
struct Pigeonhole<HammingDistance>
{
    enum { ONE_PER_DIAGONAL = 0 };
    enum { HAMMING_ONLY = 1 };
};

template <typename TDistance>
struct SwiftSpec
{
    typedef SwiftSemiGlobal Type;
};

template <>
struct SwiftSpec<HammingDistance>
{
    typedef SwiftSemiGlobalHamming Type;
};

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function setupArgumentParser()
// ----------------------------------------------------------------------------

template <typename TOptions>
inline void setupArgumentParser(ArgumentParser & parser, TOptions const & options)
{
    setAppName(parser, "iBench Filter");
    setShortDescription(parser, "Benchmark filtration efficiency");
    setCategory(parser, "Stringology");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] <\\fITEXT|INDEX FILE\\fP> <\\fIQUERY FILE\\fP>");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addOption(parser, ArgParseOption("v", "tsv", "Tab separated value output."));

    addSection(parser, "Main Options");
    setAlphabetType(parser, options);
    setIndexType(parser, options);
    setTextLimits(parser, options);

    addSection(parser, "Search Options");
    setAlgorithmType(parser, options);
    addOption(parser, ArgParseOption("e", "error-rate", "Error rate.", ArgParseOption::INTEGER));
    setMinValue(parser, "error-rate", "0");
    setMaxValue(parser, "error-rate", "10");
    setDefaultValue(parser, "error-rate", options.errorRate);
    addOption(parser, ArgParseOption("ed", "edit-distance", "Edit distance. Default: Hamming distance."));
    addOption(parser, ArgParseOption("vf", "verify", "Verify candidate locations. Default: filter only."));

    addSection(parser, "Seeds Filtering Options");
    addOption(parser, ArgParseOption("se", "seeds-errors", "Set maximum errors per seed.", ArgParseOption::INTEGER));
    setMinValue(parser, "seeds-errors", "0");
    setMaxValue(parser, "seeds-errors", "3");
    setDefaultValue(parser, "seeds-errors", options.seedsErrors);
    addOption(parser, ArgParseOption("so", "seeds-online", "Online filter - for exact seeds only. Default: indexed filter."));

    addSection(parser, "q-Grams Filtering Options");
    addOption(parser, ArgParseOption("qw", "qgrams-weight", "Set q-gram weight.", ArgParseOption::INTEGER));
    setMinValue(parser, "qgrams-weight", "4");
    setMaxValue(parser, "qgrams-weight", "31");
    setDefaultValue(parser, "qgrams-weight", options.qgramsWeight);
    addOption(parser, ArgParseOption("qt", "qgrams-threshold", "Set minimum q-gram threshold.", ArgParseOption::INTEGER));
    setMinValue(parser, "qgrams-threshold", "1");
    setMaxValue(parser, "qgrams-threshold", "100");
    setDefaultValue(parser, "qgrams-threshold", options.qgramsThreshold);
    addOption(parser, ArgParseOption("qs", "qgrams-shape", "Set q-gram shape.", ArgParseOption::STRING, "BITSTRING"));
}

// ----------------------------------------------------------------------------
// Function shapeWeight()
// ----------------------------------------------------------------------------

template <typename TString>
inline unsigned shapeWeight(TString const & shape)
{
    unsigned ones = count(shape, '1');
    unsigned zeros = count(shape, '0');

    if (ones + zeros != length(shape))
        throw RuntimeError("Shape must be a binary string");

    if (ones < 4 || ones > 31)
        throw RuntimeError("Shape must contain at least 4 and at most 31 '1's");

    if (front(shape) != '1' || back(shape) != '1')
        throw RuntimeError("Shape must start and end with '1'");

    return ones;
}

// ----------------------------------------------------------------------------
// Function isOnline()
// ----------------------------------------------------------------------------

template <typename TOptions>
inline bool isOnline(TOptions const & options)
{
    return options.algorithmType == Options::ALGO_QGRAMS || (options.seedsOnline && !options.seedsErrors);
}

// ----------------------------------------------------------------------------
// Function isOffline()
// ----------------------------------------------------------------------------

template <typename TOptions>
inline bool isOffline(TOptions const & options)
{
    return !isOnline(options);
}

// ----------------------------------------------------------------------------
// Function parseCommandLine()
// ----------------------------------------------------------------------------

template <typename TOptions>
ArgumentParser::ParseResult
inline parseCommandLine(TOptions & options, ArgumentParser & parser, int argc, char const ** argv)
{
    ArgumentParser::ParseResult res = parse(parser, argc, argv);

    if (res != ArgumentParser::PARSE_OK)
        return res;

    getArgumentValue(options.queryFile, parser, 1);
    getOptionValue(options.tsv, parser, "tsv");

    getAlgorithmType(options, parser);
    getAlphabetType(options, parser);
    getIndexType(options, parser);
    getTextLimits(options, parser);

    getOptionValue(options.editDistance, parser, "edit-distance");

    unsigned errorRate;
    if (getOptionValue(errorRate, parser, "error-rate"))
        options.errorRate = errorRate / 100.0;

    getOptionValue(options.verify, parser, "verify");

    getOptionValue(options.seedsErrors, parser, "seeds-errors");
    getOptionValue(options.seedsOnline, parser, "seeds-online");

    if (isSet(parser, "qgrams-shape"))
    {
        getOptionValue(options.qgramsShape, parser, "qgrams-shape");
        options.qgramsWeight = shapeWeight(options.qgramsShape);
    }
    else
    {
        getOptionValue(options.qgramsWeight, parser, "qgrams-weight");
        options.qgramsShape.assign(options.qgramsWeight, '1');
    }
    getOptionValue(options.qgramsThreshold, parser, "qgrams-threshold");

    if (isOnline(options))
        getArgumentValue(options.textFile, parser, 0);
    else
        getArgumentValue(options.textIndexFile, parser, 0);

    return ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function runOffline()
// ----------------------------------------------------------------------------
// Exact or approximate seeds.

template <typename TIndex, typename TQueries, typename TDistance, typename TSeeding>
inline void runOffline(Options const & options, TIndex & index, TQueries & queries, TDistance const &, TSeeding const &)
{
    typedef typename Fibre<TIndex, FibreText>::Type         TText;
    typedef typename Fibre<TIndex, FibreSA>::Type           TSA;
    typedef typename Infix<TSA const>::Type                 TOccurrences;
    typedef typename Value<TOccurrences>::Type              TOccurrence;

    typedef typename Size<TQueries>::Type                   TQueryId;
    typedef typename Value<TQueries>::Type                  TQuery;
    typedef typename Size<TQuery>::Type                     TQuerySize;

    typedef Extender<TText, TQuery, TDistance>              TExtender;

    double timer = sysTime();

    TExtender extender(indexText(index));

    unsigned char queryErrors = options.errorRate * length(front(queries));

    filter(index, queries, queryErrors,
           [&](TQueryId queryId,
               TOccurrence textBegin, TOccurrence textEnd,
               TQuerySize queryBegin, TQuerySize queryEnd,
               unsigned char seedErrors)
            {
                Stats::verifications[queryId]++;

                extend(extender,
                       queries[queryId],
                       textBegin, textEnd,
                       queryBegin, queryEnd,
                       seedErrors, queryErrors,
                       [&](TOccurrence, TOccurrence, unsigned char)
                       {
                            Stats::matches[queryId]++;
                       });
            },
            Seeds<TSeeding>(options.seedsErrors));

    Stats::totalTime = sysTime() - timer;
}

template <typename TText, typename TQueries, typename TDistance, typename TSeeding>
inline void runOffline(Options const &, Index<TText, IndexEsa<void> > &, TQueries &, TDistance const &, TSeeding const &)
{
    throw RuntimeError("Unsupported index");
}

template <typename TText, typename TQueries, typename TIndexSpec, typename TDistance, typename TSeeding>
inline void runOffline(Options const &, Index<TText, FMIndex<void, TIndexSpec> > &, TQueries &, TDistance const &, TSeeding const &)
{
    throw RuntimeError("Unsupported index");
}

template <typename TText, typename TQueries, typename TDistance>
inline void runOffline(Options const & options, TText & text, TQueries & queries, TDistance const & dist)
{
    if (options.verify)
    {
        if (options.seedsErrors)
            runOffline(options, text, queries, dist, dist);
        else
            runOffline(options, text, queries, dist, Exact());
    }
    else
    {
        if (options.seedsErrors)
            runOffline(options, text, queries, Nothing(), dist);
        else
            runOffline(options, text, queries, Nothing(), Exact());
    }
}

template <typename TText, typename TQueries>
inline void runOffline(Options const & options, TText & text, TQueries & queries)
{
    if (options.editDistance)
        runOffline(options, text, queries, EditDistance());
    else
        runOffline(options, text, queries, HammingDistance());
}

// ----------------------------------------------------------------------------
// Function runOnline()
// ----------------------------------------------------------------------------
// q-Grams or exact seeds.

template <typename TPattern, typename TSpec>
inline void _runOnlineInit(TPattern & pattern, Options const & options, Pigeonhole<TSpec>)
{
    _patternInit(pattern, options.errorRate);
}

template <typename TPattern, typename TSpec>
inline void _runOnlineInit(TPattern & pattern, Options const & options, Swift<TSpec>)
{
    pattern.params.minThreshold = options.qgramsThreshold;
    _patternInit(pattern, options.errorRate, 0);
}

template <typename TText, typename TQueries, typename TDistance, typename TAlgorithm, typename TShape>
inline void runOnline(Options const & options, TText & text, TQueries & queries, TDistance const &, TAlgorithm const & algo, TShape const & shape)
{
    typedef IndexQGram<TShape, OpenAddressing>              TIndexSpec;
    typedef Index<TQueries, TIndexSpec>                     TPatternIndex;
    typedef Pattern<TPatternIndex, TAlgorithm>              TPattern;

    typedef typename Value<TText>::Type const               THaystack;
    typedef typename Size<TText>::Type                      THaystackSize;
    typedef Finder<THaystack, TAlgorithm>                   TFinder;

    typedef typename Size<TQueries>::Type                   TQueryId;
    typedef typename Value<TQueries>::Type                  TNeedle;
    typedef Verifier<THaystack, TNeedle, TDistance>         TVerifier;

    double timer = sysTime();

    TPatternIndex patternIndex(queries, shape);
    TPattern pattern(patternIndex);
    _runOnlineInit(pattern, options, algo);

    TVerifier verifier;

    for (THaystackSize i = 0; i < length(text); ++i)
    {
        TFinder finder(text[i]);
        while (find(finder, pattern, options.errorRate))
        {
            TQueryId queryId = getSeqNo(position(pattern));

            Stats::verifications[queryId]++;

            unsigned char queryErrors = options.errorRate * length(queries[queryId]);

            verify(verifier,
                   infix(finder), infix(pattern), queryErrors,
                   [&](typename Infix<THaystack>::Type const &, unsigned char)
                   {
                        Stats::matches[queryId]++;
                   });
        }
    }

    Stats::totalTime = sysTime() - timer;
}

template <typename TText, typename TQueries, typename TDistance, typename TAlgorithm>
inline void _runOnline(Options const & options, TText & text, TQueries & queries, TDistance const & dist, TAlgorithm const & algo)
{
    typedef typename Value<TText>::Type     THaystack;
    typedef typename Value<THaystack>::Type TAlphabet;

//    Shape<TAlphabet, UngappedShape<9> > contiguous;
    Shape<TAlphabet, SimpleShape>     contiguous;
    Shape<TAlphabet, GenericShape>    gapped;

    if (stringToShape(contiguous, options.qgramsShape))
        runOnline(options, text, queries, dist, algo, contiguous);
    else if (stringToShape(gapped, options.qgramsShape))
        runOnline(options, text, queries, dist, algo, gapped);
    else
        throw RuntimeError("Unsupported q-gram shape");
}

template <typename TText, typename TQueries, typename TDistance, typename TSeeding>
inline void runOnline(Options const & options, TText & text, TQueries & queries, TDistance const & dist, TSeeding const &)
{
    switch (options.algorithmType)
    {
    case Options::ALGO_SEEDS:
        _runOnline(options, text, queries, dist, Pigeonhole<TSeeding>());
        return;

    case Options::ALGO_QGRAMS:
        _runOnline(options, text, queries, dist, Swift<typename SwiftSpec<TSeeding>::Type>());
        return;

    default:
        throw RuntimeError("Unsupported filter");
    }
}

template <typename TText, typename TQueries, typename TDistance>
inline void runOnline(Options const & options, TText & text, TQueries & queries, TDistance const & dist)
{
    if (options.verify)
        runOnline(options, text, queries, dist, dist);
    else
        runOnline(options, text, queries, Nothing(), dist);
}

template <typename TText, typename TQueries>
inline void runOnline(Options const & options, TText & text, TQueries & queries)
{
    if (options.editDistance)
        runOnline(options, text, queries, EditDistance());
    else
        runOnline(options, text, queries, HammingDistance());
}

// ----------------------------------------------------------------------------
// Function run()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TLimits, typename TSetLimits, typename TIndexSpec>
inline void run(Options const & options)
{
    typedef typename TextCollection<TAlphabet, Limits<__uint8> >::Type      TQueries;
    typedef typename TextCollection<TAlphabet, TLimits, TSetLimits>::Type   TText;
    typedef Index<TText, TIndexSpec>                                        TIndex;

    TQueries queries;

    if (!open(queries, toCString(options.queryFile)))
        throw RuntimeError("Error while loading queries");

    TIndex index;
    TText text;

    resize(Stats::verifications, length(queries), 0u, Exact());
    resize(Stats::matches, length(queries), 0u, Exact());

    if (isOffline(options))
    {
        if (!open(index, toCString(options.textIndexFile)))
            throw RuntimeError("Error while loading full-text index");

        runOffline(options, index, queries);
    }
    else
    {
        if (!open(text, toCString(options.textFile)))
            throw RuntimeError("Error while loading text");

        runOnline(options, text, queries);
    }

    unsigned long verificationsCount = sum(Stats::verifications);
    unsigned long matchesCount = sum(Stats::matches);

    if (options.tsv)
    {
        std::cout << verificationsCount << '\t' << matchesCount << '\t' <<
                     std::fixed << Stats::totalTime << std::endl;

//        forEach(Stats::verifications, [&](unsigned verificationsCount) { std::cout << verificationsCount << '\n'; });
    }
    else
    {
        std::cout << length(queries) << " queries" << std::endl;
        std::cout << lengthSum(queries) << " symbols" << std::endl;
        std::cout << verificationsCount << " verifications" << std::endl;
        std::cout << matchesCount << " matches" << std::endl;
        std::cout << 100.0 * matchesCount / verificationsCount << " % PPV" << std::endl;
        std::cout << std::fixed << Stats::totalTime << " sec" << std::endl;
    }
}

int main(int argc, char const ** argv)
{
    return run<Options>(argc, argv);
}
