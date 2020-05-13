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

// LoadableModuleTemplate Logic includes
#include "vtkSlicerMlcPositionLogic.h"

// Slicer Models includes
#include <vtkSlicerModelsLogic.h>

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLMarkupsClosedCurveNode.h> // MLC curve
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLTableNode.h>

#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelDisplayNode.h>

// Subject Hierarchy includes
#include <vtkMRMLSubjectHierarchyConstants.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// SlicerRT MRML includes
#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSegmentation.h>
#include <vtkOrientedImageData.h>
#include <vtkImageData.h>
#include <vtkDoubleArray.h>
#include <vtkTable.h>
#include <vtkPlane.h>
#include <vtkCubeSource.h>

#include <vtkTransformPolyDataFilter.h>

#include <vtkCellLocator.h>
#include <vtkModifiedBSPTree.h>
#include <vtkOBBTree.h>

#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <vtkIdList.h>
#include <vtkPointsProjectedHull.h>

#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkPointData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkDataArray.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h> // cross, dot vector operations

// SlicerRtCommon includes
#include <vtkSlicerRtCommon.h>
#include <vtkCollisionDetectionFilter.h>

// STD includes
#include <array>
#include <algorithm>

namespace
{

const char* MLCX_BoundaryAndPosition = "MLCX_BoundaryAndPosition";
//const char* MLCY_BoundaryAndPosition = "MLCY_BoundaryAndPosition";

/*
const double boundary[leaves + 1] = {
  -80.0 - 2.5,
  -75.0 - 2.5,
  -70.0 - 2.5,
  -65.0 - 2.5,
  -60.0 - 2.5,
  -55.0 - 2.5,
  -50.0 - 2.5,
  -45.0 - 2.5,
  -40.0 - 2.5,
  -35.0 - 2.5,
  -30.0 - 2.5,
  -25.0 - 2.5,
  -20.0 - 2.5,
  -15.0 - 2.5,
  -10.0 - 2.5,
   -5.0 - 2.5,
    0.0 - 2.5,
   +5.0 - 2.5,
  +10.0 - 2.5,
  +15.0 - 2.5,
  +20.0 - 2.5,
  +25.0 - 2.5,
  +30.0 - 2.5,
  +35.0 - 2.5,
  +40.0 - 2.5,
  +45.0 - 2.5,
  +50.0 - 2.5,
  +55.0 - 2.5,
  +60.0 - 2.5,
  +65.0 - 2.5,
  +70.0 - 2.5,
  +75.0 - 2.5,
  +86.5 - 2.5
};
*/

const size_t leaves = 32;
const double boundary[leaves + 1] = {
  -80.0,
  -75.0,
  -70.0,
  -65.0,
  -60.0,
  -55.0,
  -50.0,
  -45.0,
  -40.0,
  -35.0,
  -30.0,
  -25.0,
  -20.0,
  -15.0,
  -10.0,
   -5.0,
    0.0,
   +5.0,
  +10.0,
  +15.0,
  +20.0,
  +25.0,
  +30.0,
  +35.0,
  +40.0,
  +45.0,
  +50.0,
  +55.0,
  +60.0,
  +65.0,
  +70.0,
  +75.0,
  +86.5
};

const size_t leaves1 = 16; 
//const size_t leaves = 16; 
const double boundary_l1[leaves1 + 1] = {
//const double boundary[leaves + 1] = {
  -75.0,
  -65.0,
  -55.0,
  -45.0,
  -35.0,
  -25.0,
  -15.0,
   -5.0,
   +5.0,
  +15.0,
  +25.0,
  +35.0,
  +45.0,
  +55.0,
  +65.0,
  +75.0,
  +86.5
};

const double boundary_l2[leaves1 + 1] = {
//const double boundary[leaves + 1] = {
  -80.0,
  -70.0,
  -60.0,
  -50.0,
  -40.0,
  -30.0,
  -20.0,
  -10.0,
    0.0,
  +10.0,
  +20.0,
  +30.0,
  +40.0,
  +50.0,
  +60.0,
  +70.0,
  +81.5
};

} // namespace

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMlcPositionLogic);
//vtkCxxSetObjectMacro(vtkSlicerLoadableModuleTemplateLogic, ModelsLogic, vtkSlicerModelsLogic);

//----------------------------------------------------------------------------
vtkSlicerMlcPositionLogic::vtkSlicerMlcPositionLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerMlcPositionLogic::~vtkSlicerMlcPositionLogic()
{
}

//----------------------------------------------------------------------------
void
vtkSlicerMlcPositionLogic::PrintSelf( ostream& os, vtkIndent indent)
{
  os << indent << "vtkSlicerModuleLogic:     " << this->GetClassName() << "\n";
  this->Superclass::PrintSelf( os, indent);
}

