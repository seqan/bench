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

#ifndef APP_BENCH_TYPES_H_
#define APP_BENCH_TYPES_H_

using namespace seqan;

// ============================================================================
// Limits
// ============================================================================

// ----------------------------------------------------------------------------
// Tag Limits
// ----------------------------------------------------------------------------

template <typename T1 = void, typename T2 = void>
struct Limits;

// ----------------------------------------------------------------------------
// Metafunction Value<>
// ----------------------------------------------------------------------------

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

// ============================================================================
// Container Types
// ============================================================================

// --------------------------------------------------------------------------
// StringSet Types
// --------------------------------------------------------------------------

namespace seqan
{
template <typename TAlphabet, typename TSpec, typename TSize, typename TSum>
struct Concatenator<StringSet<String<TAlphabet, TSpec>, Owner<ConcatDirect<Limits<TSize, TSum> > > > >
{
    typedef String<TAlphabet, Alloc<Limits<TSum> > > Type;
};

template <typename TString, typename TSize, typename TSum>
struct StringSetLimits<StringSet<TString, Owner<ConcatDirect<Limits<TSize, TSum> > > > >
{
    typedef String<TSum, Alloc<Limits<TSum> > > Type;
};
}

// ----------------------------------------------------------------------------
// String Size
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
// StringSet Size
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
}

// ----------------------------------------------------------------------------
// Metafunction TextCollection
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TLimits = Limits<__uint32>, typename TSetLimits = Limits<__uint32, __uint32> >
struct TextCollection
{
    typedef StringSet<String<TAlphabet, Alloc<TLimits> >, Owner<ConcatDirect<TSetLimits> > >    Type;
};

// ============================================================================
// Index Types
// ============================================================================

// ----------------------------------------------------------------------------
// QGramIndex Config
// ----------------------------------------------------------------------------

template <typename TValue = char, typename TSpec = void>
struct ShapeWeight
{
    static const unsigned VALUE = 3;
};

template <typename TSpec>
struct ShapeWeight<AminoAcid, TSpec>
{
    static const unsigned VALUE = 5;
};

template <typename TSpec>
struct ShapeWeight<Dna, TSpec>
{
    static const unsigned VALUE = 12;
};

template <>
struct ShapeWeight<AminoAcid, Nothing>
{
    static const unsigned VALUE = 4;
};

template <>
struct ShapeWeight<Dna, Nothing>
{
    static const unsigned VALUE = 10;
};

template <typename TValue = char, typename TSpec = void>
struct DefaultShape
{
    typedef UngappedShape<ShapeWeight<TValue, TSpec>::VALUE>   Type;
};

// ----------------------------------------------------------------------------
// MinLength
// ----------------------------------------------------------------------------

namespace seqan {
template <typename TAlphabet, typename TStringSpec, typename TSpec>
struct MinLength<StringSet<String<TAlphabet, TStringSpec>, TSpec> > : ShapeWeight<TAlphabet> {};
}

// ----------------------------------------------------------------------------
// FmIndex Config
// ----------------------------------------------------------------------------

template <typename TSpec>
struct TLFMIndexConfig;

template <typename TSize, typename TSum>
struct TLFMIndexConfig<Limits<TSize, TSum> >
{
    typedef TSum                                        LengthSum;
    typedef Levels<void, LevelsRDConfig<LengthSum> >    Bwt;
    typedef Levels<void, LevelsRDConfig<LengthSum> >    Sentinels;

    static const unsigned SAMPLING = 10;
};

template <typename TSum>
struct TLFMIndexConfig<Limits<__uint8, TSum> >
{
    typedef TSum                                        LengthSum;
    typedef Levels<void, LevelsRDConfig<LengthSum> >    Bwt;
    typedef Naive<void, RDConfig<LengthSum> >           Sentinels;

    static const unsigned SAMPLING = 10;
};

template <typename TSpec>
struct WTFMIndexConfig;

