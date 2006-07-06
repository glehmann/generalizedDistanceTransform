#ifndef __itkGeneralizedDistanceTransformImageFilter_txx
#define __itkGeneralizedDistanceTransformImageFilter_txx

#include <iostream>

#include "itkGeneralizedDistanceTransformImageFilter.h"
#include "itkImageLinearIteratorWithIndex.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

namespace itk
{


/**
 *    Constructor
 */
template < class TFunctionImage,class TDistanceImage,
  bool CreateVoronoiMap, class TLabelImage,
  bool UseSpacing, class TSpacingType, unsigned char MinimalSpacingPrecision >
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::GeneralizedDistanceTransformImageFilter()
{
  if (CreateVoronoiMap)
  {
    this->SetNumberOfRequiredInputs(2);
    this->SetNumberOfRequiredOutputs(2);
  }
  else
  {
    this->SetNumberOfRequiredInputs(1);
    this->SetNumberOfRequiredOutputs(1);
  }

  DistanceImagePointer distance = DistanceImageType::New();
  this->SetNthOutput(0, distance.GetPointer());

  if (CreateVoronoiMap)
  {
    LabelImagePointer voronoiMap = LabelImageType::New();
    this->SetNthOutput(1, voronoiMap.GetPointer());
  }
}

/**
 * Connect the function image
 */
template < class TFunctionImage,class TDistanceImage,
  bool CreateVoronoiMap, class TLabelImage,
  bool UseSpacing, class TSpacingType, unsigned char MinimalSpacingPrecision >
void
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::SetInput1(const FunctionImageType *functionImage)
{
  // Process object is not const-correct so the const casting is required.
  SetNthInput(0, const_cast<FunctionImageType *>(functionImage));
}

/**
 * Connect the label image
 */
template < class TFunctionImage,class TDistanceImage,
  bool CreateVoronoiMap, class TLabelImage,
  bool UseSpacing, class TSpacingType, unsigned char MinimalSpacingPrecision >
void
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::SetInput2(const LabelImageType *labelImage)
{
  // Process object is not const-correct so the const casting is required.
  SetNthInput(1, const_cast<LabelImageType *>(labelImage));
}

/**
 *  Return the distance map
 */
template < class TFunctionImage,class TDistanceImage,
  bool CreateVoronoiMap, class TLabelImage,
  bool UseSpacing, class TSpacingType, unsigned char MinimalSpacingPrecision >
typename
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::DistanceImageType*
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::GetDistance(void)
{
  return  dynamic_cast<DistanceImageType *>(this->ProcessObject::GetOutput(0));
}

/**
 *  Return the voronoi map if CreateVoronoiMap == true
 */
template < class TFunctionImage,class TDistanceImage,
  bool CreateVoronoiMap, class TLabelImage,
  bool UseSpacing, class TSpacingType, unsigned char MinimalSpacingPrecision >
typename
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::LabelImageType*
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::GetVoronoiMap()
{
  assert(CreateVoronoiMap);
  return  dynamic_cast<LabelImageType *>(this->ProcessObject::GetOutput(1));
}

/** 
 * The whole output will be produced regardless of the region requested.
 */
template < class TFunctionImage,class TDistanceImage,
  bool CreateVoronoiMap, class TLabelImage,
  bool UseSpacing, class TSpacingType, unsigned char MinimalSpacingPrecision >
void
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::EnlargeOutputRequestedRegion(DataObject *)
{
  this->GetDistance()->SetRequestedRegion(this->GetDistance()->GetLargestPossibleRegion());
  if (CreateVoronoiMap)
    this->GetVoronoiMap()->SetRequestedRegion(this->GetVoronoiMap()->GetLargestPossibleRegion());
}

/**
 * Allocate and initialize output images. Helper function for GenerateData()
 */
template < class TFunctionImage,class TDistanceImage,
  bool CreateVoronoiMap, class TLabelImage,
  bool UseSpacing, class TSpacingType, unsigned char MinimalSpacingPrecision >
void 
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::PrepareData() 
{

  // Copy the function image into the distance image
  FunctionImageConstPointer functionImage  =
    dynamic_cast<FunctionImageType *>(ProcessObject::GetInput(0));

  DistanceImagePointer distance = this->GetDistance();
  distance->SetBufferedRegion(distance->GetRequestedRegion());
  distance->Allocate();

  ImageRegionConstIterator<FunctionImageType> 
    functionIt(functionImage, distance->GetRequestedRegion());
  ImageRegionIterator<DistanceImageType>
    distanceIt(distance, distance->GetRequestedRegion());

  functionIt.GoToBegin();
  distanceIt.GoToBegin();
  while(!distanceIt.IsAtEnd())
  {
    distanceIt.Set(static_cast<typename DistanceImageType::PixelType>(functionIt.Get()));

    ++functionIt;
    ++distanceIt;
  }

  if (CreateVoronoiMap)
  {
    // Copy the label image into the voronoi map
    LabelImagePointer labelImage  =
      dynamic_cast<LabelImageType *>(ProcessObject::GetInput(1));

    LabelImagePointer voronoiMap = this->GetVoronoiMap();
    voronoiMap->SetBufferedRegion(voronoiMap->GetRequestedRegion());
    voronoiMap->Allocate();

    ImageRegionConstIterator<LabelImageType> 
      labelIt(labelImage, distance->GetRequestedRegion());
    ImageRegionIterator<LabelImageType>
      voronoiIt(voronoiMap, distance->GetRequestedRegion());

    labelIt.GoToBegin();
    voronoiIt.GoToBegin();
    while(!voronoiIt.IsAtEnd())
    {
      voronoiIt.Set(labelIt.Get());
      ++labelIt;
      ++voronoiIt;
    }
  }
}


/**
 *  Compute Distance and Voronoi maps
 *  \todo Support progress methods/callbacks.
 */
template < class TFunctionImage,class TDistanceImage,
  bool CreateVoronoiMap, class TLabelImage,
  bool UseSpacing, class TSpacingType, unsigned char MinimalSpacingPrecision >
void 
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::GenerateData() 
{
  this->PrepareData();

  // We need the size and probably the spacing of the images.
  DistanceImagePointer distance = this->GetDistance();
  typename DistanceImageType::SpacingType spacing = distance->GetSpacing();
  typename DistanceImageType::SizeType size = distance->GetRequestedRegion().GetSize();

  // The distance image has been initialized to contain the function values
  // f(x) at x = (x1 x2 ... xN).
  // It is transformed into the lower envelope of spherical paraboloids rooted
  // at (x1 x2 ... xN f(x)) by iteration over the dimensions of the image.
  // Information on the region covered by a paraboloid is provided optionally
  // by copying the label at x.
  //
  // The iterations visit each scanline in each dimension
  //
  // \todo Row-major image layouts can cause a lot of cache misses for each
  //       iteration but the first. Blocked image layouts might be of
  //       advantage in that case.

  typedef itk::ImageLinearIteratorWithIndex<DistanceImageType> DIt;
  DIt distanceIt(distance, distance->GetRequestedRegion());

  // We create an iterator for the voronoi map to make the
  // following loop compileable. It won't be used, if voronoi maps are
  // disabled and should be optimized away.
  //
  // A cleaner solution would be specializations of GenerateData() on
  // CreateVoronoiMap and UseSpacing.
  typedef itk::ImageLinearIteratorWithIndex<LabelImageType> LIt;
  LIt voronoiMapIt;
  if (CreateVoronoiMap)
  {
    LabelImagePointer voronoiMap = 
      dynamic_cast<LabelImageType *>(this->ProcessObject::GetOutput(1));
    voronoiMapIt = LIt(voronoiMap, voronoiMap->GetRequestedRegion());
  }

  // Loop over all dimensions and compute the generalized distance transform
  // and voronoi map for each scanline.
  //
  // \todo The scanlines are be handled independently and thus are a good
  // opportunity for parallelization/threading.
  //
  // \todo non-local memory access for dimensions >= 1 can lead to ineffective
  // use of the cache. Another layout could improve performance.

  for (unsigned int d = 0; d < FunctionImageType::ImageDimension; ++d)
  {
    distanceIt.SetDirection(d);
    distanceIt.GoToBegin();

    if (CreateVoronoiMap)
    {
      voronoiMapIt.SetDirection(d);
      voronoiMapIt.GoToBegin();
    }

    while (!distanceIt.IsAtEnd())
    {
      // Compute the generalized distance transform for the current scanline

      // First compute the lower envelope of parabolas
      // The spacing is ignored by LEOP if UseSpacing == false. We provide a
      // dummy value of 1 anyway.
      LEOP envelope(size[d], UseSpacing ? static_cast<TSpacingType>(spacing[d]) : 1);

      while (!distanceIt.IsAtEndOfLine())
      {
        typename LEOP::AbscissaIndexType i = distanceIt.GetIndex()[d];

        if (CreateVoronoiMap)
        {
          envelope.addParabola(i, distanceIt.Value(), voronoiMapIt.Value());
          ++voronoiMapIt;
          ++distanceIt;
        }
        else
        {
          envelope.addParabola(i, distanceIt.Value());
          ++distanceIt;
        }
      }

      // And now evaluate the lower envelope for the whole scanline
      distanceIt.GoToBeginOfLine();
      if (CreateVoronoiMap)
        voronoiMapIt.GoToBeginOfLine();

      if (CreateVoronoiMap)
      {
        envelope.uniformSample(distanceIt.GetIndex()[d], size[d], distanceIt, voronoiMapIt);
        voronoiMapIt.NextLine();
        distanceIt.NextLine();
      }
      else
      {
        envelope.uniformSample(distanceIt.GetIndex()[d], size[d], distanceIt);
        distanceIt.NextLine();
      }
    }
  }
} // end GenerateData()

/**
 *  Print Self
 *  \todo Add information on the constraints on abscissas and apex heights.
 */
template < class TFunctionImage,class TDistanceImage,
  bool CreateVoronoiMap, class TLabelImage,
  bool UseSpacing, class TSpacingType, unsigned char MinimalSpacingPrecision >
void 
GeneralizedDistanceTransformImageFilter< TFunctionImage, TDistanceImage, 
  CreateVoronoiMap, TLabelImage,
  UseSpacing, TSpacingType, MinimalSpacingPrecision >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Generalized Distance Transform: " << std::endl;
}
} // end namespace itk
#endif
