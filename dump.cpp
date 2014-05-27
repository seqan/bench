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
#include <seqan/seq_io.h>
#include <seqan/random.h>
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
// Class Options
// ----------------------------------------------------------------------------

struct Options : BaseOptions
{
    CharString      outputFile;
    unsigned        limitLength;
    unsigned        limitCount;

    Options() :
        BaseOptions(),
        limitLength(0),
        limitCount(0)
    {}
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
    setAppName(parser, "iBench Dump");
    setShortDescription(parser, "Dump any sequence file as a StringSet");
    setCategory(parser, "Stringology");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] <\\fITEXT FILE\\fP> <\\fIOUTPUT FILE\\fP>");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));

    addSection(parser, "Main Options");
    setAlphabetType(parser, options);
    setTextLimits(parser, options);

    addSection(parser, "Dump Options");
    addOption(parser, ArgParseOption("ll", "limit-length", "Limit the length of the texts.", ArgParseOption::INTEGER));
    setDefaultValue(parser, "limit-length", options.limitLength);
    addOption(parser, ArgParseOption("lc", "limit-count", "Limit the number of texts.", ArgParseOption::INTEGER));
    setDefaultValue(parser, "limit-count", options.limitLength);
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

    getArgumentValue(options.textFile, parser, 0);
    getArgumentValue(options.outputFile, parser, 1);

    getAlphabetType(options, parser);
    getTextLimits(options, parser);
    getOptionValue(options.limitLength, parser, "limit-length");
    getOptionValue(options.limitLength, parser, "limit-count");

    return ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function randomize()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TString>
inline void randomize(TString & me)
{
    typedef typename Iterator<TString, Standard>::Type      TIter;
    typedef typename Value<TString>::Type                   TValue;

    Rng<MersenneTwister> rng(0xBADC0FFE);

    TIter it = begin(me, Standard());
    TIter itEnd = end(me, Standard());

    while (it != itEnd)
    {
        for (; it != itEnd && value(it) == TValue(TAlphabet(value(it))); ++it) ;

        if (it == itEnd) break;

        for (; it != itEnd && value(it) != TValue(TAlphabet(value(it))); ++it)
            value(it) = TAlphabet(pickRandomNumber(rng) % ValueSize<TAlphabet>::VALUE);
    }
}

// ----------------------------------------------------------------------------
// Function run()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TLimits, typename TSetLimits, typename TIndexSpec>
inline void run(Options & options)
{
    typedef typename TextCollection<TAlphabet, TLimits, TSetLimits>::Type   TText;
    typedef StringSet<CharString, Owner<ConcatDirect<> > >                  TCharStringSet;

    SequenceStream seqStream(toCString(options.textFile));

    TCharStringSet seqs;
    CharString  seq;
    CharString  id;

    while (!atEnd(seqStream))
    {
        if (readRecord(id, seq, seqStream) != 0)
            throw RuntimeError("Error while reading text");

        if (options.limitLength > 0 && options.limitLength < length(seq))
            resize(seq, options.limitLength);

        appendValue(seqs, seq, Generous());

        if (options.limitCount > 0 && options.limitCount <= length(seqs))
            break;
    }

    if (maxLength(seqs) > MaxValue<typename Value<TLimits, 1>::Type>::VALUE)
        throw RuntimeError("Too long sequences");

    if (length(seqs) > MaxValue<typename Value<TSetLimits, 1>::Type>::VALUE)
        throw RuntimeError("Too many sequences");

    if (lengthSum(seqs) > MaxValue<typename Value<TSetLimits, 2>::Type>::VALUE)
        throw RuntimeError("Too many symbols");

    if (IsSameType<TAlphabet, Dna>::VALUE)
        randomize<Dna>(concat(seqs));

    TText text;
    assign(text, seqs);

    if (!save(text, toCString(options.outputFile)))
        throw RuntimeError("Error while saving text");
}

int main(int argc, char const ** argv)
{
    return run<Options>(argc, argv);
}