//---------------------------------------------------------------------------
void
vtkSlicerMlcPositionLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  this->SetAndObserveMRMLSceneEventsInternal( newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void
vtkSlicerMlcPositionLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void
vtkSlicerMlcPositionLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void
vtkSlicerMlcPositionLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//-----------------------------------------------------------------------------
/*
vtkMRMLMarkupsCurveNode*
vtkSlicerLoadableModuleTemplateLogic::CalculatePositionCurve( 
  vtkMRMLRTBeamNode* beamNode, vtkPolyData* targetPoly)
{
  if (!beamNode)
  {
    vtkErrorMacro("CalculatePositionCurve: Beam node is invalid");
    return nullptr;
  }

  vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
  vtkTransform* beamTransform = nullptr;
  if (beamTransformNode)
  {
    beamTransform = vtkTransform::SafeDownCast(beamTransformNode->GetTransformToParent());
  }
  else
  {
    vtkErrorMacro("CalculatePositionCurve: Beam transform node is invalid");
    return nullptr;
  }

  vtkNew<vtkMatrix4x4> inverseMatrix;
  if (beamTransform)
  {
    beamTransform->GetInverse(inverseMatrix);
  }
  else
  {
    vtkErrorMacro("CalculatePositionCurve: Matrix transform is invalid");
    return nullptr;
  }

  // Create markups node (subject hierarchy node is created automatically)
  vtkNew<vtkMRMLMarkupsCurveNode> curveNode;
  curveNode->SetCurveTypeToLinear();
  curveNode->SetName("ProjectionCurve");

  this->GetMRMLScene()->AddNode(curveNode);

  // external points for MLC opening calculation, projected on the isocenter plane
  vtkNew<vtkPointsProjectedHull> points; 

  for( vtkIdType i = 0; i < targetPoly->GetNumberOfPoints(); i++)
  {
    double projectedPoint[4] = {}; // projected point in plane coordinates
    double M[4] = { 0., 0., 0., 1. }; // target region point

    targetPoly->GetPoint( i, M);

    inverseMatrix->MultiplyPoint( M, projectedPoint);

    // projection on XY plane of BEAM LIMITING DEVICE frame
    points->InsertPoint( i, projectedPoint[0], projectedPoint[1], 0.0);
  }

  int zSize = points->GetSizeCCWHullZ();
  if (zSize > 0)
  {
    double* pts = new double[zSize * 2];
    points->GetCCWHullZ( pts, zSize);

//    inverseMatrix->Invert();
    for( int i = 0; i < zSize; i++)
    {
      double xval = pts[2 * i];
      double yval = pts[2 * i + 1];
//      double worldPoint[4]; // projected hull point in world coordinates
//      double projectedPoint[4] = { xval, yval, 0.0, 1. }; // projected hull point on XY plane of BEAM LIMITING DEVICE frame
//      inverseMatrix->MultiplyPoint( projectedPoint, worldPoint);

//      vtkVector3d point( worldPoint[0], worldPoint[1], worldPoint[2]);
      vtkVector3d point( xval, yval, 0.0);
      curveNode->AddControlPoint(point);
    }

    delete pts;
//    curveNode->ResampleCurveWorld(5.);
    return curveNode.GetPointer();
  }
  else
  {
    this->GetMRMLScene()->RemoveNode(curveNode);
    curveNode->Delete();
    return nullptr;
  }
}
*/

vtkMRMLMarkupsCurveNode*
vtkSlicerMlcPositionLogic::CalculatePositionConvexHullCurve( 
  vtkMRMLRTBeamNode* beamNode, vtkPolyData* targetPoly, bool parallelBeam)
{
  if (!beamNode)
  {
    vtkErrorMacro("CalculatePositionCurve: Beam node is invalid");
    return nullptr;
  }

  vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
  vtkTransform* beamTransform = nullptr;
  if (beamTransformNode)
  {
    beamTransform = vtkTransform::SafeDownCast(beamTransformNode->GetTransformToParent());
  }
  else
  {
    vtkErrorMacro("CalculatePositionCurve: Beam transform node is invalid");
    return nullptr;
  }

  vtkNew<vtkMatrix4x4> inverseMatrix;
  if (beamTransform)
  {
    beamTransform->GetInverse(inverseMatrix);
  }
  else
  {
    vtkErrorMacro("CalculatePositionCurve: Matrix transform is invalid");
    return nullptr;
  }

  // Create markups node (subject hierarchy node is created automatically)
  vtkNew<vtkMRMLMarkupsClosedCurveNode> curveNode;
  curveNode->SetCurveTypeToLinear();
  curveNode->SetName("ProjectionCurve");

  this->GetMRMLScene()->AddNode(curveNode);

  // external points for MLC opening calculation, projected on the isocenter plane
  vtkNew<vtkPointsProjectedHull> points; 

  if (parallelBeam)
  {
    for( vtkIdType i = 0; i < targetPoly->GetNumberOfPoints(); i++)
    {
      double projectedPoint[4] = {}; // projected point in plane coordinates
      double M[4] = { 0., 0., 0., 1. }; // target region point

      targetPoly->GetPoint( i, M);

      inverseMatrix->MultiplyPoint( M, projectedPoint);

      // projection on XY plane of BEAM LIMITING DEVICE frame
      points->InsertPoint( i, projectedPoint[0], projectedPoint[1], 0.0);
    }
  }
  else
  {
    vtkNew<vtkPlane> projectionPlane;
    projectionPlane->SetOrigin( 0., 0., 0.);
    projectionPlane->SetNormal( 0., 0., 1.);
    for( vtkIdType i = 0; i < targetPoly->GetNumberOfPoints(); i++)
    {
      double beamFramePoint[4] = {}; // target region point in beam frame coordinates
      double worldPoint[4] = { 0., 0., 0., 1. }; // target region point in world
      double t = 0.0001;
      double projectedPoint[3]; // target region point projected on plane
      targetPoly->GetPoint( i, worldPoint);

      inverseMatrix->MultiplyPoint( worldPoint, beamFramePoint);

      double sourcePoint[3] = { 0., 0., -1. * beamNode->GetSAD() };
      double targetPoint[3] = { beamFramePoint[0], beamFramePoint[1], projectedPoint[2] };

      if (projectionPlane->IntersectWithLine( sourcePoint, targetPoint, t, projectedPoint))
      {
        // projection on XY plane of BEAM LIMITING DEVICE frame
        points->InsertPoint( i, projectedPoint[0], projectedPoint[1], 0.0);
      }
    }
  }

  int zSize = points->GetSizeCCWHullZ();
  if (zSize >= 3)
  {
    double* pts = new double[zSize * 2];
    points->GetCCWHullZ( pts, zSize);

    for( int i = 0; i < zSize; i++)
    {
      double xval = pts[2 * i];
      double yval = pts[2 * i + 1];

      vtkVector3d point( xval, yval, 0.0);
      curveNode->AddControlPoint(point);
    }

    delete pts;
    return curveNode.GetPointer();
  }
  else
  {
    this->GetMRMLScene()->RemoveNode(curveNode);
    curveNode->Delete();
    return nullptr;
  }
}

//---------------------------------------------------------------------------
vtkMRMLTableNode*
vtkSlicerMlcPositionLogic::CreateMultiLeafCollimatorTableNodeBoundaryData()
{
  vtkNew<vtkMRMLTableNode> tableNode;
  this->GetMRMLScene()->AddNode(tableNode);
  tableNode->SetName(MLCX_BoundaryAndPosition);

  vtkTable* table = tableNode->GetTable();
  if (table)
  {
    // Column 0; Leaf pair boundary values
    vtkNew<vtkDoubleArray> boundaryArray;
    boundaryArray->SetName("Boundary");
    table->AddColumn(boundaryArray);

    // Column 1; Leaf positions on the side "1"
    vtkNew<vtkDoubleArray> pos1Array;
    pos1Array->SetName("1");
    table->AddColumn(pos1Array);

    // Column 2; Leaf positions on the side "2"
    vtkNew<vtkDoubleArray> pos2Array;
    pos2Array->SetName("2");
    table->AddColumn(pos2Array);

    vtkIdType size = leaves;
    table->SetNumberOfRows(size + 1);

    for ( vtkIdType row = 0; row < size + 1; ++row)
    {
      table->SetValue( row, 0, boundary[row]);
    }
    for ( vtkIdType row = 0; row < size; ++row)
    {
      table->SetValue( row, 1, 0.0);
      table->SetValue( row, 2, 0.0);
    }
    table->SetValue( size, 1, 0.); // side "1" set last unused value to zero
    table->SetValue( size, 2, 0.); // side "2" set last unused value to zero

    tableNode->SetUseColumnNameAsColumnHeader(true);
    tableNode->SetColumnDescription( "Boundary", "Leaf pair boundary");
    tableNode->SetColumnDescription( "1", "Leaf position on the side \"1\"");
    tableNode->SetColumnDescription( "2", "Leaf position on the side \"2\"");
    return tableNode;
  }
  else
  {
    vtkErrorMacro("CreateMultiLeafCollimatorBoundaryTableNode: Unable to create vtkTable to fill MLC data");
  }
  return nullptr;
}

//---------------------------------------------------------------------------
bool
vtkSlicerMlcPositionLogic::CalculateMultiLeafCollimatorPosition( vtkMRMLTableNode* mlcTableNode, 
  vtkMRMLMarkupsCurveNode* curveNode)
{
  if (!mlcTableNode)
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorPosition: invalid MLC table node");
    return false;
  }

  size_t nofLeafPairs = 0;
  if (mlcTableNode)
  {
    nofLeafPairs = mlcTableNode->GetNumberOfRows() - 1;
    if (nofLeafPairs <= 0)
    {
      nofLeafPairs = 0;
    }
  }

  double curveBounds[4];
  if (!nofLeafPairs || !CalculateCurveBoundary( curveNode, curveBounds))
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorPosition: Number of leaf pairs is zero or unable to calculate curve boundary");
    return false;
  }

  vtkTable* mlcTable = mlcTableNode->GetTable();

  int leafPairStart, leafPairEnd;
  FindLeafPairRangeIndexes( curveBounds, mlcTable, leafPairStart, leafPairEnd);
  if (leafPairStart == -1 || leafPairEnd == -1)
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorPosition: Unable to find leaves range");
    return false;
  }

  if (leafPairStart > 0)
  {
    leafPairStart -= 1;
  }
  if (leafPairEnd < int(nofLeafPairs - 1))
  {
    leafPairEnd += 1;
  }

