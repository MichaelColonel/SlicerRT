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
#include <vtkMRMLChannel25GeometryNode.h>

// STD includes
#include <cstdlib>

#include "vtkSlicerPatientPositioningModuleLogicExport.h"
#include "vtkSlicerTableTopRobotTransformLogic.h"

class vtkMRMLLinearTransformNode;
class vtkMRMLSliceCompositeNode;
class vtkMRMLSliceNode;
class vtkMRMLPatientPositioningNode;
class vtkMRMLRTBeamNode;

class vtkMatrix4x4;
class vtkPolyData;
class vtkVector3d;

class VTK_SLICER_PATIENTPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerPatientPositioningLogic :
  public vtkSlicerModuleLogic
{
public:
  static const char* TREATMENT_MACHINE_DESCRIPTOR_FILE_PATH_ATTRIBUTE_NAME;
  static unsigned long MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS;
//  static const char* FIXEDREFERENCE_MODEL_NAME; // Fixed Reference
//  static const char* ROBOT_BASE_FIXED_MODEL_NAME; // Fixed robot model
//  static const char* ROBOT_BASE_ROTATION_MODEL_NAME; // Rotated robot model
//  static const char* ROBOT_SHOULDER_MODEL_NAME; // Table Support Left-Right Movement (Longitudinal)
//  static const char* ROBOT_ELBOW_MODEL_NAME; // Table Origin Support
//  static const char* ROBOT_WRIST_MODEL_NAME; // Table Mirror Support
//  static const char* TABLETOP_MODEL_NAME; // Table Top

  
  static const char* ROBOT_BASE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME; // Fiducial shows Base origin position
  static const char* ROBOT_SHOULDER_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME; // Fiducial shows Shoulder origin position
  static const char* ROBOT_ELBOW_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME; // Fiducial shows Elbow origin position
  static const char* ROBOT_WRIST_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME; // Fiducial shows Wrist origin position
  static const char* ROBOT_TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME; // Fiducial shows TableTop center position
  static const char* FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME; //  Beam axis line in fixed reference frame

  static const char* DRR_TRANSFORM_NODE_NAME;
  static const char* DRR_TRANSLATE_NODE_NAME;

  static vtkSlicerPatientPositioningLogic *New();
  vtkTypeMacro(vtkSlicerPatientPositioningLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Load and setup components of the treatment machine into the scene based on its description.
  /// \param parameterNode Parameter node contains the treatment machine descriptor file path.
  /// \return List of parts that were successfully set up.
  std::vector<vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier> LoadTreatmentMachine(vtkMRMLChannel25GeometryNode* parameterNode);
  /// Set up the IEC transforms and model properties on the treatment machine models.
  /// \param forceEnableCollisionDetection Enable collision detection between parts even if calculation is potentially
  ///        lengthy absed on the number of triangles of the parts.
  /// \return List of parts that were successfully set up.
  std::vector<vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier> SetupTreatmentMachineModels(
    vtkMRMLChannel25GeometryNode* parameterNode, bool forceEnableCollisionDetection=false);

  void LoadTreatmentMachine(vtkMRMLPatientPositioningNode* parameterNode);
  void BuildRobotTableGeometryTransformHierarchy();
  void ResetModelsToInitialPosition(vtkMRMLPatientPositioningNode* parameterNode);
  void SetupTreatmentMachine(vtkMRMLPatientPositioningNode* parameterNode);

  void SetXrayImagesProjection(vtkMRMLPatientPositioningNode* parameterNode, vtkMRMLPatientPositioningNode::XrayProjectionType projection,
    vtkMRMLSliceCompositeNode* sliceCompNode, vtkMRMLSliceNode* sliceNode);
  vtkMRMLLinearTransformNode* GetXrayImageRasToIjkMatrixTransformNode(vtkMRMLScalarVolumeNode* xrayImageNode, vtkMRMLRTBeamNode* xrayBeamNode);

// Get treatment machine properties from descriptor file
public:
  /// Get part type as string
  const char* GetTreatmentMachinePartTypeAsString(vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier type);

  vtkGetObjectMacro(TableTopRobotLogic, vtkSlicerTableTopRobotTransformLogic);

  /// Get part name for part type in the currently loaded treatment machine description
  std::string GetNameForPartType(std::string partType);
  /// Get relative file path for part type in the currently loaded treatment machine description
  std::string GetFilePathForPartType(std::string partType);
  /// Get transform matrix between loaded part file and RAS for part type in the currently loaded treatment machine description
  /// \param fileToPartTransformMatrix Output file to RAS
  /// \return Success flag
  bool GetFileToRASTransformMatrixForPartType(std::string partType, vtkMatrix4x4* fileToPartTransformMatrix);
  /// Get color for part type in the currently loaded treatment machine description
  vtkVector3d GetColorForPartType(std::string partType);
  /// Get state for part type in the currently loaded treatment machine description.
  /// Valid states are "Disabled" (not loaded), "Active" (loaded and collisions computed), "Passive" (loaded but no collisions).
  std::string GetStateForPartType(std::string partType);
  /// Get TableTopRobotTransformLogic
  vtkSlicerTableTopRobotTransformLogic* GetTableTopRobotTransformLogic() const;

protected:
  vtkSlicerPatientPositioningLogic();
  ~vtkSlicerPatientPositioningLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  /// Handles events registered in the observer manager
  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

private:

  vtkSlicerPatientPositioningLogic(const vtkSlicerPatientPositioningLogic&); // Not implemented
  void operator=(const vtkSlicerPatientPositioningLogic&); // Not implemented

  vtkSlicerTableTopRobotTransformLogic* TableTopRobotLogic{ nullptr };

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal; // For access from the callback function
};

#endif