/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// SlicerRT PlanarImage includes
#include <vtkSlicerPlanarImageModuleLogic.h>
#include <vtkMRMLPlanarImageNode.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>

// SlicerRT MRML includes
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTPlanNode.h>

// VTK includes
#include <vtkTransform.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <string>

// ITK includes
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkInvertIntensityImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkFlipImageFilter.h>

// SlicerRT includes
#include <vtkSlicerRtCommon.h>

#include "itkImageFileWriter.h"

#include "itkSmoothingRecursiveGaussianImageFilter.h"

#include "itkPluginUtilities.h"

#include "plastimatch_slicer_drrCLP.h"

#include "plmreconstruct_config.h"
#include "drr_options.h"
#include "drr.h"
#include "plm_math.h"
#include "threading.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

int DoSetupDRR( int argc, char * argv[], Drr_options& options) throw(std::string)
{
  PARSE_ARGS;

  // Single image mode
  options.have_angle_diff = false;
  options.num_angles = 1;
  options.start_angle = 0.;

  // Imager MUST be with image window and image center
  options.have_image_center = 1;
  options.have_image_window = 1;
  options.image_resolution[0] = imagerResolution[0];
  options.image_resolution[1] = imagerResolution[1];
  options.image_size[0] = imagerResolution[0] * imagerSpacing[0];
  options.image_size[1] = imagerResolution[1] * imagerSpacing[1];

  float imageCenterX = float(imageWindow[2]) + float(imageWindow[3] - imageWindow[2] - 1) / 2.f; // columns
  float imageCenterY = float(imageWindow[0]) + float(imageWindow[1] - imageWindow[0] - 1) / 2.f; // rows

  options.image_center[0] = imageCenterX; // columns
  options.image_center[1] = imageCenterY; // rows

  // geometry
  // vup and normal vectors
  options.vup[0] = viewUpVector[0];
  options.vup[1] = viewUpVector[1];
  options.vup[2] = viewUpVector[2];

  options.have_nrm = 1;
  options.nrm[0] = normalVector[0];
  options.nrm[1] = normalVector[1];
  options.nrm[2] = normalVector[2];

  if (sourceImagerDistance < sourceAxisDistance)
  {
    throw std::string("SID is less than SAD");
  }
  options.sad = sourceAxisDistance;
  options.sid = sourceImagerDistance;

  // Isocenter DICOM (LPS) position
  options.isocenter[0] = isocenterPosition[0];
  options.isocenter[1] = isocenterPosition[1];
  options.isocenter[2] = isocenterPosition[2];

  // intensity
  options.exponential_mapping = static_cast<int>(exponentialMapping);
  options.autoscale = autoscale;
  if (autoscaleRange[0] >= autoscaleRange[1])
  {
    throw std::string("Autoscale range is wrong");
  }
  options.autoscale_range[0] = autoscaleRange[0];
  options.autoscale_range[1] = autoscaleRange[1];

  // processing
  options.threading = threading_parse(threading);

  if (!huconversion.compare("preprocess"))
  {
    options.hu_conversion = PREPROCESS_CONVERSION;
  }
  else if (!huconversion.compare("inline"))
  {
    options.hu_conversion = INLINE_CONVERSION;
  }
  else if (!huconversion.compare("none"))
  {
    options.hu_conversion = NO_CONVERSION;
  }
  else
  {
    throw std::string("Wrong huconversion value");
  }

  if (!algorithm.compare("exact"))
  {
    options.algorithm = DRR_ALGORITHM_EXACT;
  }
  else if (!algorithm.compare("uniform"))
  {
    options.algorithm = DRR_ALGORITHM_UNIFORM;
  }
  else
  {
    throw std::string("Wrong algorithm value");
  }

  if (!outputFormat.compare("pgm"))
  {
    options.output_format = OUTPUT_FORMAT_PGM;
  }
  else if (!algorithm.compare("pfm"))
  {
    options.output_format = OUTPUT_FORMAT_PFM;
  }
  else if (!algorithm.compare("raw"))
  {
    options.output_format = OUTPUT_FORMAT_RAW;
  }
  else
  {
    throw std::string("Wrong output file format");
  }

  options.output_prefix = std::string("Out");

/*

  switch (this->AlgorithmReconstuction)
  {
    case AlgorithmReconstuctionType::EXACT:
      command << "-i exact ";
      break;
    case AlgorithmReconstuctionType::UNIFORM:
      command << "-i uniform ";
      break;
    default:
      break;
  }

  switch (this->HUConversion)
  {
    case HounsfieldUnitsConversionType::NONE:
      command << "-P none ";
      break;
    case HounsfieldUnitsConversionType::PREPROCESS:
      command << "-P preprocess ";
      break;
    case HounsfieldUnitsConversionType::INLINE:
      command << "-P inline ";
      break;
    default:
      break;
  }

  command << "-O Out -t raw";
*/
  return EXIT_SUCCESS;
}

