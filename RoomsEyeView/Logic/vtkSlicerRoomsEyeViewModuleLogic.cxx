/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// RoomsEyeView includes
#include "vtkSlicerRoomsEyeViewModuleLogic.h"
#include "vtkMRMLRoomsEyeViewNode.h"
#include "vtkSlicerIECTransformLogic.h"

// SlicerRT includes
#include "vtkMRMLRTBeamNode.h"
#include "vtkCollisionDetectionFilter.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelDisplayNode.h>

// Slicer includes
#include <vtkSlicerModelsLogic.h>
#include <vtkSlicerSegmentationsModuleLogic.h>

// vtkSegmentationCore includes
#include <vtkSegmentationConverter.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkTransform.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTransformPolyDataFilter.h>
#include <vtkGeneralTransform.h>
#include <vtkTransformFilter.h>

//----------------------------------------------------------------------------
// Treatment machine component names
const char* vtkSlicerRoomsEyeViewModuleLogic::COLLIMATOR_MODEL_NAME = "Collimator";
const char* vtkSlicerRoomsEyeViewModuleLogic::GANTRY_MODEL_NAME = "Gantry";
const char* vtkSlicerRoomsEyeViewModuleLogic::PATIENTSUPPORT_MODEL_NAME = "PatientSupport";
const char* vtkSlicerRoomsEyeViewModuleLogic::TABLETOP_MODEL_NAME = "TableTop";

const char* vtkSlicerRoomsEyeViewModuleLogic::LINACBODY_MODEL_NAME = "LinacBody";
const char* vtkSlicerRoomsEyeViewModuleLogic::IMAGINGPANELLEFT_MODEL_NAME = "ImagingPanelLeft";
const char* vtkSlicerRoomsEyeViewModuleLogic::IMAGINGPANELRIGHT_MODEL_NAME = "ImagingPanelRight";
const char* vtkSlicerRoomsEyeViewModuleLogic::FLATPANEL_MODEL_NAME = "FlatPanel";

const char* vtkSlicerRoomsEyeViewModuleLogic::APPLICATORHOLDER_MODEL_NAME = "ApplicatorHolder";
const char* vtkSlicerRoomsEyeViewModuleLogic::ELECTRONAPPLICATOR_MODEL_NAME = "ElectronApplicator";

const char* vtkSlicerRoomsEyeViewModuleLogic::ORIENTATION_MARKER_MODEL_NODE_NAME = "RoomsEyeViewOrientationMarker";

// Transform names
//TODO: Add this dynamically to the IEC transform map
static const char* ADDITIONALCOLLIMATORMOUNTEDDEVICES_TO_COLLIMATOR_TRANSFORM_NODE_NAME = "AdditionalCollimatorDevicesToCollimatorTransform";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerRoomsEyeViewModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerRoomsEyeViewModuleLogic::vtkSlicerRoomsEyeViewModuleLogic()
  : GantryPatientCollisionDetection(nullptr)
  , GantryTableTopCollisionDetection(nullptr)
  , GantryPatientSupportCollisionDetection(nullptr)
  , CollimatorPatientCollisionDetection(nullptr)
  , CollimatorTableTopCollisionDetection(nullptr)
{
  this->IECLogic = vtkSlicerIECTransformLogic::New();

  this->GantryPatientCollisionDetection = vtkCollisionDetectionFilter::New();
  this->GantryTableTopCollisionDetection = vtkCollisionDetectionFilter::New();
  this->GantryPatientSupportCollisionDetection = vtkCollisionDetectionFilter::New();
  this->CollimatorPatientCollisionDetection = vtkCollisionDetectionFilter::New();
  this->CollimatorTableTopCollisionDetection = vtkCollisionDetectionFilter::New();
}

