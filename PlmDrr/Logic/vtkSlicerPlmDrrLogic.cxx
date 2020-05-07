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
  assert(this->GetMRMLScene() != 0);
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
void vtkSlicerPlmDrrLogic::CreateDefaultMarkupsNodes(vtkMRMLRTBeamNode* vtkNotUsed(beamNode))
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("CreateDefaultMarkupsNodes: Invalid MRML scene");
    return;
  }

  // Create markups nodes if they don't exist

  // Detector boundary closed curve node
  vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode> detectorMarkupsClosedCurveNode;
  if (!scene->GetFirstNodeByName(DETECTOR_BOUNDARY_MARKUPS_NODE_NAME))
  {
    detectorMarkupsClosedCurveNode = vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode>::New();
    detectorMarkupsClosedCurveNode->SetName(DETECTOR_BOUNDARY_MARKUPS_NODE_NAME);
    detectorMarkupsClosedCurveNode->SetHideFromEditors(1);
    std::string singletonTag = std::string("DRR_") + DETECTOR_BOUNDARY_MARKUPS_NODE_NAME;
    detectorMarkupsClosedCurveNode->SetSingletonTag(singletonTag.c_str());
    scene->AddNode(detectorMarkupsClosedCurveNode);
  }
  else
  {
    detectorMarkupsClosedCurveNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(DETECTOR_BOUNDARY_MARKUPS_NODE_NAME));
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
    vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(caller);
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
    vtkMRMLRTPlanNode* planNode = vtkMRMLRTPlanNode::SafeDownCast(caller);

    if (event == vtkMRMLRTPlanNode::IsocenterModifiedEvent)
    {
      vtkErrorMacro("ProcessMRMLNodesEvents: RTPlan isocenter has been changed");
    }
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPlmDrrLogic::UpdateIsocenterToDetectorDistance(vtkMRMLRoomsEyeViewNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateIsocenterToDetectorDistance: Invalid parameter set node");
    return;
  }

  // Get detector markups node
  // Create detector markups singleton tag
  std::string detectorSingletonTag = std::string(parameterNode->GetSingletonTag()) + "_" + DETECTOR_BOUNDARY_MARKUPS_NODE_NAME;

  vtkMRMLMarkupsClosedCurveNode* detectorMarkupsNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(detectorSingletonTag.c_str(), "vtkMRMLMarkupsClosedCurveNode") );
  if (detectorMarkupsNode)
  {
    vtkWarningMacro("UpdateIsocenterToDetectorDistance: Detector markups node is valid");
  }
  else
  {
    vtkErrorMacro("UpdateIsocenterToDetectorDistance: Failed to get detector markups node");
  }

/*
  vtkMRMLLinearTransformNode* collimatorToGantryTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Collimator, vtkSlicerIECTransformLogic::Gantry);

  vtkNew<vtkTransform> collimatorToGantryTransform;
  collimatorToGantryTransform->RotateZ(parameterNode->GetCollimatorRotationAngle());
  collimatorToGantryTransformNode->SetAndObserveTransformToParent(collimatorToGantryTransform);
*/
}
