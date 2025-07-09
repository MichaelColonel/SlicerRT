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

// .NAME vtkSlicerTestMe2Logic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerTestMe2Logic_h
#define __vtkSlicerTestMe2Logic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
//#include <vtkMRMLTestMe2Node.h>

// STD includes
#include <cstdlib>

#include "vtkSlicerTestMe2ModuleLogicExport.h"

class vtkMRMLMarkupsNode;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLLinearTransformNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLRTBeamNode;

class vtkMRMLTestMe2Node;

//class vtkMatrix4x4;
class vtkTransform;

class VTK_SLICER_TESTME2_MODULE_LOGIC_EXPORT vtkSlicerTestMe2Logic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerTestMe2Logic *New();
  vtkTypeMacro(vtkSlicerTestMe2Logic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void createControlPoint(vtkMRMLMarkupsFiducialNode*);
  void updateFiducialTransformLink(vtkMRMLMarkupsFiducialNode*, vtkMRMLLinearTransformNode*);
  void updateTransform(vtkMRMLLinearTransformNode*, double height, double rotateXAngle);
  void showDRR(vtkMRMLScalarVolumeNode* drrNode, vtkMRMLRTBeamNode* beamNode, vtkMRMLSliceNode* sliceNode);

protected:
  vtkSlicerTestMe2Logic();
  ~vtkSlicerTestMe2Logic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

private:

  vtkSlicerTestMe2Logic(const vtkSlicerTestMe2Logic&); // Not implemented
  void operator=(const vtkSlicerTestMe2Logic&); // Not implemented
};

#endif
