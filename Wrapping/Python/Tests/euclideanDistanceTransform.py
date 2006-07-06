
import itk
# itk.auto_progress(True)
from sys import argv, exit

if len(argv) != 3:
  print """Compute the euclidean distance transform of an image.

USAGE:  <input image> <output image>
  <input image>: An image where background voxels have value 0.
  <output image>: An image that denotes the euclidean distance to the
                  closest foreground voxel."""
  exit(1)

dimension = 3;
PixelType = itk.US
ImageType = itk.Image[PixelType, dimension];

DistancePixelType = itk.F
DistanceImageType = itk.Image[DistancePixelType, dimension];

# Read the input image
input = itk.ImageFileReader[ImageType].New( FileName=argv[1] )

# We are using the distance transform without Voronoi map generation
Distance = itk.GeneralizedDistanceTransformImageFilter[ImageType, DistanceImageType]

# For the label image l, create an indicator image i with 
# i(x) = (l(x) == 0 ?  infinity : 0).
# We retrieve the largest possible value from the Distance type.
indicator = itk.BinaryThresholdImageFilter[ImageType, ImageType].New( input,
                                  LowerThreshold=0,
                                  UpperThreshold=0,
                                  OutsideValue=0,
                                  InsideValue=65535)
#                                  InsideValue=Distance.GetMaximumApexHeight())

# Now the indicator image is fed into the distance transform...
distance = Distance.New(indicator, CreateVoronoiMap=False)
#print itk.range(distance)

# ...and converted from the squared euclidean distance to the regular
# euclidean distance
sqrt = itk.SqrtImageFilter[DistanceImageType, DistanceImageType].New( distance )

cast = itk.CastImageFilter[DistanceImageType, ImageType].New( sqrt )
itk.write( cast, argv[2] )

