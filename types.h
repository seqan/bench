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

#ifndef APP_IBENCH_TYPES_H_
#define APP_IBENCH_TYPES_H_

using namespace seqan;

// ----------------------------------------------------------------------------
// Tag Limits
// ----------------------------------------------------------------------------

template <typename T1 = void, typename T2 = void>
struct Limits;

namespace seqan
{
template <typename T1, typename T2>
struct Value<Limits<T1, T2>, 1>
{
    typedef T1 Type;
};

template <typename T1, typename T2>
struct Value<Limits<T1, T2>, 2>
{
        typedef T2 Type;
};
}

// ----------------------------------------------------------------------------
// Function appendValue(StringSet<Limits>)
// ----------------------------------------------------------------------------

template <typename TString, typename TSize, typename TSum, typename TSource, typename TExpand>
inline void
appendValue(StringSet<TString, Owner<ConcatDirect<Limits<TSize, TSum> > > > & me,
            TSource const & obj,
            Tag<TExpand> tag)
{
    appendValue(me.limits, lengthSum(me) + length(obj), tag);
    append(me.concat, obj, tag);
}

// ============================================================================
// Basic Types
// ============================================================================

typedef StringSet<CharString, Owner<ConcatDirect<> > >          CharStringSet;

// ----------------------------------------------------------------------------
// Metafunction TextCollection
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TLimits, typename TSetLimits>
struct TextCollection
{
    typedef StringSet<String<TAlphabet, Alloc<TLimits> >, Owner<ConcatDirect<TSetLimits> > >    Type;
};

// ----------------------------------------------------------------------------
// String Types
// ----------------------------------------------------------------------------

namespace seqan
{
template <typename TValue, typename TSize>
struct Size<String<TValue, Alloc<Limits<TSize> > > >
{
    typedef TSize Type;
};
}

// ----------------------------------------------------------------------------
// StringSet Types
// ----------------------------------------------------------------------------

namespace seqan
{
template <typename TString, typename TSize, typename TSum>
struct Size<StringSet<TString, Owner<ConcatDirect<Limits<TSize, TSum> > > > >
{
    typedef TSize Type;
};

template <typename TString, typename TSize, typename TSum>
struct LengthSum<StringSet<TString, Owner<ConcatDirect<Limits<TSize, TSum> > > > >
{
    typedef TSum Type;
};
};

// ----------------------------------------------------------------------------
// EsaIndex Fibres
// ----------------------------------------------------------------------------

//namespace seqan {
//template <typename TText>
//struct Fibre<Index<TText, IndexEsa<void> >, FibreLcp>
//{
//    typedef String<typename Size<TText>::Type>   Type;
//};
//
//template <typename TText>
//struct Fibre<Index<TText, IndexEsa<void> >, FibreChildtab>
//{
//    typedef String<typename LengthSum<TText>::Type>   Type;
//};
//}

// ----------------------------------------------------------------------------
// QGramIndex Fibres
// ----------------------------------------------------------------------------

//template <typename TValue = char>
//struct ShapeWeight
//{
//    static const unsigned VALUE = 3;
//};
//
//template <>
//struct ShapeWeight<AminoAcid>
//{
//    static const unsigned VALUE = 5;
//};
//
//template <>
//struct ShapeWeight<Dna5>
//{
//    static const unsigned VALUE = 10;
//};
//
//template <>
//struct ShapeWeight<Dna>
//{
//    static const unsigned VALUE = 12;
//};
//
//template <typename TValue = char>
//struct QGramShape
//{
//    typedef UngappedShape<ShapeWeight<TValue>::VALUE>   Type;
//};

//namespace seqan {
//template <typename TText, typename TSpec>
//struct Fibre<Index<TText, IndexQGram<TSpec> >, FibreDir>
//{
//    typedef String<__uint32>   Type;
//};
//}

// ----------------------------------------------------------------------------
// FmIndex Fibres
// ----------------------------------------------------------------------------

struct WTFMIndex
{
    typedef WaveletTree<void>  TValuesSpec;
    typedef Naive<void>        TSentinelsSpec;

    static const unsigned SAMPLING = 10;
};

struct TLFMIndex
{
    typedef TwoLevels<void>    TValuesSpec;
    typedef Naive<void>        TSentinelsSpec;

    static const unsigned SAMPLING = 10;
};

// ----------------------------------------------------------------------------
// RankDictionary Size
// ----------------------------------------------------------------------------

namespace seqan {
template <typename TSpec>
struct Size<RankDictionary<Dna, TwoLevels<TSpec> > >
{
    typedef __uint32 Type;
};

template <typename TSpec>
struct Size<RankDictionary<bool, TwoLevels<TSpec> > >
{
    typedef __uint32 Type;
};

template <typename TSpec>
struct Size<RankDictionary<bool, Naive<TSpec> > >
{
    typedef __uint32 Type;
};
}

// ----------------------------------------------------------------------------
// SparseString Size
// ----------------------------------------------------------------------------

namespace seqan {
// TODO(esiragusa): Overload Size<CSA> instead of Size<SparseString>
template <typename TValueString>
struct Size<SparseString<TValueString, void> >
{
    typedef __uint32    Type;
};
}

#endif  // #ifndef APP_IBENCH_TYPES_H_
