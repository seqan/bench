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

// ============================================================================
// Prerequisites
// ============================================================================

// ----------------------------------------------------------------------------
// SeqAn headers
// ----------------------------------------------------------------------------

#include <seqan/basic.h>
#include <seqan/sequence.h>
#include <seqan/index.h>
#include <seqan/arg_parse.h>

#include "options.h"
#include "types.h"
#include "run.h"

using namespace seqan;

// ============================================================================
// Classes
// ============================================================================

// ----------------------------------------------------------------------------
// Class Options
// ----------------------------------------------------------------------------

struct Options : BaseOptions
{
    typedef std::string             TString;
    typedef std::vector<TString>    TList;

    enum AlgorithmType
    {
        ALGO_SINGLE, ALGO_DFS, ALGO_BFS
    };

    CharString      queryFile;

    AlgorithmType   algorithmType;
    TList           algorithmTypeList;

    unsigned        errors;
    bool            locate;

    Options() :
        BaseOptions(),
        algorithmType(ALGO_SINGLE),
        errors(0),
        locate(false)
    {
        algorithmTypeList.push_back("single");
        algorithmTypeList.push_back("dfs");
        algorithmTypeList.push_back("bfs");
    }
};

// ----------------------------------------------------------------------------
// Function setupArgumentParser()
// ----------------------------------------------------------------------------

template <typename TOptions>
inline void setupArgumentParser(ArgumentParser & parser, TOptions const & options)
{
    setAppName(parser, "iBench Query");
    setShortDescription(parser, "Benchmark full-text index query time");
    setCategory(parser, "Stringology");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] <\\fIINDEX FILE\\fP> <\\fIQUERY FILE\\fP>");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));

    addSection(parser, "Main Options");
    setAlphabetType(parser, options);
    setIndexType(parser, options);
    setTextLimits(parser, options);

    addSection(parser, "Query Options");
    setAlgorithmType(parser, options);
    addOption(parser, ArgParseOption("e", "errors", "Number of errors.", ArgParseOption::INTEGER));
    setMinValue(parser, "errors", "0");
    setMaxValue(parser, "errors", "5");
    setDefaultValue(parser, "errors", options.errors);
    addOption(parser, ArgParseOption("l", "locate", "Locate occurrences. Default: count only."));
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

    getArgumentValue(options.textIndexFile, parser, 0);
    getArgumentValue(options.queryFile, parser, 1);

    getAlphabetType(options, parser);
    getIndexType(options, parser);
    getTextLimits(options, parser);
    getAlgorithmType(options, parser);
    getOptionValue(options.errors, parser, "errors");
    getOptionValue(options.locate, parser, "locate");

    return ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function find()
// ----------------------------------------------------------------------------
// Overloaded to accept C++11 && delegate

template <typename TText, typename TIndexSpec, typename TPattern, typename TDelegate>
SEQAN_HOST_DEVICE inline void
find(Finder2<Index<TText, TIndexSpec>, TPattern, Backtracking<Exact> > & finder,
     TPattern const & pattern,
     TDelegate && delegate)
{
    if (goDown(textIterator(finder), pattern))
    {
        delegate(finder);
    }
}

template <typename TText, typename TIndexSpec, typename TPattern, typename TSpec, typename TDelegate>
inline void
find(Finder2<Index<TText, TIndexSpec>, TPattern, Backtracking<HammingDistance, TSpec> > & finder,
     TPattern const & pattern,
     TDelegate && delegate)
{
    typedef Index<TText, TIndexSpec>                                        TIndex;
    typedef Backtracking<HammingDistance, TSpec>                            TFinderSpec;
    typedef typename TextIterator_<TIndex, TPattern, TFinderSpec>::Type     TTextIterator;
    typedef typename PatternIterator_<TIndex, TPattern, TFinderSpec>::Type  TPatternIterator;

    setPatternIterator(finder, begin(pattern));

    TTextIterator & textIt = textIterator(finder);
    TPatternIterator & patternIt = patternIterator(finder);

    do
    {
        // Exact case.
        if (finder._score == finder._scoreThreshold)
        {
            if (goDown(textIt, suffix(pattern, position(patternIt))))
            {
                delegate(finder);
            }

            goUp(textIt);

            // Termination.
            if (isRoot(textIt)) break;
        }

        // Approximate case.
        else if (finder._score < finder._scoreThreshold)
        {
            // Base case.
            if (atEnd(patternIt))
            {
                delegate(finder);
            }

            // Recursive case.
            else if (goDown(textIt))
            {
                finder._score += _getVertexScore(finder);
                goNext(patternIt);
                continue;
            }
        }

        // Backtrack.
        do
        {
            // Termination.
            if (isRoot(textIt)) break;

            goPrevious(patternIt);
            finder._score -= _getVertexScore(finder);
        }
        while (!goRight(textIt) && goUp(textIt));

        // Termination.
        if (isRoot(textIt)) break;

        finder._score += _getVertexScore(finder);
        goNext(patternIt);
    }
    while (true);
}

