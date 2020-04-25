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
#include "vtkSlicerLoadableModuleTemplateLogic.h"

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
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLTableNode.h>

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
#include <vtkCellLocator.h>

#include <vtkOBBTree.h>
#include <vtkPoints.h>
#include <vtkIdList.h>
#include <vtkPointsProjectedHull.h>

#include <vtkLookupTable.h>
#include <vtkColorTransferFunction.h>

#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkPointData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkDataArray.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h> // cross, dot vector operations

#include <vtkSlicerRtCommon.h>

// STD includes
#include <array>
#include <algorithm>

namespace
{

const char* MLCY_Boundary = "MLCY_Boundary";
const char* MLCY_Position = "MLCY_Position";

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

} // namespace

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerLoadableModuleTemplateLogic);

//----------------------------------------------------------------------------
vtkSlicerLoadableModuleTemplateLogic::vtkSlicerLoadableModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerLoadableModuleTemplateLogic::~vtkSlicerLoadableModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
void
vtkSlicerLoadableModuleTemplateLogic::PrintSelf( ostream& os, vtkIndent indent)
{
  os << indent << "vtkSlicerModuleLogic:     " << this->GetClassName() << "\n";
  this->Superclass::PrintSelf( os, indent);
}

//---------------------------------------------------------------------------
void
vtkSlicerLoadableModuleTemplateLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
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
vtkSlicerLoadableModuleTemplateLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void
vtkSlicerLoadableModuleTemplateLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void
vtkSlicerLoadableModuleTemplateLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//-----------------------------------------------------------------------------
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
  vtkNew<vtkMRMLMarkupsClosedCurveNode> curveNode;
  curveNode->SetCurveTypeToLinear();
  curveNode->SetName("MLCProjectionCurve");

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
vtkMRMLDoubleArrayNode*
vtkSlicerLoadableModuleTemplateLogic::CreateMultiLeafCollimatorDoubleArrayNode()
{
  vtkNew<vtkMRMLDoubleArrayNode> arrayNode;
  this->GetMRMLScene()->AddNode(arrayNode);
  arrayNode->SetName(MLCY_Boundary);

  // Leaf boundaries
  vtkNew<vtkDoubleArray> b;
  b->SetNumberOfComponents(1);
  b->SetNumberOfTuples(leaves + 1);
  for ( size_t i = 0; i < leaves + 1; ++i)
  {
    b->InsertTuple( i, boundary + i);
  }
  arrayNode->SetArray(b);
  return arrayNode;
}

//---------------------------------------------------------------------------
vtkMRMLTableNode*
vtkSlicerLoadableModuleTemplateLogic::CreateMultiLeafCollimatorTableNode(const std::vector<double>& mlcPositions)
{
  vtkNew<vtkMRMLTableNode> tableNode;
  this->GetMRMLScene()->AddNode(tableNode);
  tableNode->SetName(MLCY_Position);

  vtkTable* table = tableNode->GetTable();
  if (table)
  {
    // Leaf position on the side "1"
    vtkNew<vtkDoubleArray> pos1;
    pos1->SetName("1");
    table->AddColumn(pos1);

    // Leaf position on the side "2"
    vtkNew<vtkDoubleArray> pos2;
    pos2->SetName("2");
    table->AddColumn(pos2);

    vtkIdType size = mlcPositions.size() / 2;
    table->SetNumberOfRows(size);
    for ( vtkIdType row = 0; row < size; ++row)
    {
      table->SetValue( row, 0, mlcPositions[row]);
      table->SetValue( row, 1, mlcPositions[row + size]);
    }
    tableNode->SetUseColumnNameAsColumnHeader(true);
    tableNode->SetColumnDescription( "1", "Leaf position on the side \"1\"");
    tableNode->SetColumnDescription( "2", "Leaf position on the side \"2\"");
    return tableNode;
  }
  else
  {
    vtkErrorMacro("CreateMultiLeafCollimatorTableNode: unable to create vtkTable to fill MLC positions");
  }
  return nullptr;
}

