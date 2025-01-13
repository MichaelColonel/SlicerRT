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

#ifndef __vtkSlicerCabin26ARobotsTransformLogic_h
#define __vtkSlicerCabin26ARobotsTransformLogic_h

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
class vtkMRMLCabin26AGeometryNode;

// FixedReference -> TableBaseFixed -> TableBaseRotation -> TableShoulder -> TableElbow -> TableWrist -> TableFlange -> TableTop
// FixedReference -> CArmBaseFixed -> CArmBaseRotation -> CArmShoulder -> CArmElbow -> CArmWrist -> CArmFlange -> CArm
// CArm -> XraySource
// CArm -> Imager
class VTK_SLICER_PATIENTPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerCabin26ARobotsTransformLogic : public vtkMRMLAbstractLogic
{
public:
  enum CoordinateSystemIdentifier : int
  {
    RAS = 0,
    FixedReference,
    TableBaseFixed, // Translate from BaseFixed center to FixedReference center
    TableBaseRotation, // Rotation along Z-axis of BaseFixed
    TableShoulder, // Rotation along Y-axis of BaseRotation
    TableElbow, // Rotation along Y-axis of Shoulder
    TableWrist, // Rotation along Y-axis of Elbow
    TableFlange, // Mounted under the Table Top center
    TableTop, // Translate from Wrist flange center to Table Top center
    Patient, // Translate from Table Top center to Patient center
    CArmBaseFixed, // Translate from BaseFixed center to FixedReference center
    CArmBaseRotation, // Rotation along Z-axis of BaseFixed
    CArmShoulder, // Rotation along Y-axis of BaseRotation
    CArmElbow, // Rotation along Y-axis of Shoulder
    CArmWrist, // Rotation along Y-axis of Elbow
    CArmFlange, // Mounted under the Table Top center
    CArm, // Translate from Wrist flange center to CArm center
    XRay,
    Imager,
    CoordinateSystemIdentifier_Last // Last index used for adding more coordinate systems externally
  };
  typedef std::list< CoordinateSystemIdentifier > CoordinateSystemsList;

