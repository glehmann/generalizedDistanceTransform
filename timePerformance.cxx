// Compare distance transformation performance
//
// Defining COMPAREWITHSIGNEDMAURER will include
// itk::SignedMaurerDistanceMapImageFilter in the performance test
//#define COMPAREWITHSIGNEDMAURER

#include <iostream>
// Defining NDEBUG disables the assertions in
// itk::GeneralizedDistanceTransformImageFilter
#define NDEBUG
#include "itkGeneralizedDistanceTransformImageFilter.h"

#ifdef COMPAREWITHSIGNEDMAURER
#include "itkSignedMaurerDistanceMapImageFilter.h"
#endif

#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkDanielssonDistanceMapImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkTimeProbe.h"


const int dimension = 3;

// Defining the image types
typedef itk::Image<short, dimension> LabelImage;
typedef itk::Image<short, dimension> FunctionImage;

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    std::cerr << 
      "Comparison of itk::DanielsonDistanceMapImageFilter and\n"
      "itk:GeneralizedDistanceTransformImageFilter.\n"
      "Produces no output.\n"
      "\n"
      "USAGE: " << argv[0] << " <label image>\n";
    return 1;
  }

  // Read the input
  typedef itk::ImageFileReader<LabelImage> Reader;
  Reader::Pointer img = Reader::New();
  img->SetFileName(argv[1]);
  img->Update();
  std::cout << "Image Size: " << img->GetOutput()->GetLargestPossibleRegion().GetSize() << "\n";


  // DanielsonDistanceMapImageFilter with spacing
  {
    typedef itk::DanielssonDistanceMapImageFilter<LabelImage, FunctionImage> Distance;
    Distance::Pointer distance = Distance::New();
    distance->UseImageSpacingOn();
    distance->SquaredDistanceOn();
    distance->SetInput(img->GetOutput());

    img->Update();
    itk::TimeProbe timer;
    timer.Start();
    distance->Update();
    timer.Stop();
    std::cout << "DanielsonDistanceMapImageFilter with spacing: " 
              << timer.GetMeanTime() << " seconds.\n";
  }

  // DanielsonDistanceMapImageFilter without spacing
  {
    typedef itk::DanielssonDistanceMapImageFilter<LabelImage, FunctionImage> Distance;
    Distance::Pointer distance = Distance::New();
    distance->UseImageSpacingOff();
    distance->SquaredDistanceOn();
    distance->SetInput(img->GetOutput());

    img->Update();
    itk::TimeProbe timer;
    timer.Start();
    distance->Update();
    timer.Stop();
    std::cout << "DanielsonDistanceMapImageFilter without spacing: " 
              << timer.GetMeanTime() << " seconds.\n";
  }

#ifdef COMPAREWITHSIGNEDMAURER
  // SignedMaurerDistanceMapImageFilter with spacing
  {
    typedef itk::SignedMaurerDistanceMapImageFilter<LabelImage, FunctionImage> Distance;
    Distance::Pointer distance = Distance::New();
    distance->UseImageSpacingOn();
    distance->SquaredDistanceOn();
    distance->SetInput(img->GetOutput());

    img->Update();
    itk::TimeProbe timer;
    timer.Start();
    distance->Update();
    timer.Stop();
    std::cout << "SignedMaurerDistanceMapImageFilter with spacing: " 
              << timer.GetMeanTime() << " seconds.\n";
  }

  // SignedMaurerDistanceMapImageFilter without spacing
  {
    typedef itk::SignedMaurerDistanceMapImageFilter<LabelImage, FunctionImage> Distance;
    Distance::Pointer distance = Distance::New();
    distance->UseImageSpacingOff();
    distance->SquaredDistanceOn();
    distance->SetInput(img->GetOutput());

    img->Update();
    itk::TimeProbe timer;
    timer.Start();
    distance->Update();
    timer.Stop();
    std::cout << "SignedMaurerDistanceMapImageFilter without spacing: " 
              << timer.GetMeanTime() << " seconds.\n";
  }
