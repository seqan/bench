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

//    static const unsigned MIN_LENGTH = Max<MinLength<TStringSet>::VALUE, 1>::VALUE;

    typedef UngappedShape<12>                       TShapeSpec;
    typedef typename Value<TString>::Type           TAlphabet;
    typedef Shape<TAlphabet, TShapeSpec>            TShape;
    typedef typename Value<TShape>::Type            THash;

    // Fill hashes.
    String<THash> hashes;
    resize(hashes, length(me), Exact());
    transform(hashes, me, [](TStringRef string)
    {
        TShape shape;
        SEQAN_ASSERT_LEQ(weight(shape), length(string));
        return hash(shape, begin(string, Standard()));
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

template <typename TString, typename TSSetSpec, typename TIds>
inline void shuffle(StringSet<TString, TSSetSpec> & me, TIds & ids)
{
    typedef StringSet<TString, TSSetSpec>           TStringSet;
    typedef typename Value<TIds>::Type              TId;

    TStringSet sorted;
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

template <typename TString, typename TSSetSpec>
inline void radixSort(StringSet<TString, TSSetSpec> & me)
{
    radixSort(me, Serial());
}

// ----------------------------------------------------------------------------
// Function radixSort()
// ----------------------------------------------------------------------------

template <typename TTarget, typename TString, typename TSSetSpec>
inline void radixSort(TTarget & target, StringSet<TString, TSSetSpec> & me)
{
    typedef StringSet<TString, TSSetSpec>                   TStringSet;
    typedef typename StringSetPosition<TStringSet>::Type    TPos;
    typedef typename Id<TStringSet>::Type                   TId;
    typedef String<TId>                                     TIds;

    // Obtain sorted ids.
    TIds ids;
    sortedIds(ids, me, Serial());

    setHost(target, me);
    reserve(target, length(me), Exact());
    forEach(ids, [&](TId id) { appendInfix(target, TPos(id, 0), TPos(id, length(me[id])), Exact()); });
}

#endif  // #ifndef APP_IBENCH_MISC_H_
