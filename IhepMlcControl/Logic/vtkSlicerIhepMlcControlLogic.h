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

// .NAME vtkSlicerIhepMlcControlLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerIhepMlcControlLogic_h
#define __vtkSlicerIhepMlcControlLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerIhepMlcControlModuleLogicExport.h"

class vtkMRMLRTBeamNode;
class vtkMRMLIhepMlcControlNode;
class vtkMRMLTableNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_IHEPMLCCONTROL_MODULE_LOGIC_EXPORT vtkSlicerIhepMlcControlLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerIhepMlcControlLogic *New();
  vtkTypeMacro(vtkSlicerIhepMlcControlLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a table node with the Multi Leaf Collimator boundary data.
  /// Based on DICOMRT BeamLimitingDeviceEntry description of MLC.
  /// DICOM standard describes two kinds of multi-leaf collimators: 
  /// "MLCX" - leaves moves along X-axis, "MCLY" - leaves moves along Y-axis.
  /// X-axis and Y-axis are axises of IEC BEAM LIMITING DEVICE coordinate system.
  /// \param parameterNode for MLC parameters
  /// \return valid pointer if successfull, nullptr otherwise
  vtkMRMLTableNode* CreateMlcTableNodeBoundaryData(vtkMRMLIhepMlcControlNode* parameterNode);
  /// Update a already created table node with the Multi Leaf Collimator boundary data.
  /// \param parameterNode for MLC parameters
  /// \return true if success false otherwise
  bool UpdateMlcTableNodeBoundaryData(vtkMRMLIhepMlcControlNode* parameterNode);
  bool SetBeamParentForMlcTableNode(vtkMRMLRTBeamNode* beamNode, vtkMRMLTableNode* tableNode);
  bool SetupPositionsFromMlcTableNode(vtkMRMLIhepMlcControlNode* parameterNode, vtkMRMLTableNode* tableNode);

protected:
  vtkSlicerIhepMlcControlLogic();
  ~vtkSlicerIhepMlcControlLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

private:

  vtkSlicerIhepMlcControlLogic(const vtkSlicerIhepMlcControlLogic&); // Not implemented
  void operator=(const vtkSlicerIhepMlcControlLogic&); // Not implemented
};

#endif
