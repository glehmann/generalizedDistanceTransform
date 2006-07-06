#ifndef __itkNegateInMaskFunctor_h
#define __itkNegateInMaskFunctor_h

namespace itk
{
  
/** \class NegateInMask
 * \brief Negates values that are inside a mask image
 *
 * This functor is parametrized over the types of the two 
 * input values and the output value. The second input value
 * serves as the mask image and 0 is considered the background value.
 *
 * Numeric conversions (castings) are done by the C++ defaults.
 * 
 * The filter expect all images to have the same dimension 
 * (e.g. all 2D, or all 3D, or all ND)
 */
namespace Functor {  
  
template< class TInput1, class TInput2, class TOutput>
class NegateInMask
{
public:
  NegateInMask() {};
  ~NegateInMask() {};
  bool operator!=(const NegateInMask &) const
  {
    return false;
  }

  bool operator==(const NegateInMask & other) const
  {
    return !(*this != other);
  }

  inline TOutput operator()(const TInput1 & value, const TInput2 & mask)
  {
    return static_cast<TOutput>( mask ? -value : value );
  }
}; 
}

} // end namespace itk
#endif
