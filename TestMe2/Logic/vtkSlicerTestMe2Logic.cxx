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
//#include <vtkIntArray.h>
#include <vtkNew.h>
//#include <vtkObjectFactory.h>

#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLTestMe2Node.h>



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
  this->Superclass::SetMRMLSceneInternal(newScene);

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerTestMe2Logic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }

  if (!scene->IsNodeClassRegistered("vtkMRMLTestMe2Node"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLTestMe2Node>::New());
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
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
    if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
  }

  if (node->IsA("vtkMRMLTestMe2Node"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTestMe2Logic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerTestMe2Logic::ProcessMRMLNodesEvents(vtkObject *caller, unsigned long eventID, void *callData)
{
  if (caller->IsA("vtkMRMLTestMe2Node"))
  {
    if (eventID == vtkCommand::ModifiedEvent)
    {
      vtkMRMLTestMe2Node* parameterNode = vtkMRMLTestMe2Node::SafeDownCast(caller);
      if (parameterNode)
      {
        vtkMRMLMarkupsFiducialNode* fiducialNode = parameterNode->GetFiducialNode();
        vtkMRMLLinearTransformNode* transformNode = parameterNode->GetTransformNode();
        double height = parameterNode->GetHeight();
        double rotateXAngle = parameterNode->GetRotateXAngle();
 //       this->updateTransform(transformNode, parameterNode->GetHeight(), parameterNode->GetRotateXAngle());
        if (!transformNode)
          {
            vtkErrorMacro("updateTransform: Transform node is invalid");
            return;
          }
          vtkNew<vtkTransform> translate, rotate;
          translate->Identity();
          rotate->Identity();
          translate->Translate(0,0,height);
          rotate->RotateX(rotateXAngle);
          rotate->Concatenate(translate);
          transformNode->SetAndObserveTransformToParent(rotate);
              }
    }
  }
}

void vtkSlicerTestMe2Logic::createControlPoint(vtkMRMLMarkupsFiducialNode* inputFiducial)
{
    vtkVector3d point(0.0, 0.0, 0.0);
    inputFiducial->AddControlPoint(point, "Point_F");
}

void vtkSlicerTestMe2Logic::updateFiducialTransformLink(vtkMRMLMarkupsFiducialNode* inputFiducial, vtkMRMLLinearTransformNode* inputTransform)
{
  if (inputFiducial && inputTransform)
  {
    inputFiducial->SetAndObserveTransformNodeID(inputTransform->GetID());
  }

}

void vtkSlicerTestMe2Logic::updateTransform(vtkMRMLLinearTransformNode* inputTransform, double height, double rotateXAngle)
{
  if (!inputTransform)
  {
    vtkErrorMacro("updateTransform: Transform node is invalid");
    return;
  }
  vtkNew<vtkTransform> translate, rotate;
  translate->Identity();
  rotate->Identity();
  translate->Translate(0,0,height);
  rotate->RotateX(rotateXAngle);
  rotate->Concatenate(translate);
  inputTransform->SetAndObserveTransformToParent(rotate);
//  inputFiducial->SetAndObserveTransformNodeID(inputTransform->GetID());
//  vtkNew<vtkMatrix4x4> matrixTransform;
//  matrixTransform->SetElement(2,3,height);
//  inputTransform->SetMatrixTransformToParent(matrixTransform);
}
/*
void vtkSlicerTestMe2Logic::showDRR(vtkMRMLScalarVolumeNode* inputDRR, vtkMRMLRTBeamNode* inputBeam);
{
  if (inputDRR && inputBeam)
  {

  }

  else
  {
    vtkErrorMacro("showDRR: Nodes are invalid");
    return;
  }
}
*/