#endif

  // GeneralizedDistanceTransformImageFilter with spacing, with Voronoi map
  {
    typedef itk::GeneralizedDistanceTransformImageFilter<FunctionImage, FunctionImage> DTF;

    // Convert the label image into an indicator function
    // Of course, we include the computation of the indicator function
    // in the timings
    typedef itk::BinaryThresholdImageFilter<LabelImage, FunctionImage> Threshold;
    Threshold::Pointer threshold = Threshold::New();
    threshold->SetInput(img->GetOutput());
    threshold->SetLowerThreshold(1);
    threshold->SetUpperThreshold(std::numeric_limits<FunctionImage::PixelType>::max());
    threshold->SetInsideValue(0);
    threshold->SetOutsideValue(DTF::LEOPUV::maxApexHeight);
    
    DTF::Pointer distance = DTF::New();
    distance->SetInput1(threshold->GetOutput());
    distance->SetInput2(img->GetOutput());

    img->Update();
    itk::TimeProbe timer;
    timer.Start();
    distance->Update();
    timer.Stop();
    std::cout << "GeneralizedDistanceTransformImageFilter with spacing, with Voronoi map: " 
              << timer.GetMeanTime() << " seconds.\n";
  }

  // GeneralizedDistanceTransformImageFilter with spacing, without Voronoi map
  {
    typedef itk::GeneralizedDistanceTransformImageFilter<FunctionImage, FunctionImage> DTF;

    // Convert the label image into an indicator function
    // Of course, we include the computation of the indicator function
    // in the timings
    typedef itk::BinaryThresholdImageFilter<LabelImage, FunctionImage> Threshold;
    Threshold::Pointer threshold = Threshold::New();
    threshold->SetInput(img->GetOutput());
    threshold->SetLowerThreshold(1);
    threshold->SetUpperThreshold(std::numeric_limits<FunctionImage::PixelType>::max());
    threshold->SetInsideValue(0);
    threshold->SetOutsideValue(DTF::LEOPUV::maxApexHeight);
    
    DTF::Pointer distance = DTF::New();
    distance->SetInput1(threshold->GetOutput());

    img->Update();
    itk::TimeProbe timer;
    timer.Start();
    distance->Update();
    timer.Stop();
    std::cout << "GeneralizedDistanceTransformImageFilter with spacing, without Voronoi map: " 
              << timer.GetMeanTime() << " seconds.\n";
  }

  // GeneralizedDistanceTransformImageFilter without spacing, with Voronoi map
  {
    typedef itk::GeneralizedDistanceTransformImageFilter<FunctionImage,
            FunctionImage, FunctionImage, 0> DTF;

    // Convert the label image into an indicator function
    // Of course, we include the computation of the indicator function
    // in our timings
    typedef itk::BinaryThresholdImageFilter<LabelImage, FunctionImage> Threshold;
    Threshold::Pointer threshold = Threshold::New();
    threshold->SetInput(img->GetOutput());
    threshold->SetLowerThreshold(1);
    threshold->SetUpperThreshold(std::numeric_limits<FunctionImage::PixelType>::max());
    threshold->SetInsideValue(0);
    threshold->SetOutsideValue(DTF::LEOPUV::maxApexHeight);
    
    DTF::Pointer distance = DTF::New();
    distance->SetInput1(threshold->GetOutput());
    distance->SetInput2(img->GetOutput());
    distance->SetUseSpacing(true);
    distance->SetCreateVoronoiMap(true);

    img->Update();
    itk::TimeProbe timer;
    timer.Start();
    distance->Update();
    timer.Stop();
    std::cout << "GeneralizedDistanceTransformImageFilter without spacing, with Voronoi map: " 
              << timer.GetMeanTime() << " seconds.\n";
  }

  // GeneralizedDistanceTransformImageFilter without spacing, without Voronoi map
  {
    typedef itk::GeneralizedDistanceTransformImageFilter<FunctionImage,
            FunctionImage, FunctionImage, 0> DTF;

    // Convert the label image into an indicator function
    // Of course, we include the computation of the indicator function
    // in our timings
    typedef itk::BinaryThresholdImageFilter<LabelImage, FunctionImage> Threshold;
    Threshold::Pointer threshold = Threshold::New();
    threshold->SetInput(img->GetOutput());
    threshold->SetLowerThreshold(1);
    threshold->SetUpperThreshold(std::numeric_limits<FunctionImage::PixelType>::max());
    threshold->SetInsideValue(0);
    threshold->SetOutsideValue(DTF::LEOPUV::maxApexHeight);
    
    DTF::Pointer distance = DTF::New();
    distance->SetInput1(threshold->GetOutput());
    distance->SetUseSpacing(false);
    distance->SetCreateVoronoiMap(false);

    img->Update();
    itk::TimeProbe timer;
    timer.Start();
    distance->Update();
    timer.Stop();
    std::cout << "GeneralizedDistanceTransformImageFilter without spacing, without Voronoi map: " 
              << timer.GetMeanTime() << " seconds.\n";
  }
  return 0;
}

