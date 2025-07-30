/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Vinith Suriyakumar and Csaba Pinter,
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __vtkSlicerChannel26Cabin3RobotsTransformLogic_h
#define __vtkSlicerChannel26Cabin3RobotsTransformLogic_h

#include "vtkSlicerPatientPositioningModuleLogicExport.h"

// Slicer includes
#include "vtkMRMLAbstractLogic.h"

// STD includes
#include <map>
#include <vector>
#include <list>

class vtkGeneralTransform;
class vtkTransform;
class vtkMRMLRTBeamNode;
class vtkMRMLLinearTransformNode;
class vtkMRMLChannel26GeometryNode;

// FixedReference -> TableRobotBaseFixed -> TableRobotBaseRotation -> TableRobotShoulder
// TableRobotShoulder -> TableRobotElbowShoulder -> TableRobotElbowWrist -> TableRobotWrist
// TableRobotWrist -> TableRobotFlange -> TableFlange -> TableTop -> Patient
class VTK_SLICER_PATIENTPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerChannel26Cabin3RobotsTransformLogic : public vtkMRMLAbstractLogic
{
public:
  enum CoordinateSystemIdentifier : int
  {
    RAS = 0,
    FixedReference,
    TableRobotBaseFixed, // Mounted on FixedReference. Translate from TableRobotBaseFixed center to FixedReference center
    TableRobotBaseRotation, //Mounted on TableRobotBaseFixed, performes A1 rotation. Rotation along Z-axis of BaseFixed
    TableRobotShoulder, // Mounted on TableRobotBaseRotation, performes A2 rotation. Rotation along Y-axis of BaseRotation
    TableRobotElbowShoulder, // Mounted on TableRobotShoulder, performes A3 rotation. Rotation along Y-axis of Shoulder
    TableRobotElbowWrist, // Mounted on TableRobotElbowShoulderr, performes A4 rotation. Rotation along Y-axis of Elbow
    TableRobotWrist, // Mounted on TableRobotElbowWrist, performes A5 rotation
    TableRobotFlange, // Mounted on TableRobotWrist, performes A6 rotation
    TableFlange, // Mounted on TableRobotFlange under the Table Top center (for Xray receptor)
    TableTop, // Mounted on TableFlange. Translate from TableFlange flange center to Table Top center
    CarmRobotBaseFixed, // Mounted on FixedReference. Translate from CarmRobotBaseFixed center to FixedReference center
    CarmRobotBaseRotation, // Mounted on CarmRobotBaseFixed, performes A1 rotation. Rotation along Z-axis of BaseFixed
    Patient, // Mounted on TableTop. Translate from Table Top center to Patient center
    CoordinateSystemIdentifier_Last // Last index used for adding more coordinate systems externally
  };
  typedef std::list< CoordinateSystemIdentifier > CoordinateSystemsList;

