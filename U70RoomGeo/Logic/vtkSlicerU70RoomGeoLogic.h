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

// .NAME vtkSlicerU70RoomGeoLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes

#ifndef __vtkSlicerU70RoomGeoLogic_h
#define __vtkSlicerU70RoomGeoLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerU70RoomGeoModuleLogicExport.h"

#include "vtkSlicerChannel26Cabin3RobotsTransformLogic.h"

class vtkMRMLU70RoomGeoNode;
class vtkMRMLChannel26GeometryNode;

class vtkMRMLSegmentationNode;
class vtkMRMLMarkupsPlaneNode;
class vtkMRMLMarkupsLineNode;
class vtkMRMLMarkupsFiducialNode;

class vtkMatrix4x4;
class vtkVector3d;

class VTK_SLICER_U70ROOMGEO_MODULE_LOGIC_EXPORT vtkSlicerU70RoomGeoLogic : public vtkSlicerModuleLogic
{
public:
  static const char* TREATMENT_MACHINE_DESCRIPTOR_FILE_PATH_ATTRIBUTE_NAME;
  static unsigned long MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS;

  static const char* FIXEDBEAMAXIS_MARKUPS_LINE_NODE_NAME; //  Beam axis line in fixed reference frame
  static const char* FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME; //  isocenter point in fixed reference frame

  static const char* TABLETOP_MARKUPS_PLANE_NODE_NAME;
  static const char* TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME;

  static vtkSlicerU70RoomGeoLogic* New();
  vtkTypeMacro(vtkSlicerU70RoomGeoLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Load and setup components of the treatment machine into the scene based on its description.
  /// \param parameterNode Parameter node contains the treatment machine descriptor file path.
  /// \return List of parts that were successfully set up.
  std::vector<vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier> LoadTreatmentMachineComponents(
    vtkMRMLU70RoomGeoNode* parameterNode);
  /// Set up the IEC transforms and model properties on the treatment machine models.
  /// \param forceEnableCollisionDetection Enable collision detection between parts even if calculation is potentially
  ///        lengthy absed on the number of triangles of the parts.
  /// \return List of parts that were successfully set up.
  std::vector<vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier> SetupTreatmentMachineModels(
    vtkMRMLU70RoomGeoNode* parameterNode, bool forceEnableCollisionDetection=false);

  void BuildRobotsTransformHierarchy();
  void ShowModelsNodes(vtkMRMLU70RoomGeoNode* parameterNode, bool show = true);
  void ShowMarkupsNodes(vtkMRMLU70RoomGeoNode* parameterNode, bool show = true);

  /// Get Cabin26RobotsTransformLogic
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
  vtkMRMLMarkupsLineNode* CreateBeamAxisLineNode(vtkMRMLU70RoomGeoNode* parameterNode);
  /// Creates a Cabin-3 isocenter fiducial node (point in FixedReference frame)
  /// \return a valid markups fiducial node pointer or nullptr otherwise
  vtkMRMLMarkupsFiducialNode* CreateIsocenterFiducialNode(vtkMRMLU70RoomGeoNode* parameterNode);

  /// Get treatment machine properties from descriptor file
  /// Get part type as string
  const char* GetTreatmentMachinePartTypeAsString(vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier type);
  vtkGetObjectMacro(Channel26Cabin3RobotsLogic, vtkSlicerChannel26Cabin3RobotsTransformLogic);

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

protected:
  vtkSlicerU70RoomGeoLogic();
  ~vtkSlicerU70RoomGeoLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  /// Handles events registered in the observer manager
  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  vtkSlicerChannel26Cabin3RobotsTransformLogic* Channel26Cabin3RobotsLogic{ nullptr };

private:
  vtkSlicerU70RoomGeoLogic(const vtkSlicerU70RoomGeoLogic&); // Not implemented
  void operator=(const vtkSlicerU70RoomGeoLogic&);            // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal; // For access from the callback function
};

#endif
