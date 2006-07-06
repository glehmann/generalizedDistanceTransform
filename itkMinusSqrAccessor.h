#ifndef ITKMINUSSQRACCESSOR_H
#define ITKMINUSSQRACCESSOR_H

namespace itk
{
namespace Accessor
{

/** \class An accessor that converts a value v to -v^2.
 *
 * Intended for use with itk::GeneralizedDistanceTransformImageFilter when
 * that filter is used to compute the union of spheres where the sphere radius
 * is given by a radius image. */
template <class TInternalType, class TExternalType >
class ITK_EXPORT MinusSqrAccessor  
{
  public:
    typedef TExternalType ExternalType;
    typedef TInternalType InternalType;

    static inline TExternalType Get(const TInternalType & input) 
    { return (TExternalType)(-input*input); }
};
  
}
}
#endif