  static vtkSlicerCabin26ARobotsTransformLogic *New();
  vtkTypeMacro(vtkSlicerCabin26ARobotsTransformLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create or get transforms taking part in the TableTop robot logic, and build the transform hierarchy
  void BuildRobotsTransformHierarchy();

  /// Get transform node between two coordinate systems is exists
  /// \return Transform node if there is a direct transform between the specified coordinate frames, nullptr otherwise
  ///   Note: If it does not specify a transform between the given coordinate frames, then there will be no node with the returned name.
  vtkMRMLLinearTransformNode* GetTransformNodeBetween(
    CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame );

  /// Get general transform from one coordinate frame to another (toFrame->fromFrame)
  /// @param transformForBeam - calculate dynamic transformation for beam model or other models
  /// \return Success flag (false on any error)
  bool GetTransformBetween(CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame, vtkGeneralTransform* outputTransform, bool transformForBeam = true);
  /// Get linear transform from one coordinate frame to another (toFrame->fromFrame)
  /// @param transformForBeam - calculate dynamic transformation for beam model or other models
  /// \return Success flag (false on any error)
  bool GetTransformBetween(CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame, vtkTransform* outputTransform, bool transformForBeam = true);

  /// \warning This is an internal method
  /// Get point coordinate transform from one coordinate frame to another
  /// 1. Calculate point coordinate fromFrame system into RAS (fromFrame->RAS transform)
  /// 2. Calculate point coordinate from RAS into toFrame system (toFrame->RAS inverse transform)
  /// Dynamic Path, fromFrame (point in fromFrame) -> RAS (point in RAS) -> toFrame (point in toFrame)
  /// @param transformForBeam - calculate dynamic transformation for beam model or other models
  /// \return Success flag (false on any error)
  bool GetTransformForPointBetweenFrames( CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame,
    const double fromFramePoint[3], double toFramePoint[3], bool transformForBeam = true);

  /// Reset models position to initial ones
  void ResetToInitialPositions();

  /// Update BaseFixedToFixedReference transform based on translation
  /// Apply new BaseFixed to FixedReference translate (BaseFixed->FixedReference)
  void UpdateBaseFixedToFixedReferenceTransform(vtkMRMLCabin26AGeometryNode* channelNode);
  /// Apply new TableTop to Flange transform (TableTop->Flange)
  void UpdateTableTopToFlangeTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Apply new TableTop to Flange transform (Flange->Wrist)
  void UpdateFlangeToWristTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Apply new Wrist to Elbow transform (Wrist->Elbow)
  void UpdateWristToElbowTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Apply new Elbow to Shoulder transform (Elbow->Shoulder)
  void UpdateElbowToShoulderTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Apply new Shoulder to BaseRotation transform (Shoulder->BaseRotation)
  void UpdateShoulderToBaseRotationTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Apply new BaseRotation to BaseFixed translate (BaseRotation->BaseFixed)
  void UpdateBaseRotationToBaseFixedTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Apply new Patient to TableTop translate (Patient->TableTop)
  void UpdatePatientToTableTopTransform(vtkMRMLCabin26AGeometryNode* parameterNode);

  /// Update (or create if absent) RAS to TableTop transform
  vtkMRMLLinearTransformNode* UpdateRasToTableTopTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Update (or create if absent) RAS to Flange transform
  vtkMRMLLinearTransformNode* UpdateRasToFlangeTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Update (or create if absent) RAS to BaseFixed transform
  vtkMRMLLinearTransformNode* UpdateRasToBaseFixedTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Update (or create if absent) RAS to BaseRotation transform
  vtkMRMLLinearTransformNode* UpdateRasToBaseRotationTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Update (or create if absent) RAS to Shoulder transform
  vtkMRMLLinearTransformNode* UpdateRasToShoulderTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Update (or create if absent) RAS to Elbow transform
  vtkMRMLLinearTransformNode* UpdateRasToElbowTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Update (or create if absent) RAS to Wrist transform
  vtkMRMLLinearTransformNode* UpdateRasToWristTransform(vtkMRMLCabin26AGeometryNode* parameterNode);
  /// Update (or create if absent) RAS to FixedReference transform
  vtkMRMLLinearTransformNode* UpdateRasToFixedReferenceTransform(vtkMRMLCabin26AGeometryNode* parameterNode);

  /// Get RAS to FixedReference transform
  vtkMRMLLinearTransformNode* GetFixedReferenceTransform();
  /// Get RAS to Patient transform
  vtkMRMLLinearTransformNode* GetPatientTransform();
  /// Get RAS to TableTop transform
  vtkMRMLLinearTransformNode* GetTableTopTransform();
  /// Get RAS to Elbow transform
  vtkMRMLLinearTransformNode* GetElbowTransform();
  /// Get RAS to Flange transform
  vtkMRMLLinearTransformNode* GetFlangeTransform();
  /// Get RAS to Wrist transform
  vtkMRMLLinearTransformNode* GetWristTransform();
  /// Get RAS to Shoulder transform
  vtkMRMLLinearTransformNode* GetShoulderTransform();
  /// Get RAS to BaseRotation
  vtkMRMLLinearTransformNode* GetBaseRotationTransform();
  /// Get RAS to BaseFixed
  vtkMRMLLinearTransformNode* GetBaseFixedTransform();

  /// Get part type as string
  const char* GetTreatmentMachinePartTypeAsString(CoordinateSystemIdentifier type);

protected:
  vtkSlicerCabin26ARobotsTransformLogic();
  ~vtkSlicerCabin26ARobotsTransformLogic() override;

  /// Get name of transform node between two coordinate systems
  /// \return Transform node name between the specified coordinate frames.
  ///   Note: If system does not specify a transform between the given coordinate frames, then there will be no node with the returned name.
  std::string GetTransformNodeNameBetween(CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame);

  /// @brief Get coordinate system identifiers from frame system up to root system
  /// Root system = FixedReference system
  bool GetPathToRoot( CoordinateSystemIdentifier frame, CoordinateSystemsList& path);

  /// @brief Get coordinate system identifiers from root system down to frame system
  /// Root system = FixedReference system
  bool GetPathFromRoot( CoordinateSystemIdentifier frame, CoordinateSystemsList& path);

  /// Map from \sa CoordinateSystemIdentifier to coordinate system name. Used for getting transforms
  std::map<CoordinateSystemIdentifier, std::string> CoordinateSystemsMap;

  /// List of table top coordinate system transforms
  std::vector< std::pair<CoordinateSystemIdentifier, CoordinateSystemIdentifier> > RobotsTransforms;

  // TODO: for hierarchy use tree with nodes, something like graph
  /// Map of TableTop robot coordinate systems hierarchy
  std::map< CoordinateSystemIdentifier, CoordinateSystemsList > CoordinateSystemsHierarchy;

private:
  vtkSlicerCabin26ARobotsTransformLogic(const vtkSlicerCabin26ARobotsTransformLogic&) = delete;
  void operator=(const vtkSlicerCabin26ARobotsTransformLogic&) = delete;
};

#endif
