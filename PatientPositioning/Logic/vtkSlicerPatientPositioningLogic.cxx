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

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

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
::SetXrayImagesProjection(vtkMRMLPatientPositioningNode* parameterNode, vtkMRMLPatientPositioningNode::XrayProjectionType projection)
{
  if (!parameterNode && projection == vtkMRMLPatientPositioningNode::XrayProjectionType_Last)
  {
    return;
  }
}
