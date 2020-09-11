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

// LoadableModuleTemplate Logic includes
#include "vtkSlicerPlmDrrLogic.h"

// SlicerRT includes
#include "vtkSlicerPlanarImageModuleLogic.h"
#include "vtkMRMLPlanarImageNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLLinearTransformNode.h>

#include <vtkMRMLMarkupsClosedCurveNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsLineNode.h>

#include "vtkMRMLPlmDrrNode.h"

// SubjectHierarchy includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// VTK includes
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <string>

// Plastimatch reconstruct module
#include <drr.h>
#include <drr_options.h>

// ITK includes
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkInvertIntensityImageFilter.h>
#include <itkCastImageFilter.h>

// SlicerRT includes
#include <vtkSlicerRtCommon.h>

const char* vtkSlicerPlmDrrLogic::IMAGER_BOUNDARY_MARKUPS_NODE_NAME = "ImagerBoundary"; // curve
const char* vtkSlicerPlmDrrLogic::IMAGE_WINDOW_MARKUPS_NODE_NAME = "ImageWindow"; // curve
const char* vtkSlicerPlmDrrLogic::FIDUCIALS_MARKUPS_NODE_NAME = "FiducialPoints"; // fiducial
const char* vtkSlicerPlmDrrLogic::NORMAL_VECTOR_MARKUPS_NODE_NAME = "NormalVector"; // line
const char* vtkSlicerPlmDrrLogic::VUP_VECTOR_MARKUPS_NODE_NAME = "VupVector"; // line

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPlmDrrLogic);
//vtkCxxSetObjectMacro(vtkSlicerPlmDrrLogic, PlanarImageLogic, vtkSlicerPlanarImageModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerPlmDrrLogic::vtkSlicerPlmDrrLogic()
  :
  PlanarImageLogic(nullptr)
{
}

