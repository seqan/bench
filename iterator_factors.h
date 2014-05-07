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
// Iterate a container as a sequence of factors.
// ==========================================================================

#ifndef SEQAN_EXTRAS_BASIC_ITERATOR_FACTORS_H_
#define SEQAN_EXTRAS_BASIC_ITERATOR_FACTORS_H_

namespace seqan {

// ============================================================================
// Forwards
// ============================================================================

// ============================================================================
// Tags, Classes, Enums
// ============================================================================

struct FactorsIterator_;
typedef Tag<FactorsIterator_>   FactorsIterator;

template <typename TContainer>
class Iter<TContainer, FactorsIterator>
{
public:
    typedef typename Iterator<TContainer, Rooted>::Type TIterator;
    typedef typename Size<TContainer>::Type             TSize;

    TIterator   data_iterator;
    TIterator   _lastFactorBegin;
    TSize       _factorsSize;

    // ------------------------------------------------------------------------
    // Constructors
    // ------------------------------------------------------------------------

    Iter() :
        _factorsSize(0)
    {}

//    template <typename TValue>
//    Iter(TValue factorsSize) :
//        _factorsSize(factorsSize)
//    {}

    Iter(Iter const & other) :
        data_iterator(other.data_iterator),
        _lastFactorBegin(other._lastFactorBegin),
        _factorsSize(other._factorsSize)
    {}

//    Iter(TIterator const & iter) :
//        data_iterator(iter),
//        _lastFactorBegin(end(container(iter)) - 1),
//        _factorsSize(1)
//    {}

//    template <typename TValue>
//    Iter(TContainer & cont, TValue factorsSize, TValue lastFactorSize) :
//        data_iterator(begin(cont)),
//        _factorsSize(factorsSize)
//    {
//        _setLastFactor(*this, lastFactorSize);
//    }

    template <typename TValue>
    Iter(TContainer const & cont, TValue factorsSize, TValue lastFactorSize) :
        data_iterator(begin(cont)),
        _factorsSize(factorsSize)
    {
        _setLastFactor(*this, lastFactorSize);
    }

//    template <typename TValue>
//    Iter(TContainer & cont, TValue factorsSize) :
//        Iter(cont, factorsSize, factorsSize)
//    {}
//
//    template <typename TValue>
//    Iter(TContainer const & cont, TValue factorsSize) :
//        Iter(cont, factorsSize, factorsSize)
//    {}
//
//    Iter(TContainer & cont) :
//        Iter(cont, 1, 1)
//    {}
//
//    Iter(TContainer const & cont) :
//        Iter(cont, 1, 1)
//    {}

    // ------------------------------------------------------------------------
    // Assignment Operators;  Have to be defined in the class.
    // ------------------------------------------------------------------------

