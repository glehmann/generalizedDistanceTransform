#include "itkImageFileReader.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkGeneralizedDistanceTransformImageFilter.h"
#include "itkSqrtImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkImageFileWriter.h"

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    std::cerr <<
      "Compute the euclidean distance transform and vector distance map of an image.\n"
      "\n"
      "USAGE: " << argv[0] << " <label image> <distance output> <vector output>\n"
      "  <label image>: An image where background voxels have label 0.\n"
      "  <distance output>: An image that denotes the euclidean distance to\n"
      "     the closest foreground voxel.\n"
      "  <vector output>: An image that denotes the offset to the closest\n"
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

  // Create an image that contains the position of a voxel (aka identity)
  typedef itk::Vector<float, dimension> VectorType;
  typedef itk::Image<VectorType, dimension> VectorImageType;
  VectorImageType::Pointer position = VectorImageType::New();
  input->Update();
  position->SetRegions(input->GetOutput()->GetLargestPossibleRegion());
  position->SetSpacing(input->GetOutput()->GetSpacing());
  position->Allocate();

  ImageType::SpacingType spacing = input->GetOutput()->GetSpacing();

  itk::ImageRegionIteratorWithIndex<VectorImageType> 
    positionIt(position, position->GetLargestPossibleRegion());
  positionIt.GoToBegin();
  while (!positionIt.IsAtEnd())
  {
    VectorImageType::IndexType i = positionIt.GetIndex();
    VectorType v;
    for (unsigned int j = 0; j < dimension; ++j)
      v[j] = i[j] * spacing[j];
    positionIt.Set(v);

    ++positionIt;
  }
  std::cout << "Vector image has been created.\n";

  // The default for GeneralizedDistanceTransformImageFilter is to compute a
  // Voronoi map that has the same type as the distance image.
  typedef itk::GeneralizedDistanceTransformImageFilter<ImageType, ImageType,
          true, VectorImageType> Distance;

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
  indicator->Update();
  std::cout << "Indicator updated.\n";

  // Now the indicator and label images are fed into the distance transform...
  Distance::Pointer distance = Distance::New();
  distance->SetInput1(indicator->GetOutput());
  distance->SetInput2(position);
  distance->Update();
  std::cout << "distance updated.\n";

  // ...and the squared euclidean distance is converted to the regular
  // euclidean distance
  typedef itk::SqrtImageFilter<ImageType, ImageType> Sqrt;
  Sqrt::Pointer sqrt = Sqrt::New();
  sqrt->SetInput(distance->GetOutput());
  sqrt->Update();
  std::cout << "sqrt updated.\n";

  // We also convert the Voronoi map, which contains th position of the
  // closest foreground voxel, into an vector distance map that contains the
  // offset to the closest foreground voxel
  typedef itk::SubtractImageFilter<VectorImageType, VectorImageType, VectorImageType> Subtract;
  Subtract::Pointer offset = Subtract::New();
  offset->SetInput1(distance->GetVoronoiMap());
  offset->SetInput2(position);
  offset->Update();
  std::cout << "offset updated.\n";

  // Write the distance image
  typedef itk::ImageFileWriter<ImageType> Writer;
  Writer::Pointer writer = Writer::New();
  writer->SetInput(sqrt->GetOutput());
  writer->SetFileName(argv[2]);
  writer->Update();
  std::cout << "writer updated.\n";


  // Write the vector image
  typedef itk::ImageFileWriter<VectorImageType> VectorWriter;
  VectorWriter::Pointer vectorwriter = VectorWriter::New();
  vectorwriter->SetInput(offset->GetOutput());
  vectorwriter->SetFileName(argv[3]);
  vectorwriter->Update();
  std::cout << "vectorwriter updated.\n";
}
