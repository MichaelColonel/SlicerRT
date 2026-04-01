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
#include <vtkMRMLChannel26GeometryNode.h>

// STD includes
#include <cstdlib>

#include "vtkSlicerPatientPositioningModuleLogicExport.h"
#include "vtkSlicerChannel26Cabin3RobotsTransformLogic.h"

class vtkMRMLLinearTransformNode;
class vtkMRMLSliceCompositeNode;
class vtkMRMLSliceNode;
class vtkMRMLTransformNode;
class vtkMRMLPatientPositioningNode;
class vtkMRMLRTBeamNode;
class vtkMRMLMarkupsPlaneNode;
class vtkMRMLMarkupsLineNode;
class vtkMRMLMarkupsFiducialNode;

class vtkMatrix4x4;
class vtkPolyData;
class vtkVector3d;
class vtkCollisionDetectionFilter;

class vtkSlicerDrrImageComputationLogic;
class vtkSlicerPlanarImageModuleLogic;
class vtkMRMLDrrImageComputationNode;

class VTK_SLICER_PATIENTPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerPatientPositioningLogic :
  public vtkSlicerModuleLogic
{
public:
  enum Channel26CabinGeometryType : int {
    CABIN1,
    CABIN2,
    CABIN3,
    Channel26CabinGeometryType_Last
  };
  static const char* TREATMENT_MACHINE_DESCRIPTOR_FILE_PATH_ATTRIBUTE_NAME;
  static unsigned long MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS;

  static const char* FIXEDBEAMAXIS_MARKUPS_LINE_NODE_NAME; //  Beam axis line in fixed reference frame
  static const char* FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME; //  isocenter point in fixed reference frame

  static const char* DRR_TRANSFORM_NODE_NAME;
  static const char* DRR_TRANSLATE_NODE_NAME;

  static const char* TABLETOP_MARKUPS_PLANE_NODE_NAME;
  static const char* TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME;

  static vtkSlicerPatientPositioningLogic *New();
  vtkTypeMacro(vtkSlicerPatientPositioningLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Load and setup components of the treatment machine into the scene based on its description.
  /// \param parameterNode Parameter node contains the treatment machine descriptor file path.
  /// \return List of parts that were successfully set up.
  std::vector<vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier> LoadTreatmentMachineComponents(
    vtkMRMLPatientPositioningNode* parameterNode);
  /// Set up the IEC transforms and model properties on the treatment machine models.
  /// \param forceEnableCollisionDetection Enable collision detection between parts even if calculation is potentially
  ///        lengthy absed on the number of triangles of the parts.
  /// \return List of parts that were successfully set up.
  std::vector<vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier> SetupTreatmentMachineModels(
    vtkMRMLPatientPositioningNode* parameterNode, bool forceEnableCollisionDetection=false);

  void BuildRobotsTransformHierarchy();
  void ShowModelsNodes(vtkMRMLPatientPositioningNode* parameterNode, bool show = true);
  void ShowMarkupsNodes(vtkMRMLPatientPositioningNode* parameterNode, bool show = true);

  /// Get Cabin26ARobotsTransformLogic
  vtkSlicerChannel26Cabin3RobotsTransformLogic* GetChannel26RobotsTransformLogic() const;

  /// Create TableTop plane markups node for visualization
  vtkMRMLMarkupsPlaneNode* CreateTableTopPlaneNode(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Create TableTop fiducial markups node for visualization of fix holes
  vtkMRMLMarkupsFiducialNode* CreateTableTopFiducialNode(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update TableTop markups plane node using parameter node data and geometry hierarchy
  void UpdateTableTopPlaneNode(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update TableTop markups fiducial node using parameter node data and geometry hierarchy
  void UpdateTableTopFiducialNode(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Creates a Cabin-3 beam axis line node (axis in FixedReference frame)
  /// \return a valid markups line node pointer or nullptr otherwise
  vtkMRMLMarkupsLineNode* CreateCabin3BeamAxisLineNode(vtkMRMLPatientPositioningNode* parameterNode);
  /// Creates a Cabin-3 isocenter fiducial node (point in FixedReference frame)
  /// \return a valid markups fiducial node pointer or nullptr otherwise
  vtkMRMLMarkupsFiducialNode* CreateCabin3IsocenterFiducialNode(vtkMRMLPatientPositioningNode* parameterNode);
  /// Create Cabin-3 fixed reference ion beam and plan and add beam to the parameter node
  vtkMRMLRTChannel26Cabin3BeamNode* CreateCabin3BeamPlanAndNode(vtkMRMLPatientPositioningNode* parameterNode);
  /// Create C-arm x-ray beam and plan and add C-arm x-ray beam to the parameter node
  vtkMRMLRTCarmBeamNode* CreateCarmXrayPlanAndNode(vtkMRMLPatientPositioningNode* parameterNode);
  /// Create a DRR node for C-arm x-ray beam and detector and add tp the parameter node
  vtkMRMLDrrImageComputationNode* CreateCarmXrayDrrNode(vtkMRMLPatientPositioningNode* parameterNode);
  /// Apply C-arm x-ray detector transform to radiogram scalar volume from C-arm
  /// Create display node similar to DRR image
  bool ApplyCarmXrayDetectorTransformToXrayImage(vtkMRMLPatientPositioningNode* parameterNode,
    vtkMRMLScalarVolumeNode* xrayImageVolume);
  /// Setup (or update) C-arm x-ray model display node, as a texture using PlanarImageLogic
  bool SetupXrayImageGeometry(vtkMRMLPatientPositioningNode* parameterNode,
    vtkMRMLScalarVolumeNode* xrayImageVolume);
  /// Get default registration transform node (hidden from SH and editor)
  vtkMRMLLinearTransformNode* GetDefaultRegistrationTransformNode();
  /// Update patient isocenter to fixed isocenter in frame
  bool UpdateIsocenterTranslate(vtkMRMLPatientPositioningNode* parameterNode,
    vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier frame,
    double translate[3]);

public:
  // Get treatment machine properties from descriptor file
  /// Get part type as string
  const char* GetTreatmentMachinePartTypeAsString(vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier type);

  vtkGetObjectMacro(Channel26RobotsLogic, vtkSlicerChannel26Cabin3RobotsTransformLogic);

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

  // Set/get methods for collision filters
  vtkGetObjectMacro(TableTopElbowCollisionDetection, vtkCollisionDetectionFilter);
  vtkGetObjectMacro(TableTopShoulderCollisionDetection, vtkCollisionDetectionFilter);
  vtkGetObjectMacro(TableTopBaseRotationCollisionDetection, vtkCollisionDetectionFilter);
  vtkGetObjectMacro(TableTopBaseFixedCollisionDetection, vtkCollisionDetectionFilter);
  vtkGetObjectMacro(TableTopFixedReferenceCollisionDetection, vtkCollisionDetectionFilter);
  vtkGetObjectMacro(CollimatorPatientCollisionDetection, vtkCollisionDetectionFilter);
  vtkGetObjectMacro(CollimatorTableTopCollisionDetection, vtkCollisionDetectionFilter);
  vtkGetObjectMacro(AdditionalModelsTableTopCollisionDetection, vtkCollisionDetectionFilter);
  vtkGetObjectMacro(AdditionalModelsPatientSupportCollisionDetection, vtkCollisionDetectionFilter);

  /// Set DRR Image Computation module logic
  void SetDrrImageComputationLogic(vtkSlicerDrrImageComputationLogic* drrImageComputationLogic);
  /// Get DRR Image Computation module logic
  vtkGetObjectMacro(DrrImageComputationLogic, vtkSlicerDrrImageComputationLogic);

  /// Set Planar Image module logic
  void SetPlanarImageLogic(vtkSlicerPlanarImageModuleLogic* planarImageLogic);
  /// Get Planar Image module logic
  vtkGetObjectMacro(PlanarImageLogic, vtkSlicerPlanarImageModuleLogic);

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

  vtkSlicerChannel26Cabin3RobotsTransformLogic* Channel26RobotsLogic{ nullptr };

  vtkCollisionDetectionFilter* TableTopElbowCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* TableTopShoulderCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* TableTopBaseRotationCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* TableTopBaseFixedCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* TableTopFixedReferenceCollisionDetection{ nullptr };

  vtkCollisionDetectionFilter* CollimatorPatientCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* CollimatorTableTopCollisionDetection{ nullptr };

  vtkCollisionDetectionFilter* AdditionalModelsTableTopCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* AdditionalModelsPatientSupportCollisionDetection{ nullptr };

  /// DRR Image Computation module logic instance
  vtkSlicerDrrImageComputationLogic* DrrImageComputationLogic{ nullptr };
  /// Planar Image logic instance
  vtkSlicerPlanarImageModuleLogic* PlanarImageLogic{ nullptr };

private:
  vtkSlicerPatientPositioningLogic(const vtkSlicerPatientPositioningLogic&); // Not implemented
  void operator=(const vtkSlicerPatientPositioningLogic&); // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal; // For access from the callback function
};

#endif
