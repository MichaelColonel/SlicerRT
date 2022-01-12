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
class vtkMRMLRTIonBeamNode;
class vtkMRMLRTFixedIonBeamNode;
class vtkMRMLLinearTransformNode;
class vtkMRMLMarkupsLineNode;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLMarkupsPlaneNode;
class vtkMRMLCameraNode;

class vtkTransform;

/// \ingroup Slicer_QtModules_IhepStandGeometry
class VTK_SLICER_IHEPSTANDGEOMETRY_MODULE_LOGIC_EXPORT vtkSlicerIhepStandGeometryLogic :
  public vtkSlicerModuleLogic
{
public:

  static const char* FIXEDREFERENCE_MODEL_NAME; // Fixed Reference
  static const char* PATIENTSUPPORT_MODEL_NAME; // Patient Support Rotation
  static const char* TABLE_PLATFORM_MODEL_NAME; // Table Platform Inferior-Superior Movement (Lateral)
  static const char* TABLE_SUPPORT_MODEL_NAME; // Table Support Left-Right Movement (Longitudinal)
  static const char* TABLE_ORIGIN_MODEL_NAME; // Table Origin Support
  static const char* TABLE_MIRROR_MODEL_NAME; // Table Mirror Support
  static const char* TABLE_MIDDLE_MODEL_NAME; // Table Middle Support
  static const char* TABLETOP_MODEL_NAME; // Table Top

  static const char* TABLETOP_MARKUPS_PLANE_NODE_NAME; // TableTop plane position and orientation from three fiducials below
  static const char* TABLE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME; // Fiducial shows TableTop position Z origin
  static const char* TABLE_MIRROR_MARKUPS_FIDUCIAL_NODE_NAME; // Fiducial shows TableTop position Z mirror
  static const char* TABLE_MIDDLE_MARKUPS_FIDUCIAL_NODE_NAME; // Fiducial shows TableTop position Z middle
  static const char* FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME; //  Beam axis line in fixed reference frame
  static const char* FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME; // Fiducial shows  fixed isocenter

  static vtkSlicerIhepStandGeometryLogic *New();
  vtkTypeMacro(vtkSlicerIhepStandGeometryLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Create Table fiducial markups node for visualization
  vtkMRMLMarkupsFiducialNode* CreateTableOriginFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode);
  vtkMRMLMarkupsFiducialNode* CreateTableMiddleFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode);
  vtkMRMLMarkupsFiducialNode* CreateTableMirrorFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode);
  
  /// Create TableTop plane markups node for visualization
  vtkMRMLMarkupsPlaneNode* CreateTableTopPlaneNode( vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Create FixedReference line markups node for visualization
  vtkMRMLMarkupsLineNode* CreateFixedReferenceLineNode(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Update Table markups fiducial node using parameter node data and geometry hierarchy
  void UpdateTableOriginFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode);
  void UpdateTableMiddleFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode);
  void UpdateTableMirrorFiducialNode(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Update TableTop markups plane node using parameter node data and geometry hierarchy
  void UpdateTableTopPlaneNode(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Update FixedReference markups line node using parameter node data and geometry hierarchy
  void UpdateFixedReferenceLineNode(vtkMRMLIhepStandGeometryNode* parameterNode);

  void UpdateTableTopToTableTopSupportTransform(double posOrigin[3], double posMirror[3], double posMiddle[3]);
  /// Calculate vertical position difference for TableTopMirror and TableTopMiddle from TableTopOrigin
  /// Using TableTopToTableOriginTransform, TableTopToTableSupportTransform and TableTopPlaneNode
  /// @param parameterNode - parameter
  /// @param mirrorPosition - vertical difference between mirror support from origin support 
  /// @param middlePosition - vertical difference between middle support from origin support
  /// @return true if mirror and middle differences are calculated, false otherwise
  bool CalculateTableTopPositionsFromPlaneNode( vtkMRMLIhepStandGeometryNode* parameterNode, double& mirrorPosition, double& middlePosition);

  /// Show markups
  void ShowMarkupsNodes(bool toggled = false);

  /// Load pre-defined components of the treatment machine into the scene
  /// \param parameterNode Parameter node contains the type of treatment machine
  ///        (must match folder name where the models can be found)
  void LoadTreatmentMachineModels(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// All angles to zero and only translation applied
  void ResetModelsToInitialPosition(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Apply new TableTop to TableTopOrigin transform (TableTop->TableTopOrigin)
  void UpdateTableTopToTableOriginTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Apply new TableTopOrigin to TableTopSupport translate (TableTopOrigin->TableTopSupport)
  void UpdateTableOriginToTableSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Apply new TableMirror to TableTopSupport translate (TableTopMirror->TableTopSupport)
  void UpdateTableMirrorToTableSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Apply new TableMiddle to TableTopSupport translate (TableTopMiddle->TableTopSupport)
  void UpdateTableMiddleToTableSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Apply new TableTopSupport to TablePlatform translate (TableTopSupport->TablePlatform)
  /// Longitudinal movement of the table top
  void UpdateTableSupportToTablePlatformTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  
  /// Apply new TablePlatform to PatientSupport translate (TablePlatform->PatientSupport)
  /// Lateral movement of the table top
  void UpdateTablePlatformToPatientSupportTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Apply new PatientSupport to FixedReference transform (PatientSupport->FixedReference)
  /// Rotation around Zt-axis
  void UpdatePatientSupportToFixedReferenceTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Apply new Patient to TableTop translate (Patient->TableTop)
  void UpdatePatientToTableTopTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Set up the IHEP transforms and model properties on the treatment machine models
  void SetupTreatmentMachineModels(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Set up the IHEP transforms and model properties on the treatment machine models
  void CalculateMovementsForBeam(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Create fixed reference beam and plan and add beam to the parameter node
  void CreateFixedBeamPlanAndNode(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Calculate table top translation for patient isocenter to fixed beam isocenter
  void CalculateTableTopTranslation( vtkMRMLIhepStandGeometryNode* parameterNode, const double patientIsocenter[3], const double fixedIsocenter[3]);
  /// Calculate transform matrix between RT patient beam and fixed reference beam
  void CalculateTransformBetweenBeams( vtkMRMLIhepStandGeometryNode* parameterNode, vtkMRMLRTBeamNode* patientBeam, vtkMRMLRTBeamNode* fixedBeam);

//  void SetupFixedReferenceModel(vtkMRMLIhepStandGeometryNode* parameterNode);
//  void UpdateFixedReferenceModel(vtkMRMLIhepStandGeometryNode* parameterNode);
  
//  void SetupPatientSupportModel(vtkMRMLIhepStandGeometryNode* parameterNode);
//  void UpdatePatientSupportModel(vtkMRMLIhepStandGeometryNode* parameterNode);

//  void SetupTablePlatformModel(vtkMRMLIhepStandGeometryNode* parameterNode);
//  void UpdateTablePlatformModel(vtkMRMLIhepStandGeometryNode* parameterNode);

//  void SetupTableTopSupportModel(vtkMRMLIhepStandGeometryNode* parameterNode);
//  void UpdateTableTopSupportModel(vtkMRMLIhepStandGeometryNode* parameterNode);

//  void SetupTableTopOriginModel(vtkMRMLIhepStandGeometryNode* parameterNode);
//  void UpdateTableTopOriginModel(vtkMRMLIhepStandGeometryNode* parameterNode);

//  void SetupTableTopMiddleModel(vtkMRMLIhepStandGeometryNode* parameterNode);
//  void UpdateTableTopMiddleModel(vtkMRMLIhepStandGeometryNode* parameterNode);

//  void SetupTableTopMirrorModel(vtkMRMLIhepStandGeometryNode* parameterNode);
//  void UpdateTableTopMirrorModel(vtkMRMLIhepStandGeometryNode* parameterNode);

//  void SetupTableTopModel(vtkMRMLIhepStandGeometryNode* parameterNode);
//  void UpdateTableTopModel(vtkMRMLIhepStandGeometryNode* parameterNode);
  

  /// Initial Translation for different transforms
  void InitialSetupTransformTranslations(vtkMRMLIhepStandGeometryNode* parameterNode);

  /// Get transform node for line markups node (FixedReference)
  vtkMRMLLinearTransformNode* GetFixedReferenceTransform();
  vtkMRMLLinearTransformNode* GetPatientTransform();
  vtkMRMLLinearTransformNode* GetTableTopTransform();
  vtkMRMLLinearTransformNode* GetTableTopSupportTransform();
  vtkMRMLLinearTransformNode* GetTableTopOriginTransform();
  vtkMRMLLinearTransformNode* GetTableTopMiddleTransform();
  vtkMRMLLinearTransformNode* GetTableTopMirrorTransform();
  vtkMRMLLinearTransformNode* GetTablePlatformTransform();
  vtkMRMLLinearTransformNode* GetPatientSupportTransform();

  /// Set 3D View camera for FixedReference model
  void SetFixedReferenceCamera(vtkMRMLCameraNode* cameraNode);

  /// Calculate TableTop center to FixedIsocenter translate
  /// \param translateTableTopFrame - translation of the TableTop frame
  /// \return true if success, false otherwise
  bool GetTableTopCenterToFixedIsocenterTranslate(vtkMRMLIhepStandGeometryNode* parameterNode,
    double translateTableTopFrame[3]);

  /// Calculate Patient isocenter to FixedIsocenter translate
  /// \param translatePatientFrame - translation of the Patient frame
  /// \return true if success, false otherwise
  bool GetPatientIsocenterToFixedIsocenterTranslate(vtkMRMLIhepStandGeometryNode* parameterNode,
    double translatePatientFrame[3]);

  /// Calculate DRRBeam to FixedBeam trasform
  /// \param patientBeamNode - DRR beam transform in patient (RAS)
  /// \param fixedBeamNode - Fixed ion beam transform in RAS
  /// \param transform - resulted transform
  /// \return true if success, false otherwise
  bool GetPatientBeamToFixedBeamTransform(vtkMRMLIhepStandGeometryNode* parameterNode,
    vtkMRMLRTBeamNode* patientBeamNode, vtkMRMLRTFixedIonBeamNode* fixedBeamNode,
    vtkTransform* transform);

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

  /// Update transform for fiducial markups nodes
  vtkMRMLLinearTransformNode* UpdateTableOriginMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  vtkMRMLLinearTransformNode* UpdateTableMiddleMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  vtkMRMLLinearTransformNode* UpdateTableMirrorMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode);

  vtkMRMLLinearTransformNode* UpdateTableTopPlaneTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  
  /// Update transform for line markups node
  vtkMRMLLinearTransformNode* UpdateFixedReferenceMarkupsTransform(vtkMRMLIhepStandGeometryNode* parameterNode);
  /// Update FixedReference 3D view camera position
  void UpdateFixedReferenceCamera(vtkMRMLIhepStandGeometryNode* parameterNode, vtkMRMLCameraNode* cameraNode = nullptr);

  vtkSlicerIhepStandGeometryLogic(const vtkSlicerIhepStandGeometryLogic&) = delete;
  void operator=(const vtkSlicerIhepStandGeometryLogic&) = delete;
};

#endif
