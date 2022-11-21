/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLIhepMlcControlDisplayableManager.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include "vtkMRMLIhepMlcControlNode.h"
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

// STL includes
#include <algorithm>
#include <cstring>
#include <numeric>

namespace
{
const int RENDERER_LAYER = 1; // layer ID where the legent will be displayed
}

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLIhepMlcControlDisplayableManager);

//---------------------------------------------------------------------------
class vtkMRMLIhepMlcControlDisplayableManager::vtkInternal
{
public:

  vtkInternal(vtkMRMLIhepMlcControlDisplayableManager * external);
  virtual ~vtkInternal();

  vtkObserverManager* GetMRMLNodesObserverManager();
  void Modified();

  vtkMRMLSliceCompositeNode* FindSliceCompositeNode();
  bool IsVolumeVisibleInSliceView(vtkMRMLVolumeNode* volumeNode);

  // Update MLC
  void UpdateMlc();

  // Update actor and widget representation.
  // Returns true if the actor has changed.
  bool UpdateActor(vtkMRMLIhepMlcControlNode* ihepNode);

  // Show/hide the actor by adding to the renderer and enabling visibility; or removing from the renderer.
  // Returns true if visibility changed.
  bool ShowActor(vtkActor* actor, bool show);

  void UpdateSliceNode();
  void SetSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode);
  void UpdateActorsVisibilityFromSliceCompositeNode();

  vtkMRMLIhepMlcControlDisplayableManager* External;

  /// For volume nodes we need to observe the slice composite node so that we can show color legend
  /// only for nodes that are visible in the slice view.
  vtkWeakPointer<vtkMRMLSliceCompositeNode> SliceCompositeNode;

  vtkSmartPointer<vtkRenderer> ColorLegendRenderer;
};


//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::vtkInternal(vtkMRMLIhepMlcControlDisplayableManager* external)
: External(external)
{
  this->ColorLegendRenderer = vtkSmartPointer<vtkRenderer>::New();
  // Prevent erasing Z-buffer (important for quick picking and markup label visibility assessment)
  this->ColorLegendRenderer->EraseOff();
}

