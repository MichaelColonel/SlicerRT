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

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLLinearTransformNode.h>

#include <vtkMRMLMarkupsClosedCurveNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsLineNode.h>

#include "vtkMRMLPlmDrrNode.h"

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <string>
#include <cassert>

// Plastimatch reconstruct module
#include <drr.h>
#include <drr_options.h>

const char* vtkSlicerPlmDrrLogic::IMAGER_BOUNDARY_MARKUPS_NODE_NAME = "ImagerBoundary"; // fiducial
const char* vtkSlicerPlmDrrLogic::IMAGE_WINDOW_MARKUPS_NODE_NAME = "ImageWindow"; // fiducial
const char* vtkSlicerPlmDrrLogic::ORIGIN_POINT_MARKUPS_NODE_NAME = "OriginPoint"; // fiducial
const char* vtkSlicerPlmDrrLogic::NORMAL_VECTOR_MARKUPS_NODE_NAME = "NormalVector"; // line
const char* vtkSlicerPlmDrrLogic::VUP_VECTOR_MARKUPS_NODE_NAME = "VupVector"; // line

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPlmDrrLogic);

//----------------------------------------------------------------------------
vtkSlicerPlmDrrLogic::vtkSlicerPlmDrrLogic()
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
  assert(this->GetMRMLScene() != 0);
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
bool vtkSlicerPlmDrrLogic::ComputeDRR(Drr_options* opts)
{
  if (!opts)
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerPlmDrrLogic::LoadDRR( vtkMRMLVolumeNode* volumeNode, const std::string& vtkNotUsed(filename))
{
  if (!volumeNode)
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::CreateMarkupsNodes(vtkMRMLPlmDrrNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("CreateDefaultMarkupsNodes: Invalid MRML scene");
    return;
  }

  // Create markups nodes if they don't exist

  // Imager boundary markups node
  vtkSmartPointer<vtkMRMLMarkupsFiducialNode> imagerMarkupsNode;
  if (!scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME))
  {
    imagerMarkupsNode = this->CreateImagerBoundary(parameterNode);
  }
  else
  {
    imagerMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME));
    vtkWarningMacro("CreateDefaultMarkupsNodes: Update imager points using parameter node data");
  }

  // Image window markups node
  vtkSmartPointer<vtkMRMLMarkupsFiducialNode> imageWindowMarkupsNode;
  if (!scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME))
  {
    imageWindowMarkupsNode = this->CreateImageWindow(parameterNode);
  }
  else
  {
    imageWindowMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME));
    vtkWarningMacro("CreateDefaultMarkupsNodes: Update image window points using parameter node data");
  }

  // vector markups node
  vtkSmartPointer<vtkMRMLMarkupsLineNode> vectorMarkupsNode;
  if (!scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME))
  {
    vectorMarkupsNode = this->CreateImageFirstRowColumn(parameterNode);
  }
  else
  {
    vectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME));
    vtkWarningMacro("CreateDefaultMarkupsNodes: Update VUP vector points using parameter node data");
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
  // Imager boundary markups node
  if (scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* imagerMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
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

    double* p;
    p = imagerMarkupsNode->GetNthControlPointPosition(0);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();

    p = imagerMarkupsNode->GetNthControlPointPosition(1);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();

    p = imagerMarkupsNode->GetNthControlPointPosition(2);
    p[0] = p2.GetX();
    p[1] = p2.GetY();
    p[2] = p2.GetZ();

    p = imagerMarkupsNode->GetNthControlPointPosition(3);
    p[0] = p3.GetX();
    p[1] = p3.GetY();
    p[2] = p3.GetZ();

    imagerMarkupsNode->Modified();

    // Update markups transform node if it's changed    
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
    vtkMRMLMarkupsFiducialNode* imageWindowMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
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

    double* p;
    p = imageWindowMarkupsNode->GetNthControlPointPosition(0);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();

    p = imageWindowMarkupsNode->GetNthControlPointPosition(1);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();

    p = imageWindowMarkupsNode->GetNthControlPointPosition(2);
    p[0] = p2.GetX();
    p[1] = p2.GetY();
    p[2] = p2.GetZ();

    p = imageWindowMarkupsNode->GetNthControlPointPosition(3);
    p[0] = p3.GetX();
    p[1] = p3.GetY();
    p[2] = p3.GetZ();

    imageWindowMarkupsNode->Modified();

    // Update markups transform node if it's changed    
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

  // VUP Vector markups line node
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
    vtkVector3d p0( offset[0], offset[1], -distance);
    vtkVector3d p1( -x + offset[0], y + offset[1], -distance);

    double* p;
    p = vectorMarkupsNode->GetNthControlPointPosition(0);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();

    p = vectorMarkupsNode->GetNthControlPointPosition(1);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();

    vectorMarkupsNode->Modified();

    // Update markups transform node if it's changed    
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
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerPlmDrrLogic::CreateImagerNormal(vtkMRMLPlmDrrNode* vtkNotUsed(node))
{
  return nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerPlmDrrLogic::CreateImagerBoundary(vtkMRMLPlmDrrNode* parameterNode)
{
  auto imagerMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsFiducialNode>::New();
  this->GetMRMLScene()->AddNode(imagerMarkupsNode);
  imagerMarkupsNode->SetName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME);
  imagerMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("DRR_") + IMAGER_BOUNDARY_MARKUPS_NODE_NAME;
  imagerMarkupsNode->SetSingletonTag(singletonTag.c_str());
  vtkWarningMacro("CreateImagerBoundary: Add points to the curve using parameter node data");

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

    imagerMarkupsNode->AddControlPoint( p0, std::string("Upper Left")); // "-x,y"
    imagerMarkupsNode->AddControlPoint( p1, std::string("Upper Right")); // "x,y"
    imagerMarkupsNode->AddControlPoint( p2, std::string("Lower Right")); // "x,-y"
    imagerMarkupsNode->AddControlPoint( p3, std::string("Lower Left")); // "-x,-y"

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkWarningMacro("CreateDetectorBoundary: beam node is valid");
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();

      if (beamTransformNode)
      {
        vtkWarningMacro("CreateDetectorBoundary: beam transform is observed");
        imagerMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return imagerMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerPlmDrrLogic::CreateImageWindow(vtkMRMLPlmDrrNode* parameterNode)
{
  auto imageWindowMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsFiducialNode>::New();
  this->GetMRMLScene()->AddNode(imageWindowMarkupsNode);
  imageWindowMarkupsNode->SetName(IMAGE_WINDOW_MARKUPS_NODE_NAME);
  imageWindowMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("DRR_") + IMAGE_WINDOW_MARKUPS_NODE_NAME;
  imageWindowMarkupsNode->SetSingletonTag(singletonTag.c_str());
  vtkWarningMacro("CreateImageWindow: Add points to the curve using parameter node data");

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

    imageWindowMarkupsNode->AddControlPoint( p0, std::string("r1,c1"));
    imageWindowMarkupsNode->AddControlPoint( p1, std::string("r2,c1"));
    imageWindowMarkupsNode->AddControlPoint( p2, std::string("r2,c2"));
    imageWindowMarkupsNode->AddControlPoint( p3, std::string("r1,c2"));

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkWarningMacro("CreateImageWindow: beam node is valid");
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();

      if (beamTransformNode)
      {
        vtkWarningMacro("CreateImageWindow: beam transform is observed");
        imageWindowMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return imageWindowMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerPlmDrrLogic::CreateImageFirstRowColumn(vtkMRMLPlmDrrNode* parameterNode)
{
  auto vectorMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsLineNode>::New();
  this->GetMRMLScene()->AddNode(vectorMarkupsNode);
  vectorMarkupsNode->SetName(VUP_VECTOR_MARKUPS_NODE_NAME);
  vectorMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("DRR_") + VUP_VECTOR_MARKUPS_NODE_NAME;
  vectorMarkupsNode->SetSingletonTag(singletonTag.c_str());
  vtkWarningMacro("CreateImageFirstRowColumn: Add points to vector using parameter node data");

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
    vtkVector3d p0( 0, 0, -distance);
    vtkVector3d p1( -x, y, -distance);

    vectorMarkupsNode->AddControlPoint( p0, "");
    vectorMarkupsNode->AddControlPoint( p1, std::string("(0,0)"));

    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkWarningMacro("CreateImageFirstRowColumn: beam node is valid");
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();

      if (beamTransformNode)
      {
        vtkWarningMacro("CreateImageFirstRowColumn: beam transform is observed");
        vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return vectorMarkupsNode;
}

//----------------------------------------------------------------------------
std::string vtkSlicerPlmDrrLogic::GeneratePlastimatchDrrArgs( vtkMRMLVolumeNode* vtkNotUsed(volumeNode), vtkMRMLPlmDrrNode* vtkNotUsed(parameterNode))
{
  return std::string();
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
      vtkErrorMacro("ProcessMRMLNodesEvents: RTBeam transformation has been changed");
    }
    else if (event == vtkMRMLRTBeamNode::BeamGeometryModified)
    {
      vtkErrorMacro("ProcessMRMLNodesEvents: RTBeam geometry has been changed");
    }
  }
  else if (caller->IsA("vtkMRMLRTPlanNode"))
  {
//    vtkMRMLRTPlanNode* planNode = vtkMRMLRTPlanNode::SafeDownCast(caller);
    if (event == vtkMRMLRTPlanNode::IsocenterModifiedEvent)
    {
      vtkErrorMacro("ProcessMRMLNodesEvents: RTPlan isocenter has been changed");
    }
  }
  else if (caller->IsA("vtkMRMLPlmDrrNode"))
  {
//    vtkMRMLRTPlanNode* planNode = vtkMRMLRTPlanNode::SafeDownCast(caller);
    if (event == vtkCommand::ModifiedEvent)
    {
      vtkErrorMacro("ProcessMRMLNodesEvents: Plastimatch DRR node modified");
    }
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::UpdateIsocenterImagerDistance(vtkMRMLPlmDrrNode* parameterNode)
{
  this->UpdateMarkupsNodes(parameterNode);
/*
  vtkMRMLScene* mrmlScene = this->GetMRMLScene();
  if (!mrmlScene)
  {
    vtkErrorMacro("UpdateIsocenterDetectorDistance: Invalid MRML scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateIsocenterDetectorDistance: Invalid parameter set node");
    return;
  }

  // Get detector markups node
  // Create detector markups singleton tag
  std::string detectorSingletonTag = std::string(parameterNode->GetSingletonTag()) + "_" + DETECTOR_BOUNDARY_MARKUPS_NODE_NAME;

  vtkMRMLMarkupsFiducialNode* detectorMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
    mrmlScene->GetSingletonNode(detectorSingletonTag.c_str(), "vtkMRMLMarkupsFiducialNode") );

  if (detectorMarkupsNode)
  {
    vtkWarningMacro("UpdateIsocenterDetectorDistance: Detector markups node is valid");
    double* p;
    p = detectorMarkupsNode->GetNthControlPointPosition(0);
    p[2] = -1. * parameterNode->GetIsocenterDetectorDistance();
    p = detectorMarkupsNode->GetNthControlPointPosition(1);
    p[2] = -1. * parameterNode->GetIsocenterDetectorDistance();
    p = detectorMarkupsNode->GetNthControlPointPosition(2);
    p[2] = -1. * parameterNode->GetIsocenterDetectorDistance();
    p = detectorMarkupsNode->GetNthControlPointPosition(3);
    p[2] = -1. * parameterNode->GetIsocenterDetectorDistance();

    detectorMarkupsNode->Modified();
  }
  else
  {
    vtkErrorMacro("UpdateIsocenterToDetectorDistance: Failed to get detector markups node");
  }
*/
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::UpdateImageSpacing(vtkMRMLPlmDrrNode* parameterNode)
{
  this->UpdateMarkupsNodes(parameterNode);
/*
  vtkMRMLScene* mrmlScene = this->GetMRMLScene();
  if (!mrmlScene)
  {
    vtkErrorMacro("UpdateImageSpacing: Invalid MRML scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateImageSpacing: Invalid parameter set node");
    return;
  }

  // Get detector markups node
  // Create detector markups singleton tag
  std::string detectorSingletonTag = std::string(parameterNode->GetSingletonTag()) + "_" + DETECTOR_BOUNDARY_MARKUPS_NODE_NAME;

  vtkMRMLMarkupsFiducialNode* detectorMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
    mrmlScene->GetSingletonNode(detectorSingletonTag.c_str(), "vtkMRMLMarkupsFiducialNode") );

//  vtkMRMLMarkupsFiducialNode* detectorMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
//    mrmlScene->GetSingletonNode(detectorSingletonTag.c_str(), "vtkMRMLMarkupsFiducialNode") );
  if (detectorMarkupsNode)
  {
    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double x = spacing[0] * dimention[0] / 2.;
    double y = spacing[1] * dimention[1] / 2.;

    vtkWarningMacro("UpdateImageSpacing: Detector markups node is valid");
    double* p;
    p = detectorMarkupsNode->GetNthControlPointPosition(0);
    p[0] = -1. * x;
    p[1] = y;
    p = detectorMarkupsNode->GetNthControlPointPosition(1);
    p[0] = x;
    p[1] = y;
    p = detectorMarkupsNode->GetNthControlPointPosition(2);
    p[0] = x;
    p[1] = -1. * y;
    p = detectorMarkupsNode->GetNthControlPointPosition(3);
    p[0] = -1. * x;
    p[1] = -1. * y;
    
    detectorMarkupsNode->Modified();
  }
  else
  {
    vtkErrorMacro("UpdateImageSpacing: Failed to get detector markups node");
  }
*/
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::UpdateImageDimention(vtkMRMLPlmDrrNode* parameterNode)
{
  this->UpdateMarkupsNodes(parameterNode);
/*
  vtkMRMLScene* mrmlScene = this->GetMRMLScene();
  if (!mrmlScene)
  {
    vtkErrorMacro("UpdateImageDimention: Invalid MRML scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateImageDimention: Invalid parameter set node");
    return;
  }

  // Get detector boundary markups node
  // Create detector markups singleton tag
  std::string detectorSingletonTag = std::string(parameterNode->GetSingletonTag()) + "_" + DETECTOR_BOUNDARY_MARKUPS_NODE_NAME;

  vtkMRMLMarkupsFiducialNode* detectorMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
    mrmlScene->GetSingletonNode(detectorSingletonTag.c_str(), "vtkMRMLMarkupsFiducialNode") );

  if (detectorMarkupsNode)
  {
    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double x = spacing[0] * dimention[0] / 2.;
    double y = spacing[1] * dimention[1] / 2.;

    vtkWarningMacro("UpdateImageDimention: Detector markups node is valid");
    double* p;
    p = detectorMarkupsNode->GetNthControlPointPosition(0);
    p[0] = -1. * x;
    p[1] = y;
    p = detectorMarkupsNode->GetNthControlPointPosition(1);
    p[0] = x;
    p[1] = y;
    p = detectorMarkupsNode->GetNthControlPointPosition(2);
    p[0] = x;
    p[1] = -1. * y;
    p = detectorMarkupsNode->GetNthControlPointPosition(3);
    p[0] = -1. * x;
    p[1] = -1. * y;
    
    detectorMarkupsNode->Modified();
  }
  else
  {
    vtkErrorMacro("UpdateImageDimention: Failed to get detector markups node");
  }
*/
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::UpdateImagerCenterOffset(vtkMRMLPlmDrrNode* parameterNode)
{
  this->UpdateMarkupsNodes(parameterNode);
}
