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

#include <vtkPoints.h>
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

const int leaves = 32;
const double boundary[33] = {
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

/*
bool
AreEqual( double v1, double v2)
{
  return (fabs(v1 - v2) < std::numeric_limits<double>::epsilon());
}

//! Use a color transfer Function to generate the colors in the lookup table.
void
MakeLUTFromCTF( size_t tableSize, vtkLookupTable* lut)
{
  vtkNew<vtkColorTransferFunction> ctf;
  ctf->SetColorSpaceToDiverging();

  // Yellow scale
  ctf->AddRGBPoint( 0.0, 0.0, 0.0, 0.);
  ctf->AddRGBPoint( 0.5, 0.5, 0.5, 0.);
  ctf->AddRGBPoint( 1.0, 1.0, 1.0, 0.);

  lut->SetNumberOfTableValues(tableSize);
  lut->SetRange( -1200.0, 2895.0);
  lut->Build();

  for( size_t i = 0; i < tableSize; ++i)
  {
    double *rgb = ctf->GetColor( double(i) / tableSize);
    lut->SetTableValue( i, rgb);
  }
}
*/

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
vtkSlicerLoadableModuleTemplateLogic::CalculateMultiLeafCollimatorOpening( 
  vtkMRMLRTBeamNode* beamNode, vtkPolyData* targetPoly)
{
  if (!beamNode)
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorOpening: Beam node is invalid");
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
    vtkErrorMacro("CalculateMultiLeafCollimatorOpening: Beam transform node is invalid");
    return nullptr;
  }

  vtkNew<vtkMatrix4x4> inverseMatrix;
  if (beamTransform)
  {
    beamTransform->GetInverse(inverseMatrix);
  }
  else
  {
    vtkErrorMacro("CalculateMultiLeafCollimatorOpening: Matrix transform is invalid");
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

//    std::vector< vtkMRMLMarkupsNode::ControlPoint* >* controlPoints = curveNode->GetControlPoints();
    double bounds[4];
    CalculateCurveBoundary( curveNode, bounds);
    int b, e;
    FindLeavesRangeIndexes( bounds, b, e);
    vtkIdType start = FindFiducialIdForX( curveNode, bounds[0]);
    vtkIdType stop = FindFiducialIdForX( curveNode, bounds[1]);
    for( int i = 0; i < leaves; ++i)
    {
      vtkErrorMacro("Leaf: " << i);
      FindBoundaryForLeaf( curveNode, i, start, bounds, b,e );
    }
//    vtkErrorMacro("Leaves: " << b << " " << e << " " << start << " " << stop);
/*
    std::vector<int> indexedLeaves;

    std::vector< vtkMRMLMarkupsNode::ControlPoint* >* points = curveNode->GetControlPoints();
    for ( size_t i = 0; i < points->size() - 1; ++i)
    {

//      vtkMRMLMarkupsNode::ControlPoint* point = points->at(p);
      vtkNew<vtkPoints> sampledPoints;
      if (curveNode->GetSampledCurvePointsBetweenStartEndPointsWorld( sampledPoints, .1, vtkIdType(i + 1), vtkIdType(i + 2)))
      {
        vtkErrorMacro("Curve Sampled OK! Index: " << i);

        for ( vtkIdType p = 0; p < sampledPoints->GetNumberOfPoints(); ++p)
        {
          double pt[3];
          sampledPoints->GetPoint( p, pt);
          pt[0] *= 0.1;
          pt[1] *= 0.1;
          pt[2] *= 0.1;
          for ( int j = 0; j < leaves; ++j)
          {
            double leaf[2] = { boundary[j], boundary[j + 1] };
            if (pt[0] < leaf[0])
            {
             ; // out of bound
            }
            else if (pt[0] >= leaf[0] && pt[0] <= leaf[1])
            {
              vtkErrorMacro("Sampled curve point inside leaf j,x,y: " << j << " " << pt[0] << " " << pt[1]);
              break;
            }
            else
            {
              ; // out of bound
            }
          }
        }
      }
      else {
        vtkErrorMacro("Curve Sampled not OK!");
      }
    }
/*
//    for ( auto it = points->begin(); it != points->end(); ++it)
//    {
    for ( vtkIdType p = 0; p < points->size() - 1; ++p)
//      vtkMRMLMarkupsNode::ControlPoint* point = *it;
      vtkMRMLMarkupsNode::ControlPoint* point = points[p];
      double x = point->Position[0];
      vtkNew<vtkPoints> sampledPoints;
      if (curveNode->GetSampledCurvePointsBetweenStartEndPointsWorld( sampledPoints, 0.1, p, p + 1))
      {
        for ( int i = 0; i < leaves; ++i)
        {
          double leaf[2] = { boundary[i], boundary[i + 1] };
//        double y = point->Position[1];
          if (x < leaf[0])
          {
//            vtkErrorMacro("Out of bounds, opening 0");
          }
          else if (x >= leaf[0] && x < leaf[1])
          {
            vtkErrorMacro("Curve in leaf bound, leaf index: " << i << " " << leaf[0] << " " << x << " " << leaf[1]);
            indexedLeaves.push_back(i);
          }
        }
      }
   
    if (indexedLeaves.size())
    {
      int min = *std::min_element( indexedLeaves.begin(), indexedLeaves.end());
      int max = *std::max_element( indexedLeaves.begin(), indexedLeaves.end());
      vtkErrorMacro("Index boundary: " << min << " " << max);
    }
*/
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
  vtkSmartPointer<vtkMRMLDoubleArrayNode> arrayNode = vtkSmartPointer<vtkMRMLDoubleArrayNode>::New();
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
vtkSlicerLoadableModuleTemplateLogic::CreateMultiLeafCollimatorTableNode( 
  vtkMRMLMarkupsCurveNode* curveNode, vtkMRMLDoubleArrayNode* vtkNotUsed(mlcBoundaryNode))
{
  for ( int i = 0; i < leaves - 1; ++i)
  {
    double leaf[2] = { boundary[i], boundary[i + 1] };

    std::vector< vtkMRMLMarkupsNode::ControlPoint* >* points = curveNode->GetControlPoints();
    for ( auto it = points->begin(); it != points->end(); ++it)
    {
      vtkMRMLMarkupsNode::ControlPoint* point = *it;
      double x = point->Position[0];
      double y = point->Position[1];
      if (y < leaf[0])
      {
//        vtkErrorMacro("Out of bounds, opening 0");
      }
      else if (y >= leaf[0] && y < leaf[1])
      {
        vtkErrorMacro("Curve in leaf bound, leaf index: " << i);
        break;
      }
    }
  }
  return nullptr;
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
      vtkErrorMacro("Not enough points in closed curve node");
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
  vtkErrorMacro("Invalid closed curve node");
  return false;
}

//---------------------------------------------------------------------------
void
vtkSlicerLoadableModuleTemplateLogic::FindLeavesRangeIndexes( double* b, int& leafIndexFirst, int& leafIndexLast)
{
  leafIndexFirst = -1;
  leafIndexLast = -1;
  for ( int i = 0; i < leaves; ++i)
  {
    double leafBegin = boundary[i];
    double leafEnd = boundary[i + 1];
    if (b[0] >= leafBegin && b[0] <= leafEnd)
    {
      leafIndexFirst = i;
    }
    if (b[1] >= leafBegin && b[1] <= leafEnd)
    {
      leafIndexLast = i;
    }
  }
}

//---------------------------------------------------------------------------
vtkIdType
vtkSlicerLoadableModuleTemplateLogic::FindFiducialIdForX( vtkMRMLMarkupsCurveNode* curveNode, double b)
{
  std::vector<int> indexedLeaves;

  const std::vector< vtkMRMLMarkupsNode::ControlPoint* >* points = curveNode->GetControlPoints();
  for ( size_t i = 0; i < points->size(); ++i)
  {
    const vtkMRMLMarkupsNode::ControlPoint* point = points->at(i);
    double x = point->Position[0];
    if (vtkSlicerRtCommon::AreEqualWithTolerance( b, x))
    {
      return vtkIdType(i + 1);
    }
  }
  return -1;
}

//---------------------------------------------------------------------------
bool
vtkSlicerLoadableModuleTemplateLogic::FindBoundaryForLeaf( vtkMRMLMarkupsCurveNode* curveNode, 
  int leafIndex, vtkIdType vtkNotUsed(curveId), double *b, int beginLeaf, int endLeaf)
{
  double leafStart = boundary[leafIndex];
  double leafEnd = boundary[leafIndex + 1];
  int lessInd = -1, moreInd = -1;
  double lessPos = b[0];
  double morePos = b[1];

  vtkWarningMacro("Leaf boundary: " << leafStart << " " << leafEnd);

  std::vector< int > indexes;
  std::vector< int > ids;
  const std::vector< vtkMRMLMarkupsNode::ControlPoint* >* tmp_points = curveNode->GetControlPoints();
  std::vector< std::pair< int , std::array< double, 2 > > > new_points;
  std::vector< vtkMRMLMarkupsNode::ControlPoint* > points( tmp_points->begin(), tmp_points->end());

  for ( size_t i = 0; i < tmp_points->size(); ++i)
  {
    vtkMRMLMarkupsNode::ControlPoint* point = tmp_points->at(i);
    double x = point->Position[0];
    double y = point->Position[1];
    new_points.push_back(std::make_pair( int(i) - tmp_points->size(), std::array< double, 2 >({x, y})));
  }
  for ( size_t i = 0; i < tmp_points->size(); ++i)
  {
    vtkMRMLMarkupsNode::ControlPoint* point = tmp_points->at(i);
    double x = point->Position[0];
    double y = point->Position[1];
    new_points.push_back(std::make_pair( i, std::array< double, 2 >({x, y})));
  }
//  for ( size_t i = 0; i < tmp_points->size(); ++i)
//  {
//    vtkMRMLMarkupsNode::ControlPoint* point = tmp_points->at(i);
//    points.push_back(point);
//  }

  for ( size_t i = 0; i < new_points.size(); ++i)
  {
//    const vtkMRMLMarkupsNode::ControlPoint* point = points.at(i);
//    double x = point->Position[0];
    double x = (new_points[i].second)[1];
    if (x < leafStart)
    {
      indexes.push_back(-1);
    }
    else if (x > leafEnd)
    {
      indexes.push_back(1);
    }
    else
    {
      indexes.push_back(0);
//      const vtkMRMLMarkupsNode::ControlPoint* p = points.at(pos);
//      double x = p->Position[0];
//      double y = point->Position[1];
//      ids.push_back(i);
      ids.push_back(new_points[i].first);
  //    vtkErrorMacro("Within Leaf: " << i << " " << x << " " << y);
    }
  }
/*    if (x < leafStart)
    {
      // find nearest less
      if (fabs(leafStart - x) < fabs(leafStart - lessPos)) 
      {
        lessPos = x;
        lessInd = i;
      }
    }
    else if (x > leafEnd)
    {
      // find nearest less
      if (fabs(leafEnd - x) < fabs(leafEnd - morePos)) 
      {
        morePos = x;
        moreInd = i;
      }
    }
    else if (x >= leafStart && x <= leafEnd)
    {
      if (lessInd != -1)
      {
        indexes.push_back(lessInd);
      }
      if (moreInd != -1)
      {
        indexes.push_back(moreInd);
      }
      vtkErrorMacro("Index within: " << i);
      indexes.push_back(i);
    }
  }
  indexes.push_back(lessInd);
  indexes.push_back(moreInd);

  vtkErrorMacro("LessInd, MoreInd: " << lessInd << " " << moreInd);
*/
  std::array< int, 2 > vOne_Zero{ 1, 0 };
  std::array< int, 2 > vZero_One{ 0, 1 };
  std::array< int, 2 > vMinusOne_Zero{ -1, 0 };
  std::array< int, 2 > vZero_MinusOne{ 0, -1 };
  std::array< int, 2 > vMinusOne_One{ -1, 1 };
  std::array< int, 2 > vOne_MinusOne{ 1, -1 };
  
  auto itOne_Zero = std::search( indexes.begin(), indexes.end(), vOne_Zero.begin(), vOne_Zero.end());
  auto itZero_One = std::search( indexes.begin(), indexes.end(), vZero_One.begin(), vZero_One.end());
  auto itMinusOne_Zero = std::search( indexes.begin(), indexes.end(), vMinusOne_Zero.begin(), vMinusOne_Zero.end());
  auto itZero_MinusOne = std::search( indexes.begin(), indexes.end(), vZero_MinusOne.begin(), vZero_MinusOne.end());
  auto itMinusOne_One = std::search( indexes.begin(), indexes.end(), vMinusOne_One.begin(), vMinusOne_One.end());
  auto itOne_MinusOne = std::search( indexes.begin(), indexes.end(), vOne_MinusOne.begin(), vOne_MinusOne.end());
  bool inRight = false;
  bool outRight = false;
  bool inLeft = false;
  bool outLeft = false;
  bool betweenRight = false;
  bool betweenLeft = false;

  if (itOne_Zero != indexes.end())
  {
    size_t pos = itOne_Zero - indexes.begin();
//    const vtkMRMLMarkupsNode::ControlPoint* p = points.at(pos);
//    double x = p->Position[0];
//    double y = p->Position[1];
 //   vtkErrorMacro("Inside Leaf Right (1 0): " << pos << " " << x << " " << y);
    ids.push_back(pos);
    inRight = true;
  }
  if (itZero_One != indexes.end())
  {
    size_t pos = itZero_One - indexes.begin() + 1;
//    const vtkMRMLMarkupsNode::ControlPoint* p = points.at(pos);
//    double x = p->Position[0];
//    double y = p->Position[1];
//    vtkErrorMacro("Outsize Leaf Right (0 1): " << pos << " "  << x << " " << y);
    ids.push_back(pos);
    outRight = true;
  }
  if (itMinusOne_Zero != indexes.end())
  {
    size_t pos = itMinusOne_Zero - indexes.begin();
//    const vtkMRMLMarkupsNode::ControlPoint* p = points.at(pos);
//    double x = p->Position[0];
//    double y = p->Position[1];
 //   vtkErrorMacro("Inside Leaf Left (-1 0): " << pos << " "  << x << " " << y);
    ids.push_back(pos);
    inLeft = true;
  }
  if (itZero_MinusOne != indexes.end())
  {
    size_t pos = itZero_MinusOne - indexes.begin() + 1;
//    const vtkMRMLMarkupsNode::ControlPoint* p = points.at(pos);
//    double x = p->Position[0];
//    double y = p->Position[1];
 //   vtkErrorMacro("Outsize Leaf Left (0 -1): " << pos << " "  << x << " " << y);
    ids.push_back(pos);
    outLeft = true;
  }
  if (itMinusOne_One != indexes.end())
  {
    size_t pos = itMinusOne_One - indexes.begin();
//    const vtkMRMLMarkupsNode::ControlPoint* p = points.at(pos);
//    double x = p->Position[0];
//    double y = p->Position[1];
 //   vtkErrorMacro("Between Leaf Left (-1 1): " << pos << " "  << x << " " << y);
    ids.push_back(pos);
//    p = points.at(pos + 1);
//    x = p->Position[0];
//    y = p->Position[1];
 //   vtkErrorMacro("Between Leaf Left (-1 1): " << pos + 1 << " "  << x << " " << y);
    ids.push_back(pos + 1);
    betweenLeft = true;
  }
  if (itOne_MinusOne != indexes.end())
  {
    size_t pos = itOne_MinusOne - indexes.begin();
//    const vtkMRMLMarkupsNode::ControlPoint* p = points.at(pos);
//    double x = p->Position[0];
//    double y = p->Position[1];
    ids.push_back(pos);
//    vtkErrorMacro("Between Leaf Right (1 -1): " << pos << " "  << x << " " << y);
//    p = points.at(pos + 1);
//    x = p->Position[0];
//    y = p->Position[1];
//    vtkErrorMacro("Between Leaf Left (1 -1): " << pos + 1 << " "  << x << " " << y);
    ids.push_back(pos + 1);
    betweenRight = true;
  }

  std::sort( ids.begin(), ids.end());

  for ( int v : ids)
  {
    vtkErrorMacro("Ids: " << v);
  }

  std::pair< std::vector< int >, std::vector< int > > curveSections;
  if ((inRight && outLeft && betweenLeft && !betweenRight && !inLeft && !outRight) || 
    (inRight && outLeft && inLeft && outRight && !betweenRight && !betweenLeft) || 
    (betweenRight && inLeft && outRight && !betweenLeft && !inRight && !outLeft) || 
    (betweenRight && betweenLeft && !inLeft && !outRight && !inRight && !outLeft))
  {
    vtkErrorMacro("Not Margin");
    size_t firstSection = 0;
    for ( size_t i = 0; i < ids.size() - 1; ++i)
    {
      if (ids[i + 1] - ids[i] != 1)
      {
        firstSection = i;
        break;
      }
    }
    for ( size_t i = 0; i <= firstSection; ++i)
    {
      curveSections.first.push_back(ids[i]);
    }
    for ( size_t i = firstSection + 1; i < ids.size(); ++i)
    {
      curveSections.second.push_back(ids[i]);
    }
  }
  if (inRight && outRight && !betweenRight && !betweenLeft && !inLeft && !outLeft)
  {
    vtkErrorMacro("Margin left");
    for ( int i = 0; i < ids.size(); ++i)
    {
      curveSections.first.push_back(ids[i]);
    }
  }
  if (inLeft && outLeft && !betweenLeft && !betweenRight && !outRight && !inRight)
  {
    vtkErrorMacro("Margin right");
    for ( int i = 0; i < ids.size(); ++i)
    {
      curveSections.first.push_back(ids[i]);
    }
  }

  vtkErrorMacro("Section first");
  for ( int v : curveSections.first)
  {
    vtkErrorMacro("Ids: " << v);
  }

  vtkErrorMacro("Section second");
  for ( int v : curveSections.second)
  {
    vtkErrorMacro("Ids: " << v);
  }

  return true;
}