//----------------------------------------------------------------------------
vtkSlicerRoomsEyeViewModuleLogic::~vtkSlicerRoomsEyeViewModuleLogic()
{
  if (this->IECLogic)
  {
    this->IECLogic->Delete();
    this->IECLogic = nullptr;
  }

  if (this->GantryPatientCollisionDetection)
  {
    this->GantryPatientCollisionDetection->Delete();
    this->GantryPatientCollisionDetection = nullptr;
  }
  if (this->GantryTableTopCollisionDetection)
  {
    this->GantryTableTopCollisionDetection->Delete();
    this->GantryTableTopCollisionDetection = nullptr;
  }
  if (this->GantryPatientSupportCollisionDetection)
  {
    this->GantryPatientSupportCollisionDetection->Delete();
    this->GantryPatientSupportCollisionDetection = nullptr;
  }
  if (this->CollimatorPatientCollisionDetection)
  {
    this->CollimatorPatientCollisionDetection->Delete();
    this->CollimatorPatientCollisionDetection = nullptr;
  }
  if (this->CollimatorTableTopCollisionDetection)
  {
    this->CollimatorTableTopCollisionDetection->Delete();
    this->CollimatorTableTopCollisionDetection = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLRoomsEyeViewNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRoomsEyeViewNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  this->Superclass::SetMRMLSceneInternal(newScene);

  this->IECLogic->SetMRMLScene(newScene);
}

//---------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::BuildRoomsEyeViewTransformHierarchy()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("BuildRoomsEyeViewTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Build IEC hierarchy
  //TODO: Add the REV transform to the IEC transform map and use it for the GetTransform... functions
  this->IECLogic->BuildIECTransformHierarchy();
}

//----------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::LoadTreatmentMachineModels(vtkMRMLRoomsEyeViewNode* parameterNode)
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
  this->BuildRoomsEyeViewTransformHierarchy();

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
  vtkMRMLLinearTransformNode* gantryToRasTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Gantry, vtkSlicerIECTransformLogic::RAS);
  if (gantryToRasTransformNode)
  {
    vtkWarningMacro("SetupTreatmentMachineModels: Gantry to RAS valid");
    gantryModel->SetAndObserveTransformNodeID(gantryToRasTransformNode->GetID());
    gantryModel->CreateDefaultDisplayNodes();
    gantryModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
  }
  else
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Gantry to RAS invalid");
  }

  // Collimator - mandatory
  vtkMRMLModelNode* collimatorModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(COLLIMATOR_MODEL_NAME) );
  if (!collimatorModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access collimator model");
    return;
  }
  vtkMRMLLinearTransformNode* collimatorToRasTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Collimator, vtkSlicerIECTransformLogic::RAS);
  if (collimatorToRasTransformNode)
  {
    vtkWarningMacro("SetupTreatmentMachineModels: Collimator to RAS valid");
    collimatorModel->SetAndObserveTransformNodeID(collimatorToRasTransformNode->GetID());
    collimatorModel->CreateDefaultDisplayNodes();
    collimatorModel->GetDisplayNode()->SetColor(0.7, 0.7, 0.95);
  }
  else
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Collimator to RAS invalid");
  }

  // Patient support - mandatory
  vtkMRMLModelNode* patientSupportModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(PATIENTSUPPORT_MODEL_NAME) );
  if (!patientSupportModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access patient support model");
    return;
  }
  vtkMRMLLinearTransformNode* patientSupportToRasTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::PatientSupport, vtkSlicerIECTransformLogic::RAS);
  if (patientSupportToRasTransformNode)
  {
    vtkWarningMacro("SetupTreatmentMachineModels: Patient support to RAS valid");
    patientSupportModel->SetAndObserveTransformNodeID(patientSupportToRasTransformNode->GetID());
    patientSupportModel->CreateDefaultDisplayNodes();
    patientSupportModel->GetDisplayNode()->SetColor(0.85, 0.85, 0.85);
  }
  else
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Patient support to RAS invalid");
  }

  // Table top - mandatory
  vtkMRMLModelNode* tableTopModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(TABLETOP_MODEL_NAME) );
  if (!tableTopModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top model");
    return;
  }
  vtkMRMLLinearTransformNode* tableTopToRasTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::TableTop, vtkSlicerIECTransformLogic::RAS);
  if (tableTopToRasTransformNode)
  {
    vtkWarningMacro("SetupTreatmentMachineModels: Table top to RAS valid");
    tableTopModel->SetAndObserveTransformNodeID(tableTopToRasTransformNode->GetID());
    tableTopModel->CreateDefaultDisplayNodes();
    tableTopModel->GetDisplayNode()->SetColor(0, 0, 0);
  }
  else
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Table top to RAS invalid");
  }

  // Linac body - optional
  vtkMRMLModelNode* linacBodyModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(LINACBODY_MODEL_NAME) );
  if (linacBodyModel)
  {
    vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode =
      this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::FixedReference, vtkSlicerIECTransformLogic::RAS);
    if (fixedReferenceToRasTransformNode)
    {
      vtkWarningMacro("SetupTreatmentMachineModels: Fixed reference to RAS valid");
      linacBodyModel->SetAndObserveTransformNodeID(fixedReferenceToRasTransformNode->GetID());
      linacBodyModel->CreateDefaultDisplayNodes();
      linacBodyModel->GetDisplayNode()->SetColor(0.9, 0.9, 0.9);
    }
    else
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Fixed reference to RAS invalid");
    }
  }

  //
  // Set up collision detection between components
  this->GantryTableTopCollisionDetection->SetInput(0, gantryModel->GetPolyData());
  this->GantryTableTopCollisionDetection->SetInput(1, tableTopModel->GetPolyData());

  this->GantryPatientSupportCollisionDetection->SetInput(0, gantryModel->GetPolyData());
  this->GantryPatientSupportCollisionDetection->SetInput(1, patientSupportModel->GetPolyData());

  this->CollimatorTableTopCollisionDetection->SetInput(0, collimatorModel->GetPolyData());
  this->CollimatorTableTopCollisionDetection->SetInput(1, tableTopModel->GetPolyData());

  // Patient model is set when calculating collisions, as it can be changed dynamically
  this->GantryPatientCollisionDetection->SetInput(0, gantryModel->GetPolyData());
  this->CollimatorPatientCollisionDetection->SetInput(0, collimatorModel->GetPolyData());
  // Set identity transform for patient (parent transform is taken into account when getting poly data from segmentation)
  vtkNew<vtkTransform> identityTransform;
  identityTransform->Identity();
  this->GantryPatientCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(identityTransform));
  this->CollimatorPatientCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(identityTransform));
}

