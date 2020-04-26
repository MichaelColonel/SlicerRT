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

// .NAME vtkSlicerLoadableModuleTemplateLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerLoadableModuleTemplateLogic_h
#define __vtkSlicerLoadableModuleTemplateLogic_h

// Slicer includes
#include <vtkSlicerModuleLogic.h>

#include "vtkSlicerLoadableModuleTemplateModuleLogicExport.h"

class vtkPolyData;
class vtkMRMLMarkupsCurveNode;
class vtkMRMLRTBeamNode;
class vtkMRMLDoubleArrayNode;
class vtkDoubleArray;
class vtkMRMLTableNode;
class vtkSlicerModelsLogic;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_LOADABLEMODULETEMPLATE_MODULE_LOGIC_EXPORT vtkSlicerLoadableModuleTemplateLogic : public vtkSlicerModuleLogic {
public:

  static vtkSlicerLoadableModuleTemplateLogic *New();
  vtkTypeMacro( vtkSlicerLoadableModuleTemplateLogic, vtkSlicerModuleLogic);
  void PrintSelf( ostream& os, vtkIndent indent);

  /// Set Models logic
  void SetModelsLogic(vtkSlicerModelsLogic* modelsLogic);

  /// Calculate position curve on isocenter plane
  vtkMRMLMarkupsCurveNode* CalculatePositionCurve( 
    vtkMRMLRTBeamNode* beamNode, vtkPolyData* targetPoly);

  vtkMRMLTableNode* CalculateMultiLeafCollimatorPosition( 
    vtkMRMLMarkupsCurveNode* curveNode, 
    vtkMRMLDoubleArrayNode* mlcBoundary);

  vtkMRMLDoubleArrayNode* CreateMultiLeafCollimatorDoubleArrayNode();

  double CalculateMultiLeafCollimatorPositionArea( 
    vtkMRMLDoubleArrayNode* mlcBoundary, 
    vtkMRMLTableNode* mlcPosition);

  double CalculateCurvePolygonArea(vtkMRMLMarkupsCurveNode* curveNode);
  void SetParentForMultiLeafCollimatorPosition( vtkMRMLRTBeamNode* beamNode, vtkMRMLTableNode* mlcPositionNode);
  void CreateMultiLeafCollimatorModelPolyData(vtkMRMLRTBeamNode* beamNode);

protected:
  vtkSlicerLoadableModuleTemplateLogic();
  virtual ~vtkSlicerLoadableModuleTemplateLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);

  /// Reimplemented to delete the storage/display nodes when a displayable
  /// node is being removed.
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:
  vtkSlicerLoadableModuleTemplateLogic(const vtkSlicerLoadableModuleTemplateLogic&); // Not implemented
  void operator=(const vtkSlicerLoadableModuleTemplateLogic&); // Not implemented

  vtkMRMLTableNode* CreateMultiLeafCollimatorTableNode(const std::vector<double>& positions);
  bool CalculateCurveBoundary( vtkMRMLMarkupsCurveNode* node, double b[4]);
  void FindLeafPairRangeIndexes( double* b, vtkDoubleArray* mlcBoundaryArray, int& leafPairIndexFirst, int& leafPairIndexLast);
  bool FindLeafPairPositions( vtkMRMLMarkupsCurveNode* node, vtkMRMLDoubleArrayNode* mlcBoundaryNode, size_t leafPairIndex, 
    double& side1, double& side2, int strategy = 1, 
    double maxPositionDistance = 100., double positionStep = 0.01);

  /// Models module logic instance
  vtkSlicerModelsLogic* ModelsLogic;
};

#endif
