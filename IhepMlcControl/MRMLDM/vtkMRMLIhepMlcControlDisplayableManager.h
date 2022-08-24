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

#ifndef __vtkMRMLIhepMlcControlDisplayableManager_h
#define __vtkMRMLIhepMlcControlDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"
#include "vtkSlicerIhepMlcControlModuleMRMLDisplayableManagerExport.h"

class vtkMRMLIhepMlcControlNode;
class vtkMRMLScene;

/// \brief Displayable manager for MLC visualization
///
/// This displayable manager implements MLC position and boundary in both 2D and 3D views.
class VTK_SLICER_IHEPMLCCONTROL_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLIhepMlcControlDisplayableManager : public vtkMRMLAbstractDisplayableManager
{
public:
  static vtkMRMLIhepMlcControlDisplayableManager* New();
  vtkTypeMacro(vtkMRMLIhepMlcControlDisplayableManager, vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkMRMLIhepMlcControlDisplayableManager();
  ~vtkMRMLIhepMlcControlDisplayableManager() override;

  /// Initialize the displayable manager based on its associated
  /// vtkMRMLSliceNode
  void Create() override;

  /// Called from RequestRender method if UpdateFromMRMLRequested is true
  /// \sa RequestRender() SetUpdateFromMRMLRequested()
  void UpdateFromMRML() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  void UpdateFromMRMLScene() override;
  void UnobserveMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void ProcessMRMLNodesEvents(vtkObject *caller,
                                      unsigned long event,
                                      void *callData) override;

  /// Called when the SliceNode or Three3DViewNode are modified. May cause MLC lines to remap its position on screen.
  void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Method to perform additional initialization
  void AdditionalInitializeStep() override;

private:
  vtkMRMLIhepMlcControlDisplayableManager(const vtkMRMLIhepMlcControlDisplayableManager&) = delete;
  void operator=(const vtkMRMLIhepMlcControlDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
