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

#ifndef APP_IBENCH_FIND_H_
#define APP_IBENCH_FIND_H_

using namespace seqan;

// ============================================================================
// Functions
// ============================================================================

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
                delegate(finder);

            goUp(textIt);
        }

        // Approximate case.
        else if (finder._score < finder._scoreThreshold)
        {
            // Base case.
            if (atEnd(patternIt))
                delegate(finder);

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

template <typename TText, typename TTextIndexSpec, typename TPattern, typename TPatternIndexSpec, typename TDistance,
          typename TSpec, typename TValue, typename TDelegate>
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

    setScoreThreshold(finder, maxScore);
    _initState(finder, textIt, patternIt);
    _find(finder, delegate, StageInitial_());
    _popState(finder, StageInitial_());
}


#endif  // #ifndef APP_IBENCH_FIND_H_
