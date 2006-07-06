#include "itkImageFileReader.h"
#include "itkMinusSqrAccessor.h"
#include "itkIndicatorAccessor.h"
#include "itkAdaptImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkGeneralizedDistanceTransformImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageFileWriter.h"

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    std::cerr << 
      "Compute the union of spheres when sphere radii are given with a\n"
      "radius image\n"
      "\n"
      "USAGE: " << argv[0] << " <radius image> <label image> \\\n"
      "                         <union output> <voronoi output>\n"
      "  <radius image>: Each position x hold the radius of a sphere\n"
      "    centered at x.\n"
      "  <label image>: An image where background voxels have label 0.\n"
      "  <union output>: A binary image that is 1 for points inside a sphere.\n"
      "  <voronoi output>: Contains the label of the closest sphere center\n";
    return 1;
  }

  const unsigned int dimension=3;
  typedef short RadiusPixelType;
  typedef short DistancePixelType;
  typedef short LabelPixelType;
  typedef itk::Image<RadiusPixelType, dimension> RadiusImageType;
  typedef itk::Image<DistancePixelType, dimension> DistanceImageType;
  typedef itk::Image<LabelPixelType, dimension> LabelImageType;

  // Read the input images
  typedef itk::ImageFileReader<RadiusImageType> RadiusReaderType;
  RadiusReaderType::Pointer radius = RadiusReaderType::New();
  radius->SetFileName(argv[1]);

  typedef itk::ImageFileReader<LabelImageType> LabelReaderType;
  LabelReaderType::Pointer label = LabelReaderType::New();
  label->SetFileName(argv[2]);

  // For a given r, the 0-level set of f(p) = p^2 - r^2 is a sphere of radius
  // r. First we convert the radius image r(x) to r'(x) = -r(x)*r(x).
  typedef itk::Accessor::MinusSqrAccessor<RadiusPixelType, DistancePixelType> RadiusAccessor;
  typedef itk::Functor::AccessorFunctor<RadiusPixelType, RadiusAccessor> RadiusFunctor;
  typedef itk::UnaryFunctorImageFilter<RadiusImageType, DistanceImageType, RadiusFunctor>
    NegSquaredRadius;
  NegSquaredRadius::Pointer negSquaredRadius = NegSquaredRadius::New();
  negSquaredRadius->SetInput(radius->GetOutput());

  // We further have to indicate the background voxels.  We get the background
  // indicator value from GeneralizedDistanceTransformImageFilter's instance
  // of itk::LowerEnvelopeOfParabolas
  typedef itk::GeneralizedDistanceTransformImageFilter<DistanceImageType, DistanceImageType,true, LabelImageType> GDT;
  typedef itk::Accessor::IndicatorAccessor<DistancePixelType, DistancePixelType> IndicatorAccessor;
  typedef itk::Functor::AccessorFunctor<DistancePixelType, IndicatorAccessor> IndicatorFunctor;
  typedef itk::UnaryFunctorImageFilter<DistanceImageType, DistanceImageType, IndicatorFunctor> Indicator;
  Indicator::Pointer sampledFunction = Indicator::New();
  sampledFunction->GetFunctor().GetAccessor().SetNotThereValue(GDT::LEOP::maxApexHeight);
  sampledFunction->SetInput(negSquaredRadius->GetOutput());

  // Now all is set for the GeneralizedDistanceTransformImageFilter.
  GDT::Pointer distance = GDT::New();
  distance->SetInput1(sampledFunction->GetOutput());
  // Voronoi maps also work for the generalized distance.
  distance->SetInput2(label->GetOutput());

  // Threshold the output for <=0 in order to get the union of spheres
  typedef itk::BinaryThresholdImageFilter<DistanceImageType, DistanceImageType> Threshold;
  Threshold::Pointer threshold = Threshold::New();
  threshold->SetLowerThreshold(std::numeric_limits<short>::min());
  threshold->SetUpperThreshold(0);
  threshold->SetInsideValue(1);
  threshold->SetOutsideValue(0);
  threshold->SetInput(distance->GetOutput());

  // Write the union of spheres and the voronoi map
  typedef itk::ImageFileWriter<DistanceImageType> DistanceWriterType;
  DistanceWriterType::Pointer spheresWriter = DistanceWriterType::New();
  spheresWriter->SetInput(threshold->GetOutput());
  spheresWriter->SetFileName(argv[3]);
  spheresWriter->Update();

  typedef itk::ImageFileWriter<LabelImageType> LabelWriterType;
  LabelWriterType::Pointer voronoiWriter = LabelWriterType::New();
  voronoiWriter->SetInput(distance->GetVoronoiMap());
  voronoiWriter->SetFileName(argv[4]);
  voronoiWriter->Update();
}
