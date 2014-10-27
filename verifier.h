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

#ifndef APP_BENCH_VERIFIER_H_
#define APP_BENCH_VERIFIER_H_

using namespace seqan;

// ============================================================================
// Classes
// ============================================================================

// ----------------------------------------------------------------------------
// Class Extender
// ----------------------------------------------------------------------------

template <typename THaystack, typename TNeedle>
struct Extender<THaystack, TNeedle, EditDistance, NMatchesN_>
{
    typedef typename Infix<THaystack const>::Type       THaystackInfix;
    typedef ModifiedString<THaystackInfix, ModReverse>  THaystackInfixRev;
    typedef typename Infix<TNeedle const>::Type         TNeedleInfix;
    typedef ModifiedString<TNeedleInfix, ModReverse>    TNeedleInfixRev;

    typedef MyersUkkonenGlobalBanded                    TAlgorithm;
    typedef Finder<THaystackInfix>                      TFinderRight;
    typedef Finder<THaystackInfixRev>                   TFinderLeft;
    typedef PatternState_<TNeedleInfix, TAlgorithm>     TPatternRight;
    typedef PatternState_<TNeedleInfixRev, TAlgorithm>  TPatternLeft;

    THaystack const &   haystack;
    TPatternRight       patternRight;
    TPatternLeft        patternLeft;

    Extender(THaystack const & haystack) :
        haystack(haystack)
    {}
};

// ----------------------------------------------------------------------------
// Class Verifier
// ----------------------------------------------------------------------------

template <typename THaystack, typename TNeedle, typename TSpec = Nothing>
struct Verifier {};

// ----------------------------------------------------------------------------
// Class Verifier<EditDistance>
// ----------------------------------------------------------------------------

template <typename THaystack, typename TNeedle>
struct Verifier<THaystack, TNeedle, EditDistance>
{
    typedef typename Infix<THaystack const>::Type       THaystackInfix;
    typedef ModifiedString<THaystackInfix, ModReverse>  THaystackInfixRev;
    typedef typename Infix<TNeedle const>::Type         TNeedleInfix;
    typedef ModifiedString<TNeedleInfix, ModReverse>    TNeedleInfixRev;

    typedef MyersUkkonenBanded                          TAlgoEnd;
    typedef MyersUkkonenGlobalBanded                    TAlgoBegin;
    typedef Finder<THaystackInfix>                      TFinderEnd;
    typedef Finder<THaystackInfixRev>                   TFinderBegin;
    typedef PatternState_<TNeedleInfix, TAlgoEnd>       TPatternEnd;
    typedef PatternState_<TNeedleInfixRev, TAlgoBegin>  TPatternBegin;

    TPatternEnd         patternEnd;
    TPatternBegin       patternBegin;
};

// ----------------------------------------------------------------------------
// Function verify<Nothing>()
// ----------------------------------------------------------------------------

template <typename THaystack, typename TNeedle, typename THaystackInfix, typename TNeedleInfix, typename TErrors, typename TDelegate>
inline void
verify(Verifier<THaystack, TNeedle, Nothing> & /* verifier */,
       THaystackInfix const & /* haystackInfix */,
       TNeedleInfix const & /* needle */,
       TErrors /* threshold */,
       TDelegate && /* delegate */)
{}

// ----------------------------------------------------------------------------
// Function verify<HammingDistance>()
// ----------------------------------------------------------------------------

