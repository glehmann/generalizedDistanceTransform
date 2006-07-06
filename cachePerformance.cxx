// Execute the GeneralizedDistanceTransformImageFilter in order to analyze
// memory access performance with the non-intrusive profiling tool OProfile
//
// You can modify itk::GeneralizedDistanceTransformImageFilter to process the
// scanlines of only one dimension instead of iterating over all of them. Then
// memory latency problems due to cache misses become apparent.

// Defining NDEBUG disables the assertions in
// itk::GeneralizedDistanceTransformImageFilter
#include <iostream>
#define NDEBUG
#include "itkGeneralizedDistanceTransformImageFilter.h"
#include "itkImageFileReader.h"
#include "itkTimeProbe.h"


// Defining the image types
const int dimension = 3;
typedef itk::Image<short, dimension> ImageType;

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    std::cerr << 
      "Perform a single run of itk::GeneralizedDistanceTransformImageFilter\n"
      "Used to test memory performance. No output is produced.\n"
      "\n"
      "USAGE: " << argv[0] << " <function image>\n";

    return 1;
  }

  // Read the input
  typedef itk::ImageFileReader<ImageType> Reader;
  Reader::Pointer img = Reader::New();
  img->SetFileName(argv[1]);
  img->Update();
  std::cout << "Image Size: " << img->GetOutput()->GetLargestPossibleRegion().GetSize() << "\n";


  typedef itk::GeneralizedDistanceTransformImageFilter<ImageType, ImageType> DTF;
  DTF::Pointer distance = DTF::New();
  distance->SetInput1(img->GetOutput());
  distance->SetInput2(img->GetOutput()); // Label image for the Voronoi map

  img->Update(); // Make sure that the input image is up to date

  itk::TimeProbe timer;
  timer.Start();
  distance->Update();
  timer.Stop();
  std::cout << "GeneralizedDistanceTransformImageFilter took " 
            << timer.GetMeanTime() << " seconds.\n";

  return 0;
}

