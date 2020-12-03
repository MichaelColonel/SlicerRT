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

// StandGeo Logic includes
#include "vtkSlicerIhepStandGeometryLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// SlicerRT MRML includes
#include <vtkMRMLIhepStandGeometryNode.h>

// SlicerRT IEC transformation logic from Beam module
#include <vtkSlicerIECTransformLogic.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes

//----------------------------------------------------------------------------
// Treatment machine component names
const char* vtkSlicerIhepStandGeometryLogic::CANYON_MODEL_NAME = "Canyon";
const char* vtkSlicerIhepStandGeometryLogic::ROTATION_MODEL_NAME = "PatientSupportRotation";
const char* vtkSlicerIhepStandGeometryLogic::STAND_MODEL_NAME = "PatientSupportStand";
const char* vtkSlicerIhepStandGeometryLogic::TABLETOP_MODEL_NAME = "TableTop";

const char* vtkSlicerIhepStandGeometryLogic::ORIENTATION_MARKER_MODEL_NODE_NAME = "IhepStandGeometryOrientationMarker";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerIhepStandGeometryLogic);

//----------------------------------------------------------------------------
vtkSlicerIhepStandGeometryLogic::vtkSlicerIhepStandGeometryLogic()
  :
  IECLogic(vtkSlicerIECTransformLogic::New())
{
}

