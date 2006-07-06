#ifndef LOWERENVELOPEOFPARABOLAS_TXX
#define LOWERENVELOPEOFPARABOLAS_TXX
#include <limits>
#include <cmath>
#include <cassert>

namespace itk
{

//
// Constraints on minimal spacing, abscissa index, and apex height.
//
// The minimal spacing m is chosen by the user
//   m := 10^-MinimalSpacingPrecision
//
// For the other values we have to take a look at the expression evaluated in
// the intersection code:
//   (i+j + (a-b)/(d^2*(i-j))) / 2
// for abscissa indices i and j, apex heights a and b, and a spacing d.
// Intersections are never done for |i-j| < 1.
//
// To prevent an overflow on the sums of abscissa indices, they have to be
// limited to the interval [-o, o], o := max(AbscissaIndexType)/2
//
// The rest of the computation is done with SpacingType which covers a
// larger interval than AbscissaIndexType.
//
// Let S := max(SpacingType) and we are searching for the largest
// ApexHeightType s with a,b in [-s, s] so that arithmetic overflows are
// avoided:
//     (i+j + (a-b)/(d^2*(i-j))) / 2 <= (o+o + (s'-(-s'))/(m^2*1))  <= S
//  => (2o + 2s'/m^2) <= S
// <=> s' <= (S - 2o) * m^2/2
// Please note that the right side of the inequality is a valid SpacingType
// number, because m is never larger than 1.
//
// The sum a-b is carried out in ApexHeightType and the result of the whole
// computation is casted to AbscissaIndexType, so we choose
//   s = min(s', max(ApexHeightType) / 2)
//
// I'm sorry that the following definitions are a not as easy on the eye as
// the equations above.
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
const SpacingType
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::minimalSpacing = static_cast<SpacingType>(pow(10, -MinimalSpacingPrecision));

template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
const AbscissaIndexType
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::maxAbscissa = std::numeric_limits<AbscissaIndexType>::max() / 2;

template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
const ApexHeightType
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::maxApexHeight = static_cast<ApexHeightType>(
    std::min(
      static_cast<SpacingType>(
        (
         std::numeric_limits<SpacingType>::max() -
         static_cast<SpacingType>(std::numeric_limits<AbscissaIndexType>::max())
        ) * 
        static_cast<SpacingType>(pow(10, -MinimalSpacingPrecision*2)) / 2),
        static_cast<SpacingType>(std::numeric_limits<ApexHeightType>::max() / 2)
      )
    );

//
// Constructor of the embedded Parabola struct
// 
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
inline
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::Parabola
::Parabola(const AbscissaIndexType& _i, const ApexHeightType& _y, const LabelType& _l) :
  i(_i), y(_y), l(_l)
{
  assert(-maxAbscissa <= i && i <= maxAbscissa);
  assert(-maxApexHeight <= y && y <= maxApexHeight);
};

//
// Implementation of the embedded ParabolaRegion struct
//
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
inline
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::ParabolaRegion
::ParabolaRegion(const Parabola &_p, const AbscissaIndexType &_d) : p(_p), dominantFrom(_d)
{
  assert(-maxAbscissa <= dominantFrom && dominantFrom <= maxAbscissa);
};


//
// Implementation of the embedded Iterator
//

// Constructor
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::Iterator
::Iterator(const Parabolas &_envelope, const AbscissaIndexType &_from, const AbscissaIndexType &_to)
  : envelope(_envelope), currentIndex(_from), end(_to), currentParabolaNumber(0)
{
  // The sampling interval has to be well-formed
  assert(-maxAbscissa <= currentIndex);
  assert(currentIndex <= end);
  assert(end <= maxAbscissa);

  // The envelope should contain a front and a back sentinel parabola. The
  // latter must dominate from maxAbscissa
  assert(envelope.size() >= 2);
  assert(envelope.back().dominantFrom == maxAbscissa);
  
  // Skip to the correct parabola
  while (envelope[currentParabolaNumber + 1].dominantFrom < currentIndex)
    ++currentParabolaNumber;
}

// Return current index
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
const
typename LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::AbscissaIndexType&
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::Iterator
::currentAbscissaIndex()
{
  return currentIndex;
}

// Return current parabola
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
inline
const
typename LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::Parabola&
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::Iterator
::currentParabola()
{
  return envelope[currentParabolaNumber].p;
}

// Proceed to the next index
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
typename LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::Iterator&
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::Iterator
::operator++()
{
  ++currentIndex;

  // Skip to the correct parabola.
  while (envelope[currentParabolaNumber + 1].dominantFrom < currentIndex)
  {
    ++currentParabolaNumber;

    // The back sentinel should never be reached because it's dominance
    // region begins at the rightmost possible place
    assert(currentParabolaNumber != envelope.size() - 1);
  }

  return *this;
}

// Testing for end of iteration
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
inline
bool
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::Iterator
::IsAtEnd()
{
  return currentIndex == end;
}

//
// Compute the intersection abscissa of two parabolas p and q. Their apex abscissas
// have to be different.
//
// Returns the smallest index for which the parabola q is below p
//
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
inline
typename LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::AbscissaIndexType
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::intersection(const Parabola &p, const Parabola &q)
{
  // Parabolas must be different, because the intersection abscissa is not
  // defined otherwise
  assert(p.i != q.i);

  // We have fp(x) = (x - px)^2 + py and fq(x) = (x - qx)^2 + qy
  //     fp(x) = fq(x)
  // <=> (x - px)^2 + py = (x - qx)^2 + qy
  // <=> x^2 - 2xpx + px^2 + py = x^2 - 2xqx + qx^2 + qy
  // <=> -2xpx + 2xqx = qx^2 - px^2 + qy - py
  // <=> x(2qx - 2px) = qx^2 - px^2 + qy - py
  // <=> x = (qx^2 - px^2 + qy - py) / (2 (qx - px))
  // <=> x = 1/2 * ((qx^2 - px^2) / (qx - px) + (qy - py) / (qx - px))
  // <=> x = 1/2 * (qx + px + (qy - py) / (qx - px))
  //
  //   using the index i and spacing s instead of x (x = i s)
  // 
  // <=> i = 1/2 * (qi + pi + (qy - py) / (s^2(qi - pi)))
  //
  // This has to be evaluated in floating point precision, because
  // the denominator of the fraction can be < 1.0 for small spacings.
  //
  // We return the largest index inside the interval where p is below or at q,
  // clamped to the range [-maxAbscissa, maxAbscissa].

  SpacingType i;
  if (UseSpacing)
    i = (static_cast<SpacingType>(q.i+p.i) + 
        static_cast<SpacingType>(q.y-p.y) / (sqr(s) * static_cast<SpacingType>(q.i-p.i))) / 2;
  else
    // If the user chose SpacingType = ApexHeightType = AbscissaIndexType,
    // there will be no casting involved.
    // If an integer type is used, the computation can be off by at most -1.
    i = ((q.i + p.i) + (q.y - p.y) / (q.i - p.i)) / 2;

  return clampToAbscissaIndices(i);
}

//
// Clamp x to the range of abscissa indices.
//
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
inline
typename LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::AbscissaIndexType
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::clampToAbscissaIndices(const SpacingType &x)
{
  // To compare without warnings, we need maxAbscissa in the type
  // SpacingType.
  const SpacingType o(maxAbscissa);
  return static_cast<AbscissaIndexType>(x < -o ? -o : (x > o ? o : x));
}


//
// Constructor for a LowerEnvelopeOfParabolas
//
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::LowerEnvelopeOfParabolas(
    const typename Parabolas::size_type &expectedNumberOfParabolas,
    const SpacingType &_s) : s(_s)
{
  // First check the constraints on the types
  // All numeric types must be signed
  assert(std::numeric_limits<AbscissaIndexType>::is_signed);
  assert(std::numeric_limits<ApexHeightType>::is_signed);
  assert(std::numeric_limits<SpacingType>::is_signed);

  // minimalSpacing must be less than or equal to 1.
  assert(minimalSpacing <= 1);

  // SpacingType must be able to express minimalSpacing^2
  assert(0 < minimalSpacing * minimalSpacing);

  // The interval covered by SpacingType must contain those covered by
  // AbscissaIndexType and ApexHeightType. We only check for the maximum,
  // because numeric_limits<T>::min() is negative for integer types and
  // positive for floating points types.
  assert(std::numeric_limits<SpacingType>::max() >= std::numeric_limits<AbscissaIndexType>::max());
  assert(std::numeric_limits<SpacingType>::max() >= std::numeric_limits<SpacingType>::max());

  // Make sure that a vector can hold the expected number of parabolas and the
  // sentinels.
  assert(expectedNumberOfParabolas <= std::numeric_limits<typename Parabolas::size_type>::max() - 2);

  // Make room for the expected number of parabolas and two sentinel parabolas
  envelope.reserve(expectedNumberOfParabolas + 2);

  // Add a sentinel parabola in the front. It has the effect that every
  // parabola with a larger apex abscissa will intersect at a position larger
  // than -maxAbscissa.
  // This fascilitates the addParabola method.
  envelope.push_back(ParabolaRegion(
        Parabola(-maxAbscissa, maxApexHeight),
        -maxAbscissa));
}

//
// Add a new parabola.
//
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
inline
void
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::addParabola(const AbscissaIndexType& pi, const ApexHeightType& py, const LabelType& l)
{
  Parabola p(pi, py, l);

  // Parabolas have to be added with increasing abscissas
  assert(p.i > envelope.back().p.i);

  // The region where the newly added parabola is minimal is extending to
  // +maxAbscissa, because with the highest abscissa it will eventually be below any
  // of the other parabolas in the envelope.
  // We are finding the abscissa where the minimal region is begining by
  // intersection with the parabolas in the envelope.
  AbscissaIndexType i;
  while ((i = intersection(envelope.back().p, p)) < envelope.back().dominantFrom)
  {
    // The new parabola is below the whole last parabola region. Hence, the
    // last parabola region is not part of the envelope anymore.
    envelope.pop_back();

    // Note that the intersection with the front sentinel (see constructor) is
    // never smaller than -maxAbscissa because of the clamping in
    // intersection(). At the same time, the dominance region for the sentinel
    // starts at -maxAbscissa. Hence, the evelope can never be empty and
    // the reference into envelope in the while condition will never be
    // invalid.
    assert(!envelope.empty());
  }

  // All parabolas that are made obsolete by the new one have been removed and
  // the new one can be inserted.
  envelope.push_back(ParabolaRegion(p, i));
}

//
// Evaluate the parabola at the abscissa index i
// Here, we don't prevent overflows explicitly.
template < bool UseSpacing, class SpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class LabelType,
           class AbscissaIndexType, class ApexHeightType >
inline
ApexHeightType
LowerEnvelopeOfParabolas< UseSpacing, SpacingType, MinimalSpacingPrecision,
                          CreateVoronoiMap, LabelType,
                          AbscissaIndexType, ApexHeightType >
::value(const Parabola& p, const AbscissaIndexType &i)
{
  if (UseSpacing)
    return static_cast<ApexHeightType>(
        sqr(s) *
        sqr(static_cast<SpacingType>(i - p.i)) + 
        static_cast<SpacingType>(p.y));
  else
    return sqr(static_cast<ApexHeightType>(i - p.i)) + p.y;
}
} // end namespace itk
#endif
