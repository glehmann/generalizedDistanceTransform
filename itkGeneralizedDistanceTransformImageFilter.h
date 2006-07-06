#ifndef __itkGeneralizedDistanceTransformImageFilter_h
#define __itkGeneralizedDistanceTransformImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkLowerEnvelopeOfParabolas.h"

namespace itk
{

/** \class GeneralizedDistanceTransformImageFilter
*
* This filter computes a generalized variant of the distance transform with a
* squared euclidean metric. It can optionally compute a voronoi map as well.
*
* DEFINITION
* For a scalar function f, the generalized distance transform is defined as
*   dt_f(x) = min_p{(p-x)^2 + f(x)}
*
* If S is a foreground shape and we use an indicator function
*   i(x) = 0 if x \in S, \infinity otherwise
* then dt_i is the usual distance transform with a squared euclidean distance.
*
* APPLICATION
* The generalization is useful to create the union of spheres with different
* radius, for example. If r is a radius map and f(x) = -r(x)^2, then the <=0
* level set of dt_f is the union of all spheres implied by the radius map.
* This can be used in the context of group morphology with spherical structure
* elements that can vary in size across the image.
*
* ALTERNATIVE DESCRIPTION
* For an N-dimensional image, dt_f effectively computes the lower envelope of
* the spherical paraboloids of dimension N+1 with apexes at 
* (x1 ... xn f((x1 ...  xn)))
* 
* VORONOI MAP
* For the special case of dt_i, the paraboloids that participate in the lower
* envelope have their apexes on the foreground voxels. The voronoi map copies
* information from the apex abscissas to the whole region where the paraboloid
* is minimal. This information can be a label, a unique number, a position or
* anything else that is provided in a label map.
*
* USAGE TIPS
* Two of the template parameters are mandatory: The type of the function
* image which is the first input, and the type of the distance image, which is
* the standard output.
*
* If voronoi maps are generated, which is the default, you can specify the
* type of the label image as well. You can use any pixel type you like.
*
* If you don't care for spacing you can switch it off. In this case, using a
* different SpacingType might further improve runtime performance. It must
* cover the range of TFunctionImage::IndexValueType and
* TFunctionImage::PixelType, however. IndexValueType is most certainly long.
*
* MinimalSpacingPrecision m means that the minimal spacing that guarantees a
* correct computation of the distance transform is 10^-m. This must be covered
* by SpacingType as well, so you have to set MinimalSpacingPrecision to 0 if
* you are able to use an integer SpacingType.
*
* Fewer casts are made if 
*   TFunctionImage::PixelType == TFunctionImage::IndexValueType == SpacingType.
*
* REFERENCES
* The Implementation is based on the generalized distance transform with the
* squared euclidean metric described in:
* 
* Distance Transforms of Sampled Functions.
* Pedro F. Felzenszwalb and Daniel P. Huttenlocher.
* Cornell Computing and Information Science TR2004-1963.
*
* TODO
* - The algorithm could be made multithreaded: It iterates over the image
*   dimensions and during each iteration and individual scanlines are computed
*   independently of each other.
*
* - The iteration scanlines for dimensions > 0 are not memory local due to the
*   row-major layout of ITK's images. This trashes the cache. To solve this
*   issue, a blocked image layout could be used internally.
*   Maybe someone should write a itk::ImageToBlockedImageFilter and
*   itk::BlockedImageLinearIterator then.
*
* \ingroup ImageFeatureExtraction 
*
*/

template <
  class TFunctionImage,class TDistanceImage, class TLabelImage=TFunctionImage,
        unsigned char MinimalSpacingPrecision=3 >
class ITK_EXPORT GeneralizedDistanceTransformImageFilter :
    public ImageToImageFilter<TFunctionImage,TDistanceImage>
{
public:
  /** Standard class typedefs. */
  typedef GeneralizedDistanceTransformImageFilter Self;
  typedef ImageToImageFilter<TFunctionImage,TDistanceImage> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GeneralizedDistanceTransformImageFilter, ImageToImageFilter);

