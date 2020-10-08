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

// .NAME vtkSlicerRtImageLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerRtImageLogic_h
#define __vtkSlicerRtImageLogic_h

// Slicer includes
#include <vtkSlicerModuleLogic.h>

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerRtImageModuleLogicExport.h"

class vtkMRMLVolumeNode;
class vtkMRMLScalarVolumeNode;

class vtkMRMLRTImageNode;
class vtkMRMLRTBeamNode;
class vtkMRMLMarkupsPlaneNode;
class vtkMRMLMarkupsClosedCurveNode;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLMarkupsLineNode;

class vtkMRMLLinearTransformNode;

class vtkSlicerBeamsModuleLogic;
class vtkSlicerPlanarImageModuleLogic;
class vtkSlicerCLIModuleLogic;

/// \ingroup Slicer_QtModules_RtImage
class VTK_SLICER_RTIMAGE_MODULE_LOGIC_EXPORT vtkSlicerRtImageLogic :
  public vtkSlicerModuleLogic
{
public:
  static const char* IMAGER_BOUNDARY_MARKUPS_NODE_NAME; // curve
  static const char* IMAGE_WINDOW_MARKUPS_NODE_NAME; // curve
  static const char* FIDUCIALS_MARKUPS_NODE_NAME; // fiducial
  static const char* NORMAL_VECTOR_MARKUPS_NODE_NAME; // line
  static const char* VUP_VECTOR_MARKUPS_NODE_NAME; // line
  static const char* RTIMAGE_TRANSFORM_NODE_NAME;

  static vtkSlicerRtImageLogic *New();
  vtkTypeMacro(vtkSlicerRtImageLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Update normal and view up vectors of RT Image
  void UpdateNormalAndVupVectors(vtkMRMLRTImageNode* parameterNode);

  /// Create markups nodes for visualization
  void CreateMarkupsNodes(vtkMRMLRTImageNode* parameterNode);
  /// Update markups nodes using parameter node data
  void UpdateMarkupsNodes(vtkMRMLRTImageNode* parameterNode);
  /// Show markups
  void ShowMarkupsNodes(bool toggled = false);

  /// Set Planar Image module logic
  void SetPlanarImageLogic(vtkSlicerPlanarImageModuleLogic* planarImageLogic);
  /// Set Plastimatch DRR CLI module logic
  void SetDRRComputationLogic(vtkSlicerCLIModuleLogic* plastimatchDrrLogic);
  /// Set Beams module logic
  void SetBeamsLogic(vtkSlicerBeamsModuleLogic* beamsLogic);

  /// Compute DRR image
  /// @param parameterNode - parameters of RTImage DRR volume
  /// @param ctInputVolume - CT volume
  bool ComputePlastimatchDRR( vtkMRMLRTImageNode* parameterNode, vtkMRMLScalarVolumeNode* ctInputVolume);

protected:
  vtkSlicerRtImageLogic();
  virtual ~vtkSlicerRtImageLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

  /// Handles events registered in the observer manager
  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

private:
  vtkSlicerRtImageLogic(const vtkSlicerRtImageLogic&); // Not implemented
  void operator=(const vtkSlicerRtImageLogic&); // Not implemented

  /// Create markups for imager normal vector 
  vtkMRMLMarkupsLineNode* CreateImagerNormal(vtkMRMLRTImageNode* node); // n
  /// Create markups for view up normal vector
  vtkMRMLMarkupsLineNode* CreateImagerVUP(vtkMRMLRTImageNode* node); // vup
  /// Create markups for imager boundary
  vtkMRMLMarkupsClosedCurveNode* CreateImagerBoundary(vtkMRMLRTImageNode* node); // Imager == Reciever == Detector
  /// Create markups for image window within imager
  vtkMRMLMarkupsClosedCurveNode* CreateImageWindow(vtkMRMLRTImageNode* node); // subwindow
  /// Create fiducial markups: (0,0), Center, etc
  vtkMRMLMarkupsFiducialNode* CreateFiducials(vtkMRMLRTImageNode* node);
  /// Setup nodes for calculated DRR image
  /// @param parameterNode - parameters of RTImage DRR volume
  /// @param drrVolumeNode - RTImage DRR volume
  bool SetupDisplayAndSubjectHierarchyNodes( vtkMRMLRTImageNode* parameterNode, vtkMRMLScalarVolumeNode* drrVolumeNode);
  /// Setup geometry for calculated DRR image
  /// @param parameterNode - parameters of RTImage DRR volume
  /// @param drrVolumeNode - RTImage DRR volume
  bool SetupGeometry( vtkMRMLRTImageNode* parameterNode, vtkMRMLScalarVolumeNode* drrVolumeNode);

  /// IEC Transformation from Gantry -> RAS (without collimator)
  vtkMRMLLinearTransformNode* UpdateRtImageTransformFromBeam(vtkMRMLRTBeamNode* node = nullptr);

  /// Planar Image logic instance
  vtkSlicerPlanarImageModuleLogic* PlanarImageLogic;
  /// Plastimatch DRR computation logic instance
  vtkSlicerCLIModuleLogic* PlastimatchDRRComputationLogic;
  /// Beams logic instance
  vtkSlicerBeamsModuleLogic* BeamsLogic;
};

#endif