template <typename THaystack, typename TNeedle, typename THaystackInfix, typename TNeedleInfix, typename TErrors, typename TDelegate>
inline void
verify(Verifier<THaystack, TNeedle, HammingDistance> & /* verifier */,
       THaystackInfix const & haystackInfix,
       TNeedleInfix const & needleInfix,
       TErrors threshold,
       TDelegate && delegate)
{
    typedef typename Position<THaystackInfix>::Type             THaystackPos;
    typedef typename Iterator<THaystackInfix, Standard>::Type   THaystackIt;
    typedef typename Iterator<TNeedleInfix, Standard>::Type     TNeedleIt;
    typedef typename Size<TNeedleInfix>::Type                   TNeedleSize;

    if (length(haystackInfix) < length(needleInfix)) return;

//    THaystackPos endPos = length(haystackInfix);
//    TErrors minErrors = threshold + 1;

    TNeedleSize nLength = length(needleInfix);
    THaystackIt iIt = begin(haystackInfix, Standard());
    THaystackIt iEnd = end(haystackInfix, Standard()) - nLength + 1;

    for (; iIt != iEnd; ++iIt)
    {
        THaystackIt hIt = iIt;
        THaystackIt hEnd = iIt + nLength;
        TNeedleIt nIt = begin(needleInfix, Standard());
        TErrors currentErrors = 0;

        for (; hIt != hEnd && currentErrors <= threshold; ++hIt, ++nIt)
            currentErrors += !ordEqual(value(hIt), value(nIt));

        THaystackPos endPos = position(hEnd, haystackInfix);

        if (currentErrors <= threshold)
            delegate(infix(haystackInfix, endPos - nLength, endPos), currentErrors);
    }
}

// ----------------------------------------------------------------------------
// Function verify<EditDistance>()
// ----------------------------------------------------------------------------

template <typename THaystack, typename TNeedle, typename THaystackInfix, typename TNeedleInfix, typename TErrors, typename TDelegate>
inline void
verify(Verifier<THaystack, TNeedle, EditDistance> & verifier,
       THaystackInfix const & haystackInfix,
       TNeedleInfix const & needleInfix,
       TErrors threshold,
       TDelegate && delegate)
{
    typedef Verifier<THaystack, TNeedle, EditDistance>  TVerifier;
    typedef typename TVerifier::TFinderEnd              TFinderEnd;
    typedef typename Position<THaystackInfix>::Type     THaystackPos;

    TFinderEnd finderEnd;
    setHost(finderEnd, haystackInfix);

    THaystackPos beginPos = 0;
    THaystackPos endPos = length(haystackInfix);
    TErrors minErrors = threshold + 1;

    while (find(finderEnd, needleInfix, verifier.patternEnd, -static_cast<int>(threshold)))
    {
        TErrors currentErrors = -getScore(verifier.patternEnd);

        if (currentErrors <= minErrors)
        {
            minErrors = currentErrors;
            endPos = position(finderEnd) + 1;
        }
    }

    if (minErrors <= threshold)
        delegate(infix(haystackInfix, beginPos, endPos), minErrors);
}

// ----------------------------------------------------------------------------
// Function collectMatch()
// ----------------------------------------------------------------------------

template <typename TMatches, typename TNeedleId, typename THaystackPos, typename TErrors>
inline void collectMatch(TMatches & matches,
                         TNeedleId needleId,
                         THaystackPos const & matchBegin,
                         THaystackPos const & matchEnd,
                         TErrors matchErrors,
                         True)
{
    typedef typename Value<TMatches>::Type TMatch;

    TMatch match;
    setContigPosition(match, matchBegin, matchEnd);
    match.readId = needleId;
    match.errors = matchErrors;
    appendValue(matches, match, Generous());
}

template <typename TMatches, typename TNeedleId, typename THaystackPos, typename TErrors>
inline void collectMatch(TMatches & /* matches */,
                         TNeedleId /* needleId */,
                         THaystackPos const & /* matchBegin */,
                         THaystackPos const & /* matchEnd */,
                         TErrors /* matchErrors */,
                         False)
{}

// ----------------------------------------------------------------------------
// Function getLocalPos()
// ----------------------------------------------------------------------------

template <typename TText, typename TInfix>
inline typename StringSetPosition<TText const>::Type
getLocalPos(TText const & text, TInfix const & infix, True)
{
    typename StringSetPosition<TText const>::Type pos;
    posLocalize(pos, beginPosition(infix), stringSetLimits(text));
    return pos;
}

template <typename TText, typename TInfix>
inline typename StringSetPosition<TText const>::Type
getLocalPos(TText const & /* text */, TInfix const & /* infix */, False)
{
    return typename StringSetPosition<TText const>::Type();
}

#endif  // #ifndef APP_BENCH_VERIFIER_H_
