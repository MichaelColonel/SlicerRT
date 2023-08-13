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

// IhepMlcControl Logic includes
#include "vtkSlicerIhepMlcControlLogic.h"

// SlicerRT Beams MRML includes
#include <vtkMRMLRTBeamNode.h>

// SlicerRT Beams Logic includes
#include <vtkSlicerBeamsModuleLogic.h>
#include <vtkSlicerMLCPositionLogic.h>

// Subject Hierarchy includes
#include <vtkMRMLSubjectHierarchyConstants.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkWeakPointer.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTableNode.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkTable.h>
#include <vtkDoubleArray.h>

// RapidJSON includes
#include <rapidjson/document.h>     // rapidjson's DOM-style API
#include <rapidjson/filereadstream.h>

// STD includes
#include <memory>

namespace
{

const char* MLCX_BOUNDARYANDPOSITION = "MLCX_BoundaryAndPosition";
const char* MLCY_BOUNDARYANDPOSITION = "MLCY_BoundaryAndPosition";

} // namespace

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerIhepMlcControlLogic);
//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkSlicerIhepMlcControlLogic, BeamsLogic, vtkSlicerBeamsModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerIhepMlcControlLogic::vtkSlicerIhepMlcControlLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerIhepMlcControlLogic::~vtkSlicerIhepMlcControlLogic()
{
  this->SetBeamsLogic(nullptr);
}

//----------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
  }
