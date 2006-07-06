#ifndef __itkLowerEnvelopeOfParabolas_h
#define __itkLowerEnvelopeOfParabolas_h

#include <vector>

namespace itk
{

/** \class A class to manage the lower envelope of a set of parabolas.
 *
 * 
 * The parabolas have to be added by providing an abscissa index i and a
 * height y for their apex. Together with the spacing s, this creates the
 * parabola
 *   f(x) = (x - i*s)^2 + y
 * If CreateVoronoiMap is enabled, you also have to provide some identifying value
 * for the parabola. It will be used when the lower envelope of the parabolas
 * is sampled.
 * 
 * The parabolas have to be added with increasing abscissa indices.
 * 
 * The lower envelope of the parabolas can be sampled uniformly at consecutive
 * index positions.
 * 
 * CONSTRAINTS
 * Signed integer types are required for the abscissa index and apex height.
 * 
 * A non-negative MinimalSpacingPrecision is mandatory, thus the largest
 * minimalSpacing is 1.
 * 
 * SpacingType must cover an interval at least as large than those covered by
 * AbscissaIndexType and ApexHeightType
 * 
 * Allowed ranges for minimal spacing, abscissa index, and apex height
 * prevent arithmetic overflows in the computation of parabola intersections
 * and are stored as member variables.
 * 
 * DEVELOPMENT INFO
 * In debugging mode, the class performs more error checking, especially on
 * the constraints.
 * 
 * Some optimizations depend on UseSpacing and
 * CreateVoronoiMap. This is limited to a few places and implemented with
 * if's. For a concrete class, one of the branches should be compiled away.
 * An alternative would be partial template spezialization.
 */

template < bool UseSpacing, class TSpacingType, unsigned char MinimalSpacingPrecision,
           bool CreateVoronoiMap, class TLabelType,
           class TAbscissaIndexType, class TApexHeightType >
class LowerEnvelopeOfParabolas
{
  public:
    /** Type traits */
    typedef TSpacingType SpacingType;
    typedef TAbscissaIndexType AbscissaIndexType;
    typedef TLabelType LabelType;
    typedef TApexHeightType ApexHeightType;

  private:
    /** Squaring routine */
    template <class T>
    inline T sqr(const T& x) { return x*x; }

    /** Clamp and cast x to the range of abscissa indices. */
    AbscissaIndexType clampToAbscissaIndices(const SpacingType &x);

    /** A parabola p(x) = (x-is)^2+y is defined by the apex abscissa index i
     * and the apex height y. Abscissa spacing s is constant for all parabolas
     * of an envelope. */
    struct Parabola
    {
      AbscissaIndexType i;
      ApexHeightType y;
      LabelType l;

      Parabola(const AbscissaIndexType&, const ApexHeightType&, const LabelType& = LabelType());
    };

    /** Evaluate the parabola at the abscissa index i */
    ApexHeightType value(const Parabola &p, const AbscissaIndexType &i);

    /** Compute the intersection abscissa of two parabolas p and q. Their apex abscissas
     * have to be different.
     *
     * Returns the largest index i for which p(i) <= q(i). */
    AbscissaIndexType intersection(const Parabola &p, const Parabola &q);

    
    /** A parabola region is a parabola and an abscissa that is the left border
     * of the open interval where the parabola is below all others. */
    struct ParabolaRegion
    {
      Parabola p;
      AbscissaIndexType dominantFrom;

      ParabolaRegion(const Parabola &p, const AbscissaIndexType &dominantFrom);
    };

    typedef std::vector<ParabolaRegion> Parabolas;

    /** An Iterator that returns the dominating parabola for a range of indices
     *
     * ATTENTION: Envelope must end in a sentinel parabola that has
     *            dominateFrom == maxAbscissa!
     *            The calling code should add one if it is not already present
     *            and remove it again if more parabolas should be added.
     *            The reason for this is to make the iteration more efficient.
     */
    class Iterator
    {
      public:
        /** Create an iterator that walks [from, to). */
        Iterator(const Parabolas& envelope, const AbscissaIndexType& from, const AbscissaIndexType &to);

        /** Get the current abscissa index. */
        const AbscissaIndexType& currentAbscissaIndex();

        /** Get the dominant parabola for the current abscissa index. */
        const Parabola& currentParabola();

        /** Proceed to the next index. */
        Iterator& operator++();

        /** Test for end of iteration. */
        bool IsAtEnd();

      private:
        const Parabolas &envelope;
        AbscissaIndexType currentIndex;
        AbscissaIndexType end;
        typename Parabolas::size_type currentParabolaNumber;
    };

    /** Member variables. */
    const SpacingType s;
    Parabolas envelope;

  public:
    /** Class constants that are used to prevent arithmetic overflow in the
     * computation of parabola intersections. */
    static const SpacingType minimalSpacing;
    static const AbscissaIndexType maxAbscissa;
    static const ApexHeightType maxApexHeight;

    /** Constructor
     *
     * Default spacing is 1. This is handy in the case that you don't want
     * spacing at all and don't want to pass a dummy value when using
     * LowerEnvelopeOfParabolas. */
    LowerEnvelopeOfParabolas(const typename Parabolas::size_type &expectedNumberOfParabolas,
        const SpacingType &s=1);

    /** Add a new parabola.
     * The apex abscissa has to be larger than those already in the envelope.
     */
    void addParabola(const AbscissaIndexType& i, const ApexHeightType& y,
        const LabelType& l = LabelType());

    /** Evaluate the lower envelope of parabolas at consecutive indices
     * Needs output iterators that have operator++ and a Set() method like
     * those of ITK.
     * Writes to the Voronoi map, too. */
    template <class ValueIt, class VoronoiIt>
    void uniformSample(const AbscissaIndexType &from, const long &steps,
        ValueIt &valueIt, VoronoiIt &voronoiIt)
    {
      // This routine accesses information that is only available if creation of
      // Voronoi maps was enabled.
      assert(CreateVoronoiMap == true);

      // Insert a sentinel parabola to define the right end of the dominance
      // region of the last parabola in the envelope
      envelope.push_back(
          ParabolaRegion(Parabola(maxAbscissa, maxApexHeight), maxAbscissa)
          );

      for (Iterator it(envelope, from, from + steps); !it.IsAtEnd(); ++it)
      {
        valueIt.Set(value(it.currentParabola(), it.currentAbscissaIndex()));
        voronoiIt.Set(it.currentParabola().l);

        ++valueIt;
        ++voronoiIt;
      }

      // Remove the back sentinel again
      envelope.pop_back();
    }

    /** Evaluate the lower envelope of parabolas at consecutive indices
     * Needs an output iterator that has operator++ and a Set() method like
     * those of ITK. */
    template <class ValueIt>
    void uniformSample(const AbscissaIndexType &from, const long &steps,
        ValueIt &valueIt)
    {
      // Insert a sentinel parabola to define the right end of the dominance
      // region of the last parabola in the envelope
      envelope.push_back(
          ParabolaRegion(Parabola(maxAbscissa, maxApexHeight), maxAbscissa)
          );

      for (Iterator it(envelope, from, from + steps); !it.IsAtEnd(); ++it)
      {
        valueIt.Set(value(it.currentParabola(), it.currentAbscissaIndex()));

        ++valueIt;
      }

      // Remove the back sentinel again
      envelope.pop_back();
    }
}; // end of LowerEnvelopeOfParabolas class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLowerEnvelopeOfParabolas.txx"
#endif

#endif
