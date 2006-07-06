#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkGeneralizedDistanceTransformImageFilter.h"
#include "itkSqrtImageFilter.h"
#include "itkImageFileWriter.h"

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    std::cerr << 
      "Compute the euclidean distance transform of an image.\n"
      "\n"
      "USAGE: " << argv[0] << " <input image> <output image>\n"
      "  <input image>: An image where background voxels have value 0.\n"
      "  <output image>: An image that denotes the euclidean distance to the\n"
      "                  closest foreground voxel.\n";
    return 1;
  }

  const unsigned int dimension = 3;
  typedef short PixelType;
  typedef itk::Image<PixelType, dimension> ImageType;

  // Read the input image
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer input = ReaderType::New();
  input->SetFileName(argv[1]);

  // We are using the distance transform without Voronoi map generation
  typedef itk::GeneralizedDistanceTransformImageFilter<ImageType, ImageType> Distance;

  // For the label image l, create an indicator image i with 
  // i(x) = (l(x) == 0 ?  infinity : 0).
  // We retrieve the largest possible value from the Distance type.
  typedef itk::BinaryThresholdImageFilter<ImageType, ImageType> Indicator;
  Indicator::Pointer indicator = Indicator::New();
  indicator->SetLowerThreshold(0);
  indicator->SetUpperThreshold(0);
  indicator->SetOutsideValue(0);
  indicator->SetInsideValue(Distance::GetMaximumApexHeight());
  indicator->SetInput(input->GetOutput());

  // Now the indicator image is fed into the distance transform...
  Distance::Pointer distance = Distance::New();
  distance->SetInput1(indicator->GetOutput());
  distance->SetCreateVoronoiMap(false);

  // ...and converted from the squared euclidean distance to the regular
  // euclidean distance
  typedef itk::SqrtImageFilter<ImageType, ImageType> Sqrt;
  Sqrt::Pointer sqrt = Sqrt::New();
  sqrt->SetInput(distance->GetOutput());

  // Write
  typedef itk::ImageFileWriter<ImageType> Writer;
  Writer::Pointer writer = Writer::New();
  writer->SetInput(sqrt->GetOutput());
  writer->SetFileName(argv[2]);
  writer->Update();
}
