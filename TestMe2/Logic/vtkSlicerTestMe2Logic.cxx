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

// TestMe2 Logic includes
#include "vtkSlicerTestMe2Logic.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>



//#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTestMe2Logic);

//----------------------------------------------------------------------------
vtkSlicerTestMe2Logic::vtkSlicerTestMe2Logic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTestMe2Logic::~vtkSlicerTestMe2Logic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerTestMe2Logic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerTestMe2Logic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerTestMe2Logic::RegisterNodes()
{
  if (this->GetMRMLScene() == nullptr)
  {
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTestMe2Logic::UpdateFromMRMLScene()
{
  if (this->GetMRMLScene() == nullptr)
  {
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTestMe2Logic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerTestMe2Logic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

void vtkSlicerTestMe2Logic::createControlPoint(vtkMRMLMarkupsFiducialNode* inputFiducial)
{
    vtkVector3d point(0.0, 0.0, 0.0);
    inputFiducial->AddControlPoint(point, "Point_F");
}

void vtkSlicerTestMe2Logic::updateHeight(vtkMRMLLinearTransformNode* inputTransform, double height)
{
      vtkNew<vtkTransform> transform;
    transform->Identity();
//    transform->RotateX(90);
    transform->Translate(0,0,height);
    inputTransform->SetAndObserveTransformToParent(transform);
//  inputFiducial->SetAndObserveTransformNodeID(inputTransform->GetID());
//  vtkNew<vtkMatrix4x4> matrixTransform;
//  matrixTransform->SetElement(2,3,height);
//  inputTransform->SetMatrixTransformToParent(matrixTransform);
}
