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
#include <vtkMRMLCabin26AGeometryNode.h>

// STD includes
#include <cstdlib>

#include "vtkSlicerPatientPositioningModuleLogicExport.h"
#include "vtkSlicerCabin26ARobotsTransformLogic.h"

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

class VTK_SLICER_PATIENTPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerPatientPositioningLogic :
  public vtkSlicerModuleLogic
{
public:
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
  std::vector<vtkSlicerCabin26ARobotsTransformLogic::CoordinateSystemIdentifier> LoadTreatmentMachineComponents(
    vtkMRMLPatientPositioningNode* parameterNode);
  /// Set up the IEC transforms and model properties on the treatment machine models.
  /// \param forceEnableCollisionDetection Enable collision detection between parts even if calculation is potentially
  ///        lengthy absed on the number of triangles of the parts.
  /// \return List of parts that were successfully set up.
  std::vector<vtkSlicerCabin26ARobotsTransformLogic::CoordinateSystemIdentifier> SetupTreatmentMachineModels(
    vtkMRMLPatientPositioningNode* parameterNode, bool forceEnableCollisionDetection=false);

  void BuildRobotTableGeometryTransformHierarchy();
  std::string CheckForCollisions(vtkMRMLPatientPositioningNode* parameterNode, bool collisionDetectionEnabled = true);

  /// Get Cabin26ARobotsTransformLogic
  vtkSlicerCabin26ARobotsTransformLogic* GetCabin26ARobotsTransformLogic() const;
  /// Get translation vector from Patient isocenter to fixed beam axis (axis in FixedReference frame)
  vtkVector3d GetIsocenterToFixedBeamAxisTranslation(vtkMRMLPatientPositioningNode* parameterNode,
    vtkSlicerCabin26ARobotsTransformLogic::CoordinateSystemIdentifier fromFrame);

  /// Get alignment angles (lateral, longitudinal, vertical) between Patient ion beam and FixedReference frame
  /// \return (lateral, longitudinal, vertical) vector
  vtkVector3d GetBeamToFixedReferenceAlignment(vtkMRMLTransformNode* patientBeamTransformNode);

  /// Creates a fixed beam axis line node (axis in FixedReference frame)
  /// \return a valid markups line node pointer or nullptr otherwise
  vtkMRMLMarkupsLineNode* CreateFixedBeamAxisLineNode(vtkMRMLPatientPositioningNode* parameterNode);
  /// Creates a fixed isocenter fiducial node (point in FixedReference frame)
  /// \return a valid markups fiducial node pointer or nullptr otherwise
  vtkMRMLMarkupsFiducialNode* CreateFixedIsocenterFiducialNode(vtkMRMLPatientPositioningNode* parameterNode);

  /// Create fixed reference beam and plan and add beam to the parameter node
  vtkMRMLRTCabin26AIonBeamNode* CreateFixedBeamPlanAndNode(vtkMRMLPatientPositioningNode* parameterNode);
  /// Create external xray beam and plan and add ext beam to the parameter node
  vtkMRMLRTFixedBeamNode* CreateExternalXrayPlanAndNode(vtkMRMLPatientPositioningNode* parameterNode);

  /// Update fixed beam axis using two new points in beam limiting device frame
  void UpdateFixedBeamAxisLineNode(vtkMRMLPatientPositioningNode* parameterNode,
    const double point0[3], const double point1[3]);

  /// Create TableTop plane markups node for visualization
  vtkMRMLMarkupsPlaneNode* CreateTableTopPlaneNode(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Create TableTop fiducial markups node for visualization of fix holes
  vtkMRMLMarkupsFiducialNode* CreateTableTopFiducialNode(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Update TableTop markups plane node using parameter node data and geometry hierarchy
  void UpdateTableTopPlaneNode(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Update TableTop markups fiducial node using parameter node data and geometry hierarchy
  void UpdateTableTopFiducialNode(vtkMRMLCabin26AGeometryNode* parameterNode);

  bool AlignTableTop(vtkMRMLCabin26AGeometryNode* parameterNode, vtkMRMLRTBeamNode* patientBeam,
    vtkTransform* fixedReferenceToPatientBeamTransform, double tableTopAngles[6]);

public:
  // Get treatment machine properties from descriptor file
  /// Get part type as string
  const char* GetTreatmentMachinePartTypeAsString(vtkSlicerCabin26ARobotsTransformLogic::CoordinateSystemIdentifier type);

  vtkGetObjectMacro(Cabin26ARobotsLogic, vtkSlicerCabin26ARobotsTransformLogic);

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

  /// Get patient body closed surface poly data from segmentation node and segment selection in the parameter node
//  bool GetPatientBodyPolyData(vtkMRMLPatientPositioningNode* parameterNode, vtkPolyData* patientBodyPolyData);

  vtkSlicerCabin26ARobotsTransformLogic* Cabin26ARobotsLogic{ nullptr };

  vtkCollisionDetectionFilter* TableTopElbowCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* TableTopShoulderCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* TableTopBaseRotationCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* TableTopBaseFixedCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* TableTopFixedReferenceCollisionDetection{ nullptr };

  vtkCollisionDetectionFilter* CollimatorPatientCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* CollimatorTableTopCollisionDetection{ nullptr };

  vtkCollisionDetectionFilter* AdditionalModelsTableTopCollisionDetection{ nullptr };
  vtkCollisionDetectionFilter* AdditionalModelsPatientSupportCollisionDetection{ nullptr };

private:
  vtkSlicerPatientPositioningLogic(const vtkSlicerPatientPositioningLogic&); // Not implemented
  void operator=(const vtkSlicerPatientPositioningLogic&); // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal; // For access from the callback function
};

#endif
