/*==============================================================================

  Copyright (c) Radiation Medicine Program, University Health Network,
  Princess Margaret Hospital, Toronto, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kevin Wang, Princess Margaret Cancer Centre
  and was supported by Cancer Care Ontario (CCO)'s ACRU program
  with funds provided by the Ontario Ministry of Health and Long-Term Care
  and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).

==============================================================================*/

// Beams includes
#include "vtkMRMLRTCabin26AIonBeamNode.h"
#include "vtkMRMLRTPlanNode.h"

// SlicerRT includes
#include "vtkSlicerRtCommon.h"

// MRML includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLTableNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLSubjectHierarchyConstants.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkIntArray.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkDoubleArray.h>
#include <vtkTable.h>
#include <vtkCellArray.h>
#include <vtkAppendPolyData.h>
//------------------------------------------------------------------------------
namespace
{

const char* const SCANSPOT_REFERENCE_ROLE = "ScanSpotRef";
double FWHM_TO_SIGMA = 1. / (2. * sqrt(2. * log(2.)));

} // namespace

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLRTCabin26AIonBeamNode);

//----------------------------------------------------------------------------
vtkMRMLRTCabin26AIonBeamNode::vtkMRMLRTCabin26AIonBeamNode()
  :
  Superclass(),
  IsocenterToMlcLayer1Distance(vtkMRMLRTIonBeamNode::IsocenterToMultiLeafCollimatorDistance),
  IsocenterToMlcLayer2Distance(IsocenterToMlcLayer1Distance)
{
  this->IsocenterToMlcLayer1Distance = 2500.;
  this->IsocenterToMlcLayer2Distance = 2500.;
}

