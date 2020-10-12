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

class vtkSlicerPlanarImageModuleLogic;
class vtkSlicerCLIModuleLogic;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_RTIMAGE_MODULE_LOGIC_EXPORT vtkSlicerRtImageLogic :
  public vtkSlicerModuleLogic
{
public:
  static const char* IMAGER_BOUNDARY_MARKUPS_NODE_NAME; // plane
  static const char* IMAGE_WINDOW_MARKUPS_NODE_NAME; // curve
  static const char* FIDUCIALS_MARKUPS_NODE_NAME; // fiducial
  static const char* NORMAL_VECTOR_MARKUPS_NODE_NAME; // line
  static const char* VUP_VECTOR_MARKUPS_NODE_NAME; // line

  static vtkSlicerRtImageLogic *New();
  vtkTypeMacro(vtkSlicerRtImageLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Create markups nodes for visualization
  void CreateMarkupsNodes(vtkMRMLRTImageNode* rtImageNode);
  /// Update markups nodes using parameter node data
  void UpdateMarkupsNodes(vtkMRMLRTImageNode* rtImageNode);
  void ShowMarkupsNodes( vtkMRMLRTImageNode* rtImageNode, bool toggled = false);

  /// Set Planar Image module logic
  void SetPlanarImageLogic(vtkSlicerPlanarImageModuleLogic* planarImageLogic);
  void SetDRRComputationLogic(vtkSlicerCLIModuleLogic* plastimatchDrrLogic);

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

  vtkMRMLMarkupsLineNode* CreateImagerNormal(vtkMRMLRTImageNode* node); // n
  vtkMRMLMarkupsLineNode* CreateImagerVUP(vtkMRMLRTImageNode* node); // vup
  vtkMRMLMarkupsPlaneNode* CreateImagerBoundary(vtkMRMLRTImageNode* node); // Imager == Reciever == Detector
  vtkMRMLMarkupsClosedCurveNode* CreateImageWindow(vtkMRMLRTImageNode* node); // subwindow
  vtkMRMLMarkupsFiducialNode* CreateFiducials(vtkMRMLRTImageNode* node);

  /// Planar Image logic instance
  vtkSlicerPlanarImageModuleLogic* PlanarImageLogic;
  /// Plastimatch DRR computation logic instance
  vtkSlicerCLIModuleLogic* PlastimatchDRRComputationLogic;
};

#endif