//  std::vector< double > positions(2 * nofLeafPairs);

  for ( int leafPairIndex = leafPairStart; leafPairIndex <= leafPairEnd; ++leafPairIndex)
  {
    double side1, side2;
    size_t leafIndex = leafPairIndex;

    if (mlcTable && FindLeafPairPositions( curveNode, mlcTableNode, leafIndex, side1, side2))
    {
      // positions found
      mlcTable->SetValue( leafIndex, 1, side1);
      mlcTable->SetValue( leafIndex, 2, side2);
//      positions[leafIndex] = side1;
//      positions[leafIndex + nofLeafPairs] = side2;
    }
  }

  return true;
}

//---------------------------------------------------------------------------
void
vtkSlicerMlcPositionLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
bool
vtkSlicerMlcPositionLogic::CalculateCurveBoundary( vtkMRMLMarkupsCurveNode* curveNode, double* curveBound)
{
  if (curveNode)
  {
//    const std::vector< vtkMRMLMarkupsNode::ControlPoint* >* controlPoints = curveNode->GetControlPoints();
    const auto controlPoints = curveNode->GetControlPoints();
    if (controlPoints->size() < 3)
    {
      vtkErrorMacro("CalculateCurveBoundary: Not enough points (less than 3) in closed curve node");
      return false;
    }
    double xmin = controlPoints->front()->Position[0];
    double xmax = controlPoints->front()->Position[0];
    double ymin = controlPoints->front()->Position[1];
    double ymax = controlPoints->front()->Position[1];
    for ( auto it = controlPoints->begin(); it != controlPoints->end(); ++it)
    {
      vtkMRMLMarkupsNode::ControlPoint* point = *it;
      xmin = std::min( xmin, point->Position[0]);
      xmax = std::max( xmax, point->Position[0]);
      ymin = std::min( ymin, point->Position[1]);
      ymax = std::max( ymax, point->Position[1]);
    }
    curveBound[0] = xmin;
    curveBound[1] = xmax;
    curveBound[2] = ymin;
    curveBound[3] = ymax;
    return true;
  }
  vtkErrorMacro("CalculateCurveBoundary: Invalid closed curve node");
  return false;
}

//---------------------------------------------------------------------------
void
vtkSlicerMlcPositionLogic::FindLeafPairRangeIndexes( double* curveBound, 
  vtkTable* mlcTable, int& leafIndexFirst, int& leafIndexLast)
{
  leafIndexFirst = -1;
  leafIndexLast = -1;
  int nofLeafPairs = mlcTable->GetNumberOfRows() - 1;

  for ( int leafPair = 0; leafPair < nofLeafPairs; ++leafPair)
  {
    double boundBegin = mlcTable->GetValue( leafPair, 0).ToDouble();
    double boundEnd = mlcTable->GetValue( leafPair + 1, 0).ToDouble();

    if (curveBound[0] >= boundBegin && curveBound[0] <= boundEnd)
    {
      leafIndexFirst = leafPair;
    }
    if (curveBound[1] >= boundBegin && curveBound[1] <= boundEnd)
    {
      leafIndexLast = leafPair;
    }
  }
}

