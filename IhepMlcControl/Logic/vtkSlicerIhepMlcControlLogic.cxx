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
vtkSlicerIhepMlcControlLogic::vtkSlicerIhepMlcControlLogic()
{
  vtkSlicerBeamsModuleLogic* beamsLogic = vtkSlicerBeamsModuleLogic::SafeDownCast(this->GetModuleLogic("Beams"));
  if (beamsLogic)
  {
    this->MlcPositionLogic = beamsLogic->GetMLCPositionLogic();
  }
}

//----------------------------------------------------------------------------
vtkSlicerIhepMlcControlLogic::~vtkSlicerIhepMlcControlLogic()
{
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
    vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(caller);

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
  if (!parameterNode && !parameterNode->GetBeamNode())
  {
    vtkErrorMacro("CreateMlcTableNodeBoundaryData: Parameter or beam node (or both) are invalid");
    return nullptr;
  }
  if (parameterNode->GetMlcTableNode())
  {
    vtkMRMLTableNode* tableNode = parameterNode->GetMlcTableNode();
    // update
    return tableNode;
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

  vtkMRMLIhepMlcControlNode::LayersType nofLayers = parameterNode->GetLayers();
  
  // Layer-1
  std::vector<double> leafPairsBoundary1(parameterNode->GetNumberOfLeafPairs());
  // Layer-2
  std::vector<double> leafPairsBoundary2;
  double middle = parameterNode->GetPairOfLeavesSize() * parameterNode->GetNumberOfLeafPairs() / 2.;
  for(auto iter = leafPairsBoundary1.begin(); iter != leafPairsBoundary1.end(); ++iter)
  {
    size_t pos = iter - leafPairsBoundary1.begin();
    *iter = -middle + parameterNode->GetIsocenterOffset() + pos * parameterNode->GetPairOfLeavesSize();
  }
  if (nofLayers == vtkMRMLIhepMlcControlNode::TwoLayers)
  {
    for(double pairOfLevesBoundary : leafPairsBoundary1)
    {
      leafPairsBoundary2.push_back(pairOfLevesBoundary + parameterNode->GetOffsetBetweenTwoLayers());
    }
  }

  vtkTable* table = tableNode->GetTable();
  if (!table)
  {
    vtkErrorMacro("CreateMultiLeafCollimatorTableNodeBoundaryData: Unable to create vtkTable to fill MLC data");
    return nullptr;
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

    table->SetNumberOfRows(leafPairsBoundary1.size());
    for (size_t row = 0; row < leafPairsBoundary1.size(); ++row)
    {
      table->SetValue(row, 0, leafPairsBoundary1[row]);
    }

    for (unsigned int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
    {
      table->SetValue(row, 1, -100.0); // default meaningful value for side "1" for layer-1
      table->SetValue(row, 2, -100.0); // default meaningful value for side "2" for layer-1
    }
    tableNode->SetUseColumnNameAsColumnHeader(true);
    tableNode->SetColumnDescription( "Boundary-1", "Pair of leaves boundary for the first layer");
    tableNode->SetColumnDescription( "1-1", "Leaf position on the side \"1\" for the first layer");
    tableNode->SetColumnDescription( "1-2", "Leaf position on the side \"2\" for the first layer");

    // Column 3; Leaf pair boundary values for layer-2
    vtkNew<vtkDoubleArray> boundaryLayer2;
    boundaryLayer2->SetName("Boundary-2");
    table->AddColumn(boundaryLayer2);

    // Column 4; Leaf positions on the side "1" for layer-2
    vtkNew<vtkDoubleArray> pos1Layer2;
    pos1Layer2->SetName("1-1");
    table->AddColumn(pos1Layer2);

    // Column 5; Leaf positions on the side "2" for layer-2
    vtkNew<vtkDoubleArray> pos2Layer2;
    pos2Layer2->SetName("1-2");
    table->AddColumn(pos2Layer2);

    table->SetNumberOfRows(leafPairsBoundary2.size());
    for (size_t row = 0; row < leafPairsBoundary2.size(); ++row)
    {
      table->SetValue(row, 0, leafPairsBoundary2[row]);
    }

    for (unsigned int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
    {
      table->SetValue(row, 1, -100.0); // default meaningful value for side "1" for layer-2
      table->SetValue(row, 2, -100.0); // default meaningful value for side "2" for layer-2
    }
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
  for ( size_t row = 0; row < leafPairsBoundary1.size(); ++row)
  {
    table->SetValue( row, 0, leafPairsBoundary1[row]);
  }

  for ( unsigned int row = 0; row < parameterNode->GetNumberOfLeafPairs(); ++row)
  {
    table->SetValue( row, 1, -100.0); // default meaningful value for side "1"
    table->SetValue( row, 2, -100.0); // default meaningful value for side "2"
  }
//  table->SetValue( nofLeafPairs, 1, 0.); // side "1" set last unused value to zero
//  table->SetValue( nofLeafPairs, 2, 0.); // side "2" set last unused value to zero

  tableNode->SetUseColumnNameAsColumnHeader(true);
  tableNode->SetColumnDescription( "Boundary", "Leaf pair boundary");
  tableNode->SetColumnDescription( "1", "Leaf position on the side \"1\"");
  tableNode->SetColumnDescription( "2", "Leaf position on the side \"2\"");
  return tableNode;
}

//---------------------------------------------------------------------------
bool vtkSlicerIhepMlcControlLogic::UpdateMlcTableNodeBoundaryData(vtkMRMLIhepMlcControlNode* parameterNode)
{
  return false;
}
