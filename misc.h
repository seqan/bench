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

#ifndef APP_IBENCH_MISC_H_
#define APP_IBENCH_MISC_H_

#include <seqan/misc/edit_environment.h>

using namespace seqan;

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function _countSubstringsImpl()
// ----------------------------------------------------------------------------

template <typename TIter, typename TCount, typename TLength>
inline void _countSubstringsImpl(TIter it, TCount & count, TLength length)
{
    if (repLength(it) >= length)
    {
         ++count;
    }
    else if (goDown(it))
    {
        do
        {
            _countSubstringsImpl(it, count, length);
        }
        while (goRight(it));
    }
}

// ----------------------------------------------------------------------------
// Function countSubstrings()
// ----------------------------------------------------------------------------

template <typename TIndex, typename TLength>
inline typename Size<TIndex>::Type
countSubstrings(TIndex & index, TLength length)
{
    typedef typename Iterator<TIndex, TopDown<> >::Type TIter;
    typedef typename Size<TIndex>::Type                 TSize;

    TSize count = 0;
    TIter it(index);

    _countSubstringsImpl(it, count, length);

    return count;
}

// ----------------------------------------------------------------------------
// Function indexBuild()
// ----------------------------------------------------------------------------

template <typename TIndex>
inline void indexBuild(TIndex & /* index */) {}

template <typename TText, typename TSpec>
inline void indexBuild(Index<TText, IndexWotd<TSpec> > & index)
{
    countSubstrings(index, maxLength(indexText(index)));
}

// ----------------------------------------------------------------------------
// Function sortedIds()
// ----------------------------------------------------------------------------

template <typename TIds, typename TContainer, typename TParallelTag>
inline void sortedIds(TIds & ids, TContainer const & me, Tag<TParallelTag> const & /* tag */)
{
    typedef Tag<TParallelTag> const     TThreading;
    typedef typename Value<TIds>::Type  TId;

    // Fill ids with identity permutation.
    resize(ids, length(me), Exact());
    iota(ids, TId(0), TThreading());

    // Sort ids by value.
    sort(ids, [&me](TId a, TId b) { return getValue(me, a) < getValue(me, b); }, TThreading());
}

// ----------------------------------------------------------------------------
// Function sortedIds()
// ----------------------------------------------------------------------------

template <typename TIds, typename TString, typename TSSetSpec, typename TParallelTag>
inline void sortedIds(TIds & ids, StringSet<TString, TSSetSpec> const & me, Tag<TParallelTag> const & /* tag */)
{
    typedef Tag<TParallelTag> const                 TThreading;
    typedef StringSet<TString, TSSetSpec> const     TStringSet;
    typedef typename Reference<TStringSet>::Type    TStringRef;
    typedef typename Value<TIds>::Type              TId;

    typedef typename Value<TString>::Type           TAlphabet;
    typedef typename DefaultShape<TAlphabet>::Type  TShapeSpec;
    typedef Shape<TAlphabet, TShapeSpec>            TShape;
    typedef typename Value<TShape>::Type            THash;

    // Fill hashes.
    String<THash> hashes;
    resize(hashes, length(me), Exact());
    transform(hashes, me, [](TStringRef string)
    {
        TShape shape;
        return hashUpper(shape, begin(string, Standard()), length(string));
    },
    TThreading());

    // Fill ids with identity permutation.
    resize(ids, length(me), Exact());
    iota(ids, TId(0), TThreading());

    // Sort ids by hash.
    sort(ids, [&hashes](TId a, TId b) { return hashes[a] < hashes[b]; }, TThreading());
}

// ----------------------------------------------------------------------------
// Function shuffle()
// ----------------------------------------------------------------------------

template <typename TContainer, typename TIds>
inline void shuffle(TContainer & me, TIds & ids)
{
    typedef typename Value<TIds>::Type              TId;

    TContainer sorted;
    reserve(sorted, length(me), Exact());
    forEach(ids, [&](TId id) { appendValue(sorted, me[id]); });
    swap(me, sorted);
}

// ----------------------------------------------------------------------------
// Function radixSort()
// ----------------------------------------------------------------------------

template <typename TContainer, typename TParallelTag>
inline void radixSort(TContainer & me, Tag<TParallelTag> const & /* tag */)
{
    typedef Tag<TParallelTag> const                 TThreading;
    typedef typename Id<TContainer>::Type           TId;
    typedef String<TId>                             TIds;

    // Obtain sorted ids.
    TIds ids;
    sortedIds(ids, me, TThreading());

    // Reorder container elements.
    shuffle(me, ids);
}

// ----------------------------------------------------------------------------
// Function sortedStringSet()
// ----------------------------------------------------------------------------