    Iter &
    operator = (Iter const & other)
    {
        data_iterator = other.data_iterator;
        _lastFactorBegin = other._lastFactorBegin;
        _factorsSize = other._factorsSize;
        return *this;
    }

//    Iter &
//    operator = (TIterator const & iter)
//    {
//        data_iterator = iter;
//        _lastFactorBegin = end(container(iter)) - 1;
//        _factorsSize = 1;
//        return *this;
//    }
};

// ============================================================================
// Metafunctions
// ============================================================================

// ----------------------------------------------------------------------------
// Metafunction Iterator
// ----------------------------------------------------------------------------

template <typename TContainer>
struct Iterator<TContainer, FactorsIterator>
{
    typedef Iter<TContainer, FactorsIterator>   Type;
};

// ----------------------------------------------------------------------------
// Metafunction Value
// ----------------------------------------------------------------------------

//template <typename TContainer>
//struct Value<Iter<TContainer, FactorsIterator> > : Infix<TContainer> {};
//
//template <typename TContainer>
//struct Value<Iter<TContainer const, FactorsIterator> > : Infix<TContainer const> {};

// ----------------------------------------------------------------------------
// Metafunction GetValue
// ----------------------------------------------------------------------------

//template <typename TContainer>
//struct GetValue<Iter<TContainer, FactorsIterator> > : Infix<TContainer> {};
//
//template <typename TContainer>
//struct GetValue<Iter<TContainer const, FactorsIterator> > : Infix<TContainer const> {};

// ----------------------------------------------------------------------------
// Metafunction Reference
// ----------------------------------------------------------------------------

//template <typename TContainer>
//struct Reference<Iter<TContainer, FactorsIterator> > : Infix<TContainer> {};
//
//template <typename TContainer>
//struct Reference<Iter<TContainer const, FactorsIterator> > : Infix<TContainer const> {};

// ============================================================================
// Functions
// ============================================================================

// --------------------------------------------------------------------------
// Function _setLastFactor()
// --------------------------------------------------------------------------

template <typename TContainer, typename TSize>
inline void
_setLastFactor(Iter<TContainer, FactorsIterator> & me, TSize lastFactorSize)
{
    SEQAN_ASSERT_GEQ(length(container(me.data_iterator)), lastFactorSize);

    TSize factorsCount = (length(container(me.data_iterator)) - lastFactorSize) / me._factorsSize;
    me._lastFactorBegin = begin(container(me.data_iterator)) + me._factorsSize * factorsCount;
}

// --------------------------------------------------------------------------
// Function value()
// --------------------------------------------------------------------------

//template <typename TContainer>
//inline typename Reference<Iter<TContainer, FactorsIterator> >::Type
//value(Iter<TContainer, FactorsIterator> & me)
//{
//    typedef typename Iterator<TContainer, Rooted>::Type TIterator;
//
//    SEQAN_ASSERT_NOT(atEnd(me));
//
//    TIterator factorEnd;
//
//    if (me.data_iterator != me._lastFactorBegin)
//        factorEnd = me.data_iterator + me._factorsSize;
//    else
//        factorEnd = end(container(me.data_iterator));
//
//    return infix(container(me.data_iterator), me.data_iterator, factorEnd);
//}

//template <typename TContainer>
//inline typename Reference<Iter<TContainer, FactorsIterator> const>::Type
//value(Iter<TContainer, FactorsIterator> const & me)
//{
//    typedef typename Iterator<TContainer, Rooted>::Type TIterator;
//
//    SEQAN_ASSERT_NOT(atEnd(me));
//
//    TIterator factorEnd;
//
//    if (me.data_iterator != me._lastFactorBegin)
//        factorEnd = me.data_iterator + me._factorsSize;
//    else
//        factorEnd = end(container(me.data_iterator));
//
//    return infix(container(me.data_iterator), me.data_iterator, factorEnd);
//}

// ----------------------------------------------------------------------------
// Function goNext()
// ----------------------------------------------------------------------------

template <typename TContainer>
inline void
goNext(Iter<TContainer, FactorsIterator> & me)
{
    SEQAN_ASSERT_NOT(atEnd(me));

    if (me.data_iterator != me._lastFactorBegin)
        goFurther(me.data_iterator, me._factorsSize);
    else
        goEnd(me.data_iterator);
}

// ----------------------------------------------------------------------------
// Function goPrevious()
// ----------------------------------------------------------------------------

template <typename TContainer>
inline void
goPrevious(Iter<TContainer, FactorsIterator> & me)
{
    if (atEnd(me.data_iterator))
        me.data_iterator = me._lastFactorBegin;
    else
        goFurther(me.data_iterator, -me._factorsSize);
}

// ----------------------------------------------------------------------------
// Function atEnd()
// ----------------------------------------------------------------------------

template <typename TContainer>
inline bool
atEnd(Iter<TContainer, FactorsIterator> & me)
{
    return atEnd(me.data_iterator);
}

template <typename TContainer>
inline bool
atEnd(Iter<TContainer, FactorsIterator> const & me)
{
    return atEnd(me.data_iterator);
}

}  // namespace seqan

#endif  // #ifndef SEQAN_EXTRAS_BASIC_ITERATOR_FACTORS_H_