template <typename TSize, typename TSum>
struct WTFMIndexConfig<Limits<TSize, TSum> >
{
    typedef TSum                                        LengthSum;
    typedef WaveletTree<void, WTRDConfig<LengthSum> >   Bwt;
    typedef Levels<void, LevelsRDConfig<LengthSum> >    Sentinels;

    static const unsigned SAMPLING = 10;
};

template <typename TSum>
struct WTFMIndexConfig<Limits<__uint8, TSum> >
{
    typedef TSum                                        LengthSum;
    typedef WaveletTree<void, WTRDConfig<LengthSum> >   Bwt;
    typedef Naive<void, RDConfig<LengthSum> >           Sentinels;

    static const unsigned SAMPLING = 10;
};

// ----------------------------------------------------------------------------
// SparseString FibreIndicators
// ----------------------------------------------------------------------------

namespace seqan {
template <typename TValue, typename TSpec>
struct Fibre<SparseString<String<TValue, TSpec>, void>, FibreIndicators>
{
    typedef RankDictionary<bool, Levels<void, __uint32> > Type;
};
}

// ----------------------------------------------------------------------------
// SparseString Size
// ----------------------------------------------------------------------------

namespace seqan {
template <typename TString, typename TSize, typename TSum>
struct Size<SparseString<TString, TLFMIndexConfig<Limits<TSize, TSum> > > >
{
    typedef TSum Type;
};

template <typename TString, typename TSize, typename TSum>
struct Size<SparseString<TString, WTFMIndexConfig<Limits<TSize, TSum> > > >
{
    typedef TSum Type;
};
}

// ----------------------------------------------------------------------------
// Lcp Fibre
// ----------------------------------------------------------------------------

//namespace seqan
//{
//#ifdef APP_BENCH_CONSTRUCT_CPP_
//template <typename TString, typename TSSetSpec, typename TSpec>
//struct Fibre<Index<StringSet<TString, TSSetSpec>, IndexEsa<TSpec> >, FibreLcp>
//{
//    typedef StringSet<TString, TSSetSpec>   TText;
//    typedef Index<TText, IndexEsa<TSpec> >  TIndex;
//
//    typedef String<typename LengthSum<TText>::Type, typename DefaultIndexStringSpec<TIndex>::Type> Type;
//};
//#else
//template <typename TString, typename TSSetSpec, typename TSpec>
//struct Fibre<Index<StringSet<TString, TSSetSpec>, IndexEsa<TSpec> >, FibreLcp>
//{
//    typedef StringSet<TString, TSSetSpec>   TText;
//    typedef Index<TText, IndexEsa<TSpec> >  TIndex;
//
//    typedef String<__uint8, typename DefaultIndexStringSpec<TIndex>::Type> Type;
//};
//#endif
//}

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function save()
// ----------------------------------------------------------------------------
// This function is overloaded to save the Lcp values in a 8 bits encoding.

//#ifdef APP_BENCH_CONSTRUCT_CPP_
//namespace seqan {
//template <typename TText, typename TSSetSpec, typename TSpec>
//inline bool save(Index<StringSet<TText, TSSetSpec>, IndexEsa<TSpec> > const & index,
//                 const char * fileName, int openMode)
//{
//    typedef String<__uint8, External<ExternalConfigLarge<> > >      TLcp;
//
//    String<char> name;
//
//    name = fileName;	append(name, ".txt");
//    if ((!save(getFibre(index, EsaText()), toCString(name), openMode)) &&
//        (!save(getFibre(index, EsaText()), fileName, openMode))) return false;
//
//    name = fileName;	append(name, ".sa");
//    if (!save(getFibre(index, EsaSA()), toCString(name), openMode)) return false;
//
//    name = fileName;    append(name, ".isa");
//    if (!save(getFibre(index, EsaIsa()), toCString(name), openMode)) return false;
//
//    name = fileName;	append(name, ".lcp");
//    TLcp extLcp;
//    if (!open(extLcp, toCString(name), openMode)) return false;
//    assign(extLcp, getFibre(index, EsaLcp()), Exact());
//
//    name = fileName;	append(name, ".child");
//    if (!save(getFibre(index, EsaChildtab()), toCString(name), openMode)) return false;
//
//    name = fileName;	append(name, ".bwt");
//    if (!save(getFibre(index, EsaBwt()), toCString(name), openMode)) return false;
//
//    return true;
//}
//}
//#endif