//---------------------------------------------------------------------------
void
vtkSlicerMlcPositionLogic::FindLeafPairRangeIndexes( 
  vtkMRMLRTBeamNode* beamNode, vtkMRMLTableNode* mlcTableNode, 
  int& leafIndexFirst, int& leafIndexLast)
{
  leafIndexFirst = -1;
  leafIndexLast = -1;

  if (!beamNode)
  {
    vtkErrorMacro("FindLeafPairRangeIndexes: invalid beam node");
    return;
  }

  if (!mlcTableNode)
  {
    vtkErrorMacro("FindLeafPairRangeIndexes: MLC table node is invalid");
    return;
  }

  const char* mlcName = mlcTableNode->GetName();
  bool typeMLCY = !strncmp( "MLCY", mlcName, strlen("MLCY"));
  bool typeMLCX = !strncmp( "MLCX", mlcName, strlen("MLCX"));
  if (typeMLCY && !typeMLCX)
  {
    typeMLCX = false;
  }

  vtkTable* mlcTable = mlcTableNode->GetTable();

  int nofLeafPairs = mlcTable->GetNumberOfRows() - 1;

  double jawBegin = beamNode->GetY1Jaw();
  double jawEnd = beamNode->GetY2Jaw();
  if (typeMLCX) // MLCX
  {
    jawBegin = beamNode->GetY1Jaw();
    jawEnd = beamNode->GetY2Jaw();
  }
  else // MLCY
  {
    jawBegin = beamNode->GetX1Jaw();
    jawEnd = beamNode->GetX2Jaw();
  }

  for ( int leafPair = 0; leafPair < nofLeafPairs; ++leafPair)
  {
    double boundBegin = mlcTable->GetValue( leafPair, 0).ToDouble();
    double boundEnd = mlcTable->GetValue( leafPair + 1, 0).ToDouble();

    if (jawBegin >= boundBegin && jawBegin <= boundEnd)
    {
      leafIndexFirst = leafPair;
    }
    if (jawEnd >= boundBegin && jawEnd <= boundEnd)
    {
      leafIndexLast = leafPair;
    }
  }
}

//---------------------------------------------------------------------------
bool
vtkSlicerMlcPositionLogic::FindLeafPairPositions( 
  vtkMRMLMarkupsCurveNode* curveNode, vtkMRMLTableNode* mlcTableNode, 
  size_t leafPairIndex, 
  double& side1, double& side2, 
  int vtkNotUsed(strategy), 
  double maxPositionDistance, 
  double positionStep)
{
  if (!mlcTableNode)
  {
    vtkErrorMacro("FindLeafPairPositions: MLC table node is invalid");
    return false;
  }

  const char* mlcName = mlcTableNode->GetName();
  bool typeMLCY = !strncmp( "MLCY", mlcName, strlen("MLCY"));
  bool typeMLCX = !strncmp( "MLCX", mlcName, strlen("MLCX"));

  vtkTable* mlcTable = mlcTableNode->GetTable();

  double leafStart = mlcTable->GetValue( leafPairIndex, 0).ToDouble();
  double leafEnd = mlcTable->GetValue( leafPairIndex + 1, 0).ToDouble();

  vtkPolyData* curvePoly = curveNode->GetCurveWorld();
  if (!curvePoly)
  {
    vtkErrorMacro("FindLeafPairPositions: Curve polydata is invalid");
    return false;
  }

  // Build locator
  vtkNew<vtkCellLocator> cellLocator;
//  vtkNew<vtkModifiedBSPTree> cellLocator;
  cellLocator->SetDataSet(curvePoly);
  cellLocator->BuildLocator();

  // intersection with side1
  bool side1Flag = false;
  double pStart[3] = { };
  double pEnd[3] = { };

  if (typeMLCX)
  {
   pStart[1] = leafStart;
   pEnd[1] = leafEnd;
  }
  else if (typeMLCY)
  {
   pStart[0] = leafStart;
   pEnd[0] = leafEnd;
  }

  int subId;
  double t, xyz[3], pcoords[3];
  for ( double c = -1. * maxPositionDistance; c <= maxPositionDistance; c += positionStep)
  {
    if (typeMLCX)
    {
      pStart[0] = c;
      pEnd[0] = c;
    }
    else if (typeMLCY)
    {
      pStart[1] = c;
      pEnd[1] = c;
    }

    if (cellLocator->IntersectWithLine( pStart, pEnd, 0.0001, t, xyz, pcoords, subId))
    {
      side1Flag = true;
      // xyz values
      if (typeMLCX)
      {
        side1 = xyz[0];
      }
      else if (typeMLCY)
      {
        side1 = xyz[1];
      }
      break;
    }
  }

  // intersection with side2
  bool side2Flag = false;
  for ( double c = maxPositionDistance; c >= -1. * maxPositionDistance; c -= positionStep)
  {
    if (typeMLCX)
    {
      pStart[0] = c;
      pEnd[0] = c;
    }
    else if (typeMLCY)
    {
      pStart[1] = c;
      pEnd[1] = c;
    }

    if (cellLocator->IntersectWithLine( pStart, pEnd, 0.0001, t, xyz, pcoords, subId))
    {
      side2Flag = true;
      // xyz values
      if (typeMLCX)
      {
        side2 = xyz[0];
      }
      else if (typeMLCY)
      {
        side2 = xyz[1];
      }
      break;
    }
  }
  return (side1Flag && side2Flag);
}

//---------------------------------------------------------------------------
double
vtkSlicerMlcPositionLogic::CalculateMultiLeafCollimatorPositionArea( 
  vtkMRMLRTBeamNode* beamNode)
{
  if (!beamNode)
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorPositionArea: invalid beam node");
    return -1.;
  }

  vtkMRMLTableNode* mlcTableNode = beamNode->GetMultiLeafCollimatorTableNode();

  int nofLeafPairs = 0;
  if (mlcTableNode)
  {
    nofLeafPairs = mlcTableNode->GetNumberOfRows() - 1;
  }

  double area = -1;
  // MLC position data
  if ((nofLeafPairs > 0) && (mlcTableNode->GetNumberOfColumns() == 3))
  {
    area = 0.;
    for ( size_t leafPair = 0; leafPair < size_t(nofLeafPairs); ++leafPair)
    {
      vtkTable* mlcTable = mlcTableNode->GetTable();

      double boundBegin = mlcTable->GetValue(leafPair, 0).ToDouble();
      double boundEnd = mlcTable->GetValue(leafPair + 1, 0).ToDouble();

      double side1 = mlcTable->GetValue( leafPair, 1).ToDouble();
      double side2 = mlcTable->GetValue( leafPair, 2).ToDouble();
//      area += (fabs(side1) + fabs(side2)) * fabs(boundEnd - boundBegin);
      area += fabs(side2 - side1) * fabs(boundEnd - boundBegin);
    }
  }

  return area;
}

