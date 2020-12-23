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

// IhepStandGeometry Logic includes
#include "vtkSlicerIhepStandGeometryLogic.h"

// SlicerRT MRML includes
#include <vtkMRMLIhepStandGeometryNode.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// SlicerRT IEC transformation logic from Beam module
#include <vtkSlicerIECTransformLogic.h>

// SlicerRT includes
#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>

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

namespace
{
// Initial positions
const double PatientSupportToTableTopSurfaceDistanceAxisZ = 925.; // mm
const double TableTopFromIsocenterDistanceAxisY = 1400.; // mm

}

//----------------------------------------------------------------------------
// Treatment machine component names
const char* vtkSlicerIhepStandGeometryLogic::CANYON_MODEL_NAME = "Canyon";
const char* vtkSlicerIhepStandGeometryLogic::PATIENTSUPPORT_MODEL_NAME = "PatientSupportRotation";
const char* vtkSlicerIhepStandGeometryLogic::TABLETOPSTAND_MODEL_NAME = "TableTopStand";
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

  // Canyon - mandatory
  std::string canyonModelSingletonTag = machineType + "_" + CANYON_MODEL_NAME;
  vtkMRMLModelNode* canyonModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(canyonModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (canyonModelNode && !canyonModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(canyonModelNode);
    canyonModelNode = nullptr;
  }
  if (!canyonModelNode)
  {
    std::string canyonModelFilePath = treatmentMachineModelsDirectory + "/" + CANYON_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(canyonModelFilePath))
    {
      canyonModelNode = modelsLogic->AddModel(canyonModelFilePath.c_str());
    }
    if (canyonModelNode)
    {
      canyonModelNode->SetSingletonTag(canyonModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> canyonModelHierarchyNode;
      scene->AddNode(canyonModelHierarchyNode);
      canyonModelHierarchyNode->SetModelNodeID(canyonModelNode->GetID());
      canyonModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      canyonModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load canyon model");
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

  // Table top inferior-superior movement stand - mandatory
  std::string tableTopStandModelSingletonTag = machineType + "_" + TABLETOPSTAND_MODEL_NAME;
  vtkMRMLModelNode* tableTopStandModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableTopStandModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableTopStandModelNode && !tableTopStandModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableTopStandModelNode);
    tableTopStandModelNode = nullptr;
  }
  if (!tableTopStandModelNode)
  {
    std::string tableTopStandModelFilePath = treatmentMachineModelsDirectory + "/" + TABLETOPSTAND_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableTopStandModelFilePath))
    {
      tableTopStandModelNode = modelsLogic->AddModel(tableTopStandModelFilePath.c_str());
    }
    if (tableTopStandModelNode)
    {
      tableTopStandModelNode->SetSingletonTag(tableTopStandModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableTopStandModelHierarchyNode;
      scene->AddNode(tableTopStandModelHierarchyNode);
      tableTopStandModelHierarchyNode->SetModelNodeID(tableTopStandModelNode->GetID());
      tableTopStandModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableTopStandModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table top stand model");
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

  if ( !canyonModelNode || !canyonModelNode->GetPolyData()
    || !tableTopStandModelNode || !tableTopStandModelNode->GetPolyData()
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
void vtkSlicerIhepStandGeometryLogic::ResetModelsToInitialPosition(vtkMRMLIhepStandGeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("ResetModelsToInitialPosition: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("ResetModelsToInitialPosition: Invalid parameter node");
    return;
  }
  this->IECLogic->UpdateStandTransform();
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::UpdatePatientSupportRotationToFixedReferenceTransform(vtkMRMLIhepStandGeometryNode* parameterNode, double rotationAngle)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdatePatientSupportRotationToFixedReferenceTransform: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("UpdatePatientSupportRotationToFixedReferenceTransform: Invalid parameter node");
    return;
  }

  using IEC = vtkSlicerIECTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* patientSupportRotationToFixedReferenceTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::PatientSupportRotation, IEC::FixedReference);

  if (patientSupportRotationToFixedReferenceTransformNode)
  {
//    double rotationAngle = parameterNode->GetPatientSupportRotationAngle();
    vtkNew<vtkTransform> patientSupportToRotatedPatientSupportTransform;
    patientSupportToRotatedPatientSupportTransform->RotateZ(-1. * rotationAngle);
    patientSupportRotationToFixedReferenceTransformNode->SetAndObserveTransformToParent(patientSupportToRotatedPatientSupportTransform);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::SetupTreatmentMachineModels()
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Invalid scene");
    return;
  }

  //TODO: Store treatment machine component color and other properties in JSON

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IEC = vtkSlicerIECTransformLogic::CoordinateSystemIdentifier;

  // Table top stand - mandatory
  vtkMRMLModelNode* tableTopStandModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(TABLETOPSTAND_MODEL_NAME) );
  if (!tableTopStandModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top stand model");
    return;
  }
  vtkMRMLLinearTransformNode* tableTopStandToFixedReferenceTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::TableTopInferiorSuperiorMovement, IEC::PatientSupportRotation);
  if (tableTopStandToFixedReferenceTransformNode)
  {
    tableTopStandModel->SetAndObserveTransformNodeID(tableTopStandToFixedReferenceTransformNode->GetID());
    tableTopStandModel->CreateDefaultDisplayNodes();
    tableTopStandModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
  }

  // Patient support - mandatory
  vtkMRMLModelNode* patientSupportModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(PATIENTSUPPORT_MODEL_NAME) );
  if (!patientSupportModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access patient support model");
    return;
  }
  vtkMRMLLinearTransformNode* patientSupportToPatientSupportRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::PatientSupport, IEC::PatientSupportRotation);
  if (patientSupportToPatientSupportRotationTransformNode)
  {
    patientSupportModel->SetAndObserveTransformNodeID(patientSupportToPatientSupportRotationTransformNode->GetID());
    patientSupportModel->CreateDefaultDisplayNodes();
    patientSupportModel->GetDisplayNode()->SetColor(0.85, 0.85, 0.85);
  }

  // Table top - mandatory
  vtkMRMLModelNode* tableTopModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(TABLETOP_MODEL_NAME) );
  if (!tableTopModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top model");
    return;
  }
  vtkMRMLLinearTransformNode* tableTopToTableTopEccentricRotationTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::TableTop, IEC::TableTopEccentricRotation);
  if (tableTopToTableTopEccentricRotationTransformNode)
  {
    tableTopModel->SetAndObserveTransformNodeID(tableTopToTableTopEccentricRotationTransformNode->GetID());
    tableTopModel->CreateDefaultDisplayNodes();
    tableTopModel->GetDisplayNode()->SetColor(0, 0, 0);
  }

  // Canyon - mandatory
  vtkMRMLModelNode* canyonModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(CANYON_MODEL_NAME) );
  if (!canyonModel)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Unable to access canyon model");
    return;
  }
  
  vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::FixedReference, IEC::RAS);
  if (fixedReferenceToRasTransformNode)
  {
    canyonModel->SetAndObserveTransformNodeID(fixedReferenceToRasTransformNode->GetID());
    canyonModel->CreateDefaultDisplayNodes();
    canyonModel->GetDisplayNode()->SetColor(0.9, 0.9, 0.9);
  }
