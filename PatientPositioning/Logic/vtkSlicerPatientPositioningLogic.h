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

// .NAME vtkSlicerPatientPositioningLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerPatientPositioningLogic_h
#define __vtkSlicerPatientPositioningLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include <vtkMRMLPatientPositioningNode.h>

// STD includes
#include <cstdlib>

#include "vtkSlicerPatientPositioningModuleLogicExport.h"

class vtkMRMLPatientPositioningNode;

class VTK_SLICER_PATIENTPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerPatientPositioningLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerPatientPositioningLogic *New();
  vtkTypeMacro(vtkSlicerPatientPositioningLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetXrayImagesProjection(vtkMRMLPatientPositioningNode* parameterNode, vtkMRMLPatientPositioningNode::XrayProjectionType projection);

protected:
  vtkSlicerPatientPositioningLogic();
  ~vtkSlicerPatientPositioningLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
private:

  vtkSlicerPatientPositioningLogic(const vtkSlicerPatientPositioningLogic&); // Not implemented
  void operator=(const vtkSlicerPatientPositioningLogic&); // Not implemented
};

#endif