  static vtkSlicerChannel26Cabin3RobotsTransformLogic *New();
  vtkTypeMacro(vtkSlicerChannel26Cabin3RobotsTransformLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create or get transforms taking part in the TableTop robot logic, and build the transform hierarchy
  void BuildRobotsTransformHierarchy();

  /// Get transform node between two coordinate systems is exists
  /// \return Transform node if there is a direct transform between the specified coordinate frames, nullptr otherwise
  ///   Note: If it does not specify a transform between the given coordinate frames, then there will be no node with the returned name.
  vtkMRMLLinearTransformNode* GetTransformNodeBetween(CoordinateSystemIdentifier fromFrame,
    CoordinateSystemIdentifier toFrame);

  /// Get general transform from one coordinate frame to another (toFrame->fromFrame)
  /// @param transformForBeam - calculate dynamic transformation for beam model or other models
  /// \return Success flag (false on any error)
  bool GetTransformBetween(CoordinateSystemIdentifier fromFrame,
    CoordinateSystemIdentifier toFrame, vtkGeneralTransform* outputTransform,
    bool transformForBeam = true);
  /// Get linear transform from one coordinate frame to another (toFrame->fromFrame)
  /// @param transformForBeam - calculate dynamic transformation for beam model or other models
  /// \return Success flag (false on any error)
  bool GetTransformBetween(CoordinateSystemIdentifier fromFrame,
    CoordinateSystemIdentifier toFrame, vtkTransform* outputTransform,
    bool transformForBeam = true);

  /// Get point coordinate transform from one coordinate frame to another
  /// 1. Calculate point coordinate fromFrame system into RAS (fromFrame->RAS transform)
  /// 2. Calculate point coordinate from RAS into toFrame system (toFrame->RAS inverse transform)
  /// Dynamic Path, fromFrame (point in fromFrame) -> RAS (point in RAS) -> toFrame (point in toFrame)
  /// @param transformForBeam - calculate dynamic transformation for beam model or other models
  /// \return Success flag (false on any error)
  bool GetTransformForPointBetweenFrames(CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame,
  const double fromFramePoint[3], double toFramePoint[3], bool transformForBeam = false);

  /// Reset models position to initial ones
  void ResetToInitialPositions();

  /// Apply new Patient to TableTop transform (Patient->TableTop)
  void UpdatePatientToTableTopTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new TableTop to TableFlange transform (TableTop->TableFlange)
  void UpdateTableTopToTableFlangeTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new TableFlange to TableRobotFlange transform (TableFlange->TableRobotFlange)
  /// This must just a translation
  void UpdateTableFlangeToTableRobotFlangeTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new TableRobotFlange to TableRobotWrist transform (TableRobotFlange->TableRobotWrist)
  void UpdateTableRobotFlangeToTableRobotWristTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new TableRobotWrist to TableRobotElbowWrist transform (TableRobotWrist->TableRobotElbowWrist)
  void UpdateTableRobotWristToTableRobotElbowWristTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new TableRobotElbowWrist to TableRobotElbowShoulder transform (TableRobotElbowWrist->TableRobotElbowShoulder)
  void UpdateTableRobotElbowWristToTableRobotElbowShoulderTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new TableRobotElbowShoulder to TableRobotShoulder transform (TableRobotElbowShoulder->TableRobotShoulder)
  void UpdateTableRobotElbowShoulderToTableRobotShoulderTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new TableRobotShoulder to TableRobotBaseRotation transform (TableRobotShoulder->TableRobotBaseRotation)
  void UpdateTableRobotShoulderToTableRobotBaseRotationTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new TableRobotBaseRotation to TableRobotBaseFixed transform (TableRobotBaseRotation->TableRobotBaseFixed)
  void UpdateTableRobotBaseRotationToTableRobotBaseFixedTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new TableRobotBaseFixed to FixedReference transform (TableRobotBaseFixed->FixedReference)
  void UpdateTableRobotBaseFixedToFixedReferenceTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new CarmRobotBaseFixed to FixedReference transform (CarmRobotBaseFixed->FixedReference)
  void UpdateCarmRobotBaseFixedToFixedReferenceTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Apply new CarmRobotBaseRotation to CarmRobotBaseFixed transform (CarmRobotBaseRotation->CarmRobotBaseFixed)
  void UpdateCarmRobotBaseRotationToCarmRobotBaseFixedTransform(vtkMRMLChannel26GeometryNode* parameterNode);

  /// Update (or create if absent) TableTop to RAS transform
  vtkMRMLLinearTransformNode* UpdateTableTopToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) TableFlange to RAS transform
  vtkMRMLLinearTransformNode* UpdateTableFlangeToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) TableRobotFlange to RAS transform
  vtkMRMLLinearTransformNode* UpdateTableRobotFlangeToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) TableRobotWrist to RAS transform
  vtkMRMLLinearTransformNode* UpdateTableRobotWristToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) TableRobotElbowWrist to RAS transform
  vtkMRMLLinearTransformNode* UpdateTableRobotElbowWristToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) TableRobotElbowShoulder to RAS transform
  vtkMRMLLinearTransformNode* UpdateTableRobotElbowShoulderToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) TableRobotShoulder to RAS transform
  vtkMRMLLinearTransformNode* UpdateTableRobotShoulderToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) TableRobotBaseRotation to RAS transform
  vtkMRMLLinearTransformNode* UpdateTableRobotBaseRotationToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) TableRobotBaseFixed to RAS transform
  vtkMRMLLinearTransformNode* UpdateTableRobotBaseFixedToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) FixedReference to RAS transform
  vtkMRMLLinearTransformNode* UpdateFixedReferenceToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) CarmRobotBaseFixed to RAS transform
  vtkMRMLLinearTransformNode* UpdateCarmRobotBaseFixedToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);
  /// Update (or create if absent) CarmRobotBaseRotation to RAS transform
  vtkMRMLLinearTransformNode* UpdateCarmRobotBaseRotationToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode);

  /// Get Patient to RAS transform
  vtkMRMLLinearTransformNode* GetPatientTransform();
  /// Get TableTop to RAS transform
  vtkMRMLLinearTransformNode* GetTableTopTransform();
  /// Get TableFlange to RAS transform
  vtkMRMLLinearTransformNode* GetTableFlangeTransform();
  /// Get TableRobotFlange to RAS transform
  vtkMRMLLinearTransformNode* GetTableRobotFlangeTransform();
  /// Get TableRobotWrist to RAS transform
  vtkMRMLLinearTransformNode* GetTableRobotWristTransform();
  /// Get TableRobotElbowWrist to RAS transform
  vtkMRMLLinearTransformNode* GetTableRobotElbowWristTransform();
  /// Get TableRobotElbowShoulder to RAS transform
  vtkMRMLLinearTransformNode* GetTableRobotElbowShoulderTransform();
  /// Get TableRobotShoulder to RAS transform
  vtkMRMLLinearTransformNode* GetTableRobotShoulderTransform();
  /// Get TableRobotBaseRotation to RAS transform
  vtkMRMLLinearTransformNode* GetTableRobotBaseRotationTransform();
  /// Get TableRobotBaseFixed to RAS transform
  vtkMRMLLinearTransformNode* GetTableRobotBaseFixedTransform();
  /// Get FixedReference to RAS transform
  vtkMRMLLinearTransformNode* GetFixedReferenceTransform();
  /// Get CarmRobotBaseFixed to RAS transform
  vtkMRMLLinearTransformNode* GetCarmRobotBaseFixedTransform();
  /// Get CarmRobotBaseRotation to RAS transform
  vtkMRMLLinearTransformNode* GetCarmRobotBaseRotationTransform();

  /// Get part type as string
  const char* GetTreatmentMachinePartTypeAsString(CoordinateSystemIdentifier type);

  void UpdateFrameToRasHierarchy(vtkMRMLChannel26GeometryNode* parameterNode, CoordinateSystemIdentifier type);
  void UpdateTransformsHierarchy(vtkMRMLChannel26GeometryNode* parameterNode, CoordinateSystemIdentifier type);