//----------------------------------------------------------------------------
vtkMRMLRTCabin26AIonBeamNode::~vtkMRMLRTCabin26AIonBeamNode()
{
  this->SetBeamDescription(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLRTCabin26AIonBeamNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLFloatMacro( isocenterToMlcLayer1Distance, IsocenterToMlcLayer1Distance);
  vtkMRMLWriteXMLFloatMacro( isocenterToMlcLayer2Distance, IsocenterToMlcLayer2Distance);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTCabin26AIonBeamNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
  
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLFloatMacro( isocenterToMlcLayer1Distance, IsocenterToMlcLayer1Distance);
  vtkMRMLReadXMLFloatMacro( isocenterToMlcLayer2Distance, IsocenterToMlcLayer2Distance);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLRTCabin26AIonBeamNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  // Do not call Copy function of the direct model base class, as it copies the poly data too,
  // which is undesired for beams, as they generate their own poly data from their properties.
  vtkMRMLDisplayableNode::Copy(anode);

  vtkMRMLRTCabin26AIonBeamNode* node = vtkMRMLRTCabin26AIonBeamNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Create transform node for beam
  this->CreateNewBeamTransformNode();

  // Add beam in the same plan if beam nodes are in the same scene
  vtkMRMLRTPlanNode* planNode = node->GetParentPlanNode();
  if (planNode && node->GetScene() == this->Scene)
  {
    planNode->AddBeam(this);
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyIntMacro(BeamNumber);
  vtkMRMLCopyStringMacro(BeamDescription);
  vtkMRMLCopyFloatMacro(BeamWeight);
  vtkMRMLCopyFloatMacro(X1Jaw);
  vtkMRMLCopyFloatMacro(X2Jaw);
  vtkMRMLCopyFloatMacro(Y1Jaw);
  vtkMRMLCopyFloatMacro(Y2Jaw);
  vtkMRMLCopyFloatMacro(VSADx);
  vtkMRMLCopyFloatMacro(VSADy);
  vtkMRMLCopyFloatMacro(IsocenterToJawsDistanceX);
  vtkMRMLCopyFloatMacro(IsocenterToJawsDistanceY);
  vtkMRMLCopyFloatMacro(IsocenterToMlcLayer1Distance);
  vtkMRMLCopyFloatMacro(IsocenterToMlcLayer2Distance);
  vtkMRMLCopyFloatMacro(IsocenterToRangeShifterDistance);
  vtkMRMLCopyVectorMacro(ScanningSpotSize, float, 2);
  vtkMRMLCopyFloatMacro(GantryAngle);
  vtkMRMLCopyFloatMacro(CollimatorAngle);
  vtkMRMLCopyFloatMacro(CouchAngle);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
  
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLRTCabin26AIonBeamNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent( anode, deepCopy);

  vtkMRMLRTCabin26AIonBeamNode* node = vtkMRMLRTCabin26AIonBeamNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyIntMacro(BeamNumber);
  vtkMRMLCopyStringMacro(BeamDescription);
  vtkMRMLCopyFloatMacro(BeamWeight);
  vtkMRMLCopyFloatMacro(X1Jaw);
  vtkMRMLCopyFloatMacro(X2Jaw);
  vtkMRMLCopyFloatMacro(Y1Jaw);
  vtkMRMLCopyFloatMacro(Y2Jaw);
  vtkMRMLCopyFloatMacro(VSADx);
  vtkMRMLCopyFloatMacro(VSADy);
  vtkMRMLCopyFloatMacro(IsocenterToJawsDistanceX);
  vtkMRMLCopyFloatMacro(IsocenterToJawsDistanceY);
  vtkMRMLCopyFloatMacro(IsocenterToMlcLayer1Distance);
  vtkMRMLCopyFloatMacro(IsocenterToMlcLayer2Distance);
  vtkMRMLCopyFloatMacro(IsocenterToRangeShifterDistance);
  vtkMRMLCopyVectorMacro(ScanningSpotSize, float, 2);
  vtkMRMLCopyFloatMacro(GantryAngle);
  vtkMRMLCopyFloatMacro(CollimatorAngle);
  vtkMRMLCopyFloatMacro(CouchAngle);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTCabin26AIonBeamNode::SetScene(vtkMRMLScene* scene)
{
  Superclass::SetScene(scene);
}

//----------------------------------------------------------------------------
void vtkMRMLRTCabin26AIonBeamNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintFloatMacro(IsocenterToMlcLayer1Distance);
  vtkMRMLPrintFloatMacro(IsocenterToMlcLayer2Distance);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLRTCabin26AIonBeamNode::SetIsocenterToMlcLayer1Distance(double distance)
{
  this->IsocenterToMlcLayer1Distance = distance;
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLRTBeamNode::BeamGeometryModified);
}

//----------------------------------------------------------------------------
void vtkMRMLRTCabin26AIonBeamNode::SetIsocenterToMlcLayer2Distance(double distance)
{
  this->IsocenterToMlcLayer2Distance = distance;
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLRTBeamNode::BeamGeometryModified);
}

//---------------------------------------------------------------------------
void vtkMRMLRTCabin26AIonBeamNode::CreateBeamPolyData(vtkPolyData* beamModelPolyData/*=nullptr*/)
{
  if (!beamModelPolyData)
  {
    beamModelPolyData = this->GetPolyData();
  }
  if (!beamModelPolyData)
  {
    vtkErrorMacro("CreateBeamPolyData: Invalid beam node");
    return;
  }

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();

  // Scan spot position map & meterset weights
  vtkMRMLTableNode* scanSpotTableNode = this->GetScanSpotTableNode();
  if (scanSpotTableNode)
  {
    vtkDebugMacro("CreateBeamPolyData: Valid scan spot parameters table node");
  }
  else
  {
    vtkWarningMacro("CreateBeamPolyData: Invalid or absent table node with " \
      "scan spot parameters for a node " 
      << "\"" << this->GetName() << "\"");
  }

  vtkMRMLTableNode* mlcTableNode = this->GetMultiLeafCollimatorTableNode();

  vtkIdType nofLeafPairs = 0;
  if (mlcTableNode)
  {
    nofLeafPairs = mlcTableNode->GetNumberOfRows() - 1;
    if (nofLeafPairs <= 0)
    {
      vtkWarningMacro("CreateBeamPolyData: Wrong number of leaf pairs in the MLC table node, " \
        "beam model poly data will be created without MLC");
      mlcTableNode = nullptr; // draw beam polydata without MLC
      nofLeafPairs = 0;
    }
  }

  // valid MLC node
  if (nofLeafPairs > 0)
  {
    // MLC boundary and position data
    if (mlcTableNode->GetNumberOfColumns() == 3)
    {
      vtkDebugMacro("CreateBeamPolyData: Single layer MLC table node is present, number of leaf pairs = " << nofLeafPairs);
    }
    // MLC boundary and position data
    else if (mlcTableNode->GetNumberOfColumns() == 6)
    {
      vtkDebugMacro("CreateBeamPolyData: Double layer MLC table node is present, number of leaf pairs = " << nofLeafPairs);
    }
    else
    {
      vtkWarningMacro("CreateBeamPolyData: Wrong number of columns in the MLC table node, " \
        "beam model poly data will be created without MLC");
      mlcTableNode = nullptr; // draw beam polydata without MLC
    }
  }

  bool xOpened = !vtkSlicerRtCommon::AreEqualWithTolerance( this->X2Jaw, this->X1Jaw);
  bool yOpened = !vtkSlicerRtCommon::AreEqualWithTolerance( this->Y2Jaw, this->Y1Jaw);

  // Scanning spot beam
  if (scanSpotTableNode)
  {
    std::vector<double> positionX, positionY;
    vtkIdType rows = scanSpotTableNode->GetNumberOfRows();
    positionX.resize(rows);
    positionY.resize(rows);
    // copy scan scot map data for processing
    for (vtkIdType row = 0; row < rows; row++)
    {
      vtkTable* table = scanSpotTableNode->GetTable();
      positionX[row] = table->GetValue( row, 0).ToDouble();
      positionY[row] = table->GetValue( row, 1).ToDouble();
    }
    double beamTopCap = std::min( this->VSADx, this->VSADy);
    double beamBottomCap = -beamTopCap;

    double M1x = (this->VSADx - beamTopCap) / this->VSADx;
    double M1y = (this->VSADy - beamTopCap) / this->VSADy;

    double M2x = (this->VSADx + beamTopCap) / this->VSADx;
    double M2y = (this->VSADy + beamTopCap) / this->VSADy;

    double sigmaX = this->ScanningSpotSize[0] * FWHM_TO_SIGMA;
    double sigmaY = this->ScanningSpotSize[1] * FWHM_TO_SIGMA;

    double borderMinX = *std::min_element( positionX.begin(), positionX.end());
    double borderMaxX = *std::max_element( positionX.begin(), positionX.end());
    double borderMinY = *std::min_element( positionY.begin(), positionY.end());
    double borderMaxY = *std::max_element( positionY.begin(), positionY.end());

    double sigmaX1 = M1x * sigmaX;
    double sigmaY1 = M1y * sigmaY;

    double sigmaX2 = M2x * sigmaX;
    double sigmaY2 = M2y * sigmaY;

    // beam begin cap
    points->InsertPoint( 0, borderMinX - sigmaX1, borderMinY - sigmaY1, beamTopCap);
    points->InsertPoint( 1, borderMinX - sigmaX1, borderMaxY + sigmaY1, beamTopCap);
    points->InsertPoint( 2, borderMaxX + sigmaX1, borderMaxY + sigmaY1, beamTopCap);
    points->InsertPoint( 3, borderMaxX + sigmaX1, borderMinY - sigmaY1, beamTopCap);

    // beam end cap
    points->InsertPoint( 4, borderMinX - sigmaX2, borderMinY - sigmaY2, beamBottomCap);
    points->InsertPoint( 5, borderMinX - sigmaX2, borderMaxY + sigmaY2, beamBottomCap);
    points->InsertPoint( 6, borderMaxX + sigmaX2, borderMaxY + sigmaY2, beamBottomCap);
    points->InsertPoint( 7, borderMaxX + sigmaX2, borderMinY - sigmaY2, beamBottomCap);

    // Add the cap to the top
    cellArray->InsertNextCell(4);
    cellArray->InsertCellPoint(0);
    cellArray->InsertCellPoint(1);
    cellArray->InsertCellPoint(2);
    cellArray->InsertCellPoint(3);

    // Side polygon 1
    cellArray->InsertNextCell(4);
    cellArray->InsertCellPoint(0);
    cellArray->InsertCellPoint(4);
    cellArray->InsertCellPoint(5);
    cellArray->InsertCellPoint(1);

    // Side polygon 2
    cellArray->InsertNextCell(4);
    cellArray->InsertCellPoint(1);
    cellArray->InsertCellPoint(5);
    cellArray->InsertCellPoint(6);
    cellArray->InsertCellPoint(2);

    // Side polygon 3
    cellArray->InsertNextCell(4);
    cellArray->InsertCellPoint(2);
    cellArray->InsertCellPoint(6);
    cellArray->InsertCellPoint(7);
    cellArray->InsertCellPoint(3);

    // Side polygon 4
    cellArray->InsertNextCell(4);
    cellArray->InsertCellPoint(3);
    cellArray->InsertCellPoint(7);
    cellArray->InsertCellPoint(4);
    cellArray->InsertCellPoint(0);

    // Add the cap to the bottom
    cellArray->InsertNextCell(4);
    cellArray->InsertCellPoint(4);
    cellArray->InsertCellPoint(5);
    cellArray->InsertCellPoint(6);
    cellArray->InsertCellPoint(7);

    beamModelPolyData->SetPoints(points);
    beamModelPolyData->SetPolys(cellArray);
    return;
  }
  else if (mlcTableNode && (mlcTableNode->GetNumberOfColumns() == 3) && xOpened && yOpened) // single layer MLC with opened Jaws
  {
    bool polydataAppended = false;
    MLCBoundaryPositionVector mlc;

    const char* mlcName = mlcTableNode->GetName();
    bool typeMLCX = !strncmp( "MLCX", mlcName, strlen("MLCX")); // MLCX by default
    bool typeMLCY = !strncmp( "MLCY", mlcName, strlen("MLCY"));
    if (typeMLCY && !typeMLCX)
    {
      typeMLCX = false;
    }

    // copy MLC data for easier processing
    for (vtkIdType leafPair = 0; leafPair < nofLeafPairs; leafPair++)
    {
      vtkTable* table = mlcTableNode->GetTable();
      double boundBegin = table->GetValue( leafPair, 0).ToDouble();
      double boundEnd = table->GetValue( leafPair + 1, 0).ToDouble();
      double pos1 = table->GetValue( leafPair, 1).ToDouble();
      double pos2 = table->GetValue( leafPair, 2).ToDouble();

      mlc.push_back({ boundBegin, boundEnd, pos1, pos2 });
    }
    CreateMlcBoundaryPositionVector(typeMLCX, this->IsocenterToMlcLayer1Distance, mlc);

    double jawBegin = this->Y1Jaw;
    double jawEnd = this->Y2Jaw;

    auto firstLeafIterator = mlc.end();
    auto lastLeafIterator = mlc.end();

    if (typeMLCX) // MLCX
    {
      double jawBegin = this->Y1Jaw * this->VSADy / (this->VSADy - this->IsocenterToMlcLayer1Distance);
      double jawEnd = this->Y1Jaw * this->VSADy / (this->VSADy - this->IsocenterToMlcLayer1Distance);
    }
    else // MLCY
    {
      jawBegin = this->X1Jaw * this->VSADx / (this->VSADx - this->IsocenterToMlcLayer1Distance);
      jawEnd = this->X1Jaw * this->VSADx / (this->VSADx - this->IsocenterToMlcLayer1Distance);
    }

    // find first and last opened leaves visible within jaws
    // and fill sections vector for further processing
    MLCSectionVector sections; // sections (first & last leaf iterator) of opened MLC
    for (auto it = mlc.begin(); it != mlc.end(); ++it)
    {
      double& bound1 = (*it)[0]; // leaf pair boundary begin
      double& bound2 = (*it)[1]; // leaf pair boundary end
      double& pos1 = (*it)[2]; // leaf position "1"
      double& pos2 = (*it)[3]; // leaf position "2"
      // if leaf pair is outside the jaws, then it is closed
      bool mlcOpened = (bound2 < jawBegin || bound1 > jawEnd) ? false : !vtkSlicerRtCommon::AreEqualWithTolerance( pos1, pos2);
      bool withinJaw = false;
      if (typeMLCX) // MLCX
      {
        withinJaw = ((pos1 < this->X1Jaw && pos2 >= this->X1Jaw && pos2 <= this->X2Jaw) || 
          (pos1 >= this->X1Jaw && pos1 <= this->X2Jaw && pos2 > this->X2Jaw) || 
          (pos1 <= this->X1Jaw && pos2 >= this->X2Jaw) || 
          (pos1 >= this->X1Jaw && pos1 <= this->X2Jaw && 
            pos2 >= this->X1Jaw && pos2 <= this->X2Jaw));
      }
      else // MLCY
      {
        withinJaw = ((pos1 < this->Y1Jaw && pos2 >= this->Y1Jaw && pos2 <= this->Y2Jaw) || 
          (pos1 >= this->Y1Jaw && pos1 <= this->Y2Jaw && pos2 > this->Y2Jaw) || 
          (pos1 <= this->Y1Jaw && pos2 >= this->Y2Jaw) || 
          (pos1 >= this->Y1Jaw && pos1 <= this->Y2Jaw && 
            pos2 >= this->Y1Jaw && pos2 <= this->Y2Jaw));
      }

      if (withinJaw && mlcOpened && firstLeafIterator == mlc.end())
      {
        firstLeafIterator = it;
      }
      if (withinJaw && mlcOpened && firstLeafIterator != mlc.end())
      {
        lastLeafIterator = it;
      }
      // check if next leaf pair is not the last
      if (it != mlc.end() - 1)
      {
        auto next_it = it + 1;
        double& next_pos1 = (*next_it)[2]; // position "1" of the next leaf
        double& next_pos2 = (*next_it)[3]; // position "2" of the next leaf
        // if there is no open space between neighbors => start new section
        if (mlcOpened && (next_pos1 > pos2 || next_pos2 < pos1))
        {
          mlcOpened = false;
        }
      }
      if (firstLeafIterator != mlc.end() && lastLeafIterator != mlc.end() && !mlcOpened)
      {
        sections.push_back({ firstLeafIterator, lastLeafIterator});
        firstLeafIterator = mlc.end();
        lastLeafIterator = mlc.end();
      }
    }

    if (!sections.size()) // no visible sections
    {
      vtkErrorMacro("CreateBeamPolyData: Unable to calculate MLC visible data");
      return;
    }

    // append one or more visible sections to beam model poly data
    vtkNew<vtkAppendPolyData> append;
    for (const MLCSectionVector::value_type& section : sections)
    {
      if (section.first != mlc.end() && section.second != mlc.end())
      {
        vtkNew<vtkPolyData> beamPolyData;
        vtkNew<vtkPoints> points;
        vtkNew<vtkCellArray> cellArray;

        MLCVisiblePointVector side12; // real points for side "1" and "2"
        CreateMLCPointsFromSectionBorder( jawBegin, jawEnd, typeMLCX, section, side12);

        // fill vtk points
        double cy1 = (this->VSADy + this->IsocenterToMultiLeafCollimatorDistance) / this->VSADy;
        double cx1 = (this->VSADx + this->IsocenterToMultiLeafCollimatorDistance) / this->VSADx;
        double cy = (this->VSADy - this->IsocenterToMultiLeafCollimatorDistance) / this->VSADy;
        double cx = (this->VSADx - this->IsocenterToMultiLeafCollimatorDistance) / this->VSADx;

        // side "1" and "2" points vector
        vtkIdType pointIds = 0;
        // points on MLC side
        vtkIdType nofPoints = side12.size();
        for (const MLCVisiblePointVector::value_type& point : side12)
        {
          const double& x = point.first;
          const double& y = point.second;
          double x_ = x * cx;
          double y_ = y * cy;
          points->InsertPoint( pointIds++, x_, y_, this->IsocenterToMultiLeafCollimatorDistance);
        }

        // points on the opposite (from isocenter) side
        for (const MLCVisiblePointVector::value_type& point : side12)
        {
          const double& x = point.first;
          const double& y = point.second;

          double x_ = x * cx1;
          double y_ = y * cy1;
          points->InsertPoint( pointIds++, x_, y_, -this->IsocenterToMultiLeafCollimatorDistance);
        }
        side12.clear(); // doesn't need anymore

        // fill cell array for side "1" and "2"
        for (vtkIdType i = 0; i < nofPoints - 1; ++i)
        {
          cellArray->InsertNextCell(4);
          cellArray->InsertCellPoint(i);
          cellArray->InsertCellPoint(i + 1);
          cellArray->InsertCellPoint(i + nofPoints + 1);
          cellArray->InsertCellPoint(i + nofPoints);
        }

        // fill cell connection between side "2" -> side "1"
        cellArray->InsertNextCell(4);
        cellArray->InsertCellPoint(0);
        cellArray->InsertCellPoint(nofPoints);
        cellArray->InsertCellPoint(2 * nofPoints - 1);
        cellArray->InsertCellPoint(nofPoints - 1);

        // Add the cap to the bottom
        cellArray->InsertNextCell(nofPoints);
        for (vtkIdType i = 0; i < nofPoints; i++)
        {
          cellArray->InsertCellPoint(i);
        }

        // Add the cap to the top
        cellArray->InsertNextCell(nofPoints);
        for (vtkIdType i = nofPoints; i < 2 * nofPoints; i++)
        {
          cellArray->InsertCellPoint(i);
        }

        beamPolyData->SetPoints(points);
        beamPolyData->SetPolys(cellArray);

        // append section to form final polydata
        append->AddInputData(beamPolyData);
        polydataAppended = true;
      }
    }
    if (polydataAppended)
    {
      append->Update();
      beamModelPolyData->DeepCopy(append->GetOutput());
      return;
    }
  }
  else if (mlcTableNode && (mlcTableNode->GetNumberOfColumns() == 6) && xOpened && yOpened) // double layer MLC with opened Jaws
  {
    bool polydataAppended = false;
    MLCBoundaryPositionVector mlcLayer1, mlcLayer2;

    const char* mlcName = mlcTableNode->GetName();
    bool typeMLCX = !strncmp( "MLCX", mlcName, strlen("MLCX")); // MLCX by default
    bool typeMLCY = !strncmp( "MLCY", mlcName, strlen("MLCY"));
    if (typeMLCY && !typeMLCX)
    {
      typeMLCX = false;
    }

    // copy MLC data for easier processing
    for (vtkIdType leafPair = 0; leafPair < nofLeafPairs; leafPair++)
    {
      vtkTable* table = mlcTableNode->GetTable();
      // Layer-1
      double boundBegin = table->GetValue( leafPair, 0).ToDouble();
      double boundEnd = table->GetValue( leafPair + 1, 0).ToDouble();
      double pos1 = table->GetValue( leafPair, 1).ToDouble();
      double pos2 = table->GetValue( leafPair, 2).ToDouble();
      mlcLayer1.push_back({ boundBegin, boundEnd, pos1, pos2 });

      // Layer-2
      boundBegin = table->GetValue( leafPair, 3).ToDouble();
      boundEnd = table->GetValue( leafPair + 1, 3).ToDouble();
      pos1 = table->GetValue( leafPair, 4).ToDouble();
      pos2 = table->GetValue( leafPair, 5).ToDouble();
      mlcLayer2.push_back({ boundBegin, boundEnd, pos1, pos2 });
    }
    
    return;
  }

  // Default beam polydata (symmetric or asymmetric jaws, no ScanSpot, no MLC)
  double beamTopCap = std::min( IsocenterToJawsDistanceX, IsocenterToJawsDistanceY);
  double beamBottomCap = -beamTopCap;

  double Mx = (this->VSADx - beamTopCap) / this->VSADx;
  double My = (this->VSADy - beamTopCap) / this->VSADy;
  double Mx1 = (this->VSADx + beamTopCap) / this->VSADx;
  double My1 = (this->VSADy + beamTopCap) / this->VSADy;

  // beam begin cap
  points->InsertPoint( 0, Mx * this->X1Jaw, My * this->Y1Jaw, beamTopCap);
  points->InsertPoint( 1, Mx * this->X1Jaw, My * this->Y2Jaw, beamTopCap);
  points->InsertPoint( 2, Mx * this->X2Jaw, My * this->Y2Jaw, beamTopCap);
  points->InsertPoint( 3, Mx * this->X2Jaw, My * this->Y1Jaw, beamTopCap);

  // beam end cap
  points->InsertPoint( 4, Mx1 * this->X1Jaw, My1 * this->Y1Jaw, beamBottomCap);
  points->InsertPoint( 5, Mx1 * this->X1Jaw, My1 * this->Y2Jaw, beamBottomCap);
  points->InsertPoint( 6, Mx1 * this->X2Jaw, My1 * this->Y2Jaw, beamBottomCap);
  points->InsertPoint( 7, Mx1 * this->X2Jaw, My1 * this->Y1Jaw, beamBottomCap);

  // Add the cap to the top
  cellArray->InsertNextCell(4);
  cellArray->InsertCellPoint(0);
  cellArray->InsertCellPoint(1);
  cellArray->InsertCellPoint(2);
  cellArray->InsertCellPoint(3);

  // Side polygon 1
  cellArray->InsertNextCell(4);
  cellArray->InsertCellPoint(0);
  cellArray->InsertCellPoint(4);
  cellArray->InsertCellPoint(5);
  cellArray->InsertCellPoint(1);

  // Side polygon 2
  cellArray->InsertNextCell(4);
  cellArray->InsertCellPoint(1);
  cellArray->InsertCellPoint(5);
  cellArray->InsertCellPoint(6);
  cellArray->InsertCellPoint(2);

  // Side polygon 3
  cellArray->InsertNextCell(4);
  cellArray->InsertCellPoint(2);
  cellArray->InsertCellPoint(6);
  cellArray->InsertCellPoint(7);
  cellArray->InsertCellPoint(3);

  // Side polygon 4
  cellArray->InsertNextCell(4);
  cellArray->InsertCellPoint(3);
  cellArray->InsertCellPoint(7);
  cellArray->InsertCellPoint(4);
  cellArray->InsertCellPoint(0);

  // Add the cap to the bottom
  cellArray->InsertNextCell(4);
  cellArray->InsertCellPoint(4);
  cellArray->InsertCellPoint(5);
  cellArray->InsertCellPoint(6);
  cellArray->InsertCellPoint(7);

  beamModelPolyData->SetPoints(points);
  beamModelPolyData->SetPolys(cellArray);
}

//---------------------------------------------------------------------------
bool vtkMRMLRTCabin26AIonBeamNode::CreateMlcBoundaryPositionVector(bool typeMLCX,
  double isocenterToMlcDistance,
  MLCBoundaryPositionVector& boundaryPositionVector)
{
  // MLCBoundaryPositionVector { boundaryBegin, boundaryEnd, posSide1, posSide2 }
  for (auto& boundaryPosition : boundaryPositionVector)
  {
    if (!typeMLCX) // MLCY
    {
      double boundaryBegin = boundaryPosition[0] * this->VSADx / (this->VSADx - isocenterToMlcDistance);
      double boundaryEnd = boundaryPosition[1] * this->VSADx / (this->VSADx - isocenterToMlcDistance);
      double posSide1 = boundaryPosition[2] * this->VSADy / (this->VSADy - isocenterToMlcDistance);
      double posSide2 = boundaryPosition[3] * this->VSADy / (this->VSADy - isocenterToMlcDistance);
      boundaryPosition[0] = boundaryBegin;
      boundaryPosition[1] = boundaryEnd;
      boundaryPosition[2] = posSide1;
      boundaryPosition[3] = posSide2;
    }
    else // MLCX
    {
      double boundaryBegin = boundaryPosition[0] * this->VSADy / (this->VSADy - isocenterToMlcDistance);
      double boundaryEnd = boundaryPosition[1] * this->VSADy / (this->VSADy - isocenterToMlcDistance);
      double posSide1 = boundaryPosition[2] * this->VSADx / (this->VSADx - isocenterToMlcDistance);
      double posSide2 = boundaryPosition[3] * this->VSADx / (this->VSADx - isocenterToMlcDistance);
      boundaryPosition[0] = boundaryBegin;
      boundaryPosition[1] = boundaryEnd;
      boundaryPosition[2] = posSide1;
      boundaryPosition[3] = posSide2;
    }
  }
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLRTCabin26AIonBeamNode::CreateMlcBoundaryVector(bool typeMLCX,
  double isocenterToMlcDistance,
  MLCBoundaryVector& boundaryVector)
{
  // MLCBoundaryVector { boundaryBegin, boundaryEnd }
  for (auto& boundary : boundaryVector)
  {
    if (!typeMLCX)
    {
      double boundaryBegin = boundary[0] * this->VSADx / (this->VSADx - isocenterToMlcDistance);
      double boundaryEnd = boundary[1] * this->VSADx / (this->VSADx - isocenterToMlcDistance);
      boundary[0] = boundaryBegin;
      boundary[1] = boundaryEnd;
    }
    else
    {
      double boundaryBegin = boundary[0] * this->VSADy / (this->VSADy - isocenterToMlcDistance);
      double boundaryEnd = boundary[1] * this->VSADy / (this->VSADy - isocenterToMlcDistance);
      boundary[0] = boundaryBegin;
      boundary[1] = boundaryEnd;
    }
  }
  return true;
}