/*

  vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::FixedReference, IEC::RAS);

  vtkNew<vtkGeneralTransform> generalTransform;
  if (this->IECLogic->GetTransformBetween( IEC::FixedReference, IEC::Collimator, generalTransform))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    vtkNew<vtkTransform> linearTransform;
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear(generalTransform, linearTransform))
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to get transform hierarchy for the fixed reference model");
      return;
    }
    // Set transform to node
    vtkWarningMacro("SetupTreatmentMachineModels: Fixed reference to RAS hierarchy is valid");

    if (fixedReferenceToRasTransformNode)
    {
      vtkWarningMacro("SetupTreatmentMachineModels: fixed to RAS reference");
//      linearTransform->Inverse();
      fixedReferenceToRasTransformNode->SetAndObserveTransformFromParent(linearTransform);
//      patientSupportRotationToFixedReferenceTransformNode->SetAndObserveTransformToParent(patientSupportToRotatedPatientSupportTransform);
//      tableTopStandModel->SetAndObserveTransformToParent(linearTransform);
      canyonModel->SetAndObserveTransformNodeID(fixedReferenceToRasTransformNode->GetID());
      canyonModel->CreateDefaultDisplayNodes();
      canyonModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
    }
  }
*/
}

//----------------------------------------------------------------------------
void vtkSlicerIhepStandGeometryLogic::MoveModelsToIsocenter(vtkMRMLIhepStandGeometryNode* parameterNode, double isocenter[3])
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("MoveModelsToIsocenter: Invalid scene");
    return;
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineType())
  {
    vtkErrorMacro("MoveModelsToIsocenter: Invalid parameter node");
    return;
  }
  // Move models

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
    using IEC = vtkSlicerIECTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode =
    this->IECLogic->GetTransformNodeBetween(IEC::FixedReference, IEC::RAS);

  vtkNew<vtkGeneralTransform> generalTransform;
  // FixedReference -> PatientSupport -> TableTopEccentricRotation -> TableTop -> Patient -> RAS
  if (this->IECLogic->GetTransformBetween( IEC::FixedReference, IEC::RAS, generalTransform))
  {
    // Convert general transform to linear
    // This call also makes hard copy of the transform so that it doesn't change when other beam transforms change
    vtkNew<vtkTransform> linearTransform;
    if (!vtkMRMLTransformNode::IsGeneralTransformLinear(generalTransform, linearTransform))
    {
      vtkErrorMacro("MoveModelsToIsocenter: Unable to get transform hierarchy for the fixed reference model");
      return;
    }
    // Set transform to node
    vtkWarningMacro("MoveModelsToIsocenter: Fixed reference to RAS hierarchy is valid");

    if (fixedReferenceToRasTransformNode)
    {
      vtkWarningMacro("MoveModelsToIsocenter: fixed to RAS reference");
    }
  }

  vtkTransform* fixedReferenceToRasTransform = vtkTransform::SafeDownCast(fixedReferenceToRasTransformNode->GetTransformToParent());

  if (beamNode)
  {
  vtkMRMLLinearTransformNode* beamTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
    beamNode->GetParentTransformNode());
  vtkErrorMacro("MoveModelsToIsocenter: 1");
  if (beamTransformNode)
  {
  vtkTransform* beamTransform = vtkTransform::SafeDownCast(beamTransformNode->GetTransformToParent());
  vtkErrorMacro("MoveModelsToIsocenter: 2");
  if (beamTransform)
  {
  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
  transformNode->SetName("CanyonToPatientTransform");
//  transformNode->SetHideFromEditors(1);
  scene->AddNode(transformNode);
  vtkErrorMacro("MoveModelsToIsocenter: 3");
  // Translation back from origin after in-place rotation
  vtkNew<vtkTransform> originToIsocenterTransform;
  originToIsocenterTransform->Identity();
  originToIsocenterTransform->Translate( 0., 0., -1550. + isocenter[2]);

  vtkNew<vtkTransform> canyonToPatientTransform;
  canyonToPatientTransform->Identity();

  vtkNew<vtkTransform> rotateZTransform;
  rotateZTransform->Identity();
  rotateZTransform->RotateZ(180);

  canyonToPatientTransform->PostMultiply();
  canyonToPatientTransform->Concatenate(originToIsocenterTransform);
  canyonToPatientTransform->Concatenate(rotateZTransform);
  canyonToPatientTransform->Concatenate(beamTransform);
//  canyonToPatientTransform->Concatenate(fixedReferenceToRasTransform);
  transformNode->SetAndObserveTransformToParent(canyonToPatientTransform);

  // Canyon - mandatory
  vtkMRMLModelNode* canyonModel = vtkMRMLModelNode::SafeDownCast(
    this->GetMRMLScene()->GetFirstNodeByName(CANYON_MODEL_NAME) );
  if (canyonModel)
  {
    vtkErrorMacro("MoveModelsToIsocenter: 4");
    canyonModel->SetAndObserveTransformNodeID(transformNode->GetID());
  }
}
}
}
}
