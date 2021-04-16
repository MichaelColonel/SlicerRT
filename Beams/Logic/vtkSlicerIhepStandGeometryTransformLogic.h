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

#ifndef __vtkSlicerIhepStandGeometryTransformLogic_h
#define __vtkSlicerIhepStandGeometryTransformLogic_h

#include "vtkSlicerBeamsModuleLogicExport.h"

// Slicer includes
#include "vtkMRMLAbstractLogic.h"

// STD includes
#include <map>
#include <vector>
#include <list>

class vtkGeneralTransform;
class vtkMRMLRTBeamNode;
class vtkMRMLLinearTransformNode;
class vtkMRMLIhepStandGeometryNode;

class VTK_SLICER_BEAMS_LOGIC_EXPORT vtkSlicerIhepStandGeometryTransformLogic : public vtkMRMLAbstractLogic
{
public:
  enum CoordinateSystemIdentifier
  {
    RAS = 0,
    FixedReference,
    Collimator,
    PatientSupport, // Rotation of patient support
    TableTopStand, // Inferior-Superior (Longitudinal), Left-Right (Lateral) movement of the table top
    TableTopVertical, // Posterior-Anterior (Vertical) movement of table top
    TableTop, // Rotations of table top (by movement of TableTopStand three vertical basements)
    Patient,
    LastIhepCoordinateFrame // Last index used for adding more coordinate systems externally
  };
  typedef std::list< CoordinateSystemIdentifier > CoordinateSystemsList;

  static vtkSlicerIhepStandGeometryTransformLogic *New();
  vtkTypeMacro(vtkSlicerIhepStandGeometryTransformLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create or get transforms taking part in the IEC logic, and build the transform hierarchy
  void BuildIHEPTransformHierarchy();

  /// Get transform node between two coordinate systems is exists
  /// \return Transform node if there is a direct transform between the specified coordinate frames, nullptr otherwise
  ///   Note: If IHEP does not specify a transform between the given coordinate frames, then there will be no node with the returned name.
  vtkMRMLLinearTransformNode* GetTransformNodeBetween(
    CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame );

  /// Get transform from one coordinate frame to another
  /// @param transformForBeam - calculate dynamic transformation for beam model or other models
  /// \return Success flag (false on any error)
  bool GetTransformBetween(CoordinateSystemIdentifier fromFrame, CoordinateSystemIdentifier toFrame, vtkGeneralTransform* outputTransform, bool transformForBeam = true);

  /// Update parent transform node of a given beam from the IHEP transform hierarchy and the beam parameters
  void UpdateBeamTransform(vtkMRMLRTBeamNode* beamNode);
  void UpdateBeamTransform( vtkMRMLRTBeamNode* beamNode, 
    vtkMRMLLinearTransformNode* beamTransformNode, double* isocenter = nullptr);

  /// Update IHEP transforms according to beam node
  void UpdateIHEPTransformsFromBeam( vtkMRMLRTBeamNode* beamNode, double* isocenter = nullptr);
  void UpdateIHEPTransformsFromParameter( vtkMRMLIhepStandGeometryNode* parameterNode, double* isocenter = nullptr);

protected:
  vtkSlicerIhepStandGeometryTransformLogic();
  ~vtkSlicerIhepStandGeometryTransformLogic() override;

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

  /// List of IHEP transforms
  std::vector< std::pair<CoordinateSystemIdentifier, CoordinateSystemIdentifier> > IhepTransforms;

  // TODO: for hierarchy use tree with nodes, something like graph
  /// Map of IHEP coordinate systems hierarchy
  std::map< CoordinateSystemIdentifier, CoordinateSystemsList > CoordinateSystemsHierarchy;

private:
  vtkSlicerIhepStandGeometryTransformLogic(const vtkSlicerIhepStandGeometryTransformLogic&) = delete;
  void operator=(const vtkSlicerIhepStandGeometryTransformLogic&) = delete;
};

#endif