//---------------------------------------------------------------------------
double
vtkSlicerMlcPositionLogic::CalculateCurvePolygonArea(vtkMRMLMarkupsCurveNode* curveNode)
{
  if (!curveNode)
  {
    vtkErrorMacro("CalculateCurvePolygonArea: invalid curve node");
    return -1.;
  }

//  const std::vector< vtkMRMLMarkupsNode::ControlPoint* >* controlPoints = curveNode->GetControlPoints();
  const auto controlPoints = curveNode->GetControlPoints();

  // Setup points
  vtkNew<vtkPoints> points;

  for ( auto it = controlPoints->begin(); it != controlPoints->end(); ++it)
  {
    vtkMRMLMarkupsNode::ControlPoint* point = *it;
    double x = point->Position[0];
    double y = point->Position[1];
    points->InsertNextPoint( x, y, 0.0);
  }

  double normal[3];
  return vtkPolygon::ComputeArea( points, controlPoints->size(), nullptr, normal);
}

//---------------------------------------------------------------------------
void
vtkSlicerMlcPositionLogic::SetParentForMultiLeafCollimatorTableNode(vtkMRMLRTBeamNode* beamNode)
{
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->GetMRMLScene());
  if (shNode)
  {
    vtkIdType beamShId = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    vtkIdType mlcTableShId = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;

    if (beamNode)
    {
      // put observed mlc data under beam and ion beam node parent
      beamShId = shNode->GetItemByDataNode(beamNode);
      vtkMRMLTableNode* mlcTableNode = beamNode->GetMultiLeafCollimatorTableNode();
      if (mlcTableNode)
      {
        mlcTableShId = shNode->GetItemByDataNode(mlcTableNode);
      }
      if (beamShId != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID && 
        mlcTableShId != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
        shNode->SetItemParent( mlcTableShId, beamShId);
      }
    }
  }
}

//---------------------------------------------------------------------------
/*
void
vtkSlicerMlcPositionLogic::CreateMultiLeafCollimatorModelPolyData(vtkMRMLRTBeamNode* beamNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateMultiLeafCollimatorModelPolyData: Invalid MRML scene");
    return;
  }

  // Add MLC to beam or ion beam
  if (!beamNode)
  {
    vtkErrorMacro("CreateMultiLeafCollimatorModelPolyData: Invalid beam node");
    return;
  }

  if (!this->ModelsLogic)
  {
    vtkErrorMacro("CreateMultiLeafCollimatorModelPolyData: Invalid models logic");
    return;
  }


  vtkMRMLTableNode* mlcPositionNode = nullptr;

  vtkIdType nofLeafPairs = 0;

  // MLC boundary data
  vtkMRMLDoubleArrayNode* mlcBoundaryNode = beamNode->GetMLCBoundaryDoubleArrayNode();
  vtkDoubleArray* mlcBoundaryArray = nullptr;
  if (mlcBoundaryNode)
  {
    mlcBoundaryArray = mlcBoundaryNode->GetArray();
    nofLeafPairs = mlcBoundaryArray ? (mlcBoundaryArray->GetNumberOfTuples() - 1) : 0;
  }

  // MLC position data
  if (nofLeafPairs)
  {
    mlcPositionNode = beamNode->GetMLCPositionTableNode();
    if (mlcPositionNode && (mlcPositionNode->GetNumberOfRows() == nofLeafPairs))
    {
      vtkDebugMacro("CreateMultiLeafCollimatorModelPolyData: Valid MLC nodes, number of leaf pairs: " << nofLeafPairs);
    }
    else
    {
      vtkErrorMacro("CreateMultiLeafCollimatorModelPolyData: Invalid MLC nodes, or " \
        "number of MLC boundaries and positions are different");
      mlcPositionNode = nullptr; // draw beam polydata without MLC
      return;
    }
  }
  else
  {
    vtkErrorMacro("CreateMultiLeafCollimatorModelPolyData: nofLeafPairs == 0");
    return;
  }

  // Create a models logic for convenient loading of components
  vtkNew<vtkSlicerModelsLogic> modelsLogic;
  modelsLogic->SetMRMLScene(scene);

  vtkPolyData* mlcModel = beamNode->CreateMultiLeafCollimatorModelPolyData();
*/
/*
  // Create model hierarchy so that the treatment machine can be shown/hidden easily
  std::string rootModelHierarchyNodeName = std::string("IHEP") + std::string("_Components");
  vtkSmartPointer<vtkMRMLModelHierarchyNode> rootModelHierarchyNode = vtkMRMLModelHierarchyNode::SafeDownCast(
    scene->GetSingletonNode(rootModelHierarchyNodeName.c_str(), "vtkMRMLModelHierarchyNode") );
  if (!rootModelHierarchyNode)
  {
    rootModelHierarchyNode = vtkSmartPointer<vtkMRMLModelHierarchyNode>::New();
    scene->AddNode(rootModelHierarchyNode);
    rootModelHierarchyNode->SetName(rootModelHierarchyNodeName.c_str());
    rootModelHierarchyNode->SetSingletonTag(rootModelHierarchyNodeName.c_str());
  }
  if (!rootModelHierarchyNode->GetDisplayNode())
  {
    vtkSmartPointer<vtkMRMLModelDisplayNode> rootModelHierarchyDisplayNode = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
    scene->AddNode(rootModelHierarchyDisplayNode);
    rootModelHierarchyNode->SetAndObserveDisplayNodeID( rootModelHierarchyDisplayNode->GetID() );
  }

  // Collimator - mandatory
  std::string collimatorModelSingletonTag = std::string(mlcPositionNode->GetName()) + "_Model";
  vtkMRMLModelNode* collimatorModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(collimatorModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (collimatorModelNode && !collimatorModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(collimatorModelNode);
    collimatorModelNode = nullptr;
  }
  if (!collimatorModelNode)
  {
    collimatorModelNode = modelsLogic->AddModel(mlcModel);
    if (collimatorModelNode)
    {
      collimatorModelNode->SetSingletonTag(collimatorModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> collimatorModelHierarchyNode;
      scene->AddNode(collimatorModelHierarchyNode);
      collimatorModelHierarchyNode->SetModelNodeID(collimatorModelNode->GetID());
      collimatorModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      collimatorModelHierarchyNode->HideFromEditorsOff();
    }
    else
    {
      vtkErrorMacro("CreateMultiLeafCollimatorModelPolyData: Failed to create collimator model");
    }
  }
*/
/*
  if (mlcModel)
  {
    vtkMRMLModelNode* mlcModelNode = modelsLogic->AddModel(mlcModel);
    if (mlcModelNode && mlcPositionNode)
    {
      std::string mlcModelName = std::string(mlcPositionNode->GetName()) + "_Model";
      mlcModelNode->SetName(mlcModelName.c_str());
      mlcModelNode->SetAndObserveTransformNodeID(beamNode->GetTransformNodeID());
      vtkMRMLDisplayNode* displayNode = mlcModelNode->GetDisplayNode();
      if (displayNode)
      {
        displayNode->SetColor( 1, 1, 1);
        displayNode->SetOpacity(0.5);
        displayNode->SetBackfaceCulling(0); // Disable backface culling to make the back side of the contour visible as well
        displayNode->VisibilityOff();
        displayNode->Visibility2DOn();
      }
    }
    mlcModel->Delete();
  }
}
*/
//---------------------------------------------------------------------------
void
vtkSlicerMlcPositionLogic::SetParentForMultiLeafCollimatorCurve( 
  vtkMRMLRTBeamNode* beamNode, vtkMRMLMarkupsCurveNode* curveNode)
{
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->GetMRMLScene());
  if (shNode)
  {
    vtkIdType beamShId = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    vtkIdType mlcCurveShId = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;

    if (beamNode)
    {
      // put observed mlc data under beam and ion beam node parent
      beamShId = shNode->GetItemByDataNode(beamNode);
      if (curveNode)
      {
        mlcCurveShId = shNode->GetItemByDataNode(curveNode);
      }
      if (beamShId != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID && 
        mlcCurveShId != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
        shNode->SetItemParent( mlcCurveShId, beamShId);
      }
    }
  }
}

