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

    Options() :
        BaseOptions(),
        algorithmType(ALGO_SINGLE),
        errors(0)
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
//    setAlgorithmType(parser, options);
    addOption(parser, ArgParseOption("e", "errors", "Number of errors.", ArgParseOption::INTEGER));
    setMinValue(parser, "errors", "0");
    setMaxValue(parser, "errors", "5");
    setDefaultValue(parser, "errors", options.errors);
}

// ----------------------------------------------------------------------------
// Function parseCommandLine()
// ----------------------------------------------------------------------------

template <typename TOptions>
ArgumentParser::ParseResult
inline parseCommandLine(TOptions & options, ArgumentParser & parser, int argc, char const ** argv)
{
    ArgumentParser::ParseResult res = parse(parser, argc, argv);

    if (res != seqan::ArgumentParser::PARSE_OK)
        return res;

    getArgumentValue(options.textIndexFile, parser, 0);
    getArgumentValue(options.queryFile, parser, 1);

    getAlphabetType(options, parser);
    getIndexType(options, parser);
    getTextLimits(options, parser);
//    getAlgorithmType(options, parser);
    getOptionValue(options.errors, parser, "errors");

    return seqan::ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function run()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TLimits, typename TSetLimits, typename TIndexSpec>
inline void run(Options & options)
{
    double start, finish;

    typedef typename TextCollection<TAlphabet>::Type                        TQuery;
    typedef typename TextCollection<TAlphabet, TLimits, TSetLimits>::Type   TText;
    typedef Index<TText, TIndexSpec>                                        TIndex;

    TIndex index;

    if (!open(index, toCString(options.textIndexFile)))
        throw RuntimeError("Error while loading full-text index");

    TQuery queries;

    if (!open(queries, toCString(options.queryFile)))
        throw RuntimeError("Error while loading queries");

    start = sysTime();
    std::cout << countOccurrences(index, queries) << std::endl;
    finish = sysTime();
    std::cout << finish - start << " sec" << std::endl;
}

int main(int argc, char const ** argv)
{
    return run<Options>(argc, argv);
}