//---------------------------------------------------------------------------
vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::~vtkInternal()
{
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::GetMRMLNodesObserverManager()
{
  return this->External->GetMRMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::ShowActor(vtkActor* actor, bool show)
{
  if (!this->ColorLegendRenderer.GetPointer())
    {
    return false;
    }
  bool wasInRenderer = this->ColorLegendRenderer->HasViewProp(actor);
  bool wasVisible = wasInRenderer && actor->GetVisibility();
  if (show && !wasInRenderer)
    {
    this->ColorLegendRenderer->AddActor2D(actor);
    }
  else if (!show && wasInRenderer)
    {
    this->ColorLegendRenderer->RemoveActor(actor);
    }
  actor->SetVisibility(show);
  return (wasVisible != show);
}

//---------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::IsVolumeVisibleInSliceView(
  vtkMRMLVolumeNode* volumeNode)
{
  if (!volumeNode)
    {
    return false;
    }
  if (!this->SliceCompositeNode.GetPointer())
    {
    return false;
    }
  const char* volumeNodeID = volumeNode->GetID();
  if (!volumeNodeID)
    {
    return false;
    }
  if (this->SliceCompositeNode->GetBackgroundVolumeID())
    {
    if (strcmp(this->SliceCompositeNode->GetBackgroundVolumeID(), volumeNodeID) == 0)
      {
      return true;
      }
    }
  if (this->SliceCompositeNode->GetForegroundVolumeID())
    {
    if (strcmp(this->SliceCompositeNode->GetForegroundVolumeID(), volumeNodeID) == 0)
      {
      return true;
      }
    }
  if (this->SliceCompositeNode->GetLabelVolumeID())
    {
    if (strcmp(this->SliceCompositeNode->GetLabelVolumeID(), volumeNodeID) == 0)
      {
      return true;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
bool vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::UpdateActor(vtkMRMLIhepMlcControlNode* vtkNotUsed(ihepNode))
{
  // modified
  return true;
}

//---------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::FindSliceCompositeNode()
{
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
    vtkGenericWarningMacro("vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::FindSliceCompositeNode failed: invalid mrmlApplogic");
    return nullptr;
    }
  vtkMRMLSliceLogic* sliceLogic = mrmlAppLogic->GetSliceLogic(sliceNode);
  if (!sliceLogic)
    {
    return nullptr;
    }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  return sliceCompositeNode;
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::UpdateSliceNode()
{
  vtkMRMLSliceCompositeNode* sliceCompositeNode = this->FindSliceCompositeNode();
  this->SetSliceCompositeNode(sliceCompositeNode);
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::vtkInternal::SetSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode)
{
  if (this->SliceCompositeNode == compositeNode)
    {
    return;
    }
  vtkSetAndObserveMRMLNodeMacro(this->SliceCompositeNode, compositeNode);
  this->External->SetUpdateFromMRMLRequested(true);
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
// vtkMRMLIhepMlcControlDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLIhepMlcControlDisplayableManager::vtkMRMLIhepMlcControlDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLIhepMlcControlDisplayableManager::~vtkMRMLIhepMlcControlDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::Create()
{
  // Create a renderer in RENDERER_LAYER that will display the color legend
  // above the default layer (above images and markups).
  vtkRenderer* renderer = this->GetRenderer();
  if (!renderer)
    {
    vtkErrorMacro("vtkMRMLIhepMlcControlDisplayableManager::Create() failed: renderer is invalid");
    return;
    }
  this->Internal->ColorLegendRenderer->InteractiveOff();
  vtkRenderWindow* renderWindow = renderer->GetRenderWindow();
  if (!renderer)
    {
    vtkErrorMacro("vtkMRMLIhepMlcControlDisplayableManager::Create() failed: render window is invalid");
    return;
    }
  if (renderWindow->GetNumberOfLayers() < RENDERER_LAYER + 1)
    {
    renderWindow->SetNumberOfLayers(RENDERER_LAYER + 1);
    }
  this->Internal->ColorLegendRenderer->SetLayer(RENDERER_LAYER);
  renderWindow->AddRenderer(this->Internal->ColorLegendRenderer);

  // TODO: needed?
  // this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::AdditionalInitializeStep()
{
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* vtkNotUsed(caller))
{
  // slice node has been updated, nothing to do
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  this->Superclass::OnMRMLSceneNodeAdded(node);

  if (!node || !this->GetMRMLScene())
    {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
    }

  if (node->IsA("vtkMRMLIhepMlcControlNode"))
    {
    vtkWarningMacro("OnMRMLSceneNodeAdded: MLC control node added for visualization");
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);

    // Create actors and other objects, assest, resources

    this->ProcessMRMLNodesEvents(node, vtkCommand::ModifiedEvent, nullptr);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  this->Superclass::OnMRMLSceneNodeRemoved(node);

  if (!node || !this->GetMRMLScene())
    {
    vtkErrorMacro("OnMRMLSceneNodeRemoved: Invalid MRML scene or input node");
    return;
    }

  if (node->IsA("vtkMRMLIhepMlcControlNode"))
    {
    vtkUnObserveMRMLNodeMacro(node);
    // Cleanup assets and resources
    }
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::UpdateFromMRML()
{
  // this gets called from RequestRender, so make sure to jump out quickly if possible
  if (this->GetMRMLScene() == nullptr)
    {
    return;
    }

  // This is called when the view node is set. Update all actors.
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData)
{
  this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);

  if (event != vtkCommand::ModifiedEvent)
  {
    return;
  }
  vtkWarningMacro("ProcessMRMLNodesEvents: Process IhepMLC events in displayable manager");
/*
  vtkMRMLColorLegendDisplayNode* dispNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(caller);
  vtkMRMLSliceCompositeNode* sliceCompositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(caller);
  if (dispNode)
    {
    if (this->Internal->UpdateActor(dispNode))
      {
      this->RequestRender();
      }
    }
  else if (sliceCompositeNode)
    {
    this->SetUpdateFromMRMLRequested(true);
    this->RequestRender();
    }
*/
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::UpdateFromMRMLScene()
{
  this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLIhepMlcControlDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->SetSliceCompositeNode(nullptr);
}
