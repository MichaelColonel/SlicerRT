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
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLMarkupsPlaneNode;
class vtkMRMLCameraNode;

/// \ingroup Slicer_QtModules_IhepStandGeometry
class VTK_SLICER_IHEPSTANDGEOMETRY_MODULE_LOGIC_EXPORT vtkSlicerIhepStandGeometryLogic :
  public vtkSlicerModuleLogic
{
public:

  static const char* FIXEDREFERENCE_MODEL_NAME; // Fixed Reference
  static const char* PATIENTSUPPORT_MODEL_NAME; // Patient Support Rotation
  static const char* TABLE_LONGITUDINAL_MODEL_NAME; // Table Platform Inferior-Superior Movement (Longitudinal)
  static const char* TABLE_LATERAL_MODEL_NAME; // Table Support Left-Right Movement (Lateral)
  static const char* TABLE_ORIGIN_MODEL_NAME; // Table Origin Support
  static const char* TABLE_MIRROR_MODEL_NAME; // Table Mirror Support
  static const char* TABLE_MIDDLE_MODEL_NAME; // Table Middle Support
  static const char* TABLETOP_MODEL_NAME; // Table Top

  static const char* TABLETOP_PLANE_MARKUPS_NODE_NAME; // TableTop plane position and orientation from three fiducials below
  static const char* TABLE_FIDUCIALS_MARKUPS_NODE_NAME; // Three fiducials show TableTop position Z origin, mirror, middle respectively
  static const char* TABLE_FIDUCIALS_TRANSFORM_NODE_NAME; // Transform for fiducials for proper positioning

  static const char* FIXEDREFERENCE_LINE_MARKUPS_NODE_NAME; //  Beam axis line in fixed reference frame
  static const char* FIXEDREFERENCE_LINE_TRANSFORM_NODE_NAME; // Transform for beam axis line for proper positioning

  static vtkSlicerIhepStandGeometryLogic *New();
  vtkTypeMacro(vtkSlicerIhepStandGeometryLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Create Table fiducial markups node for visualization
  vtkMRMLMarkupsFiducialNode* CreateTableFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Create TableTop plane markups node for visualization
  vtkMRMLMarkupsPlaneNode* CreateTableTopPlaneNode( vtkMRMLIhepStandGeometryNode* parameterNode,
    vtkMRMLMarkupsFiducialNode* fiducialNode);
  /// Create FixedReference line markups node for visualization
  vtkMRMLMarkupsLineNode* CreateFixedReferenceLineNode(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Update Table markups fiducial node using parameter node data and geometry hierarchy
  void UpdateTableFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Update TableTop markups plane node using parameter node data and geometry hierarchy
  void UpdateTableTopPlaneNode( vtkMRMLIhepStandGeometryNode* parameterNode,
    vtkMRMLMarkupsFiducialNode* fiducialNode);
  /// Update FixedReference markups line node using parameter node data and geometry hierarchy
  void UpdateFixedReferenceLineNode(vtkMRMLIhepStandGeometryNode* parameterNode);

  void UpdateTableTopToTableLateralTransform(double posOrigin[3], double posMirror[3], double posMiddle[3]);

  /// Show markups
  void ShowMarkupsNodes(bool toggled = false);

  /// Load pre-defined components of the treatment machine into the scene
  /// \param parameterNode Parameter node contains the type of treatment machine
  ///        (must match folder name where the models can be found)
  void LoadTreatmentMachineModels(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// All angles to zero and only translation applied
  void ResetModelsToInitialPosition(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Apply new TableTop to TableOrigin translate (TableTop->TableOrigin)
  void UpdateTableTopToTableOriginTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Apply new TableOrigin to TableLateral translate (TableOriginVerticalMovement->TableLateralMovement)
  void UpdateTableOriginToTableLateralTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Apply new TableMirror to TableLateral translate (TableMirrorVerticalMovement->TableLateralMovement)
  void UpdateTableMirrorToTableLateralTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Apply new TableMiddle to TableLateral translate (TableMiddleVerticalMovement->TableLateralMovement)
  void UpdateTableMiddleToTableLateralTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Apply new TableLateral to TableLongitudinal translate (TableLateralMovement->TableLongitudinalMovement)
  void UpdateTableLateralToTableLongitudinalTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Apply new TableLongitudinal to PatientSupport translate (TableLongitudinalMovement->PatientSupportRotation)
  void UpdateTableLongitudinalToPatientSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode);


  /// Apply new PatientSupport to FixedReference translate (PatientSupportRotation->FixedReference)
  void UpdatePatientSupportToFixedReferenceTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Apply new Patient to TableTop translate (Patient->TableTop)
  void UpdatePatientToTableTopTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Set up the IHEP transforms and model properties on the treatment machine models
  void SetupTreatmentMachineModels(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Initial Translation for different transforms
  void InitialSetupTransformTranslations(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Get transform node for line markups node
  vtkMRMLLinearTransformNode* GetFixedReferenceMarkupsTransform();

  /// Set 3D View camera for FixedReference model
  void SetFixedReferenceCamera(vtkMRMLCameraNode* cameraNode);

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

  /// IHEP Geometry transform hierarchy logic
  vtkSlicerIhepStandGeometryTransformLogic* IhepLogic;
  /// Camera node for 3D view
  vtkMRMLCameraNode* Camera3DViewNode;

private:
  /// Create or get transforms taking part in the IHEP logic and additional devices, and build the transform hierarchy
  void BuildIhepStangGeometryTransformHierarchy();

  /// Update transform for fiducial and plane markups nodes
  vtkMRMLLinearTransformNode* UpdateTableMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Update transform for line markups node
  vtkMRMLLinearTransformNode* UpdateFixedReferenceMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Update FixedReference 3D view camera position
  void UpdateFixedReferenceCamera(vtkMRMLIhepStandGeometryNode* parameterNode, vtkMRMLCameraNode* cameraNode = nullptr);

  vtkSlicerIhepStandGeometryLogic(const vtkSlicerIhepStandGeometryLogic&) = delete;
  void operator=(const vtkSlicerIhepStandGeometryLogic&) = delete;
};

#endif