template <typename TPixel>
int DoIt( int argc, char * argv[], std::string& fileName, TPixel )
{
  PARSE_ARGS;

  typedef TPixel InputPixelType; // CT pixel type (short)
//  typedef signed long int OutputPixelType; // Plastimatch DRR pixel type (long)

  const unsigned int Dimension = 3;

  // CT image type and reader
  typedef itk::Image< InputPixelType, Dimension> InputImageType;
  typedef itk::ImageFileReader<InputImageType> InputReaderType;

  typename InputReaderType::Pointer inputReader = InputReaderType::New();
  inputReader->SetFileName( inputVolume.c_str() );

  // MetaImageHeader temporary writer
  typedef itk::ImageFileWriter<InputImageType>  InputWriterType;
  typename InputWriterType::Pointer inputWriter = InputWriterType::New();
  
  std::size_t found = inputVolume.find_last_of("/\\");
  std::string mhaFilename = "inputVolume.mha";
  if (found < inputVolume.size() - 1)
  {
    mhaFilename = inputVolume.substr( 0, found + 1) + mhaFilename;
    fileName = mhaFilename;
  }
  else if (found == inputVolume.size() or found == std::string::npos)
  {
    return EXIT_FAILURE;
  }
  
  inputWriter->SetFileName(mhaFilename.c_str());
  inputWriter->SetInput(inputReader->GetOutput());
  try
  {
    inputWriter->Update();
  }
  catch ( itk::ExceptionObject & excep )
  {
    throw;
  }
//    typedef itk::Image<OutputPixelType, Dimension> OutputImageType;
/*
  typedef itk::SmoothingRecursiveGaussianImageFilter<
    InputImageType, OutputImageType>  FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput( reader->GetOutput() );
  filter->SetSigma( sigma );

  typedef itk::ImageFileWriter<OutputImageType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputVolume.c_str() );
  writer->SetInput( filter->GetOutput() );
  writer->SetUseCompression(1);
  writer->Update();
*/
  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  Drr_options drrOptions;
  std::string mhaFileName;
  try
  {
    // Since there are no conditional flags, EVERY options must be defined
    // and checked.
    DoSetupDRR( argc, argv, drrOptions);

    itk::GetImageType(inputVolume, pixelType, componentType);

    // This filter handles all types on input, but only produces
    // signed types
    switch( componentType )
    {
    case itk::ImageIOBase::UCHAR:
      return DoIt( argc, argv, mhaFileName, static_cast<unsigned char>(0) );
      break;
    case itk::ImageIOBase::CHAR:
      return DoIt( argc, argv, mhaFileName, static_cast<signed char>(0) );
      break;
    case itk::ImageIOBase::USHORT:
      return DoIt( argc, argv, mhaFileName, static_cast<unsigned short>(0) );
      break;
    case itk::ImageIOBase::SHORT:
      return DoIt( argc, argv, mhaFileName, static_cast<short>(0) );
      break;
    case itk::ImageIOBase::UINT:
      return DoIt( argc, argv, mhaFileName, static_cast<unsigned int>(0) );
      break;
    case itk::ImageIOBase::INT:
      return DoIt( argc, argv, mhaFileName, static_cast<int>(0) );
      break;
    case itk::ImageIOBase::ULONG:
      return DoIt( argc, argv, mhaFileName, static_cast<unsigned long>(0) );
      break;
    case itk::ImageIOBase::LONG:
      return DoIt( argc, argv, mhaFileName, static_cast<long>(0) );
      break;
    case itk::ImageIOBase::FLOAT:
      return DoIt( argc, argv, mhaFileName, static_cast<float>(0) );
      break;
    case itk::ImageIOBase::DOUBLE:
      return DoIt( argc, argv, mhaFileName, static_cast<double>(0) );
      break;
    case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
    default:
      std::cerr << "Unknown input image pixel component type: ";
      std::cerr << itk::ImageIOBase::GetComponentTypeAsString( componentType );
      std::cerr << std::endl;
      return EXIT_FAILURE;
      break;
    }
  }
  catch( itk::ExceptionObject & excep )
  {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }
  catch( std::exception& ex )
  {
    std::cerr << argv[0] << ": std exception caught !" << std::endl;
    std::cerr << "Error message: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch( std::string& errorString )
  {
    std::cerr << argv[0] << ": exception message caught !" << std::endl;
    std::cerr << "Error message: " << errorString << std::endl;
    return EXIT_FAILURE;
  }
  catch( ... )
  {
    std::cerr << "Desaster, unknown exception caught!" << std::endl;
    return EXIT_FAILURE;
  }

  // Compute DRR image if mha file name is valid
  if (!mhaFileName.empty())
  {
    drrOptions.input_file = mhaFileName;
    drr_compute(&drrOptions);
  }
  return EXIT_SUCCESS;
}
