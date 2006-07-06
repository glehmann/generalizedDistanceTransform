// Small program to create the test images for the paper.
//
#include "itkImage.h"
#include "itkImageFileWriter.h"

int main()
{
  typedef unsigned short PixelType;
  typedef itk::Image<PixelType, 3> ImageType;

  ImageType::SizeType size;
  size.Fill(31);
  ImageType::IndexType origin;
  origin.Fill(0);
  ImageType::RegionType region;
  region.SetSize(size);
  region.SetIndex(origin);

  ImageType::Pointer radius = ImageType::New();
  radius->SetRegions(region);
  radius->Allocate();
  radius->FillBuffer(0);

  ImageType::Pointer label = ImageType::New();
  label->SetRegions(region);
  label->Allocate();
  label->FillBuffer(0);

  // Put three spheres into the image
  ImageType::IndexType i;
  i[0] = i[1] = i[2] = 50;
  radius->SetPixel(i, 20);
  label->SetPixel(i, 1);

  i[0] = i[1] = 20;
  radius->SetPixel(i, 15);
  label->SetPixel(i, 2);

  i[0] = 80; i[1] = 50;
  radius->SetPixel(i, 27);
  label->SetPixel(i, 3);

  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(radius);
  writer->SetFileName("radiusImage.hdr");
  writer->Update();

  writer->SetInput(label);
  writer->SetFileName("labelImage.hdr");
  writer->Update();
}