template <typename TText, typename TTextIndexSpec, typename TPattern, typename TPatternIndexSpec, typename TDistance, typename TSpec, typename TValue, typename TDelegate>
inline void
find(Finder2<Index<TText, TTextIndexSpec>, Index<TPattern, TPatternIndexSpec>, Backtracking<TDistance, TSpec> > & finder,
     Index<TText, TTextIndexSpec> & text,
     Index<TPattern, TPatternIndexSpec> & pattern,
     TValue maxScore,
     TDelegate && delegate)
{
    typedef Index<TText, TTextIndexSpec>                                                TTextIndex;
    typedef Index<TPattern, TPatternIndexSpec>                                          TPatternIndex;
    typedef Backtracking<TDistance, TSpec>                                              TBacktracking;
    typedef typename TextIterator_<TTextIndex, TPatternIndex, TBacktracking>::Type      TTextIterator;
    typedef typename PatternIterator_<TTextIndex, TPatternIndex, TBacktracking>::Type   TPatternIterator;

    TTextIterator textIt(text);
    TPatternIterator patternIt(pattern);

    setMaxScore(finder, maxScore);
    _initState(finder, textIt, patternIt);
    _find(finder, delegate, StageInitial_());
    _popState(finder, StageInitial_());
}

// ----------------------------------------------------------------------------
// Function buildTrie()
// ----------------------------------------------------------------------------

template <typename TIndex, typename TText>
inline void buildTrie(TIndex & index, TText const & text)
{
//    // Construct index using quicksort.
//    QGramLess_<TIndexSAPos, TText const> less(db.text, seedLength);
//    std::sort(begin(sa, Standard()), end(sa, Standard()), less);
}

// ----------------------------------------------------------------------------
// Function locateOccurrences()
// ----------------------------------------------------------------------------

template <typename TSA, typename TSARange>
inline typename Size<TSA>::Type
locateOccurrences(TSA const & sa, TSARange const & saRange, True)
{
    typedef typename Value<TSARange, 1>::Type   TSAPos;
    typedef typename Size<TSA>::Type            TSASize;

    TSASize checkSum = 0;
    for (TSAPos saPos = getValueI1(saRange); saPos < getValueI2(saRange); ++saPos)
        checkSum += getSeqOffset(sa[saPos]);
    return checkSum;
}

template <typename TSA, typename TSARange>
inline typename Size<TSA>::Type
locateOccurrences(TSA const & /* sa */, TSARange const & /* saRange */, False)
{
    return 0;
}

// ----------------------------------------------------------------------------
// Function countOccurrences()
// ----------------------------------------------------------------------------

template <typename TIndex, typename TQueries, typename TLocate, typename TFinderSpec>
inline typename Size<TIndex>::Type
countOccurrences(Options const & options, TIndex & index, TQueries & queries, TLocate, TFinderSpec, Serial)
{
    typedef typename Value<TQueries>::Type                  TPattern;
    typedef Finder2<TIndex, TPattern, TFinderSpec>          TFinder;
    typedef typename Size<TIndex>::Type                     TSize;
    typedef typename Size<TQueries>::Type                   TQueriesSize;

    TFinder finder(index);
    setScoreThreshold(finder, options.errors);

    TSize count = 0;
    TSize volatile checkSum = 0;

    for (TQueriesSize i = 0; i < length(queries); ++i)
    {
        find(finder, queries[i],
             [&index, &count, &checkSum](TFinder const & finder)
             {
                count += countOccurrences(textIterator(finder));
                checkSum += locateOccurrences(indexSA(index), range(textIterator(finder)), TLocate());
             });
        clear(finder);
    }

    return count;
}