  /** Types and pointer types for the images. */
  typedef TFunctionImage FunctionImageType;
  typedef TDistanceImage DistanceImageType;
  typedef TLabelImage LabelImageType;

  typedef typename FunctionImageType::ConstPointer FunctionImageConstPointer;
  typedef typename DistanceImageType::Pointer DistanceImagePointer;
  typedef typename LabelImageType::Pointer LabelImagePointer;
  typedef typename TFunctionImage::SpacingType::ValueType TSpacingType;

  /** The main work is done by a class that computes the lower envelope of
   * parabolas. It can be tuned for performance vs. functionality by providing
   * template arguments.
   *
   * It has a few interesting class constants that provide valuable
   * information to the user of itk::GeneralizedDistanceTransformImageFilter.
   * Most noteworthy is ApexHeightInfinity, because this is the value that
   * should characterize background voxels.
   *
   * TODO: Avoid abbreviation */
  typedef itk::LowerEnvelopeOfParabolas<true, TSpacingType, MinimalSpacingPrecision, true,
          typename TLabelImage::PixelType,
          typename TFunctionImage::IndexValueType,
          typename TDistanceImage::PixelType> LEOPDV;
  typedef itk::LowerEnvelopeOfParabolas<true, TSpacingType, MinimalSpacingPrecision, false,
          typename TLabelImage::PixelType,
          typename TFunctionImage::IndexValueType,
          typename TDistanceImage::PixelType> LEOPDv;
  typedef itk::LowerEnvelopeOfParabolas<false, TSpacingType, MinimalSpacingPrecision, true,
          typename TLabelImage::PixelType,
          typename TFunctionImage::IndexValueType,
          typename TDistanceImage::PixelType> LEOPdV;
  typedef itk::LowerEnvelopeOfParabolas<false, TSpacingType, MinimalSpacingPrecision, false,
          typename TLabelImage::PixelType,
          typename TFunctionImage::IndexValueType,
          typename TDistanceImage::PixelType> LEOPdv;

  static typename TDistanceImage::PixelType GetMaximumApexHeight()
    { return LEOPDV::maxApexHeight; } 
   
  /** Connect the function image */
  void SetInput1(const FunctionImageType *functionImage);

  /** Connect the label image. Will only be used if a voronoi map is created. */
  void SetInput2(const LabelImageType *labelImage);

  /** Get distance transformed image.
   *
   * The distance for a voxel x is given by min_p((p-x)^2 + f(x)), i.e. the
   * squared eucliden metric is used. 
   *
   * This is equivalent to the standard GetOutput() method. */
  DistanceImageType* GetDistance(void);

  /** Get Voronoi Map
   *
   * For each voxel this map contains the label of the closest voxel. The word
   * "label" is loosly defined and can be anything you like, including a
   * vector that contains a voxel position from which you can derive a vector
   * distance map. */
  LabelImageType* GetVoronoiMap(void);

  /** Set/Get wether spacing should be used or not. */
  itkGetMacro(UseSpacing, bool);
  itkSetMacro(UseSpacing, bool);
  itkBooleanMacro(UseSpacing);

  /** Set/Get wether voronoi map should be created or not. */
  itkGetMacro(CreateVoronoiMap, bool);
  void SetCreateVoronoiMap(bool);
  itkBooleanMacro(CreateVoronoiMap);


protected:
  GeneralizedDistanceTransformImageFilter();
  virtual ~GeneralizedDistanceTransformImageFilter() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** The whole output will be produced regardless of the region requested. */
  void EnlargeOutputRequestedRegion(DataObject *itkNotUsed(output));

  /** Allocate and initialize output images. Helper function for
   * GenerateData() */
  void PrepareData();  

  /** Compute distance transform and optionally the voronoi map as well. */
  void GenerateData();  


private:   
  GeneralizedDistanceTransformImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  bool m_UseSpacing;
  bool m_CreateVoronoiMap;

}; // end of GeneralizedDistanceTransformImageFilter class

} //end namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGeneralizedDistanceTransformImageFilter.txx"
#endif

#endif
