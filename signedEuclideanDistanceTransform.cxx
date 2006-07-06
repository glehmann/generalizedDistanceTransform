#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkGeneralizedDistanceTransformImageFilter.h"
#include "itkNegateInMaskFunctor.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkSqrtImageFilter.h"
#include "itkImageFileWriter.h"

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    std::cerr << 
      "Compute the signed euclidean distance transform of an image.\n"
      "\n"
      "USAGE: " << argv[0] << " <input image> <output image>\n"
      "  <input image>: An image where background voxels have value 0.\n"
      "  <output image>: The signed euclidean distance.\n";
    return 1;
  }

  const unsigned int dimension = 2;
  typedef short PixelType;
  typedef itk::Image<PixelType, dimension> ImageType;

  // Read the input image
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer input = ReaderType::New();
  input->SetFileName(argv[1]);

  // Binarize
  typedef itk::BinaryThresholdImageFilter<ImageType, ImageType> Threshold;
  Threshold::Pointer binary = Threshold::New();
  binary->SetLowerThreshold(0);
  binary->SetUpperThreshold(0);
  binary->SetOutsideValue(1);
  binary->SetInsideValue(0);
  binary->SetInput(input->GetOutput());

  // Compute the border voxels by erosion and subtraction.
  typedef itk::BinaryBallStructuringElement<PixelType, dimension> Ball;
  Ball ball;
  ball.SetRadius(1);
  ball.CreateStructuringElement();

  typedef itk::BinaryErodeImageFilter<ImageType, ImageType, Ball> Erode;
  Erode::Pointer erode = Erode::New();
  erode->SetErodeValue(1);
  erode->SetBackgroundValue(0);
  erode->SetKernel(ball);
  erode->SetInput(binary->GetOutput());

  typedef itk::SubtractImageFilter<ImageType, ImageType, ImageType> Border;
  Border::Pointer border = Border::New();
  border->SetInput1(binary->GetOutput());
  border->SetInput2(erode->GetOutput());

  // We are using the distance transform without Voronoi map generation
  typedef itk::GeneralizedDistanceTransformImageFilter<ImageType, ImageType, false> Distance;

  // For the border image l, create an indicator image i with 
  // i(x) = (l(x) == 0 ?  infinity : 0).
  // We retrieve the largest possible value from the Distance type.
  Threshold::Pointer indicator = Threshold::New();
  indicator->SetLowerThreshold(0);
  indicator->SetUpperThreshold(0);
  indicator->SetOutsideValue(0);
  indicator->SetInsideValue(Distance::LEOP::maxApexHeight);
  indicator->SetInput(border->GetOutput());

  // Now the border image is fed into the distance transform...
  Distance::Pointer distance = Distance::New();
  distance->SetInput1(indicator->GetOutput());

  // ...and converted from the squared euclidean distance to the regular
  // euclidean distance
  typedef itk::SqrtImageFilter<ImageType, ImageType> Sqrt;
  Sqrt::Pointer sqrt = Sqrt::New();
  sqrt->SetInput(distance->GetOutput());

  // The last step is to negate the values inside the segmented structure.
  typedef itk::Functor::NegateInMask<PixelType, PixelType, PixelType> NegateInMaskFunctor;
  typedef itk::BinaryFunctorImageFilter<ImageType, ImageType, ImageType, 
          NegateInMaskFunctor> NegateInMask;
  NegateInMask::Pointer negateInMask = NegateInMask::New();

  negateInMask->SetInput1(sqrt->GetOutput());
  negateInMask->SetInput2(binary->GetOutput());


  // Write
  typedef itk::ImageFileWriter<ImageType> Writer;
  Writer::Pointer writer = Writer::New();
  writer->SetInput(negateInMask->GetOutput());
  writer->SetFileName(argv[2]);
  writer->Update();
}