template <typename TIndex, typename TNeedles, typename TLocate, typename TFinderSpec>
inline typename Size<TIndex>::Type
countOccurrences(Options const & options, TIndex & index, TNeedles & needles, TLocate, TFinderSpec, DfsPreorder)
{
    typedef Index<TNeedles, IndexSa<> >                     TPattern;
    typedef Finder2<TIndex, TPattern, TFinderSpec>          TFinder;
    typedef typename Size<TIndex>::Type                     TSize;

    TFinder finder;
    TPattern pattern;
    buildTrie(pattern, needles);

//    setMaxScore(finder, options.errors);

    TSize count = 0;
    TSize volatile checkSum = 0;

    find(finder, index, pattern, options.errors,
         [&index, &count, &checkSum](TFinder const & finder)
         {
            count += countOccurrences(back(finder.textStack)) * countOccurrences(back(finder.patternStack));
            checkSum += locateOccurrences(indexSA(index), range(back(finder.textStack)), TLocate());
         });
    clear(finder);

    return count;
}

template <typename TText, typename TSpec, typename TQueries, typename TLocate>
inline typename Size<Index<TText, IndexEsa<TSpec> > >::Type
countOccurrences(Options const &, Index<TText, IndexEsa<TSpec> > &, TQueries const &, TLocate, Backtracking<HammingDistance>, Serial)
{
    throw RuntimeError("Unsupported index type");
    return 0;
}

template <typename TText, typename TShape, typename TSpec, typename TQueries, typename TLocate>
inline typename Size<Index<TText, IndexQGram<TShape, TSpec> > >::Type
countOccurrences(Options const &, Index<TText, IndexQGram<TShape, TSpec> > &, TQueries const &, TLocate, Backtracking<HammingDistance>, Serial)
{
    throw RuntimeError("Unsupported index type");
    return 0;
}

// ----------------------------------------------------------------------------
// Function countOccurrences()
// ----------------------------------------------------------------------------

template <typename TIndex, typename TQueries, typename TLocate, typename TFinderSpec>
inline typename Size<TIndex>::Type
countOccurrences(Options const & options, TIndex & index, TQueries & queries, TLocate const & locateTag, TFinderSpec const & finderSpec)
{
    switch (options.algorithmType)
    {
    case Options::ALGO_SINGLE:
        return countOccurrences(options, index, queries, locateTag, finderSpec, Serial());

    case Options::ALGO_DFS:
        return countOccurrences(options, index, queries, locateTag, finderSpec, DfsPreorder());

//    case Options::ALGO_BFS:
//        return countOccurrences(options, index, queries, locateTag, finderSpec, BfsIterator());

    default:
        throw RuntimeError("Unsupported search algorithm");
    }
}

template <typename TIndex, typename TQueries, typename TLocate>
inline typename Size<TIndex>::Type
countOccurrences(Options const & options, TIndex & index, TQueries & queries, TLocate const & tag)
{
//    if (options.errors)
        return countOccurrences(options, index, queries, tag, Backtracking<HammingDistance>());
//    else
//        return countOccurrences(options, index, queries, tag, Backtracking<Exact>());
}

template <typename TIndex, typename TQueries>
inline typename Size<TIndex>::Type
countOccurrences(Options const & options, TIndex & index, TQueries & queries)
{
    if (options.locate)
        return countOccurrences(options, index, queries, True());
    else
        return countOccurrences(options, index, queries, False());
}

// ----------------------------------------------------------------------------
// Function run()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TLimits, typename TSetLimits, typename TIndexSpec>
inline void run(Options const & options)
{
    double start, finish;

    typedef typename TextCollection<TAlphabet, Limits<__uint8> >::Type      TQueries;
    typedef typename TextCollection<TAlphabet, TLimits, TSetLimits>::Type   TText;
    typedef Index<TText, TIndexSpec>                                        TIndex;

    TIndex index;

    if (!open(index, toCString(options.textIndexFile)))
        throw RuntimeError("Error while loading full-text index");

    TQueries queries;

    if (!open(queries, toCString(options.queryFile)))
        throw RuntimeError("Error while loading queries");

    start = sysTime();
    unsigned long occurrences = countOccurrences(options, index, queries);
    finish = sysTime();

    std::cout << length(queries) << " queries" << std::endl;
    std::cout << lengthSum(queries) << " symbols" << std::endl;
    std::cout << occurrences << " occurrences" << std::endl;
    std::cout << finish - start << " sec" << std::endl;

//    std::cout << countOcc(index, queries) / (double)length(queries) << " occurrences/query" << std::endl;
//    std::cout << (unsigned)(length(queries) / (finish - start)) << " query/sec" << std::endl;
}

int main(int argc, char const ** argv)
{
    return run<Options>(argc, argv);
}