protected:
  vtkSlicerChannel26Cabin3RobotsTransformLogic();
  ~vtkSlicerChannel26Cabin3RobotsTransformLogic() override;

  /// Get name of transform node between two coordinate systems
  /// \return Transform node name between the specified coordinate frames.
  ///   Note: If system does not specify a transform between the given coordinate frames, then there will be no node with the returned name.
  std::string GetTransformNodeNameBetween(CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame);

  /// @brief Get coordinate system identifiers from frame system up to root system
  /// Root system = FixedReference system
  bool GetPathToRoot(CoordinateSystemIdentifier frame, CoordinateSystemsList& path);

  /// @brief Get coordinate system identifiers from root system down to frame system
  /// Root system = FixedReference system
  bool GetPathFromRoot(CoordinateSystemIdentifier frame, CoordinateSystemsList& path);

  /// Map from \sa CoordinateSystemIdentifier to coordinate system name. Used for getting transforms
  std::map< CoordinateSystemIdentifier, std::string > CoordinateSystemsMap;

  /// List of robots coordinate system transforms
  std::vector< std::pair< CoordinateSystemIdentifier, CoordinateSystemIdentifier > > RobotsTransforms;

  // TODO: for hierarchy use tree with nodes, something like graph
  /// Map of TableTop robot coordinate systems hierarchy
  std::map< CoordinateSystemIdentifier, CoordinateSystemsList > CoordinateSystemsHierarchy;

  /// Update (or create if absent) Frame to RAS transform
  vtkMRMLLinearTransformNode* UpdateFrameToRasTransform(vtkMRMLChannel26GeometryNode* parameterNode,
    CoordinateSystemIdentifier frame);
  /// Get Frame to RAS transform
  vtkMRMLLinearTransformNode* GetFrameToRasTransform(CoordinateSystemIdentifier frame);

private:
  vtkSlicerChannel26Cabin3RobotsTransformLogic(const vtkSlicerChannel26Cabin3RobotsTransformLogic&) = delete;
  void operator=(const vtkSlicerChannel26Cabin3RobotsTransformLogic&) = delete;
};

#endif
