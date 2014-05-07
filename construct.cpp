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

struct Options : BaseOptions {};

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function setupArgumentParser()
// ----------------------------------------------------------------------------

template <typename TOptions>
inline void setupArgumentParser(ArgumentParser & parser, TOptions const & options)
{
    setAppName(parser, "iBench Construct");
    setShortDescription(parser, "Benchmark full-text index construction");
    setCategory(parser, "Stringology");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] <\\fITEXT FILE\\fP> <\\fIINDEX FILE\\fP>");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));

    addSection(parser, "Main Options");
    setAlphabetType(parser, options);
    setIndexType(parser, options);
    setTextLimits(parser, options);
//    setTmpFolder(parser);
}

// ----------------------------------------------------------------------------
// Function parseCommandLine()
// ----------------------------------------------------------------------------

template <typename TOptions>
inline ArgumentParser::ParseResult
parseCommandLine(TOptions & options, ArgumentParser & parser, int argc, char const ** argv)
{
    ArgumentParser::ParseResult res = parse(parser, argc, argv);

    if (res != ArgumentParser::PARSE_OK)
        return res;

    getArgumentValue(options.textFile, parser, 0);
    getArgumentValue(options.textIndexFile, parser, 1);

    getAlphabetType(options, parser);
    getIndexType(options, parser);
    getTextLimits(options, parser);
//    getTmpFolder(options, parser);

    return ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function construct()
// ----------------------------------------------------------------------------

template <typename TIndex>
inline void construct(TIndex & index)
{
    typedef typename Iterator<TIndex, TopDown<> >::Type TIter;

    TIter it(index);
    ignoreUnusedVariableWarning(it);
}

template <typename TText, typename TSpec, typename TConfig>
inline void construct(Index<TText, FMIndex<TSpec, TConfig> > & index)
{
    typedef Index<TText, FMIndex<TSpec, TConfig> >      TIndex;
    typedef typename Iterator<TIndex, TopDown<> >::Type TIter;

    reverse(indexText(index));
    TIter it(index);
    ignoreUnusedVariableWarning(it);
}

// ----------------------------------------------------------------------------
// Function run()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TLimits, typename TSetLimits, typename TIndexSpec>
inline void run(Options & options)
{
    typedef typename TextCollection<TAlphabet, TLimits, TSetLimits>::Type   TText;
    typedef Index<TText, TIndexSpec>                                        TIndex;

    TText text;

    if (!open(text, toCString(options.textFile)))
        throw RuntimeError("Error while loading text");

    TIndex index(text);

    double start = sysTime();
    construct(index);
    double finish = sysTime();
    std::cout << (unsigned long)length(text) << " texts" << std::endl;
    std::cout << lengthSum(text) << " symbols" << std::endl;
    std::cout << finish - start << " sec" << std::endl;
//    std::cout << (unsigned)(lengthSum(text) / (finish - start)) << " symbols/sec" << std::endl;

    if (!save(index, toCString(options.textIndexFile)))
        throw RuntimeError("Error while saving full-text index");
}

int main(int argc, char const ** argv)
{
    return run<Options>(argc, argv);
}
