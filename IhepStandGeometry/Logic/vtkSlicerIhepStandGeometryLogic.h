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

// .NAME vtkSlicerIhepStandGeometryLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerIhepStandGeometryLogic_h
#define __vtkSlicerIhepStandGeometryLogic_h

// Slicer includes
#include <vtkSlicerModuleLogic.h>

// MRML includes

// STD includes

#include "vtkSlicerIhepStandGeometryModuleLogicExport.h"

class vtkMRMLIhepStandGeometryNode;
class vtkSlicerIhepStandGeometryTransformLogic;

class vtkMRMLRTBeamNode;
class vtkMRMLLinearTransformNode;
class vtkMRMLMarkupsLineNode;

/// \ingroup Slicer_QtModules_IhepStandGeometry
class VTK_SLICER_IHEPSTANDGEOMETRY_MODULE_LOGIC_EXPORT vtkSlicerIhepStandGeometryLogic :
  public vtkSlicerModuleLogic
{
public:

  static const char* CANYON_MODEL_NAME;
  static const char* PATIENTSUPPORT_MODEL_NAME; // Patient Support Rotation
  static const char* TABLETOPSTAND_MODEL_NAME; // Table Top Inferior-Superior Movement, and Left-Right Movement (Lateral) Stand
  static const char* TABLETOP_MODEL_NAME;
  static const char* BEAMLINE_MARKUPS_NODE_NAME; // line
  static const char* BEAMLINE_TRANSFORM_NODE_NAME;

  static const char* ORIENTATION_MARKER_MODEL_NODE_NAME;

  static vtkSlicerIhepStandGeometryLogic *New();
  vtkTypeMacro(vtkSlicerIhepStandGeometryLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Create markups nodes for visualization
  void CreateMarkupsNodes(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Update markups nodes using parameter node data
  void UpdateMarkupsNodes(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Show markups
  void ShowMarkupsNodes(bool toggled = false);

  /// Load pre-defined components of the treatment machine into the scene
  /// \param parameterNode Parameter node contains the type of treatment machine
  ///        (must match folder name where the models can be found)
  void LoadTreatmentMachineModels(vtkMRMLIhepStandGeometryNode* parameterNode);
  void ResetModelsToInitialPosition(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Apply new patient support rotation angle to transform (Fixed->PatientSupport)
  void UpdatePatientSupportToFixedReferenceTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  void UpdateTableTopToTableTopStandTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  void UpdateTableTopStandToPatientSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  void UpdatePatientToTableTopTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  void MoveModelsToIsocenter(vtkMRMLIhepStandGeometryNode* parameterNode, double isocenter[3]);

  /// Set up the IHEP transforms and model properties on the treatment machine models
  void SetupTreatmentMachineModels(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Create or get transforms taking part in the IEC logic and additional devices, and build the transform hierarchy
  void BuildIhepStangGeometryTransformHierarchy();
  void RestoreOriginalGeometryTransformHierarchy();

protected:
  vtkSlicerIhepStandGeometryLogic();
  ~vtkSlicerIhepStandGeometryLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

  /// Handles events registered in the observer manager
  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

protected:
  vtkSlicerIhepStandGeometryTransformLogic* IhepLogic;

private:

  vtkMRMLLinearTransformNode* UpdateBeamlineTransformFromBeam(vtkMRMLRTBeamNode* beamNode);
  vtkMRMLMarkupsLineNode* CreateBeamline(vtkMRMLIhepStandGeometryNode* parameterNode);

  vtkSlicerIhepStandGeometryLogic(const vtkSlicerIhepStandGeometryLogic&) = delete;
  void operator=(const vtkSlicerIhepStandGeometryLogic&) = delete;
};

#endif
