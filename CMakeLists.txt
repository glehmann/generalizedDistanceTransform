#Change PROJECT_NAME to the name of your project
PROJECT(generalizedDistanceTransform)

# set the name of the input image used to run the test
SET(INPUT_IMAGE ${CMAKE_SOURCE_DIR}/images/)

#include some macros from another file...
INCLUDE(IJMacros.txt)

#The following lines are required to use Dart
ENABLE_TESTING()
INCLUDE(Dart)
FIND_PROGRAM(IMAGE_COMPARE ImageCompare)

#Declare any external dependencies that your project may have here.
#examples include: ITK, VTK, JPEG, PNG, OpenGL, ZLIB, Perl, Java
#If you're not sure what name to use, look in the Modules directory of your
#cmake install and check that a file named Find(Package).cmake exists
SET(Required_Packages
  ITK
)

#this foreach loads all of the packages that you specified as required.
#It shouldn't need to be modified.
FOREACH(Package ${Required_Packages})
  LOADPACKAGE(${Package})
ENDFOREACH(Package)

#Set any libraries that your project depends on.
#examples: ITKCommon, VTKRendering, etc
SET(Libraries
  ITKCommon
  ITKBasicFilters
  ITKIO
)


# install devel files
OPTION(INSTALL_DEVEL_FILES "Install C++ headers" ON)
IF(INSTALL_DEVEL_FILES)
FILE(GLOB develFiles *.h *.txx) 
FOREACH(f ${develFiles})
  INSTALL_FILES(/include/InsightToolkit/BasicFilters FILES ${f})
ENDFOREACH(f)
ENDIF(INSTALL_DEVEL_FILES)



# option for wrapping
OPTION(BUILD_WRAPPERS "Wrap library" OFF)
IF(BUILD_WRAPPERS)
   SUBDIRS(Wrapping)
ENDIF(BUILD_WRAPPERS)
   
   

#the following block of code is an example of how to build an executable in
#cmake.  Unmodified, it will add an executable called "MyExe" to the project.
#MyExe will be built using the files MyClass.h and MyClass.cxx, and it will
#be linked to all the libraries you specified above. 
#You can build more than one executable per project

IF(BUILD_TESTING)

  FOREACH(CurrentExe
    euclideanDistanceTransform
    signedEuclideanDistanceTransform
    euclideanDistanceAndVoronoiTransform
    euclideanDistanceAndVectorDistanceTransform
    unionOfSpheres
    timePerformance
    cachePerformance)

    ADD_EXECUTABLE(${CurrentExe} ${CurrentExe}.cxx)
    TARGET_LINK_LIBRARIES(${CurrentExe} ${Libraries})
  ENDFOREACH(CurrentExe)

ENDIF(BUILD_TESTING)

#the following line is an example of how to add a test to your project.
#Testname is the title for this particular test.  ExecutableToRun is the
#program which will be running this test.  It can either be a part of this
#project or an external executable.  After that list any args that are needed
#for this test.  Include as many tests as you like.  If your project doesn't have
#any tests you can comment out or delete the following line.
# ADD_TEST(Testname ExecutableToRun arg1 arg2 arg3)

ADD_TEST(EuclideanDistanceTransform euclideanDistanceTransform ${INPUT_IMAGE}/threeVoxels.label.img euclideanDistanceTransform.img)
ADD_TEST(EuclideanDistanceTransformCompareImage ${IMAGE_COMPARE} euclideanDistanceTransform.img ${INPUT_IMAGE}/euclideanDistanceTransform.img)

ADD_TEST(EuclideanDistanceAndVectorDistanceTransform euclideanDistanceAndVectorDistanceTransform ${INPUT_IMAGE}/threeVoxels.label.img euclideanDistanceAndVectorDistanceTransform-distance.img euclideanDistanceAndVectorDistanceTransform-vector.nrrd)
ADD_TEST(EuclideanDistanceAndVectorDistanceTransformCompareDistance ${IMAGE_COMPARE} euclideanDistanceAndVectorDistanceTransform-distance.img ${INPUT_IMAGE}/euclideanDistanceTransform.img)
ADD_TEST(EuclideanDistanceAndVectorDistanceTransformCompareVector ${IMAGE_COMPARE} euclideanDistanceAndVectorDistanceTransform-vector.nrrd ${INPUT_IMAGE}/euclideanDistanceAndVectorDistanceTransform-vector.nrrd)

ADD_TEST(EuclideanDistanceAndVoronoiTransform euclideanDistanceAndVoronoiTransform ${INPUT_IMAGE}/threeVoxels.label.img euclideanDistanceAndVoronoiTransform-distance.img euclideanDistanceAndVoronoiTransform-label.img)
ADD_TEST(EuclideanDistanceAndVoronoiTransformCompareDistance ${IMAGE_COMPARE} euclideanDistanceAndVoronoiTransform-distance.img ${INPUT_IMAGE}/euclideanDistanceTransform.img)
ADD_TEST(EuclideanDistanceAndVoronoiTransformCompareLabel ${IMAGE_COMPARE} euclideanDistanceAndVoronoiTransform-label.img ${INPUT_IMAGE}/euclideanDistanceAndVoronoiTransform-label.img)

ADD_TEST(UnionOfSpheres unionOfSpheres ${INPUT_IMAGE}/threeVoxels.radius.img ${INPUT_IMAGE}/threeVoxels.label.img unionOfSpheres-union.img unionOfSpheres-voronoi.img)
ADD_TEST(UnionOfSpheresCompareUnion ${IMAGE_COMPARE} unionOfSpheres-union.img ${INPUT_IMAGE}/unionOfSpheres-union.img)
ADD_TEST(UnionOfSpheresCompareVoronoi ${IMAGE_COMPARE} unionOfSpheres-voronoi.img ${INPUT_IMAGE}/unionOfSpheres-voronoi.img)