//---------------------------------------------------------------------------
/*
bool
vtkSlicerMlcPositionLogic::TransformMultiLeafCollimatorCurveToWorld( vtkMRMLRTBeamNode* beamNode, vtkMRMLMarkupsCurveNode* curveNode)
{
  if (!beamNode)
  {
    vtkErrorMacro("TransformMultiLeafCollimatorCurveToWorld: Beam node is invalid");
    return false;
  }

  vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
  vtkTransform* beamTransform = nullptr;
  if (beamTransformNode)
  {
    beamTransform = vtkTransform::SafeDownCast(beamTransformNode->GetTransformToParent());
  }
  else
  {
    vtkErrorMacro("TransformMultiLeafCollimatorCurveToWorld: Beam transform node is invalid");
    return false;
  }

  vtkNew<vtkMatrix4x4> matrix;
  if (beamTransform)
  {
    beamTransform->GetMatrix(matrix);
  }
  else
  {
    vtkErrorMacro("TransformMultiLeafCollimatorCurveToWorld: Matrix transform is invalid");
    return false;
  }

  for ( int i = 0; i < curveNode->GetNumberOfControlPoints(); ++i)
  {
    double worldPoint[4]; // projected hull point in world coordinates
    double projectedPoint[4] = { 0., 0., 0., 1. }; // projected hull point on XY plane of BEAM LIMITING DEVICE frame
    curveNode->GetNthControlPointPositionWorld( i, projectedPoint);

    matrix->MultiplyPoint( projectedPoint, worldPoint);
    curveNode->SetNthControlPointPositionWorld( i, worldPoint[0], worldPoint[1], worldPoint[2]);
  }
  curveNode->Modified();
  return true;
}
*/


//---------------------------------------------------------------------------
bool
vtkSlicerMlcPositionLogic::CalculateMultiLeafCollimatorPosition( vtkMRMLRTBeamNode* beamNode, 
  vtkMRMLTableNode* mlcTableNode, vtkPolyData* targetPoly)
{
  if (!beamNode)
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorPosition: invalid beam node");
    return false;
  }

  int nofLeafPairs = 0;
  if (mlcTableNode)
  {
    nofLeafPairs = mlcTableNode->GetNumberOfRows() - 1;
  }
