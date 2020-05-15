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
#include <cassert>

// Plastimatch reconstruct module
#include <drr.h>
#include <drr_options.h>

const char* vtkSlicerPlmDrrLogic::DETECTOR_BOUNDARY_MARKUPS_NODE_NAME = "DetectorBoundary"; // closed curve
const char* vtkSlicerPlmDrrLogic::IMAGE_BOUNDARY_MARKUPS_NODE_NAME = "ImageBoundary"; // closed curve
const char* vtkSlicerPlmDrrLogic::ORIGIN_POINT_MARKUPS_NODE_NAME = "OriginPoint"; // fiducial
const char* vtkSlicerPlmDrrLogic::NORMAL_VECTOR_MARKUPS_NODE_NAME = "NormalVector"; // line
const char* vtkSlicerPlmDrrLogic::VUP_VECTOR_MARKUPS_NODE_NAME = "VUPVector"; // line

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
void vtkSlicerPlmDrrLogic::CreateDefaultMarkupsNodes(vtkMRMLPlmDrrNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("CreateDefaultMarkupsNodes: Invalid MRML scene");
    return;
  }

  // Create markups nodes if they don't exist

  // Detector boundary markups node
  vtkSmartPointer<vtkMRMLMarkupsFiducialNode> detectorMarkupsNode;
  if (!scene->GetFirstNodeByName(DETECTOR_BOUNDARY_MARKUPS_NODE_NAME))
  {
    detectorMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsFiducialNode>::New();
    scene->AddNode(detectorMarkupsNode);
    detectorMarkupsNode->SetName(DETECTOR_BOUNDARY_MARKUPS_NODE_NAME);
    detectorMarkupsNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("DRR_") + DETECTOR_BOUNDARY_MARKUPS_NODE_NAME;
    detectorMarkupsNode->SetSingletonTag(singletonTag.c_str());
    vtkWarningMacro("CreateDefaultMarkupsNodes: Add points to the curve using parameter node data");
    
    if (parameterNode)
    {
      double distance = parameterNode->GetIsocenterDetectorDistance();
      
      double spacing[2] = {};
      parameterNode->GetImageSpacing(spacing);

      int dimention[2] = {};
      parameterNode->GetImageDimention(dimention);

      double offset[2] = {};
      parameterNode->GetDetectorCenterOffset(offset);

      double x = spacing[0] * dimention[0] / 2.;
      double y = spacing[1] * dimention[1] / 2.;
      // add points
      vtkVector3d p1( -x + offset[0], y + offset[1], -distance);
      vtkVector3d p2( x + offset[0], y + offset[1], -distance);
      vtkVector3d p3( x + offset[0], -y + offset[1], -distance);
      vtkVector3d p4( -x + offset[0], -y + offset[1], -distance);

      detectorMarkupsNode->AddControlPoint( p1, std::string("Upper Left")); // "-x,y"
      detectorMarkupsNode->AddControlPoint( p2, std::string("Upper Right")); // "x,y"
      detectorMarkupsNode->AddControlPoint( p3, std::string("Lower Right")); // "x,-y"
      detectorMarkupsNode->AddControlPoint( p4, std::string("Lower Left")); // "-x,-y"

      if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
      {
        vtkWarningMacro("CreateDefaultMarkupsNodes: beam node is valid");
        vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();

        if (beamTransformNode)
        {
          vtkWarningMacro("CreateDefaultMarkupsNodes: beam transform is observed");
          detectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
        }
      }
    }

  }
  else
  {
    detectorMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(DETECTOR_BOUNDARY_MARKUPS_NODE_NAME));
    vtkWarningMacro("CreateDefaultMarkupsNodes: Update curve points using parameter node data");
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
  // Detector boundary markups node
  if (scene->GetFirstNodeByName(DETECTOR_BOUNDARY_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* detectorMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(DETECTOR_BOUNDARY_MARKUPS_NODE_NAME));

    double distance = parameterNode->GetIsocenterDetectorDistance();
    
    double spacing[2] = {};
    parameterNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    parameterNode->GetImageDimention(dimention);

    double offset[2] = {};
    parameterNode->GetDetectorCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.;
    double y = spacing[1] * dimention[1] / 2.;
    // add points
    vtkVector3d p0( -x + offset[0], y + offset[1], -distance);
    vtkVector3d p1( x + offset[0], y + offset[1], -distance);
    vtkVector3d p2( x + offset[0], -y + offset[1], -distance);
    vtkVector3d p3( -x + offset[0], -y + offset[1], -distance);

    double* p;
    p = detectorMarkupsNode->GetNthControlPointPosition(0);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();

    p = detectorMarkupsNode->GetNthControlPointPosition(1);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();

    p = detectorMarkupsNode->GetNthControlPointPosition(2);
    p[0] = p2.GetX();
    p[1] = p2.GetY();
    p[2] = p2.GetZ();

    p = detectorMarkupsNode->GetNthControlPointPosition(3);
    p[0] = p3.GetX();
    p[1] = p3.GetY();
    p[2] = p3.GetZ();

    detectorMarkupsNode->Modified();
    if (vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();

      if (beamTransformNode)
      {
        detectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerPlmDrrLogic::CreateDetectorNormal(vtkMRMLPlmDrrNode* vtkNotUsed(node))
{
  return nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsClosedCurveNode* vtkSlicerPlmDrrLogic::CreateDetectorBoundary(vtkMRMLPlmDrrNode* vtkNotUsed(node))
{
  return nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsClosedCurveNode* vtkSlicerPlmDrrLogic::CreateImageBoundary(vtkMRMLPlmDrrNode* vtkNotUsed(node))
{
  return nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerPlmDrrLogic::CreateImageFirstRowColumn(vtkMRMLPlmDrrNode* vtkNotUsed(node))
{
  return nullptr;
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
void vtkSlicerPlmDrrLogic::UpdateIsocenterDetectorDistance(vtkMRMLPlmDrrNode* parameterNode)
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
void vtkSlicerPlmDrrLogic::UpdateDetectorCenterOffset(vtkMRMLPlmDrrNode* parameterNode)
{
  this->UpdateMarkupsNodes(parameterNode);
}
