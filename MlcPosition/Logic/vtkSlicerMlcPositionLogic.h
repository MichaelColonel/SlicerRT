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

// .NAME vtkSlicerMlcPositionLogic - slicer logic class for MLC Position Calculation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerMlcPositionLogic_h
#define __vtkSlicerMlcPositionLogic_h

// Slicer includes
#include <vtkSlicerModuleLogic.h>

#include "vtkSlicerMlcPositionModuleLogicExport.h"

class vtkPolyData;
class vtkMRMLMarkupsCurveNode;
class vtkMRMLRTBeamNode;
class vtkMRMLTableNode;
class vtkTable;
class vtkAlgorithmOutput;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_MLCPOSITION_MODULE_LOGIC_EXPORT vtkSlicerMlcPositionLogic : public vtkSlicerModuleLogic {
public:

  static vtkSlicerMlcPositionLogic *New();
  vtkTypeMacro( vtkSlicerMlcPositionLogic, vtkSlicerModuleLogic);
  void PrintSelf( ostream& os, vtkIndent indent);

  vtkMRMLTableNode* CreateMultiLeafCollimatorTableNodeBoundaryData();

  /// Calculate convex hull curve on isocenter plane for MLC position computation
  vtkMRMLMarkupsCurveNode* CalculatePositionConvexHullCurve( vtkMRMLRTBeamNode* beamNode, vtkPolyData* targetPoly);

  bool CalculateMultiLeafCollimatorPosition( vtkMRMLTableNode* mlcTableNode, vtkMRMLMarkupsCurveNode* curveNode);
  bool CalculateMultiLeafCollimatorPosition( vtkMRMLRTBeamNode* beamNode, 
    vtkMRMLTableNode* mlcTableNode, vtkPolyData* targetPoly);

//  bool TransformMultiLeafCollimatorCurveToWorld( vtkMRMLRTBeamNode* beamNode, vtkMRMLMarkupsCurveNode* curveNode);

  double CalculateMultiLeafCollimatorPositionArea(vtkMRMLRTBeamNode* beamNode);
  double CalculateCurvePolygonArea(vtkMRMLMarkupsCurveNode* curveNode);

  void SetParentForMultiLeafCollimatorTableNode(vtkMRMLRTBeamNode* beamNode);
  void SetParentForMultiLeafCollimatorCurve( vtkMRMLRTBeamNode* beamNode, vtkMRMLMarkupsCurveNode* curveNode);

//  void CreateMultiLeafCollimatorModelPolyData(vtkMRMLRTBeamNode* beamNode);

protected:
  vtkSlicerMlcPositionLogic();
  virtual ~vtkSlicerMlcPositionLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);

  /// Reimplemented to delete the storage/display nodes when a displayable
  /// node is being removed.
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:
  vtkSlicerMlcPositionLogic(const vtkSlicerMlcPositionLogic&); // Not implemented
  void operator=(const vtkSlicerMlcPositionLogic&); // Not implemented

  /// @param curveBound (xmin, xmax, ymin, ymax)
  bool CalculateCurveBoundary( vtkMRMLMarkupsCurveNode* node, double* curveBound);
  /// @param curveBound (xmin, xmax, ymin, ymax)
  void FindLeafPairRangeIndexes( double* curveBound, vtkTable* mlcTable, 
    int& leafPairIndexFirst, int& leafPairIndexLast);

  /// @param curveBound (xmin, xmax, ymin, ymax)
  void FindLeafPairRangeIndexes( vtkMRMLRTBeamNode* beamNode, vtkMRMLTableNode* mlcTableNode, 
    int& leafPairIndexFirst, int& leafPairIndexLast);

  /// @param mlcTableNode is used only to get leaf pair boundary data
  bool FindLeafPairPositions( vtkMRMLMarkupsCurveNode* node,
    vtkMRMLTableNode* mlcTableNode, size_t leafPairIndex, 
    double& side1, double& side2, int strategy = 1, 
    double maxPositionDistance = 100., double positionStep = 0.01);

  bool FindLeafAndTargetCollision( vtkMRMLRTBeamNode* beamNode, vtkAlgorithmOutput* leaf, vtkAlgorithmOutput* target, 
    double& sidePos, int sideType = 1, bool mlcType = true, 
    double maxPositionDistance = 100., double positionStep = 0.01);

};

#endif
