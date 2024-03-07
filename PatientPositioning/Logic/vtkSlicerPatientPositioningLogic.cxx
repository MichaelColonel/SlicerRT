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

// PatientPositioning Logic includes
#include "vtkSlicerPatientPositioningLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

const char* vtkSlicerPatientPositioningLogic::DRR_TRANSFORM_NODE_NAME = "DrrPatientPositioningTransform";
const char* vtkSlicerPatientPositioningLogic::DRR_TRANSLATE_NODE_NAME = "DrrPatientPositioningTranslate";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPatientPositioningLogic);

//----------------------------------------------------------------------------
vtkSlicerPatientPositioningLogic::vtkSlicerPatientPositioningLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerPatientPositioningLogic::~vtkSlicerPatientPositioningLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }

  if (!scene->IsNodeClassRegistered("vtkMRMLPatientPositioningNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLPatientPositioningNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::UpdateFromMRMLScene()
{
  if (this->GetMRMLScene() == nullptr)
  {
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic
::SetXrayImagesProjection(vtkMRMLPatientPositioningNode* parameterNode, vtkMRMLPatientPositioningNode::XrayProjectionType projection,
  vtkMRMLSliceCompositeNode* sliceCompNode)
{
  if (!parameterNode || !sliceCompNode || projection == vtkMRMLPatientPositioningNode::XrayProjectionType_Last)
  {
    return;
  }
  vtkMRMLScalarVolumeNode* drrNode = parameterNode->GetDrrNode(projection);
  vtkMRMLScalarVolumeNode* xrayImageNode = parameterNode->GetXrayImageNode(projection);
  if (drrNode && xrayImageNode)
  {
    sliceCompNode->SetForegroundVolumeID(drrNode->GetID());
    sliceCompNode->SetBackgroundVolumeID(xrayImageNode->GetID());
    sliceCompNode->SetForegroundOpacity(0.5);
  }

  vtkMRMLApplicationLogic* mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
    {
    vtkGenericWarningMacro("vtkSlicerPatientPositioningLogic::SetXrayImagesProjection failed: invalid mrmlApplogic");
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = mrmlAppLogic->GetSliceLogic(sliceNode);
  if (!sliceLogic)
    {
    return;
    }

/*
  vtkMRMLNode* viewNode = nullptr;
  switch (projection)
  {
  case vtkMRMLPatientPositioningNode::Horizontal:
    
    break;
  case vtkMRMLPatientPositioningNode::Vertical:
    break;
  case vtkMRMLPatientPositioningNode::Angle:
    break;
  default:
    break;
  }

  vtkMRMLNode* viewNode = this->External->GetMRMLDisplayableNode();
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(viewNode);
  if (!sliceNode)
    {
    // this displayable manager is not of a slice node
    return nullptr;
    }
  vtkMRMLApplicationLogic* mrmlAppLogic = this->External->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
    {
    vtkGenericWarningMacro("vtkMRMLColorLegendDisplayableManager::vtkInternal::FindSliceCompositeNode failed: invalid mrmlApplogic");
    return nullptr;
    }
  vtkMRMLSliceLogic* sliceLogic = mrmlAppLogic->GetSliceLogic(sliceNode);
  if (!sliceLogic)
    {
    return nullptr;
    }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  return sliceCompositeNode;
*/
}

//---------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerPatientPositioningLogic
::GetXrayImageRasToIjkMatrixTransformNode(vtkMRMLScalarVolumeNode* xrayImageNode, vtkMRMLRTBeamNode* xrayBeamNode)
{
  if (!xrayBeamNode)
  {
    vtkErrorMacro("GetXrayImageRasToIjkMatrixTransformNode: Invalid beam node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetXrayImageRasToIjkMatrixTransformNode: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (!scene->GetFirstNodeByName(DRR_TRANSFORM_NODE_NAME))
  {
    transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    transformNode->SetName(DRR_TRANSFORM_NODE_NAME);
    transformNode->SetHideFromEditors(1);
    transformNode->SetSingletonTag("DRR_Transform");
    scene->AddNode(transformNode);
  }
  else
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName(DRR_TRANSFORM_NODE_NAME));
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> translateNode;
  if (!scene->GetFirstNodeByName(DRR_TRANSLATE_NODE_NAME))
  {
    translateNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    translateNode->SetName(DRR_TRANSLATE_NODE_NAME);
    translateNode->SetHideFromEditors(1);
    translateNode->SetSingletonTag("DRR_Translate");
    scene->AddNode(translateNode);
  }
  else
  {
    translateNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName(DRR_TRANSLATE_NODE_NAME));
  }

  translateNode->SetAndObserveTransformNodeID(transformNode->GetID());
  xrayImageNode->SetAndObserveTransformNodeID(translateNode->GetID());

  return translateNode;
}