//-----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerRoomsEyeViewModuleLogic::UpdateTreatmentOrientationMarker()
{
  vtkNew<vtkAppendPolyData> appendFilter;

  vtkMRMLModelNode* gantryModel = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(GANTRY_MODEL_NAME));
  vtkMRMLModelNode* collimatorModel = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(COLLIMATOR_MODEL_NAME));
  vtkMRMLModelNode* patientSupportModel = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(PATIENTSUPPORT_MODEL_NAME));
  vtkMRMLModelNode* tableTopModel = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(TABLETOP_MODEL_NAME));
  if ( !gantryModel->GetPolyData() || !collimatorModel->GetPolyData() || !patientSupportModel->GetPolyData() || !tableTopModel->GetPolyData() )
  {
    // Orientation marker cannot be assembled if poly data is missing from the mandatory model nodes.
    // This is possible and can be completely valid, for example after closing the scene (because the model nodes are singletons)
    return nullptr;
  }

  //
  // Mandatory models

  // Gantry
  vtkNew<vtkPolyData> gantryModelPolyData;
  gantryModelPolyData->DeepCopy(gantryModel->GetPolyData());

  vtkMRMLLinearTransformNode* gantryToFixedReferenceTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Gantry, vtkSlicerIECTransformLogic::FixedReference);
  vtkNew<vtkTransformFilter> gantryTransformFilter;
  gantryTransformFilter->SetInputData(gantryModelPolyData);
  vtkNew<vtkGeneralTransform> gantryToFixedReferenceTransform;
  gantryToFixedReferenceTransformNode->GetTransformFromWorld(gantryToFixedReferenceTransform);
  gantryToFixedReferenceTransform->Inverse();
  gantryTransformFilter->SetTransform(gantryToFixedReferenceTransform);
  gantryTransformFilter->Update();

  appendFilter->AddInputData(vtkPolyData::SafeDownCast(gantryTransformFilter->GetOutput()));

  // Collimator
  vtkNew<vtkPolyData> collimatorModelPolyData;
  collimatorModelPolyData->DeepCopy(collimatorModel->GetPolyData());

  vtkMRMLLinearTransformNode* collimatorToGantryTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Collimator, vtkSlicerIECTransformLogic::Gantry);
  vtkNew<vtkTransformFilter> collimatorTransformFilter;
  collimatorTransformFilter->SetInputData(collimatorModelPolyData);
  vtkNew<vtkGeneralTransform> collimatorToGantryTransform;
  collimatorToGantryTransformNode->GetTransformFromWorld(collimatorToGantryTransform);
  collimatorToGantryTransform->Inverse();
  collimatorTransformFilter->SetTransform(collimatorToGantryTransform);
  collimatorTransformFilter->Update();

  appendFilter->AddInputData(vtkPolyData::SafeDownCast(collimatorTransformFilter->GetOutput()));

  // Patient support
  vtkNew<vtkPolyData> patientSupportModelPolyData;
  patientSupportModelPolyData->DeepCopy(patientSupportModel->GetPolyData());

  vtkMRMLLinearTransformNode* patientSupportToPatientSupportRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::PatientSupport, vtkSlicerIECTransformLogic::PatientSupportRotation);
  vtkNew<vtkTransformFilter> patientSupportTransformFilter;
  patientSupportTransformFilter->SetInputData(patientSupportModelPolyData);
  vtkNew<vtkGeneralTransform> patientSupportToPatientSupportRotationTransform;
  patientSupportToPatientSupportRotationTransformNode->GetTransformFromWorld(patientSupportToPatientSupportRotationTransform);
  patientSupportToPatientSupportRotationTransform->Inverse();
  patientSupportTransformFilter->SetTransform(patientSupportToPatientSupportRotationTransform);
  patientSupportTransformFilter->Update();

  appendFilter->AddInputData(vtkPolyData::SafeDownCast(patientSupportTransformFilter->GetOutput()));

  // Table top
  vtkNew<vtkPolyData> tableTopModelPolyData;
  tableTopModelPolyData->DeepCopy(tableTopModel->GetPolyData());

  vtkMRMLLinearTransformNode* tableTopToTableTopEccentricRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::TableTop, vtkSlicerIECTransformLogic::TableTopEccentricRotation);
  vtkNew<vtkTransformFilter> tableTopTransformFilter;
  tableTopTransformFilter->SetInputData(tableTopModelPolyData);
  vtkNew<vtkGeneralTransform> tableTopModelTransform;
  tableTopToTableTopEccentricRotationTransformNode->GetTransformFromWorld(tableTopModelTransform);
  tableTopModelTransform->Inverse();
  tableTopTransformFilter->SetTransform(tableTopModelTransform);
  tableTopTransformFilter->Update();

  appendFilter->AddInputData(vtkPolyData::SafeDownCast(tableTopTransformFilter->GetOutput()));

  // Optional models
  vtkMRMLModelNode* leftImagingPanelModel = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(IMAGINGPANELLEFT_MODEL_NAME));
  if (leftImagingPanelModel)
  {
    vtkNew<vtkPolyData> leftImagingPanelModelPolyData;
    leftImagingPanelModelPolyData->DeepCopy(leftImagingPanelModel->GetPolyData());

    vtkMRMLLinearTransformNode* leftImagingPanelToGantryTransformNode =
      this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::LeftImagingPanel, vtkSlicerIECTransformLogic::Gantry);
    vtkNew<vtkTransformFilter> leftImagingPanelTransformFilter;
    leftImagingPanelTransformFilter->SetInputData(leftImagingPanelModelPolyData);
    vtkNew<vtkGeneralTransform> leftImagingPanelToGantryTransform;
    leftImagingPanelToGantryTransformNode->GetTransformFromWorld(leftImagingPanelToGantryTransform);
    leftImagingPanelToGantryTransform->Inverse();
    leftImagingPanelTransformFilter->SetTransform(leftImagingPanelToGantryTransform);
    leftImagingPanelTransformFilter->Update();

    appendFilter->AddInputData(vtkPolyData::SafeDownCast(leftImagingPanelTransformFilter->GetOutput()));
  }
  vtkMRMLModelNode* rightImagingPanelModel = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(IMAGINGPANELRIGHT_MODEL_NAME));
  if (rightImagingPanelModel)
  {
    vtkNew<vtkPolyData> rightImagingPanelModelPolyData;
    rightImagingPanelModelPolyData->DeepCopy(rightImagingPanelModel->GetPolyData());

    vtkMRMLLinearTransformNode* rightImagingPanelToGantryTransformNode =
      this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::RightImagingPanel, vtkSlicerIECTransformLogic::Gantry);
    vtkNew<vtkTransformFilter> rightImagingPanelTransformFilter;
    rightImagingPanelTransformFilter->SetInputData(rightImagingPanelModelPolyData);
    vtkNew<vtkGeneralTransform> rightImagingPanelToGantryTransform;
    rightImagingPanelToGantryTransformNode->GetTransformFromWorld(rightImagingPanelToGantryTransform);
    rightImagingPanelToGantryTransform->Inverse();
    rightImagingPanelTransformFilter->SetTransform(rightImagingPanelToGantryTransform);
    rightImagingPanelTransformFilter->Update();

    appendFilter->AddInputData(vtkPolyData::SafeDownCast(rightImagingPanelTransformFilter->GetOutput()));
  }
  vtkMRMLModelNode* flatPanelModel = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(FLATPANEL_MODEL_NAME));
  if (flatPanelModel)
  {
    vtkNew<vtkPolyData> flatPanelModelPolyData;
    flatPanelModelPolyData->DeepCopy(flatPanelModel->GetPolyData());

    vtkMRMLLinearTransformNode* flatPanelToGantryTransformNode =
      this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::FlatPanel, vtkSlicerIECTransformLogic::Gantry);
    vtkNew<vtkTransformFilter> flatPanelTransformFilter;
    flatPanelTransformFilter->SetInputData(flatPanelModelPolyData);
    vtkNew<vtkGeneralTransform> flatPanelToGantryTransform;
    flatPanelToGantryTransformNode->GetTransformFromWorld(flatPanelToGantryTransform);
    flatPanelToGantryTransform->Inverse();
    flatPanelTransformFilter->SetTransform(flatPanelToGantryTransform);
    flatPanelTransformFilter->Update();

    appendFilter->AddInputData(vtkPolyData::SafeDownCast(flatPanelTransformFilter->GetOutput()));
  }

  vtkNew<vtkPolyData> orientationMarkerPolyData;
  appendFilter->Update();
  orientationMarkerPolyData->DeepCopy(appendFilter->GetOutput());

  // Get or create orientation marker model node
  vtkSmartPointer<vtkMRMLModelNode> orientationMarkerModel =
    vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(ORIENTATION_MARKER_MODEL_NODE_NAME));
  if (!orientationMarkerModel)
  {
    orientationMarkerModel = vtkSmartPointer<vtkMRMLModelNode>::New();
    orientationMarkerModel->SetName(ORIENTATION_MARKER_MODEL_NODE_NAME);
    this->GetMRMLScene()->AddNode(orientationMarkerModel);
  }
  orientationMarkerModel->SetAndObservePolyData(orientationMarkerPolyData);

  return orientationMarkerModel.GetPointer();
}

