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

using namespace seqan;

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function setupArgumentParser()
// ----------------------------------------------------------------------------

inline void setupArgumentParser(ArgumentParser & parser, Options const & options)
{
    setAppName(parser, "iBench Dump");
    setShortDescription(parser, "Dump any sequence file as a StringSet");
    setCategory(parser, "Stringology");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] <\\fITEXT FILE\\fP> <\\fIOUTPUT FILE\\fP>");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));

    setAlphabetType(parser, options);
}

// ----------------------------------------------------------------------------
// Function parseCommandLine()
// ----------------------------------------------------------------------------

ArgumentParser::ParseResult
inline parseCommandLine(Options & options, ArgumentParser & parser, int argc, char const ** argv)
{
    ArgumentParser::ParseResult res = parse(parser, argc, argv);

    if (res != seqan::ArgumentParser::PARSE_OK)
        return res;

    getArgumentValue(options.textFile, parser, 0);
    getArgumentValue(options.queryFile, parser, 1);

    getAlphabetType(options, parser);

    return seqan::ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function randomize()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TString>
inline void randomize(TString & me)
{
    typedef typename Iterator<TString, Standard>::Type      TIter;
    typedef typename Value<TString>::Type                   TSourceAlphabet;

    Rng<MersenneTwister> rng(0xBADC0FFE);

    TIter it = begin(me, Standard());
    TIter itEnd = end(me, Standard());

    while (it != itEnd)
    {
        for (; it != itEnd && value(it) != TSourceAlphabet('N'); ++it) ;

        if (it == itEnd) break;

        for (; it != itEnd && value(it) == TSourceAlphabet('N'); ++it)
            value(it) = pickRandomNumber(rng) % ValueSize<TAlphabet>::VALUE;
    }
}

// ----------------------------------------------------------------------------
// Function readAll(Dna StringSet)
// ----------------------------------------------------------------------------

template <typename TId, typename TIdSpec, typename TSeqSpec>
inline int readAll(StringSet<TId, TIdSpec> & ids, StringSet<String<Dna>, TSeqSpec> & seqs, SequenceStream & seqStream)
{
    typedef StringSet<String<Dna5>, Owner<ConcatDirect<> > >   TText;

    TText text;

    int ret = readAll(ids, text, seqStream);

    if (ret)
        return ret;

    randomize<Dna>(concat(text));
    assign(seqs, text);

    return ret;
}

// ----------------------------------------------------------------------------
// Function run()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TIndexSpec>
inline void run(Options & options)
{
    typedef StringSet<CharString, Owner<ConcatDirect<> > >          TCharStringSet;
    typedef StringSet<String<TAlphabet>, Owner<ConcatDirect<> > >   TText;

    SequenceStream seqStream(toCString(options.textFile));

    TText text;
    TCharStringSet ids;

    if (readAll(ids, text, seqStream) != 0)
        throw RuntimeError("Error while reading text");

    if (!save(text, toCString(options.queryFile)))
        throw RuntimeError("Error while saving text");
}

int main(int argc, char const ** argv)
{
    return run(argc, argv);
}