//----------------------------------------------------------------------------
vtkSlicerIhepStandGeometryLogic::~vtkSlicerIhepStandGeometryLogic()
{
  if (this->IECLogic)
  {
    this->IECLogic->Delete();
    this->IECLogic = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
  this->IECLogic->SetMRMLScene(newScene);
}

//-----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLIhepStandGeometryNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLIhepStandGeometryNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdateFromMRMLScene()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
  }

  if (node->IsA("vtkMRMLIhepStandGeometryNode"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::BuildIhepStangGeometryTransformHierarchy()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("BuildIhepStangGeometryTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Build IEC hierarchy
  this->IECLogic->BuildIECTransformHierarchy();
}
/*
//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::LoadTreatmentMachineModels(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Invalid parameter node");
    return;
  }

  // Make sure the transform hierarchy is in place
  this->BuildIhepStangGeometryTransformHierarchy();

  std::string moduleShareDirectory = this->GetModuleShareDirectory();
  std::string machineType(parameterNode->GetTreatmentMachineType());
  std::string treatmentMachineModelsDirectory = moduleShareDirectory + "/" + machineType;

  // Create a models logic for convenient loading of components
  vtkNew<vtkSlicerModelsLogic> modelsLogic;
  modelsLogic->SetMRMLScene(scene);

  // Create model hierarchy so that the treatment machine can be shown/hidden easily
  std::string rootModelHierarchyNodeName = machineType + std::string("_Components");
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

  //
  // Load treatment machine models

  // Collimator - mandatory
  std::string collimatorModelSingletonTag = machineType + "_" + COLLIMATOR_MODEL_NAME;
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
    std::string collimatorModelFilePath = treatmentMachineModelsDirectory + "/" + COLLIMATOR_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(collimatorModelFilePath))
    {
      collimatorModelNode = modelsLogic->AddModel(collimatorModelFilePath.c_str());
    }
    if (collimatorModelNode)
    {
      collimatorModelNode->SetSingletonTag(collimatorModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> collimatorModelHierarchyNode;
      scene->AddNode(collimatorModelHierarchyNode);
      collimatorModelHierarchyNode->SetModelNodeID(collimatorModelNode->GetID());
      collimatorModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      collimatorModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load collimator model");
    }
  }

  // Gantry - mandatory
  std::string gantryModelSingletonTag = machineType + "_" + GANTRY_MODEL_NAME;
  vtkMRMLModelNode* gantryModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(gantryModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (gantryModelNode && !gantryModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(gantryModelNode);
    gantryModelNode = nullptr;
  }
  if (!gantryModelNode)
  {
    std::string gantryModelFilePath = treatmentMachineModelsDirectory + "/" + GANTRY_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(gantryModelFilePath))
    {
      gantryModelNode = modelsLogic->AddModel(gantryModelFilePath.c_str());
    }
    if (gantryModelNode)
    {
      gantryModelNode->SetSingletonTag(gantryModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> gantryModelHierarchyNode;
      scene->AddNode(gantryModelHierarchyNode);
      gantryModelHierarchyNode->SetModelNodeID(gantryModelNode->GetID());
      gantryModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      gantryModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load gantry model");
    }
  }

  // Patient support - mandatory
  std::string patientSupportModelSingletonTag = machineType + "_" + PATIENTSUPPORT_MODEL_NAME;
  vtkMRMLModelNode* patientSupportModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(patientSupportModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (patientSupportModelNode && !patientSupportModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(patientSupportModelNode);
    patientSupportModelNode = nullptr;
  }
  if (!patientSupportModelNode)
  {
    std::string patientSupportModelFilePath = treatmentMachineModelsDirectory + "/" + PATIENTSUPPORT_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(patientSupportModelFilePath))
    {
      patientSupportModelNode = modelsLogic->AddModel(patientSupportModelFilePath.c_str());
    }
    if (patientSupportModelNode)
    {
      patientSupportModelNode->SetSingletonTag(patientSupportModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> patientSupportModelHierarchyNode;
      scene->AddNode(patientSupportModelHierarchyNode);
      patientSupportModelHierarchyNode->SetModelNodeID(patientSupportModelNode->GetID());
      patientSupportModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      patientSupportModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load patient support model");
    }
  }

  // Table top - mandatory
  std::string tableTopModelSingletonTag = machineType + "_" + TABLETOP_MODEL_NAME;
  vtkMRMLModelNode* tableTopModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableTopModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableTopModelNode && !tableTopModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableTopModelNode);
    tableTopModelNode = nullptr;
  }
  if (!tableTopModelNode)
  {
    std::string tableTopModelFilePath = treatmentMachineModelsDirectory + "/" + TABLETOP_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableTopModelFilePath))
    {
      tableTopModelNode = modelsLogic->AddModel(tableTopModelFilePath.c_str());
    }
    if (tableTopModelNode)
    {
      tableTopModelNode->SetSingletonTag(tableTopModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableTopModelHierarchyNode;
      scene->AddNode(tableTopModelHierarchyNode);
      tableTopModelHierarchyNode->SetModelNodeID(tableTopModelNode->GetID());
      tableTopModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableTopModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table top model");
    }
  }

  // Linac body - optional
  std::string linacBodyModelSingletonTag = machineType + "_" + LINACBODY_MODEL_NAME;
  vtkMRMLModelNode* linacBodyModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(linacBodyModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (linacBodyModelNode && !linacBodyModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(linacBodyModelNode);
    linacBodyModelNode = nullptr;
  }
  if (!linacBodyModelNode)
  {
    std::string linacBodyModelFilePath = treatmentMachineModelsDirectory + "/" + LINACBODY_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(linacBodyModelFilePath))
    {
      linacBodyModelNode = modelsLogic->AddModel(linacBodyModelFilePath.c_str());
    }
    if (linacBodyModelNode)
    {
      linacBodyModelNode->SetSingletonTag(linacBodyModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> linacBodyModelHierarchyNode;
      scene->AddNode(linacBodyModelHierarchyNode);
      linacBodyModelHierarchyNode->SetModelNodeID(linacBodyModelNode->GetID());
      linacBodyModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      linacBodyModelHierarchyNode->HideFromEditorsOn();
    }
  }

  // Imaging panel left - optional
  std::string imagingPanelLeftModelSingletonTag = machineType + "_" + IMAGINGPANELLEFT_MODEL_NAME;
  vtkMRMLModelNode* imagingPanelLeftModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(imagingPanelLeftModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (imagingPanelLeftModelNode && !imagingPanelLeftModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(imagingPanelLeftModelNode);
    imagingPanelLeftModelNode = nullptr;
  }
  if (!imagingPanelLeftModelNode)
  {
    std::string imagingPanelLeftModelFilePath = treatmentMachineModelsDirectory + "/" + IMAGINGPANELLEFT_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(imagingPanelLeftModelFilePath))
    {
      imagingPanelLeftModelNode = modelsLogic->AddModel(imagingPanelLeftModelFilePath.c_str());
    }
    if (imagingPanelLeftModelNode)
    {
      imagingPanelLeftModelNode->SetSingletonTag(imagingPanelLeftModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> imagingPanelLeftModelHierarchyNode;
      scene->AddNode(imagingPanelLeftModelHierarchyNode);
      imagingPanelLeftModelHierarchyNode->SetModelNodeID(imagingPanelLeftModelNode->GetID());
      imagingPanelLeftModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      imagingPanelLeftModelHierarchyNode->HideFromEditorsOn();
    }
  }

  // Imaging panel right - optional
  std::string imagingPanelRightModelSingletonTag = machineType + "_" + IMAGINGPANELRIGHT_MODEL_NAME;
  vtkMRMLModelNode* imagingPanelRightModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(imagingPanelRightModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (imagingPanelRightModelNode && !imagingPanelRightModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(imagingPanelRightModelNode);
    imagingPanelRightModelNode = nullptr;
  }
  if (!imagingPanelRightModelNode)
  {
    std::string imagingPanelRightModelFilePath = treatmentMachineModelsDirectory + "/" + IMAGINGPANELRIGHT_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(imagingPanelRightModelFilePath))
    {
      imagingPanelRightModelNode = modelsLogic->AddModel(imagingPanelRightModelFilePath.c_str());
    }
    if (imagingPanelRightModelNode)
    {
      imagingPanelRightModelNode->SetSingletonTag(imagingPanelRightModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> imagingPanelRightModelHierarchyNode;
      scene->AddNode(imagingPanelRightModelHierarchyNode);
      imagingPanelRightModelHierarchyNode->SetModelNodeID(imagingPanelRightModelNode->GetID());
      imagingPanelRightModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      imagingPanelRightModelHierarchyNode->HideFromEditorsOn();
    }
  }

  // Flat panel - optional
  std::string flatPanelModelSingletonTag = machineType + "_" + FLATPANEL_MODEL_NAME;
  vtkMRMLModelNode* flatPanelModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(flatPanelModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (flatPanelModelNode && !flatPanelModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(flatPanelModelNode);
    flatPanelModelNode = nullptr;
  }
  if (!flatPanelModelNode)
  {
    std::string flatPanelModelFilePath = treatmentMachineModelsDirectory + "/" + FLATPANEL_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(flatPanelModelFilePath))
    {
      flatPanelModelNode = modelsLogic->AddModel(flatPanelModelFilePath.c_str());
    }
    if (flatPanelModelNode)
    {
      flatPanelModelNode->SetSingletonTag(flatPanelModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> flatPanelModelHierarchyNode;
      scene->AddNode(flatPanelModelHierarchyNode);
      flatPanelModelHierarchyNode->SetModelNodeID(flatPanelModelNode->GetID());
      flatPanelModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      flatPanelModelHierarchyNode->HideFromEditorsOn();
    }
  }

  if ( !collimatorModelNode || !collimatorModelNode->GetPolyData()
    || !gantryModelNode || !gantryModelNode->GetPolyData()
    || !patientSupportModelNode || !patientSupportModelNode->GetPolyData()
    || !tableTopModelNode || !tableTopModelNode->GetPolyData() )
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Failed to load every mandatory treatment machine component");
    return;
  }

  // Setup treatment machine model display and transforms
  this->SetupTreatmentMachineModels();
}

//----------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::SetupTreatmentMachineModels()
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Invalid scene");
    return;
  }

  //TODO: Store treatment machine component color and other properties in JSON

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation

  // Gantry - mandatory
  vtkMRMLModelNode* gantryModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(GANTRY_MODEL_NAME) );
  if (!gantryModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access gantry model");
    return;
  }
  vtkMRMLLinearTransformNode* gantryToFixedReferenceTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Gantry, vtkSlicerIECTransformLogic::FixedReference);
  gantryModel->SetAndObserveTransformNodeID(gantryToFixedReferenceTransformNode->GetID());
  gantryModel->CreateDefaultDisplayNodes();
  gantryModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);

  // Collimator - mandatory
  vtkMRMLModelNode* collimatorModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(COLLIMATOR_MODEL_NAME) );
  if (!collimatorModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access collimator model");
    return;
  }
  vtkMRMLLinearTransformNode* collimatorToGantryTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Collimator, vtkSlicerIECTransformLogic::Gantry);
  collimatorModel->SetAndObserveTransformNodeID(collimatorToGantryTransformNode->GetID());
  collimatorModel->CreateDefaultDisplayNodes();
  collimatorModel->GetDisplayNode()->SetColor(0.7, 0.7, 0.95);

  // Patient support - mandatory
  vtkMRMLModelNode* patientSupportModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(PATIENTSUPPORT_MODEL_NAME) );
  if (!patientSupportModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access patient support model");
    return;
  }
  vtkMRMLLinearTransformNode* patientSupportToPatientSupportRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::PatientSupport, vtkSlicerIECTransformLogic::PatientSupportRotation);
  patientSupportModel->SetAndObserveTransformNodeID(patientSupportToPatientSupportRotationTransformNode->GetID());
  patientSupportModel->CreateDefaultDisplayNodes();
  patientSupportModel->GetDisplayNode()->SetColor(0.85, 0.85, 0.85);

  // Table top - mandatory
  vtkMRMLModelNode* tableTopModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(TABLETOP_MODEL_NAME) );
  if (!tableTopModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top model");
    return;
  }
  vtkMRMLLinearTransformNode* tableTopToTableTopEccentricRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::TableTop, vtkSlicerIECTransformLogic::TableTopEccentricRotation);
  tableTopModel->SetAndObserveTransformNodeID(tableTopToTableTopEccentricRotationTransformNode->GetID());
  tableTopModel->CreateDefaultDisplayNodes();
  tableTopModel->GetDisplayNode()->SetColor(0, 0, 0);

  // Linac body - optional
  vtkMRMLModelNode* linacBodyModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(LINACBODY_MODEL_NAME) );
  if (linacBodyModel)
  {
    vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode =
      this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::FixedReference, vtkSlicerIECTransformLogic::RAS);
    linacBodyModel->SetAndObserveTransformNodeID(fixedReferenceToRasTransformNode->GetID());
    linacBodyModel->CreateDefaultDisplayNodes();
    linacBodyModel->GetDisplayNode()->SetColor(0.9, 0.9, 0.9);
  }

  // Imaging panel left - optional
  vtkMRMLModelNode* leftImagingPanelModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(IMAGINGPANELLEFT_MODEL_NAME) );
  if (leftImagingPanelModel)
  {
    vtkMRMLLinearTransformNode* leftImagingPanelToGantryTransformNode =
      this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::LeftImagingPanel, vtkSlicerIECTransformLogic::Gantry);
    leftImagingPanelModel->SetAndObserveTransformNodeID(leftImagingPanelToGantryTransformNode->GetID());
    leftImagingPanelModel->CreateDefaultDisplayNodes();
    leftImagingPanelModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
  }

  // Imaging panel right - optional
  vtkMRMLModelNode* rightImagingPanelModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(IMAGINGPANELRIGHT_MODEL_NAME) );
  if (rightImagingPanelModel)
  {
    vtkMRMLLinearTransformNode* rightImagingPanelToGantryTransformNode =
      this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::RightImagingPanel, vtkSlicerIECTransformLogic::Gantry);
    rightImagingPanelModel->SetAndObserveTransformNodeID(rightImagingPanelToGantryTransformNode->GetID());
    rightImagingPanelModel->CreateDefaultDisplayNodes();
    rightImagingPanelModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
  }

  // Flat panel - optional
  vtkMRMLModelNode* flatPanelModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(FLATPANEL_MODEL_NAME) );
  if (flatPanelModel)
  {
    vtkMRMLLinearTransformNode* flatPanelToGantryTransformNode =
      this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::FlatPanel, vtkSlicerIECTransformLogic::Gantry);
    flatPanelModel->SetAndObserveTransformNodeID(flatPanelToGantryTransformNode->GetID());
    flatPanelModel->CreateDefaultDisplayNodes();
    flatPanelModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
  }

  //
  // Set up collision detection between components
  this->GantryTableTopCollisionDetection->SetInput(0, gantryModel->GetPolyData());
  this->GantryTableTopCollisionDetection->SetInput(1, tableTopModel->GetPolyData());

  this->GantryPatientSupportCollisionDetection->SetInput(0, gantryModel->GetPolyData());
  this->GantryPatientSupportCollisionDetection->SetInput(1, patientSupportModel->GetPolyData());

  this->CollimatorTableTopCollisionDetection->SetInput(0, collimatorModel->GetPolyData());
  this->CollimatorTableTopCollisionDetection->SetInput(1, tableTopModel->GetPolyData());

  //TODO: Whole patient (segmentation, CT) will need to be transformed when the table top is transformed
  //vtkMRMLLinearTransformNode* patientModelTransforms = vtkMRMLLinearTransformNode::SafeDownCast(
  //  this->GetMRMLScene()->GetFirstNodeByName("TableTopEccentricRotationToPatientSupportTransform"));
  //patientModel->SetAndObserveTransformNodeID(patientModelTransforms->GetID());

  // Patient model is set when calculating collisions, as it can be changed dynamically
  this->GantryPatientCollisionDetection->SetInput(0, gantryModel->GetPolyData());
  this->CollimatorPatientCollisionDetection->SetInput(0, collimatorModel->GetPolyData());
  // Set identity transform for patient (parent transform is taken into account when getting poly data from segmentation)
  vtkNew<vtkTransform> identityTransform;
  identityTransform->Identity();
  this->GantryPatientCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(identityTransform));
  this->CollimatorPatientCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(identityTransform));
}
*/
