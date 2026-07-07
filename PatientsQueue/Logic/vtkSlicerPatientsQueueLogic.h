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

// .NAME vtkSlicerPatientsQueueLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerPatientsQueueLogic_h
#define __vtkSlicerPatientsQueueLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerPatientsQueueModuleLogicExport.h"


class VTK_SLICER_PATIENTSQUEUE_MODULE_LOGIC_EXPORT vtkSlicerPatientsQueueLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerPatientsQueueLogic *New();
  vtkTypeMacro(vtkSlicerPatientsQueueLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkSlicerPatientsQueueLogic();
  ~vtkSlicerPatientsQueueLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
private:

  vtkSlicerPatientsQueueLogic(const vtkSlicerPatientsQueueLogic&); // Not implemented
  void operator=(const vtkSlicerPatientsQueueLogic&); // Not implemented
};

#endif