//----------------------------------------------------------------------------
vtkSlicerPlmDrrLogic::~vtkSlicerPlmDrrLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLRTPlanNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTPlanNode>::New());
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLRTBeamNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTBeamNode>::New());
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLPlmDrrNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLPlmDrrNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::UpdateFromMRMLScene()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateFromMRMLScene: Invalid MRML scene");
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
  }

  if (node->IsA("vtkMRMLRTBeamNode"))
  {
    // Observe beam events
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkMRMLRTBeamNode::BeamTransformModified);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
  else if (node->IsA("vtkMRMLRTPlanNode"))
  {
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkMRMLRTPlanNode::IsocenterModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
  else if (node->IsA("vtkMRMLPlmDrrNode"))
  {
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//----------------------------------------------------------------------------
bool vtkSlicerPlmDrrLogic::SaveVolumeNode( const vtkMRMLVolumeNode* volumeNode, std::string& vtkNotUsed(filename))
{
  if (!volumeNode)
  {
    return false;
  }
  
  return false;
}

//----------------------------------------------------------------------------
bool vtkSlicerPlmDrrLogic::LoadDRR( vtkMRMLScalarVolumeNode* volumeNode, const std::string& filename)
{
  if (filename.empty())
  {
    vtkErrorMacro("LoadDRR: MetaImageHeader file name is empty");
    return false;
  }

  if (!volumeNode)
  {
    vtkErrorMacro("LoadDRR: volume node is invalid");
    return false;
  }

  // Plastimatch DRR input pixel type (long)
  using InputPixelType = signed long;
  const unsigned int InputDimension = 3;

  using InputImageType = itk::Image< InputPixelType, InputDimension >;
  using ReaderType = itk::ImageFileReader< InputImageType >;

  // Inner output pixel type (double)
  using OutputPixelType = double;
  using OutputImageType = itk::Image< OutputPixelType, InputDimension >;
  using CastFilterType = itk::CastImageFilter< InputImageType, OutputImageType >;
  using RescaleFilterType = itk::RescaleIntensityImageFilter< OutputImageType, OutputImageType >;
  using InvertFilterType = itk::InvertIntensityImageFilter< OutputImageType, OutputImageType >;

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(filename.c_str());
  try
  {
    reader->Update();
  }
  catch(itk::ExceptionObject& ex)
  {
    vtkErrorMacro("LoadDRR: Problem reading data " << ex);
    return false;
  }

  CastFilterType::Pointer cast = CastFilterType::New();
  cast->SetInput(reader->GetOutput());
  try
  {
    cast->Update();
  }
  catch(itk::ExceptionObject& ex)
  {
    vtkErrorMacro("LoadDRR: Image casting exception caught " << ex);
    return false;
  }

  RescaleFilterType::Pointer rescale = RescaleFilterType::New();
  rescale->SetOutputMinimum(0.);
  rescale->SetOutputMaximum(255.);
  rescale->SetInput(cast->GetOutput());
  try
  {
    rescale->Update();
  }
  catch(itk::ExceptionObject& ex)
  {
    vtkErrorMacro("LoadDRR: Image rescaling exception caught " << ex);
    return false;
  }

  InvertFilterType::Pointer invert = InvertFilterType::New();
  invert->SetInput(rescale->GetOutput());
  invert->SetMaximum(0);
  try
  {
    invert->Update();
  }
  catch(itk::ExceptionObject& ex)
  {
    vtkErrorMacro("LoadDRR: Problem inverting data \n" << ex);
    return false;
  }

  bool res = vtkSlicerRtCommon::ConvertItkImageToVolumeNode< OutputPixelType >( invert->GetOutput(), volumeNode, VTK_DOUBLE);
  return res;
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::CreateMarkupsNodes(vtkMRMLPlmDrrNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("CreateMarkupsNodes: Invalid MRML scene");
    return;
  }

  // Create markups nodes if they don't exist

  // Imager boundary markups node
  vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode> imagerMarkupsNode;
  if (!scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME))
  {
    imagerMarkupsNode = this->CreateImagerBoundary(parameterNode);
  }
  else
  {
    imagerMarkupsNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME));
    vtkWarningMacro("CreateMarkupsNodes: Update imager points using parameter node data");
  }

  // Image window markups node
  vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode> imageWindowMarkupsNode;
  if (!scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME))
  {
    imageWindowMarkupsNode = this->CreateImageWindow(parameterNode);
  }
  else
  {
    imageWindowMarkupsNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME));
    vtkWarningMacro("CreateMarkupsNodes: Update image window points using parameter node data");
  }

  // imager normal vector markups node
  vtkSmartPointer<vtkMRMLMarkupsLineNode> normalVectorMarkupsNode;
  if (!scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME))
  {
    normalVectorMarkupsNode = this->CreateImagerNormal(parameterNode);
  }
  else
  {
    normalVectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME));
    vtkWarningMacro("CreateMarkupsNodes: Update Normal vector points using parameter node data");
  }

  // imager vup vector markups node
  vtkSmartPointer<vtkMRMLMarkupsLineNode> vupVectorMarkupsNode;
  if (!scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME))
  {
    vupVectorMarkupsNode = this->CreateImagerVUP(parameterNode);
  }
  else
  {
    vupVectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME));
    vtkWarningMacro("CreateMarkupsNodes: Update VUP vector points using parameter node data");
  }

  // fiducial markups node
  vtkSmartPointer<vtkMRMLMarkupsFiducialNode> pointsMarkupsNode;
  if (!scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME))
  {
    pointsMarkupsNode = this->CreateFiducials(parameterNode);
  }
  else
  {
    pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME));
    vtkWarningMacro("CreateMarkupsNodes: Update fiducial points using parameter node data");
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::UpdateMarkupsNodes(vtkMRMLPlmDrrNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid MRML scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid parameter set node");
    return;
  }

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
  vtkTransform* beamTransform = nullptr;
  vtkNew<vtkMatrix4x4> mat;
  mat->Identity();

  if (beamTransformNode)
  {
    beamTransform = vtkTransform::SafeDownCast(beamTransformNode->GetTransformToParent());
    beamTransform->GetMatrix(mat);
  }
  else
  {
    vtkErrorMacro("UpdateMarkupsNodes: Beam transform node is invalid");
    return;
  }

  // Imager boundary markups node
  if (scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsClosedCurveNode* imagerMarkupsNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME));

    double distance = parameterNode->GetIsocenterImagerDistance();
    
    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double offset[2] = {};
    parameterNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.;
    double y = spacing[1] * dimention[1] / 2.;

    // add points
    vtkVector3d p0( -x + offset[0], y + offset[1], -distance);
    vtkVector3d p1( x + offset[0], y + offset[1], -distance);
    vtkVector3d p2( x + offset[0], -y + offset[1], -distance);
    vtkVector3d p3( -x + offset[0], -y + offset[1], -distance);

    double p[3];
    imagerMarkupsNode->GetNthControlPointPosition( 0, p);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();
    imagerMarkupsNode->SetNthControlPointPosition( 0, p[0], p[1], p[2]);

    imagerMarkupsNode->GetNthControlPointPosition( 1, p);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();
    imagerMarkupsNode->SetNthControlPointPosition( 1, p[0], p[1], p[2]);

    imagerMarkupsNode->GetNthControlPointPosition( 2, p);
    p[0] = p2.GetX();
    p[1] = p2.GetY();
    p[2] = p2.GetZ();
    imagerMarkupsNode->SetNthControlPointPosition( 2, p[0], p[1], p[2]);

    imagerMarkupsNode->GetNthControlPointPosition( 3, p);
    p[0] = p3.GetX();
    p[1] = p3.GetY();
    p[2] = p3.GetZ();
    imagerMarkupsNode->SetNthControlPointPosition( 3, p[0], p[1], p[2]);

    // Update imager boundary markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = imagerMarkupsNode->GetParentTransformNode();

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();

      if (markupsTransformNode && beamTransformNode->GetID() != markupsTransformNode->GetID())
      {
        imagerMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }

  // Image window markups node
  if (scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsClosedCurveNode* imageWindowMarkupsNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME));

    double distance = parameterNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double offset[2] = {};
    parameterNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // columns
    double y = spacing[1] * dimention[1] / 2.; // rows

    int imageWindow[4] = {};
    parameterNode->GetImageWindow(imageWindow);

    double r1 = y - imageWindow[1] * spacing[1];
    double c1 = imageWindow[0] * spacing[0] - x;
    double r2 = y - imageWindow[3] * spacing[1];
    double c2 = imageWindow[2] * spacing[0] - x;

    // add points
    vtkVector3d p0( c1, r1, -distance);
    vtkVector3d p1( c1, r2, -distance);
    vtkVector3d p2( c2, r2, -distance);
    vtkVector3d p3( c2, r1, -distance);

    double p[3];
    imageWindowMarkupsNode->GetNthControlPointPosition( 0, p);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();
    imageWindowMarkupsNode->SetNthControlPointPosition( 0, p[0], p[1], p[2]);

    imageWindowMarkupsNode->GetNthControlPointPosition( 1, p);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();
    imageWindowMarkupsNode->SetNthControlPointPosition( 1, p[0], p[1], p[2]);

    imageWindowMarkupsNode->GetNthControlPointPosition( 2, p);
    p[0] = p2.GetX();
    p[1] = p2.GetY();
    p[2] = p2.GetZ();
    imageWindowMarkupsNode->SetNthControlPointPosition( 2, p[0], p[1], p[2]);

    imageWindowMarkupsNode->GetNthControlPointPosition( 3, p);
    p[0] = p3.GetX();
    p[1] = p3.GetY();
    p[2] = p3.GetZ();
    imageWindowMarkupsNode->SetNthControlPointPosition( 3, p[0], p[1], p[2]);

    // Update image window markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = imageWindowMarkupsNode->GetParentTransformNode();

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();

      if (markupsTransformNode && beamTransformNode->GetID() != markupsTransformNode->GetID())
      {
        imageWindowMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }

  }

  // normal vector markups line node
  if (scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* vectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME));

    double distance = parameterNode->GetIsocenterImagerDistance();

    vtkVector3d p0( 0., 0., -distance);
    vtkVector3d p1( 0., 0., -distance + 100.);

    double p[3];
    vectorMarkupsNode->GetNthControlPointPosition( 0, p);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();
    vectorMarkupsNode->SetNthControlPointPosition( 0, p[0], p[1], p[2]);

    vectorMarkupsNode->GetNthControlPointPosition( 1, p);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();
    vectorMarkupsNode->SetNthControlPointPosition( 1, p[0], p[1], p[2]);

    // Update imager normal vector markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = vectorMarkupsNode->GetParentTransformNode();

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();

      if (markupsTransformNode && beamTransformNode->GetID() != markupsTransformNode->GetID())
      {
        vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }

  // vup vector markups line node
  if (scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* vectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME));

    double distance = parameterNode->GetIsocenterImagerDistance();

    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double offset[2] = {};
    parameterNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // columns
    double y = spacing[1] * dimention[1] / 2.; // rows

    // add points
    vtkVector3d p0( 0 + offset[0], 0 + offset[1], -distance);
    vtkVector3d p1( -x + offset[0], 0 + offset[1], -distance);

    double p[3];
    vectorMarkupsNode->GetNthControlPointPosition( 0, p);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();
    vectorMarkupsNode->SetNthControlPointPosition( 0, p[0], p[1], p[2]);

    vectorMarkupsNode->GetNthControlPointPosition( 1, p);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();
    vectorMarkupsNode->SetNthControlPointPosition( 1, p[0], p[1], p[2]);

    // Update VUP VECTOR markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = vectorMarkupsNode->GetParentTransformNode();

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();

      if (markupsTransformNode && beamTransformNode->GetID() != markupsTransformNode->GetID())
      {
        vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }

  // fiducial markups line node
  if (scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME));

    double distance = parameterNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double offset[2] = {};
    parameterNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // columns
    double y = spacing[1] * dimention[1] / 2.; // rows

    // add points
    vtkVector3d p0( 0, 0, -distance); // imager center
    vtkVector3d p1( 0, 0, -distance + 100.); // imager normal vector
    vtkVector3d p2( -x + offset[0], -y + offset[1], -distance); // (0,0)
    vtkVector3d p3( -x + offset[0], 0.0, -distance); // vup vector

    double p[3];
    pointsMarkupsNode->GetNthControlPointPosition( 0, p);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();
    pointsMarkupsNode->SetNthControlPointPosition( 0, p[0], p[1], p[2]);

    pointsMarkupsNode->GetNthControlPointPosition( 1, p);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();
    pointsMarkupsNode->SetNthControlPointPosition( 1, p[0], p[1], p[2]);

    pointsMarkupsNode->GetNthControlPointPosition( 2, p);
    p[0] = p2.GetX();
    p[1] = p2.GetY();
    p[2] = p2.GetZ();
    pointsMarkupsNode->SetNthControlPointPosition( 2, p[0], p[1], p[2]);

    pointsMarkupsNode->GetNthControlPointPosition( 3, p);
    p[0] = p3.GetX();
    p[1] = p3.GetY();
    p[2] = p3.GetZ();
    pointsMarkupsNode->SetNthControlPointPosition( 3, p[0], p[1], p[2]);

    // Update fiducials markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = pointsMarkupsNode->GetParentTransformNode();

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();

      if (markupsTransformNode && beamTransformNode->GetID() != markupsTransformNode->GetID())
      {
        pointsMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsClosedCurveNode* vtkSlicerPlmDrrLogic::CreateImagerBoundary(vtkMRMLPlmDrrNode* parameterNode)
{
  auto imagerMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode>::New();
  this->GetMRMLScene()->AddNode(imagerMarkupsNode);
  imagerMarkupsNode->SetName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME);
  imagerMarkupsNode->SetCurveTypeToLinear();
  imagerMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("DRR_") + IMAGER_BOUNDARY_MARKUPS_NODE_NAME;
  imagerMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (parameterNode)
  {
    double distance = parameterNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double offset[2] = {};
    parameterNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // columns
    double y = spacing[1] * dimention[1] / 2.; // rows

    // add points
    vtkVector3d p0( -x + offset[0], y + offset[1], -distance);
    vtkVector3d p1( x + offset[0], y + offset[1], -distance);
    vtkVector3d p2( x + offset[0], -y + offset[1], -distance);
    vtkVector3d p3( -x + offset[0], -y + offset[1], -distance);

    imagerMarkupsNode->AddControlPoint(p0); // "Upper Left", "-x,y"
    imagerMarkupsNode->AddControlPoint(p1); // "Upper Right", "x,y"
    imagerMarkupsNode->AddControlPoint(p2); // "Lower Right", "x,-y"
    imagerMarkupsNode->AddControlPoint(p3); // "Lower Left", "-x,-y"

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
      if (beamTransformNode)
      {
        imagerMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return imagerMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsClosedCurveNode* vtkSlicerPlmDrrLogic::CreateImageWindow(vtkMRMLPlmDrrNode* parameterNode)
{
  auto imageWindowMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode>::New();
  this->GetMRMLScene()->AddNode(imageWindowMarkupsNode);
  imageWindowMarkupsNode->SetName(IMAGE_WINDOW_MARKUPS_NODE_NAME);
  imageWindowMarkupsNode->SetCurveTypeToLinear();
  imageWindowMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("DRR_") + IMAGE_WINDOW_MARKUPS_NODE_NAME;
  imageWindowMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (parameterNode)
  {
    double distance = parameterNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double offset[2] = {};
    parameterNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // columns
    double y = spacing[1] * dimention[1] / 2.; // rows

    int imageWindow[4] = {};
    parameterNode->GetImageWindow(imageWindow);

    double r1 = y - imageWindow[1] * spacing[1];
    double c1 = imageWindow[0] * spacing[0] - x;
    double r2 = y - imageWindow[3] * spacing[1];
    double c2 = imageWindow[2] * spacing[0] - x;

    // add points
    vtkVector3d p0( c1, r1, -distance);
    vtkVector3d p1( c1, r2, -distance);
    vtkVector3d p2( c2, r2, -distance);
    vtkVector3d p3( c2, r1, -distance);

    imageWindowMarkupsNode->AddControlPoint(p0); // r1, c1
    imageWindowMarkupsNode->AddControlPoint(p1); // r2, c1
    imageWindowMarkupsNode->AddControlPoint(p2); // r2, c2
    imageWindowMarkupsNode->AddControlPoint(p3); // r1, c2

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
      if (beamTransformNode)
      {
        imageWindowMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return imageWindowMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerPlmDrrLogic::CreateImagerNormal(vtkMRMLPlmDrrNode* parameterNode)
{
  auto vectorMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsLineNode>::New();
  this->GetMRMLScene()->AddNode(vectorMarkupsNode);
  vectorMarkupsNode->SetName(NORMAL_VECTOR_MARKUPS_NODE_NAME);
  vectorMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("DRR_") + NORMAL_VECTOR_MARKUPS_NODE_NAME;
  vectorMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (parameterNode)
  {
    double distance = parameterNode->GetIsocenterImagerDistance();

    // add points
    vtkVector3d p0( 0, 0, -distance);
    vtkVector3d p1( 0, 0, -distance + 100.);

    vectorMarkupsNode->AddControlPoint(p0);
    vectorMarkupsNode->AddControlPoint(p1);

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
      if (beamTransformNode)
      {
        vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return vectorMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerPlmDrrLogic::CreateImagerVUP(vtkMRMLPlmDrrNode* parameterNode)
{
  auto vectorMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsLineNode>::New();
  this->GetMRMLScene()->AddNode(vectorMarkupsNode);
  vectorMarkupsNode->SetName(VUP_VECTOR_MARKUPS_NODE_NAME);
  vectorMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("DRR_") + VUP_VECTOR_MARKUPS_NODE_NAME;
  vectorMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (parameterNode)
  {
    double distance = parameterNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double offset[2] = {};
    parameterNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // columns
    double y = spacing[1] * dimention[1] / 2.; // rows

    // add points
    vtkVector3d p0( 0 + offset[0], 0 + offset[1], -distance);
    vtkVector3d p1( -x + offset[0], 0 + offset[1], -distance);

    vectorMarkupsNode->AddControlPoint(p0);
    vectorMarkupsNode->AddControlPoint(p1);

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
      if (beamTransformNode)
      {
        vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return vectorMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerPlmDrrLogic::CreateFiducials(vtkMRMLPlmDrrNode* parameterNode)
{
  auto pointsMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsFiducialNode>::New();
  this->GetMRMLScene()->AddNode(pointsMarkupsNode);
  pointsMarkupsNode->SetName(FIDUCIALS_MARKUPS_NODE_NAME);
  pointsMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("DRR_") + FIDUCIALS_MARKUPS_NODE_NAME;
  pointsMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (parameterNode)
  {
    double distance = parameterNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double offset[2] = {};
    parameterNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // columns
    double y = spacing[1] * dimention[1] / 2.; // rows

    // add points
    vtkVector3d p0( 0, 0, -distance); // imager center
    vtkVector3d p1( 0, 0, -distance + 100.); // n
    vtkVector3d p2( -x + offset[0], -y + offset[1], -distance); // (0,0)
    vtkVector3d p3( -x + offset[0], 0.0, -distance); // vup

    pointsMarkupsNode->AddControlPoint( p0, "Imager center");
    pointsMarkupsNode->AddControlPoint( p1, "n");
    pointsMarkupsNode->AddControlPoint( p2, "(0,0)");
    pointsMarkupsNode->AddControlPoint( p3, "VUP");

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
      if (beamTransformNode)
      {
        pointsMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return pointsMarkupsNode;
}

//----------------------------------------------------------------------------
std::string vtkSlicerPlmDrrLogic::GeneratePlastimatchDrrArgs( vtkMRMLVolumeNode* volumeNode, 
  vtkMRMLPlmDrrNode* parameterNode, std::list< std::string >& plastimatchArguments)
{
  if (!volumeNode)
  {
    vtkErrorMacro("GeneratePlastimatchDrrArgs: Invalid volume node");
    return std::string();
  }

  if (!parameterNode)
  {
    vtkErrorMacro("GeneratePlastimatchDrrArgs: Invalid parameter set node");
    return std::string();
  }

  vtkMRMLScalarVolumeNode* imageVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode);
  if (imageVolumeNode)
  {
    double volumeOrigin[3];
    imageVolumeNode->GetOrigin(volumeOrigin);
  }

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
  vtkTransform* beamTransform = nullptr;
  vtkNew<vtkMatrix4x4> mat;
  mat->Identity();

  if (beamTransformNode)
  {
    beamTransform = vtkTransform::SafeDownCast(beamTransformNode->GetTransformToParent());
    beamTransform->GetMatrix(mat);
  }
  else
  {
    vtkErrorMacro("GeneratePlastimatchDrrArgs: Beam transform node is invalid");
    return nullptr;
  }

  double normalVector[4] = { 0., 0., -1., 0. }; // beam negative Z-axis
  double viewUpVector[4] = { 1., 0., 0., 0. }; // beam positive X-axis
  double n[4], vup[4];

  mat->MultiplyPoint( normalVector, n);
  mat->MultiplyPoint( viewUpVector, vup);

  int res[2];
  parameterNode->GetImageDimention(res);
  double spacing[2];
  parameterNode->GetImageSpacing(spacing);

  double isocenter[3];
  beamNode->GetPlanIsocenterPosition(isocenter);

  int window[4];
  parameterNode->GetImageWindow(window);

//   plastimatch drr -nrm "1 0 0" \
//    -vup "0 0 1" \
//    -g "1000 1500" \
//    -r "1024 768" \
//    -z "400 300" \
//    -c "383.5 511.5" \
//    -o "0 -20 -50" \
//    -e -i uniform -O Out -t raw input_file.mha
/*
  drrOptions.threading = THREADING_CPU_SINGLE;
  drrOptions.detector_resolution[0] = 128;
  drrOptions.detector_resolution[1] = 128;
  drrOptions.image_size[0] = 600;
  drrOptions.image_size[1] = 600;
  drrOptions.have_image_center = 0;
  drrOptions.have_image_window = 0;
  drrOptions.isocenter[0] = 0.0f;
  drrOptions.isocenter[1] = 0.0f;
  drrOptions.isocenter[2] = 0.0f;

  drrOptions.start_angle = 0.f;
  drrOptions.num_angles = 1;
  drrOptions.have_angle_diff = 0;
  drrOptions.angle_diff = 1.0f;

  drrOptions.have_nrm = 0;
  drrOptions.nrm[0] = 1.0f;
  drrOptions.nrm[1] = 0.0f;
  drrOptions.nrm[2] = 0.0f;
  drrOptions.vup[0] = 0.0f;
  drrOptions.vup[1] = 0.0f;
  drrOptions.vup[2] = 1.0f;

  drrOptions.sad = 1000.0f;
  drrOptions.sid = 1630.0f;
  drrOptions.manual_scale = 1.0f;

  drrOptions.exponential_mapping = 0;
  drrOptions.output_format= OUTPUT_FORMAT_PFM;
  drrOptions.hu_conversion = PREPROCESS_CONVERSION;
  drrOptions.output_details_prefix = "";
  drrOptions.output_details_fn = "";
  drrOptions.algorithm = DRR_ALGORITHM_EXACT;
  drrOptions.input_file = "";
  drrOptions.geometry_only = 0;
  drrOptions.output_prefix = "out_";

  // Imager resolution
  drrOptions.detector_resolution[0] = res[1];
  drrOptions.detector_resolution[1] = res[0];

  // VUP vector
  drrOptions.vup[0] = static_cast<float>(vup[0]);
  drrOptions.vup[1] = static_cast<float>(vup[1]);
  drrOptions.vup[2] = static_cast<float>(vup[2]);

  // Imager normal vector
  drrOptions.have_nrm = 1;
  drrOptions.nrm[0] = static_cast<float>(n[0]);
  drrOptions.nrm[1] = static_cast<float>(n[1]);
  drrOptions.nrm[2] = static_cast<float>(n[2]);

  // SAD, SID distance
  drrOptions.sad = static_cast<float>(beamNode->GetSAD());
  drrOptions.sid = static_cast<float>(beamNode->GetSAD() + parameterNode->GetIsocenterImagerDistance());

  // Image size
  drrOptions.image_size[0] = static_cast<float>(res[1] * spacing[1]);
  drrOptions.image_size[1] = static_cast<float>(res[0] * spacing[0]);

  // Image center
  drrOptions.have_image_center = 1;
  drrOptions.image_center[0] = static_cast<float>(res[1] / 2.);
  drrOptions.image_center[1] = static_cast<float>(res[0] / 2.);

  // Isocenter
  drrOptions.isocenter[0] = static_cast<float>(isocenter[0]);
  drrOptions.isocenter[1] = static_cast<float>(isocenter[1]);
  drrOptions.isocenter[2] = static_cast<float>(isocenter[2]);

  // Image window
  drrOptions.have_image_window = 1;
  drrOptions.image_window[0] = window[1];
  drrOptions.image_window[1] = window[3];
  drrOptions.image_window[2] = window[0];
  drrOptions.image_window[3] = window[2];

  // Algorithm
  drrOptions.algorithm = DRR_ALGORITHM_EXACT;
  // Output format
  drrOptions.output_format = OUTPUT_FORMAT_RAW;
  // Output prefix
  drrOptions.output_prefix = "Out";
*/
  std::ostringstream command;
  command << "plastimatch drr ";
  command << "--nrm" << " \"" << n[0] << " " << n[1] << " " << n[2] << "\" \\" << "\n";
  command << "\t--vup" << " \"" << vup[0] << " " << vup[1] << " " << vup[2] << "\" \\" << "\n";
  command << "\t--sad " << beamNode->GetSAD() << " --sid " << beamNode->GetSAD() + parameterNode->GetIsocenterImagerDistance() << " \\" << "\n";
  command << "\t-r" << " \"" << res[1] << " " << res[0] << "\" \\" << "\n";
  command << "\t-z" << " \"" << res[1] * spacing[1] << " " << res[0] * spacing[0] << "\" \\" << "\n";
  command << "\t-c" << " \"" << double(res[1]) / 2. << " " << double(res[0]) / 2. << "\" \\" << "\n";
  command << "\t-o" << " \"" << isocenter[0] << " " << isocenter[1] << " " << isocenter[2] << "\" \\" << "\n";
  command << "\t-w" << " \"" << window[1] << " " << window[3] << " " << window[0] << " " << window[2] << "\" \\" << "\n";
  command << "\t-e -i uniform -O Out -t raw";

  plastimatchArguments.clear();
  plastimatchArguments.push_back("drr");
  plastimatchArguments.push_back("--nrm");
  std::ostringstream arg1;
  arg1 << n[0] << " " << n[1] << " " << n[2];
  plastimatchArguments.push_back(arg1.str());
  
  plastimatchArguments.push_back("--vup");
  std::ostringstream arg2;
  arg2 << vup[0] << " " << vup[1] << " " << vup[2];
  plastimatchArguments.push_back(arg2.str());

  plastimatchArguments.push_back("--sad");
  plastimatchArguments.push_back(std::to_string(beamNode->GetSAD()));
  plastimatchArguments.push_back("--sid");
  plastimatchArguments.push_back(std::to_string(beamNode->GetSAD() + parameterNode->GetIsocenterImagerDistance()));

  plastimatchArguments.push_back("-r");
  std::ostringstream arg3;
  arg3 << res[1] << " " << res[0];
  plastimatchArguments.push_back(arg3.str());

  plastimatchArguments.push_back("-z");
  std::ostringstream arg4;
  arg4 << res[1] * spacing[1] << " " << res[0] * spacing[0];
  plastimatchArguments.push_back(arg4.str());

  plastimatchArguments.push_back("-c");
  std::ostringstream arg5;
  arg5 << double(res[1]) / 2. << " " << double(res[0]) / 2.;
  plastimatchArguments.push_back(arg5.str());
  
  plastimatchArguments.push_back("-o");
  std::ostringstream arg6;
  arg6 << isocenter[0] << " " << isocenter[1] << " " << isocenter[2];
  plastimatchArguments.push_back(arg6.str());

  plastimatchArguments.push_back("-w");
  std::ostringstream arg7;
  arg7 << window[1] << " " << window[3] << " " << window[0] << " " << window[2];
  plastimatchArguments.push_back(arg7.str());

  plastimatchArguments.push_back("-e");
  plastimatchArguments.push_back("-i");
  plastimatchArguments.push_back("uniform");
  plastimatchArguments.push_back("-O");
  plastimatchArguments.push_back("Out");
  plastimatchArguments.push_back("-t");
  plastimatchArguments.push_back("raw");

  return command.str();
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  Superclass::ProcessMRMLNodesEvents( caller, event, callData);

  vtkMRMLScene* mrmlScene = this->GetMRMLScene();
  if (!mrmlScene)
  {
    vtkErrorMacro("ProcessMRMLNodesEvents: Invalid MRML scene");
    return;
  }
  if (mrmlScene->IsBatchProcessing())
  {
    return;
  }

  if (caller->IsA("vtkMRMLRTBeamNode"))
  {
//    vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(caller);
    if (event == vtkMRMLRTBeamNode::BeamTransformModified)
    {
      vtkDebugMacro("ProcessMRMLNodesEvents: RTBeam transformation has been changed");
    }
    else if (event == vtkMRMLRTBeamNode::BeamGeometryModified)
    {
      vtkDebugMacro("ProcessMRMLNodesEvents: RTBeam geometry has been changed");
    }
  }
  else if (caller->IsA("vtkMRMLRTPlanNode"))
  {
//    vtkMRMLRTPlanNode* planNode = vtkMRMLRTPlanNode::SafeDownCast(caller);
    if (event == vtkMRMLRTPlanNode::IsocenterModifiedEvent)
    {
      vtkDebugMacro("ProcessMRMLNodesEvents: RTPlan isocenter has been changed");
    }
  }
  else if (caller->IsA("vtkMRMLPlmDrrNode"))
  {
//    vtkMRMLPlmDrrNode* parameterNode = vtkMRMLPlmDrrNode::SafeDownCast(caller);
    if (event == vtkCommand::ModifiedEvent)
    {
      vtkDebugMacro("ProcessMRMLNodesEvents: Plastimatch DRR node modified");
    }
  }
}

//---------------------------------------------------------------------------
bool vtkSlicerPlmDrrLogic::LoadRtImage( vtkMRMLPlmDrrNode* paramNode,
  vtkMRMLScalarVolumeNode* drrVolumeNode)
{
  if (!paramNode)
  {
    vtkErrorMacro("LoadRtImage: Invalid parameter node");
    return false;
  }

  vtkMRMLRTBeamNode* beamNode = paramNode->GetBeamNode();

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("LoadRtImage: Invalid MRML scene");
    return false;
  }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->GetMRMLScene());
  if (!shNode)
  {
    vtkErrorMacro("LoadRtImage: Failed to access subject hierarchy node");
    return false;
  }
  
  // Create display node for the volume
  vtkNew<vtkMRMLScalarVolumeDisplayNode> volumeDisplayNode;
  scene->AddNode(volumeDisplayNode);
  volumeDisplayNode->SetDefaultColorMap();

  int AutoscaleRange[2];
  paramNode->GetAutoscaleRange(AutoscaleRange);
  if (!paramNode->GetAutoscaleFlag() || (paramNode->GetAutoscaleFlag() &&
    !AutoscaleRange[0] && !AutoscaleRange[1]))
  {
    volumeDisplayNode->AutoWindowLevelOn();
  }
  else
  {
    // Apply given window level if available
    float center = float(AutoscaleRange[1] - AutoscaleRange[0]) / 2.;
    float width = float(AutoscaleRange[1] - AutoscaleRange[0]);

    volumeDisplayNode->AutoWindowLevelOff();
    volumeDisplayNode->SetWindowLevel(width, center);
  }

  drrVolumeNode->SetAndObserveDisplayNodeID(volumeDisplayNode->GetID());

  // Set up subject hierarchy item
  vtkIdType drrVolumeShItemID = shNode->CreateItem( shNode->GetSceneItemID(), drrVolumeNode);

  // Set RT image specific attributes
  shNode->SetItemAttribute(drrVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_RTIMAGE_IDENTIFIER_ATTRIBUTE_NAME, "1");
  shNode->SetItemAttribute(drrVolumeShItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName(), "");

  shNode->SetItemAttribute(drrVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_SOURCE_AXIS_DISTANCE_ATTRIBUTE_NAME, std::to_string(beamNode->GetSAD()));

  shNode->SetItemAttribute(drrVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_GANTRY_ANGLE_ATTRIBUTE_NAME, std::to_string(beamNode->GetGantryAngle()));

  shNode->SetItemAttribute(drrVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_COUCH_ANGLE_ATTRIBUTE_NAME, std::to_string(beamNode->GetCouchAngle()));

  shNode->SetItemAttribute(drrVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_COLLIMATOR_ANGLE_ATTRIBUTE_NAME, std::to_string(beamNode->GetCollimatorAngle()));

  shNode->SetItemAttribute(drrVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_BEAM_NUMBER_ATTRIBUTE_NAME, std::to_string(beamNode->GetBeamNumber()));

  double sid = beamNode->GetSAD() + paramNode->GetIsocenterImagerDistance();
  shNode->SetItemAttribute(drrVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_RTIMAGE_SID_ATTRIBUTE_NAME, std::to_string(sid));

  double rtImagePosition[2] = {0.0, 0.0};
  paramNode->GetRTImagePosition(rtImagePosition);
  std::string rtImagePositionString = std::to_string(rtImagePosition[0]) + std::string(" ") + std::to_string(rtImagePosition[1]);
  shNode->SetItemAttribute(drrVolumeShItemID,  vtkSlicerRtCommon::DICOMRTIMPORT_RTIMAGE_POSITION_ATTRIBUTE_NAME, rtImagePositionString);

  // Compute and set RT image geometry. Uses the referenced beam if available, otherwise the geometry will be set up when loading the referenced beam
  this->SetupRtImageGeometry( paramNode, drrVolumeNode, drrVolumeShItemID);

  return true;
}

//------------------------------------------------------------------------------
bool vtkSlicerPlmDrrLogic::SetupRtImageGeometry( vtkMRMLPlmDrrNode* paramNode,
  vtkMRMLScalarVolumeNode* drrVolumeNode, vtkIdType vtkNotUsed(rtImageShItemID))
{
  if (!paramNode)
  {
    vtkErrorMacro("SetupRtImageGeometry: Invalid parameter node");
    return false;
  }

  vtkMRMLRTBeamNode* beamNode = paramNode->GetBeamNode();
  if (!beamNode)
  {
    vtkErrorMacro("SetupRtImageGeometry: Invalid beam node");
    return false;
  }

  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->GetMRMLScene());
  if (!shNode)
  {
    vtkErrorMacro("SetupRtImageGeometry: Failed to access subject hierarchy node");
    return false;
  }

  vtkIdType rtImageShItemID = shNode->GetItemByDataNode(drrVolumeNode);
  if (!rtImageShItemID)
  {
    vtkErrorMacro("SetupRtImageGeometry: Failed to get DRR RTImage subject hierarchy node");
    return false;
  }

  // If the function is called from the LoadRtPlan function with a beam: find corresponding RT image
  if (beamNode)
  {
    // Get RT plan for beam
    vtkMRMLRTPlanNode *planNode = beamNode->GetParentPlanNode();
    if (!planNode)
    {
      vtkErrorMacro("SetupRtImageGeometry: Failed to retrieve valid plan node for beam '" << beamNode->GetName() << "'");
      return false;
    }
    vtkIdType planShItemID = planNode->GetPlanSubjectHierarchyItemID();
    if (planShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
      vtkErrorMacro("SetupRtImageGeometry: Failed to retrieve valid plan subject hierarchy item for beam '" << beamNode->GetName() << "'");
      return false;
    }
    std::string rtPlanSopInstanceUid = shNode->GetItemUID(planShItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName());
    if (rtPlanSopInstanceUid.empty())
    {
      vtkWarningMacro("SetupRtImageGeometry: Failed to get RT Plan DICOM UID for beam '" << beamNode->GetName() << "'");
    }

    // Return if a referenced displayed model is present for the RT image, because it means that the geometry has been set up successfully before
    if (drrVolumeNode)
    {
      vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(
        drrVolumeNode->GetNodeReference(vtkMRMLPlanarImageNode::PLANARIMAGE_DISPLAYED_MODEL_REFERENCE_ROLE.c_str()) );
      if (modelNode)
      {
        vtkWarningMacro("SetupRtImageGeometry: RT image '" << drrVolumeNode->GetName() << "' belonging to beam '" << beamNode->GetName() << "' seems to have been set up already.");
        return false;
      }
    }
      
    if (!drrVolumeNode)
    {
      // RT image for the isocenter is not loaded yet. Geometry will be set up upon loading the related RT image
      vtkWarningMacro("SetupRtImageGeometry: Cannot set up geometry of RT image corresponding to beam '" << beamNode->GetName()
        << "' because the RT image is not loaded yet. Will be set up upon loading the related RT image");
      return false;
    }

    // Set more user friendly DRR image name
    std::string drrName = std::string("DRR: ") + std::string(beamNode->GetName());
    drrVolumeNode->SetName(drrName.c_str());
  }
  else
  {
    vtkErrorMacro("SetupRtImageGeometry: Input node is neither a volume node nor an plan POIs markups fiducial node");
    return false;
  }

  // We have both the RT image and the isocenter, we can set up the geometry

  // Get source to RT image plane distance (along beam axis)
  double rtImageSid = 0.0;
  std::string rtImageSidStr = shNode->GetItemAttribute(rtImageShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_RTIMAGE_SID_ATTRIBUTE_NAME);
  if (!rtImageSidStr.empty())
  {
    rtImageSid = vtkVariant(rtImageSidStr).ToDouble();
  }

  // Get RT image position (the x and y coordinates (in mm) of the upper left hand corner of the image, in the IEC X-RAY IMAGE RECEPTOR coordinate system)
  double rtImagePosition[2] = {0.0, 0.0};
  std::string rtImagePositionStr = shNode->GetItemAttribute(rtImageShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_RTIMAGE_POSITION_ATTRIBUTE_NAME);
  if (!rtImagePositionStr.empty())
  {
    std::stringstream ss;
    ss << rtImagePositionStr;
    ss >> rtImagePosition[0] >> rtImagePosition[1];
  }

  // Extract beam-related parameters needed to compute RT image coordinate system
  double sourceAxisDistance = beamNode->GetSAD();
  double gantryAngle = beamNode->GetGantryAngle();
  double couchAngle = beamNode->GetCouchAngle();

  // Get isocenter coordinates
  double isocenterWorldCoordinates[3] = {0.0, 0.0, 0.0};
  if (!beamNode->GetPlanIsocenterPosition(isocenterWorldCoordinates))
  {
    vtkErrorMacro("SetupRtImageGeometry: Failed to get plan isocenter position");
    return false;
  }

  // Assemble transform from isocenter IEC to RT image RAS
  vtkSmartPointer<vtkTransform> fixedToIsocenterTransform = vtkSmartPointer<vtkTransform>::New();
  fixedToIsocenterTransform->Identity();
  fixedToIsocenterTransform->Translate(isocenterWorldCoordinates);

  vtkSmartPointer<vtkTransform> couchToFixedTransform = vtkSmartPointer<vtkTransform>::New();
  couchToFixedTransform->Identity();
  couchToFixedTransform->RotateWXYZ(couchAngle, 0.0, 1.0, 0.0);

  vtkSmartPointer<vtkTransform> gantryToCouchTransform = vtkSmartPointer<vtkTransform>::New();
  gantryToCouchTransform->Identity();
  gantryToCouchTransform->RotateWXYZ(gantryAngle, 0.0, 0.0, 1.0);

  vtkSmartPointer<vtkTransform> sourceToGantryTransform = vtkSmartPointer<vtkTransform>::New();
  sourceToGantryTransform->Identity();
  sourceToGantryTransform->Translate(0.0, sourceAxisDistance, 0.0);

  vtkSmartPointer<vtkTransform> rtImageToSourceTransform = vtkSmartPointer<vtkTransform>::New();
  rtImageToSourceTransform->Identity();
  rtImageToSourceTransform->Translate(0.0, -1. * /* sourceAxisDistance */ rtImageSid, 0.0);

  vtkSmartPointer<vtkTransform> rtImageCenterToCornerTransform = vtkSmartPointer<vtkTransform>::New();
  rtImageCenterToCornerTransform->Identity();
  rtImageCenterToCornerTransform->Translate( -1. * rtImagePosition[0], 0.0, -1. * rtImagePosition[1]);

  // Create isocenter to RAS transform
  // The transformation below is based section C.8.8 in DICOM standard volume 3:
  // "Note: IEC document 62C/269/CDV 'Amendment to IEC 61217: Radiotherapy Equipment -
  //  Coordinates, movements and scales' also defines a patient-based coordinate system, and
  //  specifies the relationship between the DICOM Patient Coordinate System (see Section
  //  C.7.6.2.1.1) and the IEC PATIENT Coordinate System. Rotating the IEC PATIENT Coordinate
  //  System described in IEC 62C/269/CDV (1999) by 90 degrees counter-clockwise (in the negative
  //  direction) about the x-axis yields the DICOM Patient Coordinate System, i.e. (XDICOM, YDICOM,
  //  ZDICOM) = (XIEC, -ZIEC, YIEC). Refer to the latest IEC documentation for the current definition of the
  //  IEC PATIENT Coordinate System."
  // The IJK to RAS transform already contains the LPS to RAS conversion, so we only need to consider this rotation
  vtkSmartPointer<vtkTransform> iecToLpsTransform = vtkSmartPointer<vtkTransform>::New();
  iecToLpsTransform->Identity();
  iecToLpsTransform->RotateX(90.0);
  iecToLpsTransform->RotateZ(-90.0);

  // Get RT image IJK to RAS matrix (containing the spacing and the LPS-RAS conversion)
  vtkSmartPointer<vtkMatrix4x4> rtImageIjkToRtImageRasTransformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  drrVolumeNode->GetIJKToRASMatrix(rtImageIjkToRtImageRasTransformMatrix);
  vtkSmartPointer<vtkTransform> rtImageIjkToRtImageRasTransform = vtkSmartPointer<vtkTransform>::New();
  rtImageIjkToRtImageRasTransform->SetMatrix(rtImageIjkToRtImageRasTransformMatrix);

  // Concatenate the transform components
  vtkSmartPointer<vtkTransform> isocenterToRtImageRas = vtkSmartPointer<vtkTransform>::New();
  isocenterToRtImageRas->Identity();
  isocenterToRtImageRas->PreMultiply();
  isocenterToRtImageRas->Concatenate(fixedToIsocenterTransform);
  isocenterToRtImageRas->Concatenate(couchToFixedTransform);
  isocenterToRtImageRas->Concatenate(gantryToCouchTransform);
  isocenterToRtImageRas->Concatenate(sourceToGantryTransform);
  isocenterToRtImageRas->Concatenate(rtImageToSourceTransform);
  isocenterToRtImageRas->Concatenate(rtImageCenterToCornerTransform);
  isocenterToRtImageRas->Concatenate(iecToLpsTransform); // LPS = IJK
  isocenterToRtImageRas->Concatenate(rtImageIjkToRtImageRasTransformMatrix);

  // Transform RT image to proper position and orientation
  drrVolumeNode->SetIJKToRASMatrix(isocenterToRtImageRas->GetMatrix());

  // Set up outputs for the planar image display
  vtkSmartPointer<vtkMRMLModelNode> displayedModelNode = vtkSmartPointer<vtkMRMLModelNode>::New();
  this->GetMRMLScene()->AddNode(displayedModelNode);
  std::string displayedModelNodeName = vtkMRMLPlanarImageNode::PLANARIMAGE_MODEL_NODE_NAME_PREFIX + std::string(drrVolumeNode->GetName());
  displayedModelNode->SetName(displayedModelNodeName.c_str());
  displayedModelNode->SetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyExcludeFromTreeAttributeName().c_str(), "1");

  // Create PlanarImage parameter set node
  std::string planarImageParameterSetNodeName;
  planarImageParameterSetNodeName = this->GetMRMLScene()->GenerateUniqueName(
    vtkMRMLPlanarImageNode::PLANARIMAGE_PARAMETER_SET_BASE_NAME_PREFIX + std::string(drrVolumeNode->GetName()) );
  vtkSmartPointer<vtkMRMLPlanarImageNode> planarImageParameterSetNode = vtkSmartPointer<vtkMRMLPlanarImageNode>::New();
  planarImageParameterSetNode->SetName(planarImageParameterSetNodeName.c_str());
  this->GetMRMLScene()->AddNode(planarImageParameterSetNode);
  planarImageParameterSetNode->SetAndObserveRtImageVolumeNode(drrVolumeNode);
  planarImageParameterSetNode->SetAndObserveDisplayedModelNode(displayedModelNode);

  // Create planar image model for the RT image
  this->PlanarImageLogic->CreateModelForPlanarImage(planarImageParameterSetNode);

  // Hide the displayed planar image model by default
  displayedModelNode->SetDisplayVisibility(0);

  return true;
}

//------------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::SetPlanarImageLogic(vtkSlicerPlanarImageModuleLogic* planarImageLogic)
{
  this->PlanarImageLogic = planarImageLogic;
}

