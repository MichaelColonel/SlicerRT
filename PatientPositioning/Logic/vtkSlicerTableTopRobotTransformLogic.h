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

#ifndef __vtkSlicerTableTopRobotTransformLogic_h
#define __vtkSlicerTableTopRobotTransformLogic_h

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
class vtkMRMLChannel25GeometryNode;

// FixedReference -> BaseFixed -> BaseRotation -> Shoulder -> Elbow -> Wrist -> TableTop
class VTK_SLICER_PATIENTPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerTableTopRobotTransformLogic : public vtkMRMLAbstractLogic
{
public:

  enum CoordinateSystemIdentifier : int
  {
    RAS = 0,
    FixedReference,
    BaseFixed,
    BaseRotation, // Rotation along Z-axis of BaseFixed
    Shoulder, // Rotation along Y-axis of BaseRotation
    Elbow, // Rotation along Y-axis of Shoulder
    Wrist, // Rotation along Y-axis of Elbow
    TableTop, // Translate from Wrist flange center to Table Top center
    Patient, // Translate from Table Top center to Patient center
    CoordinateSystemIdentifier_Last // Last index used for adding more coordinate systems externally
  };
  typedef std::list< CoordinateSystemIdentifier > CoordinateSystemsList;

  static vtkSlicerTableTopRobotTransformLogic *New();
  vtkTypeMacro(vtkSlicerTableTopRobotTransformLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create or get transforms taking part in the IEC logic, and build the transform hierarchy
  void BuildTableRobotTransformHierarchy();

  /// Get transform node between two coordinate systems is exists
  /// \return Transform node if there is a direct transform between the specified coordinate frames, nullptr otherwise
  ///   Note: If IHEP does not specify a transform between the given coordinate frames, then there will be no node with the returned name.
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

  /// Reset RAS to Patient isocenter translate, required for correct
  /// IHEP stand models transforms 
  void ResetRasToPatientIsocenterTranslate();
  /// Restore RAS to Patient isocenter translate
  void RestoreRasToPatientIsocenterTranslate(double isocenter[3]);

  /// Update fixed reference to RAS transform based on isocenter and patient support transforms
  void UpdateFixedReferenceToRASTransform(vtkMRMLChannel25GeometryNode* channelNode, double* isocenter = nullptr);
  /// Update BaseFixedToFixedReference transform based on A1 robot angle parameter
  void UpdateBaseFixedToFixedReferenceTransform(vtkMRMLChannel25GeometryNode* channelNode);

  /// Get part type as string
  const char* GetTreatmentMachinePartTypeAsString(CoordinateSystemIdentifier type);

protected:
  vtkSlicerTableTopRobotTransformLogic();
  ~vtkSlicerTableTopRobotTransformLogic() override;

  /// Get name of transform node between two coordinate systems
  /// \return Transform node name between the specified coordinate frames.
  ///   Note: If IHEP does not specify a transform between the given coordinate frames, then there will be no node with the returned name.
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
  std::vector< std::pair<CoordinateSystemIdentifier, CoordinateSystemIdentifier> > TableTopRobotTransforms;

  // TODO: for hierarchy use tree with nodes, something like graph
  /// Map of IHEP coordinate systems hierarchy
  std::map< CoordinateSystemIdentifier, CoordinateSystemsList > CoordinateSystemsHierarchy;

private:
  vtkSlicerTableTopRobotTransformLogic(const vtkSlicerTableTopRobotTransformLogic&) = delete;
  void operator=(const vtkSlicerTableTopRobotTransformLogic&) = delete;
};

#endif