/*
  vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
  vtkTransform* beamTransform = nullptr;
  if (beamTransformNode)
  {
    beamTransform = vtkTransform::SafeDownCast(beamTransformNode->GetTransformToParent());
  }
  else
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorPosition: Beam transform node is invalid");
    return false;
  }

  vtkNew<vtkMatrix4x4> beamInverseMatrix;
  vtkNew<vtkTransform> beamInverseTransform;
  if (beamTransform)
  {
    beamTransform->GetInverse(beamInverseMatrix);
    beamInverseTransform->SetMatrix(beamInverseMatrix);
  }
  else
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorPosition: Matrix transform is invalid");
    return false;
  }
*/

  vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
  vtkNew<vtkMatrix4x4> beamInverseMatrix;
  vtkNew<vtkTransform> beamInverseTransform;
  if (beamTransformNode)
  {
    beamTransformNode->GetMatrixTransformToWorld(beamInverseMatrix);
    beamInverseMatrix->Invert();
    beamInverseTransform->SetMatrix(beamInverseMatrix);
  }

  double isocenterToMLCDistance = beamNode->GetSAD() - beamNode->GetSourceToMultiLeafCollimatorDistance();

  // transform target poly data into beam frame
  auto beamInverseTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  beamInverseTransformFilter->SetTransform(beamInverseTransform);
  beamInverseTransformFilter->SetInputData(targetPoly);
  beamInverseTransformFilter->Update();

  if (nofLeafPairs > 0)
  {
    const char* mlcName = mlcTableNode->GetName();
    bool typeMLCX = !strncmp( "MLCX", mlcName, strlen("MLCX")); // MLCX by default
    bool typeMLCY = !strncmp( "MLCY", mlcName, strlen("MLCY"));
    // if MLCX then typeMLCX = true, if MLCY then typeMLCX = false
    if (typeMLCY && !typeMLCX)
    {
      typeMLCX = false;
    }

    auto leaf1 = vtkSmartPointer<vtkCubeSource>::New();
    auto leaf2 = vtkSmartPointer<vtkCubeSource>::New();
/*
    int leafPairStart;
    int leafPairEnd;
    FindLeafPairRangeIndexes( beamNode, mlcTableNode, leafPairStart, leafPairEnd);
    if (leafPairStart == -1 || leafPairEnd == -1)
    {
      vtkErrorMacro("CalculateMultiLeafCollimatorPosition: Unable to find leaves range");
      return false;
    }

    if (leafPairStart > 0)
    {
      leafPairStart -= 1;
    }
    if (leafPairEnd < int(nofLeafPairs - 1))
    {
      leafPairEnd += 1;
    }
*/
    // create polydata for a leaf pair
//    for ( vtkIdType leafPair = leafPairStart; leafPair <= leafPairEnd; leafPair++)
    for ( vtkIdType leafPair = 0; leafPair < nofLeafPairs; leafPair++)
    { // leaf pair for cycle begins
//      vtkWarningMacro("LeafID: " << leafPair);

      vtkTable* table = mlcTableNode->GetTable();
      double InitialPos1 = table->GetValue( leafPair, 1).ToDouble();
      double InitialPos2 = table->GetValue( leafPair, 2).ToDouble();

      if (vtkSlicerRtCommon::AreEqualWithTolerance( InitialPos1, InitialPos2))
      {
        continue;
      }

      double boundBegin = table->GetValue( leafPair, 0).ToDouble();
      double boundEnd = table->GetValue( leafPair + 1, 0).ToDouble();

//      double InitialPos1 = table->GetValue( leafPair, 1).ToDouble();
//      double InitialPos2 = table->GetValue( leafPair, 2).ToDouble();
      double mlcLeafLength = 200.;

        if (typeMLCX) // MLCX
        {
          leaf1->SetBounds(-1. * mlcLeafLength, 0., boundBegin, boundEnd, 
            -1. * isocenterToMLCDistance, isocenterToMLCDistance);
          leaf2->SetBounds( 0., mlcLeafLength, boundBegin, boundEnd, 
            -1. * isocenterToMLCDistance, isocenterToMLCDistance);
        }
        else // MLCY
        {
          leaf1->SetBounds( boundBegin, boundEnd, -1. * mlcLeafLength, 0.0, 
            -1. * isocenterToMLCDistance, isocenterToMLCDistance);
          leaf2->SetBounds( boundBegin, boundEnd, 0.0, mlcLeafLength, 
            -1. * isocenterToMLCDistance, isocenterToMLCDistance);
        }

        leaf1->Update();
        leaf2->Update();

        double side1 = 0.0;
        if (FindLeafAndTargetCollision2( beamNode, leaf1->GetOutputPort(), beamInverseTransformFilter->GetOutputPort(),
          side1, InitialPos1 - 1., 1, typeMLCX))
        {
//          vtkWarningMacro("CalculateMultiLeafCollimatorPosition: Side 1 collision found " << s1);
          table->SetValue( leafPair, 1, side1);
        }
        else
        {
          vtkWarningMacro("CalculateMultiLeafCollimatorPosition: Side 1 collision hasn't been found");
        }

        double side2 = 0.0;
        if (FindLeafAndTargetCollision2( beamNode, leaf2->GetOutputPort(), beamInverseTransformFilter->GetOutputPort(),
          side2, InitialPos2 + 1., 2, typeMLCX))
        {
//          vtkWarningMacro("CalculateMultiLeafCollimatorPosition: Side 2 collision found " << s2);
          table->SetValue( leafPair, 2, side2);
        }
        else
        {
          vtkWarningMacro("CalculateMultiLeafCollimatorPosition: Side 2 collision hasn't been found");
        }

    } // leafPair for cycle ends

  }

  return true;
}