template <typename THost, typename TSpec, typename TString, typename TSSetSpec>
inline void sortedStringSet(StringSet<THost, Segment<TSpec> > & target, StringSet<TString, TSSetSpec> & me)
{
    typedef StringSet<TString, TSSetSpec>                   TStringSet;
    typedef typename StringSetPosition<TStringSet>::Type    TPos;
    typedef typename Id<TStringSet>::Type                   TId;
    typedef String<TId>                                     TIds;

    // Obtain sorted ids.
    TIds ids;
    sortedIds(ids, me, Serial());

    // Fill Segment StringSet with
    setHost(target, me);
    reserve(target, length(me), Exact());
    forEach(ids, [&](TId id) { appendInfix(target, TPos(id, 0), TPos(id, length(me[id])), Exact()); });
}

// ----------------------------------------------------------------------------
// Function indexCreate()
// ----------------------------------------------------------------------------

#ifdef SEQAN_CXX11_STANDARD
template <typename TText, typename TShapeSpec, typename TSpec>
inline bool indexCreate(Index<TText, IndexQGram<TShapeSpec, TSpec> > & index, FibreSADir, Trie)
{
    typedef Index<TText, IndexQGram<TShapeSpec, TSpec> >    TIndex;
    typedef typename Fibre<TIndex, FibreSA>::Type           TSA;
    typedef typename Value<TSA>::Type                       TSAValue;
    typedef typename Fibre<TIndex, FibreDir>::Type          TDir;
    typedef typename Fibre<TIndex, FibreShape>::Type        TShape;
    typedef typename Fibre<TIndex, FibreBucketMap>::Type    TBucketMap;
    typedef typename Size<TText>::Type                      TSize;
    typedef typename Iterator<TText const, Standard>::Type  TTextIt;
    typedef typename Reference<TText const>::Type           TTextRef;

    TText const & text = indexText(index);
    TSA & sa = indexSA(index);
    TDir & dir = indexDir(index);
    TShape & shape = indexShape(index);
    TBucketMap & bucketMap = indexBucketMap(index);

    TSize textLen = length(text);
    resize(sa, textLen, Exact());

    // Init directory.
//    setStepSize(index, 0);
    resize(dir, _fullDirLength(index), Exact());
    _qgramClearDir(dir, bucketMap);

    // Count qgrams.
    forEach(text, [&](TTextRef t)
    {
        ++dir[requestBucket(bucketMap, hash(shape, begin(t, Standard())))];
    });

    // Compute cumulative sum.
    _qgramCummulativeSum(dir, False());

    // Fill suffix array.
    iterate(text, [&](TTextIt it)
    {
        sa[dir[getBucket(bucketMap, hash(shape, begin(value(it), Standard()))) + 1]++] = TSAValue(position(it), 0);
    });

    return true;
}
#endif

// ----------------------------------------------------------------------------
// Function find(index, qgramIndex, errors, [](...){}, Backtracking<HammingDistance, Threshold<2> >());
// ----------------------------------------------------------------------------

template <unsigned THRESHOLD>
struct Threshold;

template <typename THaystack, typename THaystackSpec, typename TNeedles, typename TShapeSpec, typename TNeedleSpec,
          typename TDelegate, unsigned THRESHOLD>
inline void
find(Index<THaystack, THaystackSpec> & text,
     Index<TNeedles, IndexQGram<TShapeSpec, TNeedleSpec> > & pattern,
     TDelegate && delegate,
     Backtracking<HammingDistance, Threshold<THRESHOLD> >)
{
    typedef Index<THaystack, THaystackSpec>             TText;
    typedef typename Iterator<TText, TopDown<> >::Type  TTextIt;

    typedef typename Value<TNeedles const>::Type        TNeedle;
    typedef typename Iterator<TNeedle, Standard>::Type  TNeedleIt;

    typedef IndexQGram<TShapeSpec, TNeedleSpec>         TPatternSpec;
    typedef Index<TNeedles, TPatternSpec>               TPattern;
    typedef typename Fibre<TPattern, FibreShape>::Type  TShape;
    typedef typename Host<TShape>::Type                 TAlphabet;
    typedef typename Fibre<TPattern, FibreSA>::Type     TSA;
    typedef typename Infix<TSA const>::Type             TOccurrences;
    typedef typename Value<TOccurrences>::Type          TOccurrence;

    typedef String<TAlphabet>                           TQGram;
    typedef EditEnvironment<HammingDistance, THRESHOLD> TEnvironment;
    typedef StringEnumerator<TQGram, TEnvironment>      TNeighborhood;
    typedef typename Iterator<TNeighborhood>::Type      TNeighborhoodIt;

    static const unsigned Q = WEIGHT<TShape>::VALUE;

    TShape & shape = indexShape(pattern);
    TNeedles & needles = indexText(pattern);

    static const unsigned long QGRAMS = Power<ValueSize<TAlphabet>::VALUE, Q>::VALUE;

    TQGram qgram;

    // Generate all q-grams.
    for (unsigned long qhash = 0; qhash < QGRAMS; qhash++)
    {
        unhash(qgram, qhash, Q);
        TTextIt textIt(text);
        if (!goDown(textIt, qgram))
            continue;

        TNeighborhood neighborhood(qgram);
        for (TNeighborhoodIt itU = begin(neighborhood); !atEnd(itU); goNext(itU))
        {
            hash(shape, begin(*itU));

            unsigned errors = 0;
            for (unsigned i = 0; i < THRESHOLD; i++)
                errors += (ordValue(qgram[itU.mod[i].errorPos]) != itU.mod[i].character);

            TOccurrences const & occs = getOccurrences(pattern, shape);

            forEach(occs, [&](TOccurrence const & occ)
            {
                TNeedle const & needle = needles[getSeqNo(occ)];
                TNeedleIt needleIt = begin(needle, Standard()) + Q;

                _findBacktracking(textIt, needle, needleIt, errors, THRESHOLD, [&](TTextIt endIt, unsigned errors)
                {
                    delegate(endIt, begin(needles, Standard()) + getSeqNo(occ), errors);
                },
                HammingDistance());
            });
        }
    }
}

