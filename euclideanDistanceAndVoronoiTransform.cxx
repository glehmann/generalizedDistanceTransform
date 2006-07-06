#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkGeneralizedDistanceTransformImageFilter.h"
#include "itkSqrtImageFilter.h"
#include "itkImageFileWriter.h"

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    std::cerr <<
      "Compute the euclidean distance transform and Voronoi map of an image.\n"
      "\n"
      "USAGE: " << argv[0] << " <label image> <distance output> <label output>\n"
      "  <label image>: An image where background voxels have label 0.\n"
      "  <distance output>: An image that denotes the euclidean distance to\n"
      "     the closest foreground voxel.\n"
      "  <label output>: An image that denotes the label of the closest\n"
      "     foreground voxel.\n";
    return 1;
  }

  const unsigned int dimension=3;
  typedef short PixelType;
  typedef itk::Image<PixelType, dimension> ImageType;

  // Read the label image
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer input = ReaderType::New();
  input->SetFileName(argv[1]);

  // The default for GeneralizedDistanceTransformImageFilter is to compute a
  // Voronoi map that has the same type as the distance image.
  typedef itk::GeneralizedDistanceTransformImageFilter<ImageType, ImageType> Distance;

  // For the label image l, create an indicator image i with 
  // i(x) = (l(x) == 0 ?  infinity : 0).
  // We retrieve the largest possible value from the Distance type.
  typedef itk::BinaryThresholdImageFilter<ImageType, ImageType> Indicator;
  Indicator::Pointer indicator = Indicator::New();
  indicator->SetLowerThreshold(0);
  indicator->SetUpperThreshold(0);
  indicator->SetOutsideValue(0);
  indicator->SetInsideValue(Distance::LEOP::maxApexHeight);
  indicator->SetInput(input->GetOutput());

  // Now the indicator and label images are fed into the distance transform...
  Distance::Pointer distance = Distance::New();
  distance->SetInput1(indicator->GetOutput());
  distance->SetInput2(input->GetOutput());

  // ...and the squared euclidean distance is converted to the regular
  // euclidean distance
  typedef itk::SqrtImageFilter<ImageType, ImageType> Sqrt;
  Sqrt::Pointer sqrt = Sqrt::New();
  sqrt->SetInput(distance->GetOutput());

  // Write the distance image
  typedef itk::ImageFileWriter<ImageType> Writer;
  Writer::Pointer writer = Writer::New();
  writer->SetInput(sqrt->GetOutput());
  writer->SetFileName(argv[2]);
  writer->Update();

  // Write the label image
  writer->SetInput(distance->GetVoronoiMap());
  writer->SetFileName(argv[3]);
  writer->Update();
}