//---------------------------------------------------------------------------
bool
vtkSlicerMlcPositionLogic::FindLeafAndTargetCollision( vtkMRMLRTBeamNode* beamNode, vtkAlgorithmOutput* leafOutput, vtkAlgorithmOutput* targetOutput,
  double& sidePos, int sideType, bool mlcType, 
  double maxPositionDistance, double positionStep)
{
  int contactMode = 1;
  auto targetMatrix = vtkSmartPointer<vtkMatrix4x4>::New(); // fixed target matrix
  targetMatrix->Identity();
  auto leafTransform = vtkSmartPointer<vtkTransform>::New(); // moving leaf transform
  leafTransform->Identity();
  auto collide = vtkSmartPointer<vtkCollisionDetectionFilter>::New();
  collide->SetInputConnection( 0, leafOutput);
  collide->SetTransform( 0, leafTransform);
  collide->SetInputConnection( 1, targetOutput);
  collide->SetMatrix( 1, targetMatrix);
  collide->SetBoxTolerance(0.0);
  collide->SetCellTolerance(0.0);
  collide->SetNumberOfCellsPerNode(2);
  if (contactMode == 0)
  {
    collide->SetCollisionModeToAllContacts();
  }
  else if (contactMode == 1)
  {
    collide->SetCollisionModeToFirstContact();
  }
  else
  {
    collide->SetCollisionModeToHalfContacts();
  }
  collide->GenerateScalarsOn();

  // Move the leaf
  int numSteps = 2 * maxPositionDistance / positionStep;
  if (sideType == 1)
  {
      if (mlcType) // MLCX
      {
        leafTransform->Translate( -1. * maxPositionDistance, 0.0, 0.0);
      }
      else // MLCY
      {
        leafTransform->Translate( 0., -1. * maxPositionDistance, 0.0);
      }
  }
  else
  {
      if (mlcType) // MLCX
      {
        leafTransform->Translate( maxPositionDistance, 0.0, 0.0);
      }
      else // MLCY
      {
        leafTransform->Translate( 0., maxPositionDistance, 0.0);
      }
  }

  bool res = false;
  double p[3] = {};
  for (int i = 0; i < numSteps; ++i)
  {
    if (sideType == 1)
    {
      if (mlcType) // MLCX
      {
        leafTransform->Translate( positionStep, 0.0, 0.0);
      }
      else // MLCY
      {
        leafTransform->Translate( 0.0, positionStep, 0.0);
      }
    }
    else
    {
      if (mlcType) // MLCX
      {
        leafTransform->Translate( -1. * positionStep, 0.0, 0.0);
      }
      else // MLCY
      {
        leafTransform->Translate( 0.0, -1. * positionStep, 0.0);
      }
    }
    
    leafTransform->Update();
    collide->Update();

    if (collide->GetNumberOfContacts() > 0)
    {

      vtkPolyData* contacts = collide->GetContactsOutput();
      vtkIdType nofCells = contacts->GetNumberOfCells();

//      vtkWarningMacro("Number of cells from polydata: "<< nofCells);

      for ( vtkIdType i = 0; i < nofCells; ++i)
      {
        vtkCell* cell = contacts->GetCell(i);
        if (cell)
        {
          vtkPoints* points = cell->GetPoints();
          if (points)
          {
            for ( vtkIdType j = 0; j < points->GetNumberOfPoints(); ++j)
            {
//              double p[3];
              points->GetPoint( j, p);
//              vtkWarningMacro("Coords: "<< p[0] << " " << p[1] << " " << p[2]);
              if (mlcType) // MLCX
              {
                sidePos = p[0];
              }
              else // MLCY
              {
                sidePos = p[1];
              }
            }
          }
        }
      }

      res = true;
      break;
    }
  }
/*
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorPosition: Invalid MRML scene");
    return false;
  }

  auto beamInverseTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  vtkNew<vtkTransform> transform;
  transform->Identity();
  if (mlcType) // MLCX
  {
    transform->Translate( p[0], 0.0, 0.0);
  }
  else // MLCY
  {
    transform->Translate( 0.0, p[1], 0.0);
  }

  beamInverseTransformFilter->SetTransform(transform);
  beamInverseTransformFilter->SetInputConnection(leafOutput);
  beamInverseTransformFilter->Update();

      // Create a models logic for convenient loading of components
      vtkNew<vtkSlicerModelsLogic> modelsLogic;
      modelsLogic->SetMRMLScene(scene);


      vtkMRMLModelNode* mlcModelNode = modelsLogic->AddModel(beamInverseTransformFilter->GetOutput());
      if (mlcModelNode)
      {
        mlcModelNode->SetName("Leaf1_Model");
        mlcModelNode->SetAndObserveTransformNodeID(beamNode->GetTransformNodeID());
        vtkMRMLDisplayNode* displayNode = mlcModelNode->GetDisplayNode();
        if (displayNode)
        {
          displayNode->SetColor( 1, 1, 1);
          displayNode->SetOpacity(0.5);
          displayNode->SetBackfaceCulling(0); // Disable backface culling to make the back side of the contour visible as well
          displayNode->VisibilityOff();
          displayNode->Visibility2DOn();
        }
      }
*/
  return res;
}

//---------------------------------------------------------------------------
bool
vtkSlicerMlcPositionLogic::FindLeafAndTargetCollision2( vtkMRMLRTBeamNode* beamNode, vtkAlgorithmOutput* leafOutput, vtkAlgorithmOutput* targetOutput,
  double& sidePos, double initialPosition, int sideType, bool mlcType, 
  double maxPositionDistance, double positionStep)
{
  int contactMode = 1;
  auto targetMatrix = vtkSmartPointer<vtkMatrix4x4>::New(); // fixed target matrix
  targetMatrix->Identity();
  auto leafTransform = vtkSmartPointer<vtkTransform>::New(); // moving leaf transform
  leafTransform->Identity();
  auto collide = vtkSmartPointer<vtkCollisionDetectionFilter>::New();
  collide->SetInputConnection( 0, leafOutput);
  collide->SetTransform( 0, leafTransform);
  collide->SetInputConnection( 1, targetOutput);
  collide->SetMatrix( 1, targetMatrix);
  collide->SetBoxTolerance(0.0);
  collide->SetCellTolerance(0.0);
  collide->SetNumberOfCellsPerNode(2);
  if (contactMode == 0)
  {
    collide->SetCollisionModeToAllContacts();
  }
  else if (contactMode == 1)
  {
    collide->SetCollisionModeToFirstContact();
  }
  else
  {
    collide->SetCollisionModeToHalfContacts();
  }
  collide->GenerateScalarsOn();

  // Move the leaf
  int numSteps = fabs(initialPosition) + fabs(maxPositionDistance) / fabs(positionStep);
  if (sideType == 1)
  {
      if (mlcType) // MLCX
      {
        leafTransform->Translate( initialPosition, 0.0, 0.0);
      }
      else // MLCY
      {
        leafTransform->Translate( 0., initialPosition, 0.0);
      }
  }
  else
  {
      if (mlcType) // MLCX
      {
        leafTransform->Translate( initialPosition, 0.0, 0.0);
      }
      else // MLCY
      {
        leafTransform->Translate( 0., initialPosition, 0.0);
      }
  }

  bool res = false;
  double p[3] = {};
  for (int i = 0; i < numSteps; ++i)
  {
    if (sideType == 1)
    {
      if (mlcType) // MLCX
      {
        leafTransform->Translate( positionStep, 0.0, 0.0);
      }
      else // MLCY
      {
        leafTransform->Translate( 0.0, positionStep, 0.0);
      }
    }
    else
    {
      if (mlcType) // MLCX
      {
        leafTransform->Translate( -1. * positionStep, 0.0, 0.0);
      }
      else // MLCY
      {
        leafTransform->Translate( 0.0, -1. * positionStep, 0.0);
      }
    }
    
    leafTransform->Update();
    collide->Update();

    if (collide->GetNumberOfContacts() > 0)
    {

      vtkPolyData* contacts = collide->GetContactsOutput();
      vtkIdType nofCells = contacts->GetNumberOfCells();

      for ( vtkIdType i = 0; i < nofCells; ++i)
      {
        vtkCell* cell = contacts->GetCell(i);
        if (cell)
        {
          vtkPoints* points = cell->GetPoints();
          if (points)
          {
            for ( vtkIdType j = 0; j < points->GetNumberOfPoints(); ++j)
            {
              points->GetPoint( j, p);
              if (mlcType) // MLCX
              {
                sidePos = p[0];
              }
              else // MLCY
              {
                sidePos = p[1];
              }
            }
          }
        }
      }

      res = true;
      break;
    }
  }

  return res;
}