// ----------------------------------------------------------------------------
// Function filter(index, needles, errors, [](...){}, Seeds<TDistance>());
// ----------------------------------------------------------------------------
// NOTE(esiragusa): All needles must have the same length.

template <typename TDistance = Exact>
struct Seeds
{
    unsigned char threshold;

    Seeds(unsigned char threshold = 0) :
        threshold(threshold)
    {}
};

template <typename TText, typename TSpec, typename TNeedles, typename TThreshold, typename TDelegate, typename TDistance>
inline void filter(Index<TText, TSpec> & index, TNeedles & needles, TThreshold threshold, TDelegate && delegate, Seeds<TDistance> const & config)
{
    typedef Index<TText, TSpec>                             TIndex;
    typedef typename Iterator<TIndex, TopDown<> >::Type     TIndexIt;
    typedef typename Fibre<TIndex, FibreSA>::Type           TSA;
    typedef typename Infix<TSA const>::Type                 TOccurrences;
    typedef typename Value<TOccurrences>::Type              TOccurrence;

    typedef typename Iterator<TNeedles const, Rooted>::Type TNeedlesIt;
    typedef typename Size<TNeedles>::Type                   TNeedleId;
    typedef typename Value<TNeedles>::Type                  TNeedle;
    typedef typename Size<TNeedle>::Type                    TNeedleSize;

    typedef StringSet<TNeedles, Segment<TNeedles> >         TSeeds;
    typedef typename Iterator<TSeeds const, Rooted>::Type   TSeedsIt;
    typedef typename StringSetPosition<TSeeds>::Type        TSeedPos;

    if (empty(needles)) return;

    TNeedleId needlesCount = length(needles);
    TNeedleSize needleLength = length(front(needles));
    TNeedleSize seedsCount = static_cast<TNeedleSize>(std::ceil((threshold + 1) / (config.threshold + 1.0)));
    TNeedleSize seedsLength = needleLength / seedsCount;

    TSeeds seeds(needles);

    reserve(seeds, needlesCount * seedsCount, Exact());

    for (TNeedleId needleId = 0; needleId < needlesCount; ++needleId)
        for (TNeedleSize seedId = 0; seedId < seedsCount; ++seedId)
            appendInfixWithLength(seeds, TSeedPos(needleId, seedId * seedsLength), seedsLength, Exact());

    find(index, seeds, config.threshold, [&](TIndexIt const & indexIt, TSeedsIt const & seedsIt, unsigned char seedErrors)
    {
        TOccurrences const & occs = getOccurrences(indexIt);

        forEach(occs, [&](TOccurrence const & occ)
        {
            TNeedleId needleId = position(seedsIt) / seedsCount;
            TNeedleSize seedBegin = (position(seedsIt) % seedsCount) * seedsLength;

            delegate(needleId, occ, posAdd(occ, seedsLength), seedBegin, seedBegin + seedsLength, seedErrors);
        });
    },
    Backtracking<TDistance>());
}


// ============================================================================
// Classes
// ============================================================================

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

    typedef AlignTextBanded<FindInfix,
                            NMatchesNone_,
                            NMatchesNone_>              TMyersInfix;
    typedef AlignTextBanded<FindPrefix,
                            NMatchesNone_,
                            NMatchesNone_>              TMyersPrefix;
    typedef Myers<TMyersInfix, True, void>              TAlgoEnd;
    typedef Myers<TMyersPrefix, True, void>             TAlgoBegin;

    typedef Finder<THaystackInfix const>                TFinderEnd;
    typedef Finder<THaystackInfixRev>                   TFinderBegin;
    typedef PatternState_<TNeedleInfix const, TAlgoEnd> TPatternEnd;
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

#endif  // #ifndef APP_IBENCH_MISC_H_