//---------------------------------------------------------------------------
vtkMRMLTableNode*
vtkSlicerLoadableModuleTemplateLogic::CalculateMultiLeafCollimatorPosition( 
  vtkMRMLMarkupsCurveNode* curveNode, vtkMRMLDoubleArrayNode* mlcBoundaryNode)
{

  vtkDoubleArray* mlcBoundaryArray = nullptr;
  size_t nofLeafPairs = 0;
  if (mlcBoundaryNode)
  {
    mlcBoundaryArray = mlcBoundaryNode->GetArray();
    nofLeafPairs = mlcBoundaryNode ? (mlcBoundaryArray->GetNumberOfTuples() - 1) : 0;
  }

  double curveBounds[4];
  CalculateCurveBoundary( curveNode, curveBounds);

  int leafPairStart, leafPairEnd;
  FindLeafPairRangeIndexes( curveBounds, mlcBoundaryArray, leafPairStart, leafPairEnd);
  if (leafPairStart == -1 || leafPairEnd == -1)
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorPosition: Unable to find leaves range");
    return nullptr;
  }
    
  std::vector< double > positions(2 * nofLeafPairs);
  for ( int leafPairIndex = leafPairStart; leafPairIndex <= leafPairEnd; ++leafPairIndex)
  {
    double side1, side2;
    size_t leafIndex = leafPairIndex;
    if (FindLeafPairPositions( curveNode, mlcBoundaryArray, leafIndex, side1, side2))
    {
      // positions found
      positions[leafIndex] = side1;
      positions[leafIndex + nofLeafPairs] = side2;
    }
  }
  return CreateMultiLeafCollimatorTableNode(positions);
}

//---------------------------------------------------------------------------
void
vtkSlicerLoadableModuleTemplateLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
bool
vtkSlicerLoadableModuleTemplateLogic::CalculateCurveBoundary( vtkMRMLMarkupsCurveNode* curveNode, double* b)
{
  if (curveNode)
  {
    const std::vector< vtkMRMLMarkupsNode::ControlPoint* >* controlPoints = curveNode->GetControlPoints();
    if (controlPoints->size() < 3)
    {
      vtkErrorMacro("CalculateCurveBoundary: Not enough points in closed curve node");
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
    b[0] = xmin;
    b[1] = xmax;
    b[2] = ymin;
    b[3] = ymax;
    return true;
  }
  vtkErrorMacro("CalculateCurveBoundary: Invalid closed curve node");
  return false;
}

//---------------------------------------------------------------------------
void
vtkSlicerLoadableModuleTemplateLogic::FindLeafPairRangeIndexes( double* b, vtkDoubleArray* mlcBoundaryArray, int& leafIndexFirst, int& leafIndexLast)
{
  leafIndexFirst = -1;
  leafIndexLast = -1;
  size_t nofLeafPairs = mlcBoundaryArray->GetNumberOfTuples() - 1;

  for ( size_t leafPair = 0; leafPair < nofLeafPairs; ++leafPair)
  {
    double boundBegin = mlcBoundaryArray->GetTuple1(leafPair);
    double boundEnd = mlcBoundaryArray->GetTuple1(leafPair + 1);

    if (b[0] >= boundBegin && b[0] <= boundEnd)
    {
      leafIndexFirst = int(leafPair);
    }
    if (b[1] >= boundBegin && b[1] <= boundEnd)
    {
      leafIndexLast = int(leafPair);
    }
  }
}

//---------------------------------------------------------------------------
bool
vtkSlicerLoadableModuleTemplateLogic::FindLeafPairPositions( 
   vtkMRMLMarkupsCurveNode* curveNode, vtkDoubleArray* mlcBoundaryArray, size_t leafPairIndex, 
    double& side1, double& side2, int vtkNotUsed(strategy), double maxPositionDistance, double positionStep)
{
  double leafStart = mlcBoundaryArray->GetTuple1(leafPairIndex);
  double leafEnd = mlcBoundaryArray->GetTuple1(leafPairIndex + 1);

  vtkPolyData* curvePoly = curveNode->GetCurveWorld();
  if (!curvePoly)
  {
    vtkErrorMacro("FindLeafPairPositions: Curve polydata is invalid");
    return false;
  }

  // Build locator
  vtkNew<vtkCellLocator> cellLocator;
  cellLocator->SetDataSet(curvePoly);
  cellLocator->BuildLocator();

  // intersection with side1
  bool side1Flag = false;
  double pStart[3] = { leafStart, 0, 0. };
  double pEnd[3] = { leafEnd, 0, 0. };
  int subId;
  double t, xyz[3], pcoords[3];
  for ( double y = -1. * maxPositionDistance; y < 0.; y += positionStep)
  {
    pStart[1] = y;
    pEnd[1] = y;
    if (cellLocator->IntersectWithLine( pStart, pEnd, 0.0001, t, xyz, pcoords, subId))
    {
      side1Flag = true;
      // xyz values
      side1 = xyz[1];
      break;
    }
  }

  // intersection with side2
  bool side2Flag = false;
  for ( double y = maxPositionDistance; y > 0.; y -= positionStep)
  {
    pStart[1] = y;
    pEnd[1] = y;

    if (cellLocator->IntersectWithLine( pStart, pEnd, 0.0001, t, xyz, pcoords, subId))
    {
      side2Flag = true;
      // xyz values
      side2 = xyz[1];
      break;
    }
  }
  return (side1Flag && side2Flag);
}
