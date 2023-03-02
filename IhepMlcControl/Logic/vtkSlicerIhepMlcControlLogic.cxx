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

// SlicerRT IhepMlcControl MRML includes
#include <vtkMRMLIhepMlcControlNode.h>

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

// STD includes

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
      vtkWarningMacro("ProcessMRMLNodesEvents: Process IhepMLC events in logic");
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
      table->SetValue(row, 1, -100.0); // default meaningful value for side "1" for layer-1
      table->SetValue(row, 2, -100.0); // default meaningful value for side "2" for layer-1
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
      table->SetValue(row, 4, -100.0); // default meaningful value for side "1" for layer-2
      table->SetValue(row, 5, -100.0); // default meaningful value for side "2" for layer-2
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
    table->SetValue(row, 1, -100.0); // default meaningful value for side "1"
    table->SetValue(row, 2, -100.0); // default meaningful value for side "2"
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
    }
  }
  else if (table->GetNumberOfColumns() == 3 && parameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::OneLayer) // one layer
  {
  }
  else
  {
    vtkErrorMacro("SetupPositionsFromMlcTableNode: Wrong number of layers in MLC table and parameter nodes");
    return false;
  }

  return true;
}