//----------------------------------------------------------------------------
bool vtkSlicerRoomsEyeViewModuleLogic::GetPatientBodyPolyData(vtkMRMLRoomsEyeViewNode* parameterNode, vtkPolyData* patientBodyPolyData)
{
  if (!parameterNode)
  {
    vtkErrorMacro("GetPatientBodyPolyData: Invalid parameter set node");
    return false;
  }
  if (!patientBodyPolyData)
  {
    vtkErrorMacro("GetPatientBodyPolyData: Invalid output poly data");
    return false;
  }

  // Get patient body segmentation
  vtkMRMLSegmentationNode* segmentationNode = parameterNode->GetPatientBodySegmentationNode();
  if (!segmentationNode || !parameterNode->GetPatientBodySegmentID())
  {
    return false;
  }

  // Get closed surface representation for patient body
  return vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation(
    segmentationNode, parameterNode->GetPatientBodySegmentID(),
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(),
    patientBodyPolyData );
}

//----------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::UpdateCollimatorToGantryTransform(vtkMRMLRoomsEyeViewNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateFixedReferenceIsocenterToCollimatorRotatedTransform: Invalid parameter set node");
    return;
  }

  vtkMRMLLinearTransformNode* collimatorToGantryTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Collimator, vtkSlicerIECTransformLogic::Gantry);
  if (collimatorToGantryTransformNode)
  {
    vtkWarningMacro("UpdateCollimatorToGantryTransform: CollimatorToGantryTransformNode valid");
    vtkNew<vtkTransform> collimatorToGantryTransform;
    collimatorToGantryTransform->RotateZ(parameterNode->GetCollimatorRotationAngle());
    collimatorToGantryTransformNode->SetAndObserveTransformToParent(collimatorToGantryTransform);
  }
  else
  {
    vtkErrorMacro("UpdateCollimatorToGantryTransform: CollimatorToGantryTransformNode invalid");
  }
}