//  if (!scene->IsNodeClassRegistered("vtkMRMLRTPlanNode"))
//  {
//    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTPlanNode>::New());
//  }
//  if (!scene->IsNodeClassRegistered("vtkMRMLRTBeamNode"))
//  {
//    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTBeamNode>::New());
//  }
  if (!scene->IsNodeClassRegistered("vtkMRMLIhepMlcControlNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLIhepMlcControlNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::UpdateFromMRMLScene()
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateFromMRMLScene: Invalid MRML scene");
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
  }

//  if (node->IsA("vtkMRMLRTBeamNode"))
//  {
//    // Observe beam events
//    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
//    events->InsertNextValue(vtkMRMLRTBeamNode::BeamGeometryModified);
//    events->InsertNextValue(vtkMRMLRTBeamNode::BeamTransformModified);
//    vtkObserveMRMLNodeEventsMacro(node, events);
//  }
  if (node->IsA("vtkMRMLIhepMlcControlNode"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneEndImport: Invalid MRML scene");
    return;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepMlcControlLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  Superclass::ProcessMRMLNodesEvents(caller, event, callData);

  vtkMRMLScene* mrmlScene = this->GetMRMLScene();
  if (!mrmlScene)
  {
    vtkErrorMacro("ProcessMRMLNodesEvents: Invalid MRML scene");
    return;
  }
  if (mrmlScene->IsBatchProcessing())
  {
    return;
  }

//  if (caller->IsA("vtkMRMLRTBeamNode"))
//  {
//    vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(caller);
//    if (event == vtkMRMLRTBeamNode::BeamTransformModified)
//    {
//    }
//    else if (event == vtkMRMLRTBeamNode::BeamGeometryModified)
//    {
//    }
//  }
  if (caller->IsA("vtkMRMLIhepMlcControlNode"))
  {
//    vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(caller);

    if (event == vtkCommand::ModifiedEvent)
    {
      // Update parameters using beam node data and create/update markups transform if they weren't created/updated
//      vtkWarningMacro("ProcessMRMLNodesEvents: Process IhepMLC events in logic");
    }
  }
}

//---------------------------------------------------------------------------
vtkMRMLTableNode* vtkSlicerIhepMlcControlLogic::CreateMlcTableNodeBoundaryData(vtkMRMLIhepMlcControlNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateMlcTableNodeBoundaryData: Scene node is invalid");
    return nullptr;
  }
  if (!parameterNode || (parameterNode && !parameterNode->GetBeamNode()))
  {
    vtkErrorMacro("CreateMlcTableNodeBoundaryData: Parameter or beam node (or both) are invalid");
    return nullptr;
  }
/*
  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();

  if (vtkMRMLTableNode* tableNode = beamNode->GetMultiLeafCollimatorTableNode())
  {
    // update
    if (this->UpdateMlcTableNodeBoundaryData(parameterNode, tableNode))
    {
      return tableNode;
    }
    else
    {
      vtkErrorMacro("CreateMlcTableNodeBoundaryData: Unable to update MLC table node");
      return nullptr;
    }
  }
*/
  vtkMRMLIhepMlcControlNode::OrientationType orientation = parameterNode->GetOrientation();
  const char* name = nullptr;
  switch (orientation)
  {
  case vtkMRMLIhepMlcControlNode::X:
    name = MLCX_BOUNDARYANDPOSITION;
    break;
  case vtkMRMLIhepMlcControlNode::Y:
    name = MLCY_BOUNDARYANDPOSITION;
    break;
  default:
    break;
  }
  if (!name)
  {
    vtkErrorMacro("CreateMlcTableNodeBoundaryData: MLC orientation name is invalid");
    return nullptr;
  }

  vtkMRMLTableNode* tableNode = vtkMRMLTableNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTableNode", name));

  vtkTable* table = tableNode->GetTable();
  if (!table)
  {
    vtkErrorMacro("CreateMultiLeafCollimatorTableNodeBoundaryData: Unable to create vtkTable to fill MLC data");
    return nullptr;
  }

  vtkMRMLIhepMlcControlNode::LayersType nofLayers = parameterNode->GetLayers();
  
  // Layer-1
  std::vector<double> leafPairsBoundary1(parameterNode->GetNumberOfLeafPairs() + 1);
  // Layer-2
  std::vector<double> leafPairsBoundary2;
  double middle = parameterNode->GetPairOfLeavesSize() * parameterNode->GetNumberOfLeafPairs() / 2.;
  for (auto iter = leafPairsBoundary1.begin(); iter != leafPairsBoundary1.end(); ++iter)
  {
    size_t pos = iter - leafPairsBoundary1.begin();
    double leafBoundary = -middle + parameterNode->GetIsocenterOffset() + pos * parameterNode->GetPairOfLeavesSize();
    if (!parameterNode->GetParallelBeam()) // correction for non parallel beam
    {
      // put correction here
    }
    *iter = leafBoundary;
  }
  if (nofLayers == vtkMRMLIhepMlcControlNode::TwoLayers)
  {
    if (parameterNode->GetParallelBeam()) // parallel beam is simple
    {
      for(double pairOfLevesBoundary : leafPairsBoundary1)
      {
        leafPairsBoundary2.push_back(pairOfLevesBoundary + parameterNode->GetOffsetBetweenTwoLayers());
      }
    }
    else // correction for non parallel beam for layer-2
    {
      for (auto iter = leafPairsBoundary1.begin(); iter != leafPairsBoundary1.end(); ++iter)
      {
        size_t pos = iter - leafPairsBoundary1.begin();
        double leafBoundary = -middle + parameterNode->GetIsocenterOffset() + pos * parameterNode->GetPairOfLeavesSize();
        leafBoundary += + parameterNode->GetOffsetBetweenTwoLayers();
        // put correction here
        *iter = leafBoundary;
      }
    }
  }

  if (nofLayers == vtkMRMLIhepMlcControlNode::TwoLayers)
  {
    // Column 0; Leaf pair boundary values for layer-1
    vtkNew<vtkDoubleArray> boundaryLayer1;
    boundaryLayer1->SetName("Boundary-1");
    table->AddColumn(boundaryLayer1);

    // Column 1; Leaf positions on the side "1" for layer-1
    vtkNew<vtkDoubleArray> pos1Layer1;
    pos1Layer1->SetName("1-1");
    table->AddColumn(pos1Layer1);

    // Column 2; Leaf positions on the side "2" for layer-1
    vtkNew<vtkDoubleArray> pos2Layer1;
    pos2Layer1->SetName("1-2");
    table->AddColumn(pos2Layer1);

    // Column 3; Leaf pair boundary values for layer-2
    vtkNew<vtkDoubleArray> boundaryLayer2;
    boundaryLayer2->SetName("Boundary-2");
    table->AddColumn(boundaryLayer2);

    // Column 4; Leaf positions on the side "1" for layer-2
    vtkNew<vtkDoubleArray> pos1Layer2;
    pos1Layer2->SetName("2-1");
    table->AddColumn(pos1Layer2);

    // Column 5; Leaf positions on the side "2" for layer-2
    vtkNew<vtkDoubleArray> pos2Layer2;
    pos2Layer2->SetName("2-2");
    table->AddColumn(pos2Layer2);

    table->SetNumberOfRows(leafPairsBoundary1.size());
    for (size_t row = 0; row < leafPairsBoundary1.size(); ++row)
    {
      table->SetValue(row, 0, leafPairsBoundary1[row]);
    }

    for (int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
    {
      table->SetValue(row, 1, -100.0); // default meaningful value for side "1" for layer-1
      table->SetValue(row, 2, -100.0); // default meaningful value for side "2" for layer-1
    }
    table->SetValue(parameterNode->GetNumberOfLeafPairs(), 1, 0.); // side "1" set last unused value to zero
    table->SetValue(parameterNode->GetNumberOfLeafPairs(), 2, 0.); // side "2" set last unused value to zero
    tableNode->SetUseColumnNameAsColumnHeader(true);
    tableNode->SetColumnDescription( "Boundary-1", "Pair of leaves boundary for the first layer");
    tableNode->SetColumnDescription( "1-1", "Leaf position on the side \"1\" for the first layer");
    tableNode->SetColumnDescription( "1-2", "Leaf position on the side \"2\" for the first layer");

    table->SetNumberOfRows(leafPairsBoundary2.size());
    for (size_t row = 0; row < leafPairsBoundary2.size(); ++row)
    {
      table->SetValue(row, 3, leafPairsBoundary2[row]);
    }

    for (int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
    {
      table->SetValue(row, 4, -100.0); // default meaningful value for side "1" for layer-2
      table->SetValue(row, 5, -100.0); // default meaningful value for side "2" for layer-2
    }
    table->SetValue(parameterNode->GetNumberOfLeafPairs(), 4, 0.); // side "1" set last unused value to zero
    table->SetValue(parameterNode->GetNumberOfLeafPairs(), 5, 0.); // side "2" set last unused value to zero
    tableNode->SetColumnDescription( "Boundary-2", "Pair of leaves boundary for the second layer");
    tableNode->SetColumnDescription( "2-1", "Leaf position on the side \"1\" for the second layer");
    tableNode->SetColumnDescription( "2-2", "Leaf position on the side \"2\" for the second layer");
    return tableNode;
  }
  // for the one layer MLC

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

  table->SetNumberOfRows(leafPairsBoundary1.size());
  for (size_t row = 0; row < leafPairsBoundary1.size(); ++row)
  {
    table->SetValue(row, 0, leafPairsBoundary1[row]);
  }

  for (int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
  {
    table->SetValue(row, 1, -100.0); // default meaningful value for side "1"
    table->SetValue(row, 2, -100.0); // default meaningful value for side "2"
  }
  table->SetValue(parameterNode->GetNumberOfLeafPairs(), 1, 0.); // side "1" set last unused value to zero
  table->SetValue(parameterNode->GetNumberOfLeafPairs(), 2, 0.); // side "2" set last unused value to zero

  tableNode->SetUseColumnNameAsColumnHeader(true);
  tableNode->SetColumnDescription( "Boundary", "Leaf pair boundary");
  tableNode->SetColumnDescription( "1", "Leaf position on the side \"1\"");
  tableNode->SetColumnDescription( "2", "Leaf position on the side \"2\"");
  return tableNode;
}

//---------------------------------------------------------------------------
bool vtkSlicerIhepMlcControlLogic::UpdateMlcTableNodeBoundaryData(vtkMRMLIhepMlcControlNode* parameterNode, vtkMRMLTableNode* mlcTableNode)
{
  if (!parameterNode || (parameterNode && !parameterNode->GetBeamNode()))
  {
    vtkErrorMacro("UpdateMlcTableNodeBoundaryData: Parameter or beam node (or both) are invalid");
    return false;
  }

  if (!mlcTableNode)
  {
    vtkErrorMacro("UpdateMlcTableNodeBoundaryData: Table node is invalid");
    return false;
  }

  vtkTable* table = mlcTableNode->GetTable();
  if (!table)
  {
    vtkErrorMacro("UpdateMlcTableNodeBoundaryData: Unable to get vtkTable to update MLC data");
    return false;
  }

  vtkMRMLIhepMlcControlNode::OrientationType orientation = parameterNode->GetOrientation();
  const char* name = nullptr;
  switch (orientation)
  {
  case vtkMRMLIhepMlcControlNode::X:
    name = MLCX_BOUNDARYANDPOSITION;
    break;
  case vtkMRMLIhepMlcControlNode::Y:
    name = MLCY_BOUNDARYANDPOSITION;
    break;
  default:
    break;
  }
  if (!name)
  {
    vtkErrorMacro("UpdateMlcTableNodeBoundaryData: MLC orientation name is invalid");
    return false;
  }

  vtkMRMLIhepMlcControlNode::LayersType nofLayers = parameterNode->GetLayers();
  
  // Layer-1
  std::vector<double> leafPairsBoundary1(parameterNode->GetNumberOfLeafPairs() + 1);
  // Layer-2
  std::vector<double> leafPairsBoundary2;
  double middle = parameterNode->GetPairOfLeavesSize() * parameterNode->GetNumberOfLeafPairs() / 2.;
  for (auto iter = leafPairsBoundary1.begin(); iter != leafPairsBoundary1.end(); ++iter)
  {
    size_t pos = iter - leafPairsBoundary1.begin();
    double leafBoundary = -middle + parameterNode->GetIsocenterOffset() + pos * parameterNode->GetPairOfLeavesSize();
    if (!parameterNode->GetParallelBeam()) // correction for non parallel beam
    {
      // put correction here
    }
    *iter = leafBoundary;
  }
  if (nofLayers == vtkMRMLIhepMlcControlNode::TwoLayers)
  {
    if (parameterNode->GetParallelBeam()) // parallel beam is simple
    {
      for(double pairOfLevesBoundary : leafPairsBoundary1)
      {
        leafPairsBoundary2.push_back(pairOfLevesBoundary + parameterNode->GetOffsetBetweenTwoLayers());
      }
    }
    else // correction for non parallel beam for layer-2
    {
      for (auto iter = leafPairsBoundary1.begin(); iter != leafPairsBoundary1.end(); ++iter)
      {
        size_t pos = iter - leafPairsBoundary1.begin();
        double leafBoundary = -middle + parameterNode->GetIsocenterOffset() + pos * parameterNode->GetPairOfLeavesSize();
        leafBoundary += parameterNode->GetOffsetBetweenTwoLayers();
        // put correction here
        *iter = leafBoundary;
      }
    }
  }

  if (nofLayers == vtkMRMLIhepMlcControlNode::TwoLayers)
  {
    // Column 0; Leaf pair boundary values for layer-1
    vtkNew<vtkDoubleArray> boundaryLayer1;
    boundaryLayer1->SetName("Boundary-1");
    table->AddColumn(boundaryLayer1);

    // Column 1; Leaf positions on the side "1" for layer-1
    vtkNew<vtkDoubleArray> pos1Layer1;
    pos1Layer1->SetName("1-1");
    table->AddColumn(pos1Layer1);

    // Column 2; Leaf positions on the side "2" for layer-1
    vtkNew<vtkDoubleArray> pos2Layer1;
    pos2Layer1->SetName("1-2");
    table->AddColumn(pos2Layer1);

    // Column 3; Leaf pair boundary values for layer-2
    vtkNew<vtkDoubleArray> boundaryLayer2;
    boundaryLayer2->SetName("Boundary-2");
    table->AddColumn(boundaryLayer2);

    // Column 4; Leaf positions on the side "1" for layer-2
    vtkNew<vtkDoubleArray> pos1Layer2;
    pos1Layer2->SetName("2-1");
    table->AddColumn(pos1Layer2);

    // Column 5; Leaf positions on the side "2" for layer-2
    vtkNew<vtkDoubleArray> pos2Layer2;
    pos2Layer2->SetName("2-2");
    table->AddColumn(pos2Layer2);

    table->SetNumberOfRows(leafPairsBoundary1.size());
    for (size_t row = 0; row < leafPairsBoundary1.size(); ++row)
    {
      table->SetValue(row, 0, leafPairsBoundary1[row]);
    }

    for (int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
    {
      table->SetValue(row, 1, 0.0); // default meaningful value for side "1" for layer-1
      table->SetValue(row, 2, 0.0); // default meaningful value for side "2" for layer-1
    }
    table->SetValue(parameterNode->GetNumberOfLeafPairs(), 1, 0.); // side "1" set last unused value to zero
    table->SetValue(parameterNode->GetNumberOfLeafPairs(), 2, 0.); // side "2" set last unused value to zero
    mlcTableNode->SetUseColumnNameAsColumnHeader(true);
    mlcTableNode->SetColumnDescription( "Boundary-1", "Pair of leaves boundary for the first layer");
    mlcTableNode->SetColumnDescription( "1-1", "Leaf position on the side \"1\" for the first layer");
    mlcTableNode->SetColumnDescription( "1-2", "Leaf position on the side \"2\" for the first layer");

    table->SetNumberOfRows(leafPairsBoundary2.size());
    for (size_t row = 0; row < leafPairsBoundary2.size(); ++row)
    {
      table->SetValue(row, 3, leafPairsBoundary2[row]);
    }

    for (int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
    {
      table->SetValue(row, 4, 0.0); // default meaningful value for side "1" for layer-2
      table->SetValue(row, 5, 0.0); // default meaningful value for side "2" for layer-2
    }
    table->SetValue(parameterNode->GetNumberOfLeafPairs(), 4, 0.); // side "1" set last unused value to zero
    table->SetValue(parameterNode->GetNumberOfLeafPairs(), 5, 0.); // side "2" set last unused value to zero
    mlcTableNode->SetColumnDescription( "Boundary-2", "Pair of leaves boundary for the second layer");
    mlcTableNode->SetColumnDescription( "2-1", "Leaf position on the side \"1\" for the second layer");
    mlcTableNode->SetColumnDescription( "2-2", "Leaf position on the side \"2\" for the second layer");
    return true;
  }
  // for the one layer MLC

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

  table->SetNumberOfRows(leafPairsBoundary1.size());
  for (size_t row = 0; row < leafPairsBoundary1.size(); ++row)
  {
    table->SetValue(row, 0, leafPairsBoundary1[row]);
  }

  for (int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
  {
    table->SetValue(row, 1, 0.0); // default meaningful value for side "1"
    table->SetValue(row, 2, 0.0); // default meaningful value for side "2"
  }
  table->SetValue(parameterNode->GetNumberOfLeafPairs(), 1, 0.); // side "1" set last unused value to zero
  table->SetValue(parameterNode->GetNumberOfLeafPairs(), 2, 0.); // side "2" set last unused value to zero

  mlcTableNode->SetUseColumnNameAsColumnHeader(true);
  mlcTableNode->SetColumnDescription( "Boundary", "Leaf pair boundary");
  mlcTableNode->SetColumnDescription( "1", "Leaf position on the side \"1\"");
  mlcTableNode->SetColumnDescription( "2", "Leaf position on the side \"2\"");
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerIhepMlcControlLogic::SetBeamParentForMlcTableNode(vtkMRMLRTBeamNode* beamNode, 
  vtkMRMLTableNode* tableNode)
{
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->GetMRMLScene());
  if (!shNode)
  {
    vtkErrorMacro("SetBeamParentForMlcTableNode: Subject hierarchy node is invalid");
    return false;
  }
  if (!beamNode)
  {
    vtkErrorMacro("SetBeamParentForMlcTableNode: Beam node is invalid");
    return false;
  }
  if (!tableNode)
  {
    vtkErrorMacro("SetBeamParentForMlcTableNode: MLC table node is invalid");
    return false;
  }

  vtkIdType beamShId = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  vtkIdType mlcCurveShId = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;

  // put observed mlc data under beam and ion beam node parent
  beamShId = shNode->GetItemByDataNode(beamNode);
  mlcCurveShId = shNode->GetItemByDataNode(tableNode);
  if (beamShId != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID && 
    mlcCurveShId != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    shNode->SetItemParent( mlcCurveShId, beamShId);
    beamNode->SetAndObserveMultiLeafCollimatorTableNode(tableNode);
  }
  return true;
}

//---------------------------------------------------------------------------
/*
bool vtkSlicerIhepMlcControlLogic::SetupPositionsFromMlcTableNode(vtkMRMLIhepMlcControlNode* parameterNode,
  vtkMRMLTableNode* tableNode)
{
  if (!parameterNode || !tableNode)
  {
    vtkErrorMacro("SetupPositionsFromMlcTableNode: Parameter or table nodes are invalid");
    return false;
  }
  vtkTable* table = tableNode->GetTable();
  if (!table)
  {
    vtkErrorMacro("SetupPositionsFromMlcTableNode: table is invalid");
    return false;
  }

  if (table->GetNumberOfRows() != (parameterNode->GetNumberOfLeafPairs() + 1))
  {
    vtkErrorMacro("SetupPositionsFromMlcTableNode: Wrong number of leaf pairs in MLC table and parameter nodes");
    return false;
  }

  vtkMRMLIhepMlcControlNode::LeafData leafData;
  if (table->GetNumberOfColumns() == 6 && parameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::TwoLayers) // two layers
  {
    for (int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
    {
      double posLayer1Side1 = table->GetValue( row, 1).ToDouble();
      double posLayer1Side2 = table->GetValue( row, 2).ToDouble();
      double posLayer2Side1 = table->GetValue( row, 4).ToDouble();
      double posLayer2Side2 = table->GetValue( row, 5).ToDouble();

      int stepsLayer1Side1 = static_cast<int>((posLayer1Side1 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
      int stepsLayer2Side1 = static_cast<int>((posLayer2Side1 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
      int stepsLayer1Side2 = static_cast<int>((-posLayer1Side2 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
      int stepsLayer2Side2 = static_cast<int>((-posLayer2Side2 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);

      parameterNode->GetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side1, vtkMRMLIhepMlcControlNode::Layer1);
      leafData.Steps = stepsLayer1Side1;
      leafData.RequiredPosition = stepsLayer1Side1;
      parameterNode->SetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side1, vtkMRMLIhepMlcControlNode::Layer1);

      parameterNode->GetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side2, vtkMRMLIhepMlcControlNode::Layer1);
      leafData.Steps = stepsLayer1Side2;
      leafData.RequiredPosition = stepsLayer1Side2;
      parameterNode->SetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side2, vtkMRMLIhepMlcControlNode::Layer1);

      parameterNode->GetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side1, vtkMRMLIhepMlcControlNode::Layer2);
      leafData.Steps = stepsLayer2Side1;
      leafData.RequiredPosition = stepsLayer2Side1;
      parameterNode->SetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side1, vtkMRMLIhepMlcControlNode::Layer2);

      parameterNode->GetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side2, vtkMRMLIhepMlcControlNode::Layer2);
      leafData.Steps = stepsLayer2Side2;
      leafData.RequiredPosition = stepsLayer2Side2;
      parameterNode->SetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side2, vtkMRMLIhepMlcControlNode::Layer2);
    }
  }
  else if (table->GetNumberOfColumns() == 3 && parameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::OneLayer) // one layer
  {
    for (int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
    {
      double posLayer1Side1 = table->GetValue( row, 1).ToDouble();
      double posLayer1Side2 = table->GetValue( row, 2).ToDouble();

      int stepsLayer1Side1 = static_cast<int>((posLayer1Side1 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
      int stepsLayer1Side2 = static_cast<int>((-posLayer1Side2 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);

      parameterNode->GetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side1, vtkMRMLIhepMlcControlNode::Layer1);
      leafData.Steps = stepsLayer1Side1;
      leafData.RequiredPosition = stepsLayer1Side1;
      parameterNode->SetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side1, vtkMRMLIhepMlcControlNode::Layer1);

      parameterNode->GetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side2, vtkMRMLIhepMlcControlNode::Layer1);
      leafData.Steps = stepsLayer1Side2;
      leafData.RequiredPosition = stepsLayer1Side2;
      parameterNode->SetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side2, vtkMRMLIhepMlcControlNode::Layer1);
    }
  }
  else
  {
    vtkErrorMacro("SetupPositionsFromMlcTableNode: Wrong number of layers in MLC table and parameter nodes");
    return false;
  }

  return true;
}
*/
//---------------------------------------------------------------------------
bool vtkSlicerIhepMlcControlLogic::UpdatePositionBetweenMlcTableNodes(vtkMRMLIhepMlcControlNode* parameterNode,
  vtkMRMLTableNode* prevTableNode, vtkMRMLTableNode* nextTableNode)
{
  if (!parameterNode || !prevTableNode || !nextTableNode)
  {
    vtkErrorMacro("UpdatePositionBetweenMlcTableNodes: Parameter or table nodes are invalid");
    return false;
  }
  vtkTable* prevTable = prevTableNode->GetTable();
  if (!prevTable)
  {
    vtkErrorMacro("UpdatePositionBetweenMlcTableNodes: previous table is invalid");
    return false;
  }
  vtkTable* nextTable = nextTableNode->GetTable();
  if (!nextTable)
  {
    vtkErrorMacro("UpdatePositionBetweenMlcTableNodes: next table is invalid");
    return false;
  }

  if (nextTable->GetNumberOfRows() != (parameterNode->GetNumberOfLeafPairs() + 1) || prevTable->GetNumberOfRows() != (parameterNode->GetNumberOfLeafPairs() + 1))
  {
    vtkErrorMacro("UpdatePositionBetweenMlcTableNodes: Wrong number of leaf pairs in MLC tables and parameter nodes");
    return false;
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  if (nextTable->GetNumberOfColumns() == 6 && prevTable->GetNumberOfColumns() == 6 && parameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::TwoLayers) // two layers
  {
    for (int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
    {
/*
      double prevPosLayer1Side1 = prevTable->GetValue( row, 1).ToDouble();
      double prevPosLayer1Side2 = prevTable->GetValue( row, 2).ToDouble();
      double prevPosLayer2Side1 = prevTable->GetValue( row, 4).ToDouble();
      double prevPosLayer2Side2 = prevTable->GetValue( row, 5).ToDouble();

      double nextPosLayer1Side1 = nextTable->GetValue( row, 1).ToDouble();
      double nextPosLayer1Side2 = nextTable->GetValue( row, 2).ToDouble();
      double nextPosLayer2Side1 = nextTable->GetValue( row, 4).ToDouble();
      double nextPosLayer2Side2 = nextTable->GetValue( row, 5).ToDouble();

      int prevStepsLayer1Side1 = static_cast<int>((prevPosLayer1Side1 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
      int prevStepsLayer2Side1 = static_cast<int>((prevPosLayer2Side1 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
      int prevStepsLayer1Side2 = static_cast<int>((-prevPosLayer1Side2 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
      int prevStepsLayer2Side2 = static_cast<int>((-prevPosLayer2Side2 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);

      int nextStepsLayer1Side1 = static_cast<int>((nextPosLayer1Side1 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
      int nextStepsLayer2Side1 = static_cast<int>((nextPosLayer2Side1 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
      int nextStepsLayer1Side2 = static_cast<int>((-nextPosLayer1Side2 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
      int nextStepsLayer2Side2 = static_cast<int>((-nextPosLayer2Side2 + vtkMRMLIhepMlcControlNode::IHEP_TOTAL_DISTANCE) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
*/
/*
      parameterNode->GetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side1, vtkMRMLIhepMlcControlNode::Layer1);
      leafData.Steps = stepsLayer1Side1;
      parameterNode->SetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side1, vtkMRMLIhepMlcControlNode::Layer1);

      parameterNode->GetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side2, vtkMRMLIhepMlcControlNode::Layer1);
      leafData.Steps = stepsLayer1Side2;
      parameterNode->SetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side2, vtkMRMLIhepMlcControlNode::Layer1);

      parameterNode->GetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side1, vtkMRMLIhepMlcControlNode::Layer2);
      leafData.Steps = stepsLayer2Side1;
      parameterNode->SetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side1, vtkMRMLIhepMlcControlNode::Layer2);

      parameterNode->GetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side2, vtkMRMLIhepMlcControlNode::Layer2);
      leafData.Steps = stepsLayer2Side2;
      parameterNode->SetLeafData(leafData, row, vtkMRMLIhepMlcControlNode::Side2, vtkMRMLIhepMlcControlNode::Layer2);
*/
    }
  }
  else if (nextTable->GetNumberOfColumns() == 3 && prevTable->GetNumberOfColumns() == 3 && parameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::OneLayer) // one layer
  {
  }
  else
  {
    vtkErrorMacro("SetupPositionsFromMlcTableNode: Wrong number of layers in MLC table and parameter nodes");
    return false;
  }
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerIhepMlcControlLogic::UpdateMlcTableNodePositionData(vtkMRMLIhepMlcControlNode* parameterNode, int address, int leafDataSteps, vtkMRMLIhepMlcControlNode::SideType side, vtkMRMLIhepMlcControlNode::LayerType layer)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateMlcTableNodePositionData: Parameter or table nodes are invalid");
    return false;
  }

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();

  if (!beamNode)
  {
    vtkErrorMacro("UpdateMlcTableNodePositionData: Beam node is invalid");
    return false;
  }

  vtkMRMLTableNode* tableNode = beamNode->GetMultiLeafCollimatorTableNode();
  if (!tableNode)
  {
    vtkErrorMacro("UpdateMlcTableNodePositionData: MLC positions table node is invalid");
    return false;
  }

  vtkTable* table = tableNode->GetTable();
  if (!table)
  {
    vtkErrorMacro("UpdateMlcTableNodePositionData: Table is invalid");
    return false;
  }

  int range = parameterNode->GetCalibrationRangeInLayer(layer);
  double rangeHalfDistance = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(range) / 2.;

  if (table->GetNumberOfColumns() == 6 && parameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::TwoLayers &&
    (table->GetNumberOfRows() - 1) == parameterNode->GetNumberOfLeafPairs()) // two layers
  {
    int key = -1;
    vtkMRMLIhepMlcControlNode::SideType side_ = vtkMRMLIhepMlcControlNode::Side_Last;
    int offset = parameterNode->GetLeafOffsetByAddressInLayer( address, key, side_, layer);
    double distanceSide1 = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(leafDataSteps) - rangeHalfDistance;
    double distanceSide2 = rangeHalfDistance - vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(leafDataSteps);
    if (offset != -1 && side_ == side)
    {
      if (side_ == vtkMRMLIhepMlcControlNode::Side1 && layer == vtkMRMLIhepMlcControlNode::Layer1)
      {
        table->SetValue( offset, 1, distanceSide1);
      }
      else if (side_ == vtkMRMLIhepMlcControlNode::Side1 && layer == vtkMRMLIhepMlcControlNode::Layer2)
      {
        table->SetValue( offset, 4, distanceSide1);
      }
      else if (side_ == vtkMRMLIhepMlcControlNode::Side2 && layer == vtkMRMLIhepMlcControlNode::Layer1)
      {
        table->SetValue( offset, 2, distanceSide2);
      }
      else if (side_ == vtkMRMLIhepMlcControlNode::Side2 && layer == vtkMRMLIhepMlcControlNode::Layer2)
      {
        table->SetValue( offset, 5, distanceSide2);
      }
      tableNode->Modified();
      return true;
    }
    return false;
  }
  else if (table->GetNumberOfColumns() == 3 && parameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::OneLayer &&
    (table->GetNumberOfRows() - 1) == parameterNode->GetNumberOfLeafPairs()) // one layer
  {
    int key = -1;
    vtkMRMLIhepMlcControlNode::SideType side_ = vtkMRMLIhepMlcControlNode::Side_Last;
    int offset = parameterNode->GetLeafOffsetByAddressInLayer( address, key, side_, layer);

    double distanceSide1 = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(leafDataSteps) - rangeHalfDistance;
    double distanceSide2 = rangeHalfDistance - vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(leafDataSteps);
    if (offset != -1 && side_ == side)
    {
      if (side_ == vtkMRMLIhepMlcControlNode::Side1 && layer == vtkMRMLIhepMlcControlNode::Layer1)
      {
        table->SetValue( offset, 1, distanceSide1);
      }
      else if (side_ == vtkMRMLIhepMlcControlNode::Side2 && layer == vtkMRMLIhepMlcControlNode::Layer1)
      {
        table->SetValue( offset, 2, distanceSide2);
      }
      tableNode->Modified();
      return true;
    }
    return false;
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerIhepMlcControlLogic::UpdateLeavesDataFromMlcPositionTableNode(vtkMRMLIhepMlcControlNode* parameterNode, vtkMRMLTableNode* mlcTableNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateLeavesDataFromTableNode: Parameter or table nodes are invalid");
    return false;
  }

  vtkMRMLTableNode* tableNode = mlcTableNode;
  vtkTable* table = nullptr;
  if (!tableNode)
  {
    vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();

    if (!beamNode)
    {
      vtkErrorMacro("UpdateLeavesDataFromTableNode: Beam node is invalid");
      return false;
    }

    tableNode = beamNode->GetMultiLeafCollimatorTableNode();
    if (!tableNode)
    {
      vtkErrorMacro("UpdateLeavesDataFromTableNode: MLC positions table node is invalid");
      return false;
    }
  }
  table = tableNode->GetTable();
  if (!table)
  {
    vtkErrorMacro("UpdateLeavesDataFromTableNode: Table is invalid");
    return false;
  }

  if (table->GetNumberOfColumns() == 6 && parameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::TwoLayers &&
    (table->GetNumberOfRows() - 1) == parameterNode->GetNumberOfLeafPairs()) // two layers
  {
    for (int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
    {
      vtkMRMLIhepMlcControlNode::PairOfLeavesData pairOfLeaves;
      if (parameterNode->GetPairOfLeavesData( pairOfLeaves, row, vtkMRMLIhepMlcControlNode::Layer1))
      {
        int range = parameterNode->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1);
        double rangeHalfDistance = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(range) / 2.;

        vtkMRMLIhepMlcControlNode::LeafData& side1 = pairOfLeaves.first;
        vtkMRMLIhepMlcControlNode::LeafData& side2 = pairOfLeaves.second;
        
        double posLayer1Side1 = table->GetValue( row, 1).ToDouble();
        double posLayer1Side2 = table->GetValue( row, 2).ToDouble();

        side1.Steps = static_cast<int>((posLayer1Side1 + rangeHalfDistance) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
        side1.RequiredPosition = side1.Steps;
        side2.Steps = static_cast<int>((-posLayer1Side2 + rangeHalfDistance) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
        side2.RequiredPosition = side2.Steps;
        parameterNode->SetPairOfLeavesData(pairOfLeaves, row, vtkMRMLIhepMlcControlNode::Layer1);
      }
      if (parameterNode->GetPairOfLeavesData( pairOfLeaves, row, vtkMRMLIhepMlcControlNode::Layer2))
      {
        int range = parameterNode->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer2);
        double rangeHalfDistance = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(range) / 2.;

        vtkMRMLIhepMlcControlNode::LeafData& side1 = pairOfLeaves.first;
        vtkMRMLIhepMlcControlNode::LeafData& side2 = pairOfLeaves.second;
        
        double posLayer2Side1 = table->GetValue( row, 4).ToDouble();
        double posLayer2Side2 = table->GetValue( row, 5).ToDouble();
        side1.Steps = static_cast<int>((posLayer2Side1 + rangeHalfDistance) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
        side1.RequiredPosition = side1.Steps;
        side2.Steps = static_cast<int>((-posLayer2Side2 + rangeHalfDistance) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
        side2.RequiredPosition = side2.Steps;

        parameterNode->SetPairOfLeavesData(pairOfLeaves, row, vtkMRMLIhepMlcControlNode::Layer2);
      }
    }
    tableNode->Modified();
    return true;
  }
  else if (table->GetNumberOfColumns() == 3 && parameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::OneLayer &&
    (table->GetNumberOfRows() - 1) == parameterNode->GetNumberOfLeafPairs()) // one layer
  {
    for (int row = 0; row < table->GetNumberOfRows(); ++row)
    {
      vtkMRMLIhepMlcControlNode::PairOfLeavesData pairOfLeaves;
      if (parameterNode->GetPairOfLeavesData( pairOfLeaves, row, vtkMRMLIhepMlcControlNode::Layer1))
      {
        int range = parameterNode->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1);
        double rangeHalfDistance = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(range) / 2.;

        vtkMRMLIhepMlcControlNode::LeafData& side1 = pairOfLeaves.first;
        vtkMRMLIhepMlcControlNode::LeafData& side2 = pairOfLeaves.second;
        
        double posLayer1Side1 = table->GetValue( row, 1).ToDouble();
        double posLayer1Side2 = table->GetValue( row, 2).ToDouble();

        side1.Steps = static_cast<int>((posLayer1Side1 + rangeHalfDistance) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
        side2.Steps = static_cast<int>((-posLayer1Side2 + rangeHalfDistance) * vtkMRMLIhepMlcControlNode::IHEP_MOTOR_STEPS_PER_MM);
        side1.RequiredPosition = side1.Steps;
        side2.RequiredPosition = side2.Steps;
        parameterNode->SetPairOfLeavesData(pairOfLeaves, row, vtkMRMLIhepMlcControlNode::Layer1);
      }
    }
    tableNode->Modified();
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerIhepMlcControlLogic::UpdateMlcPositionTableFromLeafData(vtkMRMLIhepMlcControlNode* parameterNode, const vtkMRMLIhepMlcControlNode::LeafData& leafData, vtkMRMLTableNode* mlcTableNode)
{
  vtkMRMLTableNode* tableNode = mlcTableNode;
  vtkTable* table = nullptr;
  if (!tableNode)
  {
    vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();

    if (!beamNode)
    {
      vtkErrorMacro("UpdateLeavesDataFromTableNode: Beam node is invalid");
      return false;
    }

    tableNode = beamNode->GetMultiLeafCollimatorTableNode();
    if (!tableNode)
    {
      vtkErrorMacro("UpdateLeavesDataFromTableNode: MLC positions table node is invalid");
      return false;
    }
  }
  table = tableNode->GetTable();
  if (!table)
  {
    vtkErrorMacro("UpdateLeavesDataFromTableNode: Table is invalid");
    return false;
  }

  int key = -1;
  vtkMRMLIhepMlcControlNode::SideType side = vtkMRMLIhepMlcControlNode::Side_Last;
  int offset = parameterNode->GetLeafOffsetByAddressInLayer(leafData.Address, key, side, leafData.Layer);
  if (offset != -1)
  {
    if (side == vtkMRMLIhepMlcControlNode::Side1 && leafData.Layer == vtkMRMLIhepMlcControlNode::Layer1)
    {
      int range = parameterNode->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1);
      double rangeHalfDistance = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(range) / 2.;
      double distanceSide1 = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(leafData.Steps) - rangeHalfDistance;

      table->SetValue( offset, 1, distanceSide1);
    }
    else if (side == vtkMRMLIhepMlcControlNode::Side1 && leafData.Layer == vtkMRMLIhepMlcControlNode::Layer2)
    {
      int range = parameterNode->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer2);
      double rangeHalfDistance = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(range) / 2.;
      double distanceSide1 = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(leafData.Steps) - rangeHalfDistance;

      table->SetValue( offset, 4, distanceSide1);
    }
    else if (side == vtkMRMLIhepMlcControlNode::Side2 && leafData.Layer == vtkMRMLIhepMlcControlNode::Layer1)
    {
      int range = parameterNode->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer1);
      double rangeHalfDistance = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(range) / 2.;
      double distanceSide2 = rangeHalfDistance - vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(leafData.Steps);

      table->SetValue( offset, 2, distanceSide2);
    }
    else if (side == vtkMRMLIhepMlcControlNode::Side2 && leafData.Layer == vtkMRMLIhepMlcControlNode::Layer2)
    {
      int range = parameterNode->GetCalibrationRangeInLayer(vtkMRMLIhepMlcControlNode::Layer2);
      double rangeHalfDistance = vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(range) / 2.;
      double distanceSide2 = rangeHalfDistance - vtkMRMLIhepMlcControlNode::InternalCounterValueToDistance(leafData.Steps);

      table->SetValue( offset, 5, distanceSide2);
    }
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
int vtkSlicerIhepMlcControlLogic::LoadMlcPositionTablesFromJSONFile(vtkMRMLIhepMlcControlNode* parameterNode, const std::string& jsonFileName)
{
  if (!parameterNode)
  {
    vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: Parameter node is invalid");
    return -1;
  }
  // Load JSON descriptor file
  FILE *fp = fopen(jsonFileName.c_str(), "r");
  if (!fp)
  {
    vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: Can't open JSON file");
    return -1;
  }
  const size_t size = 10000000;
  std::unique_ptr<char[]> buffer(new char[size]);
  rapidjson::FileReadStream fs(fp, buffer.get(), size);

  rapidjson::Document d;
  if (d.ParseStream(fs).HasParseError())
  {
    vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: Can't parse JSON file");
    fclose(fp);
    return -1;
  }
  fclose(fp);

  const rapidjson::Value& orientationValue = d["Orientation"];
  const rapidjson::Value& numberOfLayersValue = d["NumberOfLayers"];
  const rapidjson::Value& numberOfLeafPairsPerLayerValue = d["NumberOfLeavesPerLayer"];
  int numberOfLayers = numberOfLayersValue.GetInt();
  int numberOfLeafPairsPerLayer = numberOfLeafPairsPerLayerValue.GetInt();
  
  const rapidjson::Value& leafPositionBoundariesValues = d["LeafPositionBoundaries"];

  std::vector< double > leafPositionBoundariesLayer1;
  std::vector< double > leafPositionBoundariesLayer2;
  if (leafPositionBoundariesValues.IsArray() && leafPositionBoundariesValues.Capacity() == 2)
  {
    const rapidjson::Value& layer1 = leafPositionBoundariesValues[0];
    if (!layer1.IsArray())
    {
      vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: Invalid leaf position boundaries values for layer-1");
      return -1;
    }
    const rapidjson::Value& layer2 = leafPositionBoundariesValues[1];
    if (!layer2.IsArray())
    {
      vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: Invalid leaf position boundaries values for layer-2");
      return -1;
    }
    // Layer-1
    for (rapidjson::SizeType pos = 0; pos < layer1.Size(); pos++) // Uses SizeType instead of size_t
    {
      leafPositionBoundariesLayer1.push_back(layer1[pos].GetDouble());
    }
    // Layer-2
    for (rapidjson::SizeType pos = 0; pos < layer2.Size(); pos++) // Uses SizeType instead of size_t
    {
      leafPositionBoundariesLayer2.push_back(layer2[pos].GetDouble());
    }
    if (leafPositionBoundariesLayer1.size() != size_t(numberOfLeafPairsPerLayer + 1) || leafPositionBoundariesLayer2.size() != size_t(numberOfLeafPairsPerLayer + 1))
    {
      vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: Wrong number of leaf pairs and leaf pair boundaries values");
      return -1;
    }
  }

  if (leafPositionBoundariesValues.IsArray() && leafPositionBoundariesValues.Capacity() == 1)
  {
    const rapidjson::Value& layer1 = leafPositionBoundariesValues[0];
    if (!layer1.IsArray())
    {
      vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: Invalid leaf position boundaries values for layer-1");
      return -1;
    }
    // Layer-1
    for (rapidjson::SizeType pos = 0; pos < layer1.Size(); pos++) // Uses SizeType instead of size_t
    {
      leafPositionBoundariesLayer1.push_back(layer1[pos].GetDouble());
    }
    if (leafPositionBoundariesLayer1.size() != size_t(numberOfLeafPairsPerLayer + 1))
    {
      vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: Wrong number of leaf pairs and leaf pair boundaries values");
      return -1;
    }
  }

  const rapidjson::Value& a = d["ControlPoint"];
  if (a.IsArray())
  {
    for (rapidjson::SizeType i = 0; i < a.Size(); i++) // Uses SizeType instead of size_t
    {
      vtkMRMLTableNode* mlcTableNode = nullptr;
      if (a[i].HasMember("LeafPositions"))
      {
        const rapidjson::Value& controlPoint = a[i];
        int index = -1;
        vtkMRMLIhepMlcControlNode::LayerType layerType = vtkMRMLIhepMlcControlNode::Layer_Last;
        std::string name;
        std::vector< double > layer1Side1Positions;
        std::vector< double > layer1Side2Positions;
        std::vector< double > layer2Side1Positions;
        std::vector< double > layer2Side2Positions;
        if (!controlPoint.IsObject())
        {
          vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: Invalid control point object");
          continue;
        }
        for (rapidjson::Value::ConstMemberIterator controlPointIter = controlPoint.MemberBegin(); controlPointIter != controlPoint.MemberEnd(); ++controlPointIter)
        {
          if (controlPointIter->name.GetString() == std::string("Index"))
          {
            const rapidjson::Value& indexValue = controlPointIter->value;
            index = indexValue.GetInt();
          }
          if (controlPointIter->name.GetString() == std::string("Name"))
          {
            const rapidjson::Value& nameValue = controlPointIter->value;
            name = nameValue.GetString();
          }
          if (controlPointIter->name.GetString() == std::string("LeafPositions"))
          {
            const rapidjson::Value& leafPosition = controlPointIter->value;
            if (!leafPosition.IsObject())
            {
              vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: invalid leaf positions object");
              continue;
            }

            for (rapidjson::Value::ConstMemberIterator leafPositionsIter = leafPosition.MemberBegin(); leafPositionsIter != leafPosition.MemberEnd(); ++leafPositionsIter)
            {
              const rapidjson::Value& layer = leafPositionsIter->value;
              if (leafPositionsIter->name.GetString() == std::string("Layer") && layer.IsArray() && (layer.Capacity() == 2))
              {
                for (rapidjson::SizeType layerIndex = 0; layerIndex < layer.Size(); layerIndex++) // Uses SizeType instead of size_t
                {
                  if (layer[layerIndex].HasMember("Positions"))
                  {
                    const rapidjson::Value& layerValue = layer[layerIndex];
                    for (rapidjson::Value::ConstMemberIterator layerIter = layerValue.MemberBegin(); layerIter != layerValue.MemberEnd(); ++layerIter)
                    {
                      if (layerIter->name.GetString() == std::string("Number"))
                      {
                        const rapidjson::Value& numberValue = layerIter->value;
                        int number = numberValue.GetInt();
                        switch (number)
                        {
                        case 1:
                          layerType = vtkMRMLIhepMlcControlNode::Layer1;
                          break;
                        case 2:
                          layerType = vtkMRMLIhepMlcControlNode::Layer2;
                          break;
                        default:
                          layerType = vtkMRMLIhepMlcControlNode::Layer_Last;
                          break;
                        }
                      }
                      if (layerIter->name.GetString() == std::string("Positions") && numberOfLayers == 2)
                      {
                        vtkWarningMacro("LoadMlcPositionTablesFromJSONFile: layer type " << static_cast<int>(layerType) << ' ' << name);
                        const rapidjson::Value& positions = layerIter->value;
                        if (positions.IsArray() && positions.Capacity() == 2)
                        {
                          const rapidjson::Value& side1 = positions[0];
                          if (!side1.IsArray())
                          {
                            vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: not an array");
                            continue;
                          }
                          const rapidjson::Value& side2 = positions[1];
                          if (!side2.IsArray())
                          {
                            vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: not an array");
                            continue;
                          }
                          // Side-1
                          for (rapidjson::SizeType pos = 0; pos < side1.Size(); pos++) // Uses SizeType instead of size_t
                          {
                            switch (layerType)
                            {
                             case vtkMRMLIhepMlcControlNode::Layer1:
                               layer1Side1Positions.push_back(side1[pos].GetDouble());
                               break;
                             case vtkMRMLIhepMlcControlNode::Layer2:
                               layer2Side1Positions.push_back(side1[pos].GetDouble());
                               break;
                             default:
                               break;
                            }
                          }
                          // Side-2
                          for (rapidjson::SizeType pos = 0; pos < side2.Size(); pos++) // Uses SizeType instead of size_t
                          {
                            switch (layerType)
                            {
                             case vtkMRMLIhepMlcControlNode::Layer1:
                               layer1Side2Positions.push_back(side2[pos].GetDouble());
                               break;
                             case vtkMRMLIhepMlcControlNode::Layer2:
                               layer2Side2Positions.push_back(side2[pos].GetDouble());
                               break;
                             default:
                               break;
                            }
                          }
                        }
                      }
                      else if (layerIter->name.GetString() == std::string("Positions") && numberOfLayers == 1)
                      {
                        const rapidjson::Value& positions = layerIter->value;
                        if (positions.IsArray() && positions.Capacity() == 1)
                        {
                          const rapidjson::Value& side1 = positions[0];
                          if (!side1.IsArray())
                          {
                            vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: not an array");
                            continue;
                          }
                          // Side-1
                          for (rapidjson::SizeType pos = 0; pos < side1.Size(); pos++) // Uses SizeType instead of size_t
                          {
                            switch (layerType)
                            {
                             case vtkMRMLIhepMlcControlNode::Layer1:
                               layer1Side1Positions.push_back(side1[pos].GetDouble());
                               break;
                             case vtkMRMLIhepMlcControlNode::Layer2:
                               layer2Side1Positions.push_back(side1[pos].GetDouble());
                               break;
                             default:
                               break;
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
        vtkWarningMacro("LoadMlcPositionTablesFromJSONFile: JSON data " << orientationValue.GetString() << ' ' << index << ' ' << layer1Side2Positions.size() << ' ' << layer1Side1Positions.size() \
         << ' ' << layer2Side2Positions.size() << ' ' <<  layer2Side1Positions.size() << ' ' << name << ' ' << numberOfLayers << ' ' << numberOfLeafPairsPerLayer);
        if (index >= 0 && layer1Side2Positions.size() == layer1Side1Positions.size() && layer2Side2Positions.size() == layer2Side1Positions.size() && !name.empty())
        {
          mlcTableNode = this->CreateMlcTableNode(parameterNode, orientationValue.GetString(), name, numberOfLayers, numberOfLeafPairsPerLayer);
        }
        if (!mlcTableNode)
        {
          vtkErrorMacro("LoadMlcPositionTablesFromJSONFile: Unable to create MLC table node");
          return -1;
        }
        if (numberOfLayers == 1)
        {
          vtkTable* table = mlcTableNode->GetTable();
          for (int row = 0; row < numberOfLeafPairsPerLayer + 1; row++)
          {
            table->SetValue( row, 0, leafPositionBoundariesLayer1[row]);
          }
          for (int row = 0; row < numberOfLeafPairsPerLayer; row++)
          {
            table->SetValue( row, 1, layer1Side1Positions[row]);
          }
          table->SetValue( numberOfLeafPairsPerLayer, 1, 0.);
          for (int row = 0; row < numberOfLeafPairsPerLayer; row++)
          {
            table->SetValue( row, 2, layer1Side2Positions[row]);
          }
          table->SetValue( numberOfLeafPairsPerLayer, 2, 0.);
        }
        else if (numberOfLayers == 2)
        {
          vtkTable* table = mlcTableNode->GetTable();
          for (int row = 0; row < numberOfLeafPairsPerLayer + 1; row++)
          {
            table->SetValue( row, 0, leafPositionBoundariesLayer1[row]);
          }
          for (int row = 0; row < numberOfLeafPairsPerLayer; row++)
          {
            table->SetValue( row, 1, layer1Side1Positions[row]);
          }
          table->SetValue( numberOfLeafPairsPerLayer, 1, 0.);
          for (int row = 0; row < numberOfLeafPairsPerLayer; row++)
          {
            table->SetValue( row, 2, layer1Side2Positions[row]);
          }
          table->SetValue( numberOfLeafPairsPerLayer, 2, 0.);

          for (int row = 0; row < numberOfLeafPairsPerLayer + 1; row++)
          {
            table->SetValue( row, 3, leafPositionBoundariesLayer2[row]);
          }
          for (int row = 0; row < numberOfLeafPairsPerLayer; row++)
          {
            table->SetValue( row, 4, layer2Side1Positions[row]);
          }
          table->SetValue( numberOfLeafPairsPerLayer, 4, 0.);
          for (int row = 0; row < numberOfLeafPairsPerLayer; row++)
          {
            table->SetValue( row, 5, layer2Side2Positions[row]);
          }
          table->SetValue( numberOfLeafPairsPerLayer, 5, 0.);
        }
        else
        {
          vtkWarningMacro("LoadMlcPositionTablesFromJSONFile: Unsupported number of layers");
          return -1;
        }
        if (this->SetBeamParentForMlcTableNode(parameterNode->GetBeamNode(), mlcTableNode))
        {
          index = -1;
          name.clear();
          layerType = vtkMRMLIhepMlcControlNode::Layer_Last;
          layer1Side1Positions.clear();
          layer1Side2Positions.clear();
          layer2Side1Positions.clear();
          layer2Side2Positions.clear();
        }
      }
    }
  }
  return -1;
}

//---------------------------------------------------------------------------
int vtkSlicerIhepMlcControlLogic::LoadMlcCalibrationDataFromJSONFile(vtkMRMLIhepMlcControlNode* parameterNode, const std::string& jsonFileName)
{
  if (!parameterNode)
  {
    vtkErrorMacro("LoadMlcCalibrationDataFromJSONFile: Parameter node is invalid");
    return -1;
  }
  // Load JSON descriptor file
  FILE *fp = fopen(jsonFileName.c_str(), "r");
  if (!fp)
  {
    vtkErrorMacro("LoadMlcCalibrationDataFromJSONFile: Can't open JSON file");
    return -1;
  }
  const size_t size = 10000000;
  std::unique_ptr<char[]> buffer(new char[size]);
  rapidjson::FileReadStream fs(fp, buffer.get(), size);

  rapidjson::Document d;
  if (d.ParseStream(fs).HasParseError())
  {
    vtkErrorMacro("LoadMlcCalibrationDataFromJSONFile: Can't parse JSON file");
    fclose(fp);
    return -1;
  }
  fclose(fp);
  
  const rapidjson::Value& nofLayer = d["NumberOfLayers"];
  const rapidjson::Value& nofLeafPairs = d["NumberOfLeavesPerLayer"];
  const rapidjson::Value& calibValues = d["LeavesCalibrationNumbers"];
  size_t nofLeavesInLayers = nofLayer.GetInt() * nofLeafPairs.GetInt() * 2; 
  if (calibValues.IsArray() && (calibValues.Size() == nofLeavesInLayers))
  {
    for (rapidjson::SizeType i = 0; i < calibValues.Size(); i++) // Uses SizeType instead of size_t
    {
      vtkMRMLIhepMlcControlNode::LayerType layer = vtkMRMLIhepMlcControlNode::Layer_Last;
      vtkMRMLIhepMlcControlNode::SideType side = vtkMRMLIhepMlcControlNode::Side_Last;
      int value = -1;
      int address = -1;
      const rapidjson::Value& calibValue = calibValues[i];
      if (!calibValue.IsObject())
      {
        vtkErrorMacro("LoadMlcCalibrationDataFromJSONFile: Wrong format of calibration data");
        return -1;
      }
      for (rapidjson::Value::ConstMemberIterator itr2 = calibValue.MemberBegin(); itr2 != calibValue.MemberEnd(); ++itr2)
      {
        if (itr2->name.GetString() == std::string("Address"))
        {
          const auto& addressValue = itr2->value;
          address = addressValue.GetInt();
        }
        else if (itr2->name.GetString() == std::string("Layer"))
        {
          const auto& layerValue = itr2->value;
          switch (layerValue.GetInt())
          {
          case 1:
            layer = vtkMRMLIhepMlcControlNode::Layer1;
            break;
          case 2:
            layer = vtkMRMLIhepMlcControlNode::Layer2;
            break;
          default:
            break;
          }
        }
        else if (itr2->name.GetString() == std::string("Side"))
        {
          const auto& sideValue = itr2->value;
          switch (sideValue.GetInt())
          {
          case 1:
            side = vtkMRMLIhepMlcControlNode::Side1;
            break;
          case 2:
            side = vtkMRMLIhepMlcControlNode::Side2;
            break;
          default:
            break;
          }
        }
        else if (itr2->name.GetString() == std::string("Value"))
        {
          const auto& valueValue = itr2->value;
          value = valueValue.GetInt();
        }
      }
      if (layer != vtkMRMLIhepMlcControlNode::Layer_Last && side != vtkMRMLIhepMlcControlNode::Side_Last && address != -1 && value != -1)
      {
        vtkMRMLIhepMlcControlNode::LeafData leafData;
        if (parameterNode->GetLeafDataByAddressInLayer(leafData, address, layer))
        {
          if (leafData.Side == side && leafData.Layer == layer && leafData.Address == address)
          {
            leafData.CalibrationSteps = value;
            return parameterNode->SetLeafDataByAddressInLayer(leafData, address, layer);
          }
        }
      }
    }
  }
  return -1;
}

//---------------------------------------------------------------------------
vtkMRMLTableNode* vtkSlicerIhepMlcControlLogic::CreateMlcTableNode(vtkMRMLIhepMlcControlNode* parameterNode,
  const std::string& orientation, const std::string& name, int numberOfLayers, int numberOfPairLeaves)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateMlcTableNode: Scene node is invalid");
    return nullptr;
  }
  if (!parameterNode || (parameterNode && !parameterNode->GetBeamNode()))
  {
    vtkErrorMacro("CreateMlcTableNode: Parameter or beam node (or both) are invalid");
    return nullptr;
  }

  std::string tableName;
  if (orientation == std::string("MLCX"))
  {
    tableName = std::string(MLCX_BOUNDARYANDPOSITION) + '_' + name;
  }
  else if (orientation == std::string("MLCY"))
  {
    tableName = std::string(MLCY_BOUNDARYANDPOSITION) + '_' + name;
  }
  if (tableName.empty())
  {
    vtkErrorMacro("CreateMlcTableNode: MLC orientation name is invalid");
    return nullptr;
  }

  vtkMRMLTableNode* tableNode = vtkMRMLTableNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTableNode", tableName));
  vtkTable* table = tableNode->GetTable();
  if (!table)
  {
    vtkErrorMacro("CreateMlcTableNode: Unable to create vtkTable to fill MLC data");
    return nullptr;
  }

  for (int i = 0; i < numberOfLayers; ++i)
  {
    // Column 0; Leaf pair boundary values for layer
    vtkNew<vtkDoubleArray> boundaryLayer;
    std::stringstream strbuf;
    strbuf << std::string("Boundary-") << i + 1;
    std::string boundaryName = strbuf.str();
    boundaryLayer->SetName(boundaryName.c_str());
    table->AddColumn(boundaryLayer);

    // Column 1; Leaf positions on the side "1" for layer
    vtkNew<vtkDoubleArray> pos1Layer;
    std::stringstream strbuf2;
    strbuf2 << i + 1 << std::string("-1");
    std::string side1Name = strbuf2.str();
    pos1Layer->SetName(side1Name.c_str());
    table->AddColumn(pos1Layer);

    // Column 2; Leaf positions on the side "2" for layer
    vtkNew<vtkDoubleArray> pos2Layer;
    std::stringstream strbuf3;
    strbuf3 << i + 1 << std::string("-2");
    std::string side2Name = strbuf3.str();
    pos1Layer->SetName(side2Name.c_str());
    table->AddColumn(pos2Layer);
  }
  table->SetNumberOfRows(numberOfPairLeaves + 1);

  for (int i = 0; i < numberOfLayers; ++i)
  {
    std::stringstream strbuf;
    strbuf << std::string("Boundary-") << i + 1;
    std::string boundaryName = strbuf.str();

    std::stringstream strbuf2;
    strbuf2 << i + 1 << std::string("-1");
    std::string side1Name = strbuf2.str();

    std::stringstream strbuf3;
    strbuf3 << i + 1 << std::string("-2");
    std::string side2Name = strbuf3.str();

    for (int row = 0; row < numberOfPairLeaves; ++row)
    {
      table->SetValue(row, 3 * i + 0, 0.0);
    }

    for (int row = 0; row < numberOfPairLeaves; ++row)
    {
      table->SetValue(row, 3 * i + 1, 0.0); // default meaningful value for side "1" for layer
      table->SetValue(row, 3 * i + 2, 0.0); // default meaningful value for side "2" for layer
    }
    table->SetValue(numberOfPairLeaves, 3 * i + 1, 0.); // side "1" set last unused value to zero
    table->SetValue(numberOfPairLeaves, 3 * i + 1, 0.); // side "2" set last unused value to zero
    tableNode->SetColumnDescription( boundaryName.c_str(), "Pair of leaves boundary for the layer");
    tableNode->SetColumnDescription( side1Name.c_str(), "Leaf position on the side \"1\" for the layer");
    tableNode->SetColumnDescription( side2Name.c_str(), "Leaf position on the side \"2\" for the layer");
  }
  tableNode->SetUseColumnNameAsColumnHeader(true);
  return tableNode;
}
