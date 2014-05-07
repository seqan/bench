// ==========================================================================
//                 SeqAn - The Library for Sequence Analysis
// ==========================================================================
// Copyright (c) 2013 NVIDIA Corporation
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
//     * Neither the name of NVIDIA Corporation nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE
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
// Approximate string matching with suffix filters.
// ==========================================================================

#ifndef SEQAN_EXTRAS_FIND_SUFFIXFILTER_H_
#define SEQAN_EXTRAS_FIND_SUFFIXFILTER_H_

//#define SEQAN_DEBUG

namespace seqan {

namespace ext {

// ============================================================================
// Forwards
// ============================================================================

template <typename TDistance, typename TSpec>
struct SuffixFilter;

// ============================================================================
// Metafunctions
// ============================================================================

// ============================================================================
// Tags, Classes, Enums
// ============================================================================

// ----------------------------------------------------------------------------
// Tags for suffix filter specializations
// ----------------------------------------------------------------------------

struct SuffixFilterSemiGlobal_;
typedef Tag<BacktrackingSemiGlobal_>    SuffixFilterSemiGlobal;

struct SuffixFilterGlobal_;
typedef Tag<SuffixFilterGlobal_>        SuffixFilterGlobal;

// ----------------------------------------------------------------------------
// Class SuffixFilter
// ----------------------------------------------------------------------------

template <typename TDistance = HammingDistance, typename TSpec = SuffixFilterSemiGlobal>
struct SuffixFilter {};

// ----------------------------------------------------------------------------
// Class Finder
// ----------------------------------------------------------------------------

template <typename TText, typename TPattern, typename TDelegate, typename TDistance, typename TSpec>
struct Finder<TText, TPattern, TDelegate, SuffixFilter<TDistance, TSpec> >
{
    typedef Finder<TText, TPattern, TDelegate, SuffixFilter<TDistance, TSpec> > TFinder;
    typedef typename Iterator<TPattern, FactorsIterator>::Type                  TFactorsIterator;
    typedef Segment<TPattern, InfixSegment>                                     TFactor;
    typedef Backtracking<TDistance, TSpec>                                      TBacktracking;
    typedef Finder<TText, TFactor, TFinder, TBacktracking>                      TFactorFinder;
    typedef typename Score_<TBacktracking>::Type                                TScore;

    TFactor                 _factor;
    TFactorsIterator        _factorBegin;
    TFactorsIterator        _factorEnd;
    TScore                  _factorMaxScore;
    TFactorFinder           _factorFinder;
//    TScore                  maxScore;
    TDelegate               & delegate;

    // TODO(esiragusa): Do not pass *this to _factorFinder constructor!
    Finder(TDelegate & delegate) :
        _factor(),
        _factorBegin(),
        _factorEnd(),
        _factorMaxScore(),
        _factorFinder(*this),
//        maxScore(0),
        delegate(delegate)
    {}
};

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function onFind()                                                   [Finder]
// ----------------------------------------------------------------------------

template <typename TText, typename TPattern, typename TDelegate, typename TDistance, typename TSpec, typename TFactor>
inline void
onFind(Finder<TText, TPattern, TDelegate, SuffixFilter<TDistance, TSpec> > & finder,
       Finder<TText, TFactor, Finder<TText, TPattern, TDelegate, SuffixFilter<TDistance, TSpec> >,
               Backtracking<TDistance, TSpec> > const & /* _factorFinder */)
{
    typedef Backtracking<TDistance, TSpec>                                      TBacktracking;
    typedef typename PatternIterator_<TFactor, TBacktracking>::Type             TFactorIterator;

    // If the whole suffix has been searched then report a match.
    if (atEnd(finder._factorEnd))
    {
        _report(finder);
    }
    else
    {
        // Try to extend to the next factor.
        goNext(finder._factorEnd);
        setEnd(finder._factor, finder._factorEnd.data_iterator);

        // Increase the threshold.
        finder._factorMaxScore++;
        setMaxScore(finder._factorFinder, finder._factorMaxScore);

        // Initialize the backtracking finder.
        _pushIterators(finder._factorFinder, Nothing());
        _pushScore(finder._factorFinder, Nothing());

        // NOTE(esiragusa): Workaround for broken Rooted Iterator over Infix.
        TFactorIterator factorIt(finder._factor);
        setPosition(factorIt.data_iterator, position(_patternIterator(finder._factorFinder).data_iterator));
        _patternIterator(finder._factorFinder) = factorIt;

#ifdef SEQAN_DEBUG
        std::cout << finder._factor << " / " << (unsigned)finder._factorMaxScore << std::endl;
#endif

        // Find next factor.
        _find(finder._factorFinder, StageInitial_());

        // Reset finder to the previous state.
        _popState(finder._factorFinder, StageInitial_());

        // Decrease the threshold.
        finder._factorMaxScore--;
        setMaxScore(finder._factorFinder, finder._factorMaxScore);

        // Go back to the previous factor.
        goPrevious(finder._factorEnd);
        setEnd(finder._factor, finder._factorEnd.data_iterator);
    }
}

// ----------------------------------------------------------------------------
// Function getScore()                                                 [Finder]
// ----------------------------------------------------------------------------

template <typename TText, typename TPattern, typename TDelegate, typename TDistance, typename TSpec>
inline typename Score_<Backtracking<TDistance, TSpec> >::Type
getScore(Finder<TText, TPattern, TDelegate, SuffixFilter<TDistance, TSpec> > const & finder)
{
    return getScore(finder._factorFinder);
}

// ----------------------------------------------------------------------------
// Function find()                                                     [Finder]
// ----------------------------------------------------------------------------

template <typename TText, typename TPattern, typename TDelegate, typename TDistance, typename TSpec,
          typename TFactorsIterator, typename TValue>
inline void
find(Finder<TText, TPattern, TDelegate, SuffixFilter<TDistance, TSpec> > & finder,
     TText & text,
     TPattern & pattern,
     TFactorsIterator const & factorsIt,
     TValue /* maxScore */)
{
    typedef Segment<TPattern, InfixSegment>                                     TFactor;
    typedef Backtracking<TDistance, TSpec>                                      TBacktracking;
    typedef typename TextIterator_<TText, TBacktracking>::Type                  TTextIterator;
    typedef typename PatternIterator_<TFactor, TBacktracking>::Type             TFactorIterator;

//    setMaxScore(finder, maxScore);
//    finder.maxScore = maxScore;

    TTextIterator textIt(text);

    setHost(finder._factor, pattern);

    finder._factorBegin = factorsIt;
    finder._factorEnd = factorsIt;

    // Iterate over all factors of the pattern.
    while (!atEnd(finder._factorBegin))
    {
        goNext(finder._factorEnd);

        setBegin(finder._factor, finder._factorBegin.data_iterator);
        setEnd(finder._factor, finder._factorEnd.data_iterator);

        TFactorIterator factorIt(finder._factor);

        // Start from threshold 0.
        finder._factorMaxScore = 0;
        setMaxScore(finder._factorFinder, finder._factorMaxScore);

        // Start at the root of the text index.
        _initState(finder._factorFinder, textIt, factorIt);

#ifdef SEQAN_DEBUG
        std::cout << finder._factor << " / " << (unsigned)finder._factorMaxScore << std::endl;
#endif

        // Find the suffix.
        _find(finder._factorFinder, StageInitial_());

        // Reset the finder.
        _popState(finder._factorFinder, StageInitial_());

        // Get next factor.
        goNext(finder._factorBegin);
    }
}

}

}

#endif  // #ifndef SEQAN_EXTRAS_FIND_SUFFIXFILTER_H_