//----------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::UpdateGantryToFixedReferenceTransform(vtkMRMLRoomsEyeViewNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateGantryToFixedReferenceTransform: Invalid parameter set node");
    return;
  }

  vtkMRMLLinearTransformNode* gantryToFixedReferenceTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Gantry, vtkSlicerIECTransformLogic::FixedReference);
  if (gantryToFixedReferenceTransformNode)
  {
    vtkWarningMacro("UpdateGantryToFixedReferenceTransform: GantryToFixedReferenceTransformNode valid");
    vtkNew<vtkTransform> gantryToFixedReferenceTransform;
    gantryToFixedReferenceTransform->RotateY(parameterNode->GetGantryRotationAngle());
    gantryToFixedReferenceTransformNode->SetAndObserveTransformToParent(gantryToFixedReferenceTransform);
  }
  else
  {
    vtkErrorMacro("UpdateGantryToFixedReferenceTransform: GantryToFixedReferenceTransformNode invalid");
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::UpdatePatientSupportRotationToFixedReferenceTransform(vtkMRMLRoomsEyeViewNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdatePatientSupportRotationToFixedReferenceTransform: Invalid parameter set node");
    return;
  }

  vtkMRMLLinearTransformNode* patientSupportRotationToFixedReferenceTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::PatientSupportRotation, vtkSlicerIECTransformLogic::FixedReference);

  if (patientSupportRotationToFixedReferenceTransformNode)
  {
    vtkWarningMacro("UpdatePatientSupportRotationToFixedReferenceTransform: PatientSupportRotationToFixedReferenceTransformNode valid");
    double rotationAngle = parameterNode->GetPatientSupportRotationAngle();
    vtkNew<vtkTransform> patientSupportToRotatedPatientSupportTransform;
    patientSupportToRotatedPatientSupportTransform->RotateZ(rotationAngle);
    patientSupportRotationToFixedReferenceTransformNode->SetAndObserveTransformToParent(patientSupportToRotatedPatientSupportTransform);
  }
  else
  {
    vtkErrorMacro("UpdatePatientSupportRotationToFixedReferenceTransform: PatientSupportRotationToFixedReferenceTransformNode invalid");
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::UpdatePatientSupportToPatientSupportRotationTransform(vtkMRMLRoomsEyeViewNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdatePatientSupportToPatientSupportRotationTransform: Invalid parameter set node");
    return;
  }

  vtkMRMLModelNode* patientSupportModel = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(PATIENTSUPPORT_MODEL_NAME));
  if (!patientSupportModel)
  {
    vtkErrorMacro("UpdatePatientSupportToPatientSupportRotationTransform: Invalid MRML model node");
  }
  vtkPolyData* patientSupportModelPolyData = patientSupportModel->GetPolyData();
  double patientSupportModelBounds[6] = { 0, 0, 0, 0, 0, 0 };
  patientSupportModelPolyData->GetBounds(patientSupportModelBounds);

  // Translation to origin for in-place vertical scaling
  vtkNew<vtkTransform> patientSupportRotationToRasTransform;
  double patientSupportTranslationToOrigin[3] = { 0, 0, (-1.0) * patientSupportModelBounds[4]}; //TODO: Subtract [1]?
  patientSupportRotationToRasTransform->Translate(patientSupportTranslationToOrigin);

  // Vertical scaling
  double tableTopDisplacement = parameterNode->GetVerticalTableTopDisplacement();
  double tableTopDisplacementScaling = 1.0;
  char* treatmentMachineType = parameterNode->GetTreatmentMachineType();
  if (treatmentMachineType && !strcmp(treatmentMachineType, "VarianTrueBeamSTx"))
  {
    tableTopDisplacementScaling = 0.525;
  }
  else if (treatmentMachineType && !strcmp(treatmentMachineType, "SiemensArtiste"))
  {
    tableTopDisplacementScaling = 0.095;
  }
  vtkNew<vtkTransform> rasToScaledRasTransform;
  rasToScaledRasTransform->Scale(1, 1,
    ( ( fabs(patientSupportModelBounds[5]) + tableTopDisplacement*tableTopDisplacementScaling)
      / fabs(patientSupportModelBounds[5]) ) ); //TODO: Subtract [2]?

  // Translation back from origin after in-place scaling
  vtkNew<vtkTransform> scaledRasToFixedReferenceTransform;
  double patientSupportTranslationFromOrigin[3] = { 0, 0, patientSupportModelBounds[4] }; //TODO: Subtract [1]?
  scaledRasToFixedReferenceTransform->Translate(patientSupportTranslationFromOrigin);

  // Assemble transform and update node
  vtkMRMLLinearTransformNode* patientSupportToPatientSupportRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::PatientSupport, vtkSlicerIECTransformLogic::PatientSupportRotation);
  if (patientSupportToPatientSupportRotationTransformNode)
  {
    vtkWarningMacro("UpdatePatientSupportToPatientSupportRotationTransform: PatientSupportToPatientSupportRotationTransformNode valid");
    vtkNew<vtkTransform> patientSupportToFixedReferenceTransform;
    patientSupportToFixedReferenceTransform->PostMultiply();
    patientSupportToFixedReferenceTransform->Concatenate(patientSupportRotationToRasTransform);
    patientSupportToFixedReferenceTransform->Concatenate(rasToScaledRasTransform);
    patientSupportToFixedReferenceTransform->Concatenate(scaledRasToFixedReferenceTransform);
    patientSupportToPatientSupportRotationTransformNode->SetAndObserveTransformToParent(patientSupportToFixedReferenceTransform);
  }
  else
  {
    vtkErrorMacro("UpdatePatientSupportToPatientSupportRotationTransform: PatientSupportToPatientSupportRotationTransformNode invalid");
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::UpdateTableTopEccentricRotationToPatientSupportRotationTransform(vtkMRMLRoomsEyeViewNode* vtkNotUsed(parameterNode))
{
  vtkErrorMacro("UpdateTableTopEccentricRotationToPatientSupportRotationTransform: Not implemented, as table top eccentric rotation is not yet supported");
}

//-----------------------------------------------------------------------------
void vtkSlicerRoomsEyeViewModuleLogic::UpdateTableTopToTableTopEccentricRotationTransform(vtkMRMLRoomsEyeViewNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopToTableTopEccentricRotationTransform: Invalid parameter set node");
    return;
  }

  vtkMRMLLinearTransformNode* tableTopToTableTopEccentricRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::TableTop, vtkSlicerIECTransformLogic::TableTopEccentricRotation);
  if (tableTopToTableTopEccentricRotationTransformNode)
  {
    vtkWarningMacro("UpdateTableTopToTableTopEccentricRotationTransform: TableTopToTableTopEccentricRotationTransformNode valid");

    vtkTransform* tableTopEccentricRotationToPatientSupportTransform = vtkTransform::SafeDownCast(
      tableTopToTableTopEccentricRotationTransformNode->GetTransformToParent() );

    double translationArray[3] =
      { parameterNode->GetLateralTableTopDisplacement(), parameterNode->GetLongitudinalTableTopDisplacement(), parameterNode->GetVerticalTableTopDisplacement() };

    vtkNew<vtkMatrix4x4> tableTopEccentricRotationToPatientSupportMatrix;
    tableTopEccentricRotationToPatientSupportMatrix->SetElement(0,3, translationArray[0]);
    tableTopEccentricRotationToPatientSupportMatrix->SetElement(1,3, translationArray[1]);
    tableTopEccentricRotationToPatientSupportMatrix->SetElement(2,3, translationArray[2]);
    tableTopEccentricRotationToPatientSupportTransform->SetMatrix(tableTopEccentricRotationToPatientSupportMatrix);
    tableTopEccentricRotationToPatientSupportTransform->Modified();
  }
  else
  {
    vtkErrorMacro("UpdateTableTopToTableTopEccentricRotationTransform: TableTopToTableTopEccentricRotationTransformNode invalid");
  }
}

