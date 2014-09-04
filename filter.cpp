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
#include <seqan/index/find_pigeonhole.h>
#include <seqan/arg_parse.h>

#include "options.h"
#include "types.h"
#include "run.h"
#include "misc.h"

using namespace seqan;

// ============================================================================
// Classes
// ============================================================================

// ----------------------------------------------------------------------------
// Class Stats
// ----------------------------------------------------------------------------
//
//struct Stats
//{
//    double          filterTime;
//
//    Stats() :
//        filterTime()
//    {}
//};

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
// Function runOnline()
// ----------------------------------------------------------------------------

template <typename TOptions>
inline bool runOnline(TOptions const & options)
{
    return options.algorithmType == Options::ALGO_QGRAMS || (options.seedsOnline && !options.seedsErrors);
}

// ----------------------------------------------------------------------------
// Function runOffline()
// ----------------------------------------------------------------------------

template <typename TOptions>
inline bool runOffline(TOptions const & options)
{
    return !runOnline(options);
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

    if (runOnline(options))
        getArgumentValue(options.textFile, parser, 0);
    else
        getArgumentValue(options.textIndexFile, parser, 0);

    return ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function verify()
// ----------------------------------------------------------------------------

//template <typename TIndex, typename TSpec>
//inline void verify(Iter<TIndex, TSpec> const & it, True)
//{
//    typedef typename Fibre<TIndex, FibreSA>::Type   TSA;
//    typedef typename Infix<TSA const>::Type         TOccurrences;
//    typedef typename Value<TOccurrences>::Type      TOccurrence;
//    typedef typename Size<TIndex>::Type             TSize;
//
//    TOccurrences const & occs = getOccurrences(it);
//
//    forEach(occs, [&](TOccurrence const & occ)
//    {
//        TSize volatile offset = getSeqOffset(occ);
//        ignoreUnusedVariableWarning(offset);
//    });
//}

//template <typename TIndex, typename TSpec>
//inline void verify(Iter<TIndex, TSpec> const & it, True)
//{
//    Finder<TContigSeq> verifyFinder(fragStore.contigStore[i].seq);
//    setPosition(verifyFinder, beginPosition(finder));
//    Pattern<TReadSeq, HammingSimple> verifyPattern(fragStore.readSeqStore[position(pattern).i1]);
//    unsigned readLength = length(fragStore.readSeqStore[position(pattern).i1]);
//    int minScore = -static_cast<int>(options.errorRate * readLength);
//    while (find(verifyFinder, verifyPattern, minScore) && position(verifyFinder) < endPosition(infix(finder)))
//    {
//        TAlignedRead match(length(fragStore.alignedReadStore), position(pattern).i1, i,
//                           beginPosition(verifyFinder), endPosition(verifyFinder));
//        appendValue(fragStore.alignedReadStore, match);
//    }
//}

// ----------------------------------------------------------------------------
// Function filterOffline()
// ----------------------------------------------------------------------------
// Exact or approximate seeds.

template <typename TIndex, typename TQueries, typename TAlgorithm>
inline unsigned long
filterOffline(Options const & options, TIndex & index, TQueries & queries, TAlgorithm const & algo)
{
    typedef typename Iterator<TIndex, TopDown<> >::Type     TIndexIt;
    typedef typename Iterator<TQueries const, Rooted>::Type TQueriesIt;
    typedef StringSet<TQueries, Segment<TQueries> >         TSeeds;
    typedef typename Iterator<TSeeds const, Rooted>::Type   TSeedsIt;
    typedef typename StringSetPosition<TSeeds>::Type        TSeedPos;
    typedef unsigned                                        TSize;

//    double timer = sysTime();

    TSeeds seeds(queries);

    iterate(queries, [&](TQueriesIt const & it)
    {
        TSize needleLength = length(value(it));
        TSize needleErrors = needleLength * options.errorRate;
        TSize seedsCount = static_cast<TSize>(std::ceil((needleErrors + 1) / (options.seedsErrors + 1.0)));
        TSize seedsLength = needleLength / seedsCount;

        for (TSize seedId = 0; seedId < seedsCount; ++seedId)
            appendInfixWithLength(seeds, TSeedPos(position(it), seedId * seedsLength), seedsLength, Generous());
    },
    Rooted(), Serial());

    unsigned long count = 0;

    find(index, seeds, options.seedsErrors, [&](TIndexIt const & indexIt, TSeedsIt const &, unsigned)
    {
        count += countOccurrences(indexIt);
    },
    algo);

//    stats.countTime = sysTime() - timer;

    return count;
}

template <typename TText, typename TQueries, typename TAlgorithm>
inline unsigned long
filterOffline(Options const &, Index<TText, IndexEsa<void> > &, TQueries &, TAlgorithm const &)
{
    throw RuntimeError("Unsupported index");
    return 0;
}

template <typename TText, typename TQueries, typename TIndexSpec, typename TAlgorithm>
inline unsigned long
filterOffline(Options const &, Index<TText, FMIndex<void, TIndexSpec> > &, TQueries &, TAlgorithm const &)
{
    throw RuntimeError("Unsupported index");
    return 0;
}

// ----------------------------------------------------------------------------
// Function filterOnlineInit()
// ----------------------------------------------------------------------------

template <typename TPattern, typename TSpec>
inline void filterOnlineInit(TPattern & pattern, Options const & options, Pigeonhole<TSpec>)
{
    _patternInit(pattern, options.errorRate);
}

template <typename TPattern>
inline void filterOnlineInit(TPattern & pattern, Options const & options, Swift<SwiftSemiGlobal>)
{
    pattern.params.minThreshold = options.qgramsThreshold;
    _patternInit(pattern, options.errorRate, 0);
}

// ----------------------------------------------------------------------------
// Function filterOnline()
// ----------------------------------------------------------------------------

template <typename TText, typename TQueries, typename TAlgorithm, typename TShape>
inline unsigned long
filterOnline(Options const & options, TText & text, TQueries & queries, TAlgorithm const & algo, TShape const & shape)
{
//    typedef IndexQGram<TShape, OpenAddressing>              TIndexSpec;
    typedef IndexQGram<TShape>                              TIndexSpec;
    typedef Index<TQueries, TIndexSpec>                     TPatternIndex;
    typedef Pattern<TPatternIndex, TAlgorithm>              TPattern;

    typedef typename Value<TText>::Type const               THaystack;
    typedef typename Size<TText>::Type                      THaystackSize;
    typedef Finder<THaystack, TAlgorithm>                   TFinder;

    unsigned long count = 0;

//    double timer = sysTime();

    TPatternIndex patternIndex(queries, shape);
    TPattern pattern(patternIndex);
    filterOnlineInit(pattern, options, algo);

    for (THaystackSize i = 0; i < length(text); ++i)
    {
        TFinder finder(text[i]);
        while (find(finder, pattern, options.errorRate))
        {
            count++;
        }
    }

//    stats.filterTime = sysTime() - timer;

    return count;
}

// ----------------------------------------------------------------------------
// Function filterOnline()
// ----------------------------------------------------------------------------
// q-Grams or exact seeds.

template <typename TText, typename TQueries, typename TAlgorithm>
inline unsigned long
filterOnline(Options const & options, TText & text, TQueries & queries, TAlgorithm const & algo)
{
//    Shape<Dna, UngappedShape<9> > contiguous;
    Shape<Dna, SimpleShape>     contiguous;
    Shape<Dna, GenericShape>    gapped;

    if (stringToShape(contiguous, options.qgramsShape))
        return filterOnline(options, text, queries, algo, contiguous);

    if (stringToShape(gapped, options.qgramsShape))
        return filterOnline(options, text, queries, algo, gapped);

    throw RuntimeError("Unsupported q-gram shape");
}

template <typename TText, typename TQueries>
inline unsigned long filterOnline(Options const & options, TText & text, TQueries & queries)
{
    switch (options.algorithmType)
    {
    case Options::ALGO_SEEDS:
        return filterOnline(options, text, queries, Pigeonhole<void>()); //Pigeonhole<Hamming_>

    case Options::ALGO_QGRAMS:
        return filterOnline(options, text, queries, Swift<SwiftSemiGlobal>());
    }
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
    unsigned long verificationsCount = 0;

    if (runOffline(options))
    {
        if (!open(index, toCString(options.textIndexFile)))
            throw RuntimeError("Error while loading full-text index");

        verificationsCount = filterOffline(options, index, queries, Backtracking<HammingDistance>());
    }
    else
    {
        if (!open(text, toCString(options.textFile)))
            throw RuntimeError("Error while loading text");

        verificationsCount = filterOnline(options, text, queries);
    }

    if (options.tsv)
    {
//        std::cout << hitsCount << '\t' << std::fixed << stats.time << std::endl;
    }
    else
    {
        std::cout << length(queries) << " queries" << std::endl;
        std::cout << lengthSum(queries) << " symbols" << std::endl;
        std::cout << verificationsCount << " verifications" << std::endl;
//        std::cout << std::fixed << stats.time << " sec" << std::endl;
    }
}

int main(int argc, char const ** argv)
{
    return run<Options>(argc, argv);
}