// ----------------------------------------------------------------------------
// Function save()
// ----------------------------------------------------------------------------
// This function is overloaded to avoid saving the text.

namespace seqan {
template <typename TText, typename TSSetSpec, typename TSpec, typename TConfig>
inline bool save(Index<StringSet<TText, TSSetSpec>, FMIndex<TSpec, TConfig> > const & index,
                 const char * fileName, int openMode)
{
    String<char> name;

    name = fileName;    append(name, ".sa");
    if (!save(getFibre(index, FibreSA()), toCString(name), openMode)) return false;

    name = fileName;    append(name, ".lf");
    if (!save(getFibre(index, FibreLF()), toCString(name), openMode)) return false;

    return true;
}
}

// ----------------------------------------------------------------------------
// Function open()
// ----------------------------------------------------------------------------
// This function is overloaded to avoid loading the text.

namespace seqan {
template <typename TText, typename TSSetSpec, typename TSpec, typename TConfig>
inline bool open(Index<StringSet<TText, TSSetSpec>, FMIndex<TSpec, TConfig> > & index,
                 const char * fileName, int openMode)
{
    String<char> name;

    name = fileName;    append(name, ".sa");
    if (!open(getFibre(index, FibreSA()), toCString(name), openMode)) return false;

    name = fileName;    append(name, ".lf");
    if (!open(getFibre(index, FibreLF()), toCString(name), openMode)) return false;

    setFibre(getFibre(index, FibreSA()), getFibre(index, FibreLF()), FibreLF());

    return true;
}
}

// ----------------------------------------------------------------------------
// Function indexRequire()
// ----------------------------------------------------------------------------
// This function is overloaded to avoid building the index except for Wotd Dir.

#if !defined(APP_BENCH_CONSTRUCT_CPP_) && !defined(APP_BENCH_FILTER_CPP_)
namespace seqan {
template <typename TText, typename TSSetSpec, typename TSpec, typename TFibre>
inline bool indexRequire(Index<StringSet<TText, TSSetSpec>, TSpec> & index, Tag<TFibre> const fibre)
{
    if (!indexSupplied(index, fibre))
        throw RuntimeError("Index fibres not supplied.");

    return true;
}

template <typename TText, typename TSSetSpec, typename TSpec>
inline bool indexRequire(Index<StringSet<TText, TSSetSpec>, TSpec> & index, WotdDir)
{
    if (indexSupplied(index, WotdDir())) return true;
    _wotdCreateFirstLevel(index);
    return true;
}
}
#endif

// ----------------------------------------------------------------------------
// Function indexCreate()
// ----------------------------------------------------------------------------

template <typename TText, typename TSpec>
inline bool indexCreate(Index<TText, IndexWotd<TSpec> > & index, FibreSA, Trie)
{
    typedef Index<TText, IndexWotd<TSpec> >         TIndex;
    typedef typename Fibre<TIndex, FibreSA>::Type   TSA;
    typedef typename Value<TSA>::Type               TSAValue;
    typedef typename Size<TText>::Type              TSize;

    TText const & text = indexText(index);
    TSA & sa = indexSA(index);
    TSize textLen = length(text);

    resize(sa, textLen, Exact());

    // Fill the suffix array with (i, 0).
    for (TSize i = 0; i < textLen; i++)
        sa[i] = TSAValue(i, 0);

    return true;
}

#endif  // #ifndef APP_BENCH_TYPES_H_