//-----------------------------------------------------------------------------
std::string vtkSlicerRoomsEyeViewModuleLogic::CheckForCollisions(vtkMRMLRoomsEyeViewNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorMacro("CheckForCollisions: Invalid parameter set node");
    return "Invalid parameters";
  }
  if (!parameterNode->GetCollisionDetectionEnabled())
  {
    return "";
  }

  std::string statusString = "";

  // Get transforms used in the collision detection filters
  vtkMRMLLinearTransformNode* gantryToFixedReferenceTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Gantry, vtkSlicerIECTransformLogic::FixedReference);
  vtkMRMLLinearTransformNode* patientSupportToPatientSupportRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::PatientSupport, vtkSlicerIECTransformLogic::PatientSupportRotation);
  vtkMRMLLinearTransformNode* collimatorToGantryTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Collimator, vtkSlicerIECTransformLogic::Gantry);
  vtkMRMLLinearTransformNode* tableTopToTableTopEccentricRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::TableTop, vtkSlicerIECTransformLogic::TableTopEccentricRotation);

  if ( !gantryToFixedReferenceTransformNode || !patientSupportToPatientSupportRotationTransformNode
    || !collimatorToGantryTransformNode || !tableTopToTableTopEccentricRotationTransformNode )
  {
    statusString = "Failed to access IEC transforms";
    vtkErrorMacro("CheckForCollisions: " + statusString);
    return statusString;
  }

  // Get transforms to world, make sure they are linear
  vtkNew<vtkGeneralTransform> gantryToRasGeneralTransform;
  gantryToFixedReferenceTransformNode->GetTransformToWorld(gantryToRasGeneralTransform);
  vtkNew<vtkTransform> gantryToRasTransform;

  vtkNew<vtkGeneralTransform> patientSupportToRasGeneralTransform;
  patientSupportToPatientSupportRotationTransformNode->GetTransformToWorld(patientSupportToRasGeneralTransform);
  vtkNew<vtkTransform> patientSupportToRasTransform;

  vtkNew<vtkGeneralTransform> collimatorToRasGeneralTransform;
  collimatorToGantryTransformNode->GetTransformToWorld(collimatorToRasGeneralTransform);
  vtkNew<vtkTransform> collimatorToRasTransform ;

  vtkNew<vtkGeneralTransform> tableTopToRasGeneralTransform;
  tableTopToTableTopEccentricRotationTransformNode->GetTransformToWorld(tableTopToRasGeneralTransform);
  vtkNew<vtkTransform> tableTopToRasTransform;

  if ( !vtkMRMLTransformNode::IsGeneralTransformLinear(gantryToRasGeneralTransform, gantryToRasTransform)
    || !vtkMRMLTransformNode::IsGeneralTransformLinear(patientSupportToRasGeneralTransform, patientSupportToRasTransform)
    || !vtkMRMLTransformNode::IsGeneralTransformLinear(collimatorToRasGeneralTransform, collimatorToRasTransform)
    || !vtkMRMLTransformNode::IsGeneralTransformLinear(tableTopToRasGeneralTransform, tableTopToRasTransform) )
  {
    statusString = "Non-linear transform detected";
    vtkErrorMacro("CheckForCollisions: " + statusString);
    return statusString;
  }

  // If number of contacts between pieces of treatment room is greater than 0, the collision between which pieces
  // will be set to the output string and returned by the function.
  this->GantryTableTopCollisionDetection->SetTransform(0, vtkLinearTransform::SafeDownCast(gantryToRasTransform));
  this->GantryTableTopCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(tableTopToRasTransform));
  this->GantryTableTopCollisionDetection->Update();
  if (this->GantryTableTopCollisionDetection->GetNumberOfContacts() > 0)
  {
    statusString = statusString + "Collision between gantry and table top\n";
  }

  this->GantryPatientSupportCollisionDetection->SetTransform(0, vtkLinearTransform::SafeDownCast(gantryToRasTransform));
  this->GantryPatientSupportCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(patientSupportToRasTransform));
  this->GantryPatientSupportCollisionDetection->Update();
  if (this->GantryPatientSupportCollisionDetection->GetNumberOfContacts() > 0)
  {
    statusString = statusString + "Collision between gantry and patient support\n";
  }

  this->CollimatorTableTopCollisionDetection->SetTransform(0, vtkLinearTransform::SafeDownCast(collimatorToRasTransform));
  this->CollimatorTableTopCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(tableTopToRasTransform));
  this->CollimatorTableTopCollisionDetection->Update();
  if (this->CollimatorTableTopCollisionDetection->GetNumberOfContacts() > 0)
  {
    statusString = statusString + "Collision between collimator and table top\n";
  }

  //TODO: Collision detection is disabled for additional devices, see SetupTreatmentMachineModels
  //this->AdditionalModelsTableTopCollisionDetection->Update();
  //if (this->AdditionalModelsTableTopCollisionDetection->GetNumberOfContacts() > 0)
  //{
  //  statusString = statusString + "Collision between additional devices and table top\n";
  //}

  //this->AdditionalModelsPatientSupportCollisionDetection->Update();
  //if (this->AdditionalModelsPatientSupportCollisionDetection->GetNumberOfContacts() > 0)
  //{
  //  statusString = statusString + "Collision between additional devices and patient support\n";
  //}

  // Get patient body poly data
  vtkNew<vtkPolyData> patientBodyPolyData;
  if (this->GetPatientBodyPolyData(parameterNode, patientBodyPolyData))
  {
    this->GantryPatientCollisionDetection->SetInput(1, patientBodyPolyData);
    this->GantryPatientCollisionDetection->SetTransform(0, vtkLinearTransform::SafeDownCast(gantryToRasTransform));
    this->GantryPatientCollisionDetection->Update();
    if (this->GantryPatientCollisionDetection->GetNumberOfContacts() > 0)
    {
      statusString = statusString + "Collision between gantry and patient\n";
    }

    this->CollimatorPatientCollisionDetection->SetInput(1, patientBodyPolyData);
    this->CollimatorPatientCollisionDetection->SetTransform(0, vtkLinearTransform::SafeDownCast(collimatorToRasTransform));
    this->CollimatorPatientCollisionDetection->Update();
    if (this->CollimatorPatientCollisionDetection->GetNumberOfContacts() > 0)
    {
      statusString = statusString + "Collision between collimator and patient\n";
    }
  }

  return statusString;
}
