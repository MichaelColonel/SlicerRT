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

// .NAME vtkSlicerImagePositioningLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerImagePositioningLogic_h
#define __vtkSlicerImagePositioningLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
class vtkMRMLImagePositioningNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLLinearTransformNode;

// STD includes
#include <cstdlib>

#include "vtkSlicerImagePositioningModuleLogicExport.h"


class VTK_SLICER_IMAGEPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerImagePositioningLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerImagePositioningLogic *New();
  vtkTypeMacro(vtkSlicerImagePositioningLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLImagePositioningNode* GetOrCreateImagePositioningNode();
 // void SetDrrNode(vtkMRMLScalarVolumeNode* drrNode);
  void SetXrayNode(vtkMRMLScalarVolumeNode* xrayNode);
  void SetActiveOrientation(int orientation);
  vtkMRMLLinearTransformNode* GetActiveXrayTransformNode();
  vtkMRMLLinearTransformNode* GetActiveXrayScaleNode();
  void ApplyXray2DTransform(double horizontalOffsetMm, double verticalOffsetMm, double rotationDeg, int orientation);
  /// Align Xray along slice normal to match DRR plane and scale it
  /// by sourceToIsocenterMm/sourceToImagerMm.
  void AlignAndScaleXrayToDrr(double sourceToImagerMm);
  vtkMRMLScalarVolumeNode* GetOrCreateImageWithGradientFilter(vtkMRMLScalarVolumeNode* inputVolume);

protected:
  vtkSlicerImagePositioningLogic();
  ~vtkSlicerImagePositioningLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
private:

  vtkSlicerImagePositioningLogic(const vtkSlicerImagePositioningLogic&); // Not implemented
  void operator=(const vtkSlicerImagePositioningLogic&); // Not implemented
};

#endif
