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

// STD includes
#include <iostream>
#include <fstream>
#include <cassert>
#include <bitset>
#include <limits>

namespace
{

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
void
vtkSlicerLoadableModuleTemplateLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}
