#ifndef ITKINDICATORACCESSOR_H
#define ITKINDICATORACCESSOR_H

namespace itk
{
namespace Accessor
{

/** \class An Accessor that exchanges value 0 with another value
 *
 * Its indented use was with the itk::GeneralizedDistanceTransformImageFilter.
 * There, background voxels have to be marked with a high positive value if
 * they should not participate in the distance transformation. */
template <class TInternalType, class TExternalType >
class ITK_EXPORT IndicatorAccessor  
{
  public:
    typedef TExternalType ExternalType;
    typedef TInternalType InternalType;

    /** Default constructor. Sets the exchange value to the highest possible
     * value. */
    IndicatorAccessor()
      : m_NotThere(std::numeric_limits<ExternalType>::max())
    {
    }

    /** Set another value to set for 0-voxels. */
    void SetNotThereValue(const ExternalType &notThereValue)
    {
      m_NotThere = notThereValue;
    }

    /** Access the image. */
    inline TExternalType Get(const TInternalType & input) const
    { return (TExternalType)(input == 0 ? m_NotThere : input); }

  private:
    TExternalType m_NotThere;
};
  
}
}
#endif
