// ==========================================================================
//                 SeqAn - The Library for Sequence Analysis
// ==========================================================================
// Copyright (c) 2006-2014, Knut Reinert, FU Berlin
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
// ARE DISCLAIMED. IN NO EVENT SHALL KNUT REINERT OR THE FU BERLIN BE LIABLE
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
// Approximate string matching via backtracking on VSTrees
// ==========================================================================

#ifndef APP_BENCH_FIND_BACKTRACKING_DFS_H_
#define APP_BENCH_FIND_BACKTRACKING_DFS_H_

//#define SEQAN_DEBUG

namespace seqan {

// ============================================================================
// Classes
// ============================================================================

// ----------------------------------------------------------------------------
// Class Finder_
// ----------------------------------------------------------------------------

template <typename THaystack, typename TTextSpec, typename TNeedle, typename TPatternSpec>
struct Finder_<Index<THaystack, TTextSpec>, Index<TNeedle, TPatternSpec>, Backtracking<HammingDistance, DfsPreorder> > :
    Finder_<Index<THaystack, TIndexSpec>, Index<TNeedle, TPatternSpec>, void>
{
    typedef Finder_<Index<THaystack, TIndexSpec>, Index<TNeedle, TPatternSpec>, void>       TBase;

    Finder_() :
        TBase()
    {}
};

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function _getVertexScore()
// ----------------------------------------------------------------------------

template <typename THaystack, typename TTextSpec, typename TNeedle, typename TPatternSpec, typename TDistance, typename TSpec>
SEQAN_HOST_DEVICE inline typename Score_<Backtracking<TDistance, TSpec> >::Type
_getVertexScore(Finder_<Index<THaystack, TTextSpec>, Index<TNeedle, TPatternSpec>, Backtracking<TDistance, TSpec> > const & finder)
{
    return !ordEqual(parentEdgeLabel(_textIterator(finder)), parentEdgeLabel(_patternIterator(finder)));
}

//template <typename TTextIterator, typename TPatternIterator, typename TSize, typename TErrors, typename TDelegate>
//inline void _iterateDfs(TTextIterator textIt,
//                 TPatternIterator patternIt,
//                 TSize const patternsLength,
//                 TErrors const maxErrors,
//                 TSize currentDepth,
//                 TErrors errors,
//                 TDelegate & delegate)
//{
//    typedef typename EdgeLabel<TTextIterator>::Type         TTextLabel;
//    typedef typename EdgeLabel<TPatternIterator>::Type      TPatternLabel;
//
//    typedef typename Container<TPatternIterator>::Type      TPattern;
//    typedef typename Value<TPattern>::Type             TPatternAlphabet;
//    typedef typename ValueSize<TPatternAlphabet>::Type      TPatternAlphabetSize;
//
//    TPatternIterator patternChildren[ValueSize<TPatternAlphabet>::VALUE + 1];
//
//#ifdef SEQAN_DEBUG
//    std::cout << "representative: " << representative(textIt) << std::endl;
//    std::cout << "repLength:      " << repLength(textIt) << std::endl;
//    std::cout << "current depth:  " << currentDepth << std::endl;
//#endif
//
//    // NOTE(esiragusa): Using repLength() is fine only for tries.
//    SEQAN_ASSERT_EQ(repLength(textIt), currentDepth);
//    SEQAN_ASSERT_EQ(repLength(patternIt), currentDepth);
//
//    // An acceptance state was reached.
//    if (currentDepth == patternsLength)
//    {
//        onMatch(delegate, textIt, patternIt, errors);
//    }
//    else
//    {
//        // Visit all children of text and pattern.
//        if (goDown(textIt) && (currentDepth < patternsLength && goDown(patternIt)))
//        {
//            // Fetch all children of pattern.
//            TPatternAlphabetSize childrenCount = 0;
//            do
//            {
//                patternChildren[childrenCount] = patternIt;
//                ++childrenCount;
//            } while (goRight(patternIt));
//
//            // Visit all children of text.
//            do
//            {
//                TTextLabel textLabel = parentEdgeLabel(textIt);
//
//                // Visit all children of pattern.
//                for (TPatternAlphabetSize child = 0; child < childrenCount; ++child)
//                {
//                    TPatternIterator & patternChildIt = patternChildren[child];
//                    TTextLabel patternLabel = parentEdgeLabel(patternChildIt);
//
//                    // Align edge labels.
//                    TErrors distance = (textLabel == patternLabel) ? 0 : 1;
//                    TErrors newErrors = errors + distance;
//
//#ifdef SEQAN_DEBUG
//                    std::cout << "text:           " << textLabel << std::endl;
//                    std::cout << "pattern:        " << patternLabel << std::endl;
//                    std::cout << "distance:       " << static_cast<unsigned>(distance) << std::endl;
//                    std::cout << "errors:         " << static_cast<unsigned>(newErrors) << std::endl;
//#endif
//
//                    if (newErrors < maxErrors)
//                        _dfs(textIt, patternChildIt, patternsLength, maxErrors, currentDepth + 1, newErrors, delegate);
//                    else
//                        _search(textIt, patternChildIt, patternsLength, maxErrors, currentDepth + 1, newErrors, delegate);
//
//#ifdef SEQAN_DEBUG
//                    std::cout << "back to depth:  " << currentDepth << std::endl;
//#endif
//                }
//
//            } while (goRight(textIt));
//        }
//    }
//}


//template <typename TTextIterator, typename TPatternIterator, typename TSize, typename TErrors, typename TDelegate>
//inline void _search(TTextIterator textIt,
//                    TPatternIterator patternIt,
//                    TSize const patternsLength,
//                    TErrors const maxErrors,
//                    TSize currentDepth,
//                    TErrors errors,
//                    TDelegate & delegate)
//{
//    typedef typename EdgeLabel<TTextIterator>::Type         TTextLabel;
//    typedef typename EdgeLabel<TPatternIterator>::Type      TPatternLabel;
//
//#ifdef SEQAN_DEBUG
//    std::cout << "representative: " << representative(textIt) << std::endl;
//    std::cout << "repLength:      " << repLength(textIt) << std::endl;
//    std::cout << "current depth:  " << currentDepth << std::endl;
//#endif
//
//    // NOTE(esiragusa): Using repLength() is fine only for tries.
//    SEQAN_ASSERT_EQ(repLength(textIt), currentDepth);
//    SEQAN_ASSERT_EQ(repLength(patternIt), currentDepth);
//    
//    // An acceptance state was reached.
//    if (currentDepth == patternsLength || isLeaf(patternIt))
//    {
//        onMatch(delegate, textIt, patternIt, errors);
//    }
//    else
//    {
//        // Visit all children of pattern.
//        if (currentDepth < patternsLength && goDown(patternIt))
//        {
//            do
//            {
//                TTextLabel patternLabel = parentEdgeLabel(patternIt);
//                TTextIterator textChildIt = textIt;
//
//                // Search the corresponding children in the text.
//                if (goDown(textChildIt, patternLabel))
//                {
//#ifdef SEQAN_DEBUG
//                    std::cout << "pattern:        " << patternLabel << std::endl;
//                    std::cout << "errors:         " << static_cast<unsigned>(errors) << std::endl;
//#endif
//                    _search(textChildIt, patternIt, patternsLength, maxErrors, currentDepth + 1, errors, delegate);
//
//#ifdef SEQAN_DEBUG
//                    std::cout << "back to depth:  " << currentDepth << std::endl;
//#endif
//                }
//
//            } while (goRight(patternIt));
//        }
//    }
//}

// ----------------------------------------------------------------------------
// Function _recurseDfs()
// ----------------------------------------------------------------------------

template <typename TFinder, typename TTextIterator, typename TPatternIterator, typename TErrors, typename TDelegate>
inline void _recurseDfs(TFinder & me;
                        TTextIterator textIt,
                        TPatternIterator patternIt,
                        TErrors errors,
                        TDelegate & delegate)
{
#ifdef SEQAN_DEBUG
    std::cout << "repLength: " << repLength(textIt) << std::endl;
#endif

    // An acceptance state was reached.
    if (isLeaf(patternIt))
    {
        delegate(finder);
    }
    else
    {
        // Visit all children of text and pattern.
        if (goDown(textIt) && goDown(patternIt))
        {
            // Visit all children of text.
            do
            {
                TPatternIterator patternChildIt = patternIt;

                // Visit all children of pattern.
                // NOTE(esiragusa): Each children could be visited more than once.
                do
                {
                    // Align edge labels.
                    TErrors newErrors = errors + _getVertexScore(finder);

#ifdef SEQAN_DEBUG
                    std::cout << "text:    " << parentEdgeLabel(_textIterator(finder)) << std::endl;
                    std::cout << "pattern: " << parentEdgeLabel(_patternIterator(finder)) << std::endl;
                    std::cout << "errors:  " << static_cast<unsigned>(newErrors) << std::endl;
#endif

                    if (newErrors < finder._scoreThreshold)
                        _recurseDfs(finder, textIt, patternChildIt, newErrors, delegate);
                    else
                        _search(finder, textIt, patternChildIt, newErrors, delegate);

                }
                while (goRight(patternChildIt));

            }
            while (goRight(textIt));
        }
    }
}

template <typename THaystack, typename TTextSpec, typename TNeedle, typename TPatternSpec, typename TDelegate>
inline void
find(Finder_<Index<THaystack, TTextSpec>, Index<TNeedle, TPatternSpec>, Backtracking<HammingDistance, DfsPreorder> > & me
     Index<THaystack, TTextSpec> & text,
     Index<TNeedle, TPatternSpec> & pattern,
     TDelegate && delegate)
{
    _patternIterator(finder) = begin(pattern);

    TTextIterator textIt(text);
    TPatternIterator patternIt(pattern);

    _recurseDfs(me, textIt, patternIt, static_cast<TErrors>(0), delegate);

    // NOTE(esiragusa): This is necessary since tail recursion was optimised in _dfs().
//    if (finder._scoreThreshold)
//        _findDfs(textIt, patternIt, static_cast<TErrors>(0), delegate);
//    else
//        _search(textIt, patternIt, static_cast<TErrors>(0), delegate);
}

}

#endif  // #ifndef APP_BENCH_FIND_BACKTRACKING_DFS_H_
