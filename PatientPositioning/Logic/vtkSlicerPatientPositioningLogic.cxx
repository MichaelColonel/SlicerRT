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

// PatientPositioning Logic includes
#include "vtkSlicerPatientPositioningLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLMarkupsLineNode.h>
#include <vtkMRMLMarkupsPlaneNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>

#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTFixedBeamNode.h>
#include <vtkMRMLRTChannel26Cabin3BeamNode.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkCamera.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkCollisionDetectionFilter.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// RapidJSON includes
#include <rapidjson/document.h>     // rapidjson's DOM-style API
#include <rapidjson/filereadstream.h>

// Slicer includes
#include <vtkSlicerModuleLogic.h>
#include <vtkSlicerModelsLogic.h>

// Modules logic
#include <vtkSlicerDrrImageComputationLogic.h>

const char* vtkSlicerPatientPositioningLogic::FIXEDBEAMAXIS_MARKUPS_LINE_NODE_NAME = "FixedBeamAxis";
const char* vtkSlicerPatientPositioningLogic::FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME = "FixedIsocenter";

const char* vtkSlicerPatientPositioningLogic::DRR_TRANSFORM_NODE_NAME = "DrrPatientPositioningTransform";
const char* vtkSlicerPatientPositioningLogic::DRR_TRANSLATE_NODE_NAME = "DrrPatientPositioningTranslate";

const char* vtkSlicerPatientPositioningLogic::TREATMENT_MACHINE_DESCRIPTOR_FILE_PATH_ATTRIBUTE_NAME = "TreatmentMachineDescriptorFilePath";
unsigned long vtkSlicerPatientPositioningLogic::MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS = 10E+10;

const char* vtkSlicerPatientPositioningLogic::TABLETOP_MARKUPS_PLANE_NODE_NAME = "TableTopMarkupsPlane";
const char* vtkSlicerPatientPositioningLogic::TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME = "TableTopMarkupsFiducial";

namespace
{
rapidjson::Value JSON_EMPTY_VALUE;

const double TableTopUpLeftFixedReference[3] = { -264.5, 1821.6, 210. }; // table top point A, LPS coordinate system
const double TableTopUpRightFixedReference[3] = { 265.5, 1821.6, 210. }; // table top point B, LPS coordinate system
const double TableTopDownRightFixedReference[3] = { 265.5, -178.4, 210. }; // table top point C, LPS coordinate system
const double TableTopDownLeftFixedReference[3] = { -264.5, -178.4, 210. }; // table top point D, LPS coordinate system

const double TableTopHole1FixedReference[3] = { -249.5, -133.4, 210. }; // table top hole "1", LPS coordinate system
const double TableTopMirrorHole1FixedReference[3] = { 250.5, -133.4, 210. }; // table top mirror hole "1", LPS coordinate system

const double TableTopCenterFixedReference[3] = {
  TableTopUpLeftFixedReference[0] + (TableTopUpRightFixedReference[0] - TableTopUpLeftFixedReference[0]) / 2.,
  TableTopDownRightFixedReference[1] + (TableTopUpRightFixedReference[1] - TableTopDownRightFixedReference[1]) / 2., 
  TableTopDownRightFixedReference[2] + (TableTopUpRightFixedReference[2] - TableTopDownRightFixedReference[2]) / 2.
  }; // table top center, LPS coordinate system

const double TableTopUpFixedReference[3] = {
  TableTopUpLeftFixedReference[0] + (TableTopUpRightFixedReference[0] - TableTopUpLeftFixedReference[0]) / 2.,
  TableTopUpLeftFixedReference[1] + (TableTopUpRightFixedReference[1] - TableTopUpLeftFixedReference[1]) / 2., 
  TableTopUpLeftFixedReference[2] + (TableTopUpRightFixedReference[2] - TableTopUpLeftFixedReference[2]) / 2.
  }; // table top middle up, LPS coordinate system

const double TableTopLeftFixedReference[3] = {
  TableTopUpLeftFixedReference[0] + (TableTopDownLeftFixedReference[0] - TableTopUpLeftFixedReference[0]) / 2.,
  TableTopUpLeftFixedReference[1] + (TableTopDownLeftFixedReference[1] - TableTopUpLeftFixedReference[1]) / 2., 
  TableTopUpLeftFixedReference[2] + (TableTopDownLeftFixedReference[2] - TableTopUpLeftFixedReference[2]) / 2.
  }; // table top mirror left, LPS coordinate system

}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPatientPositioningLogic);
//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkSlicerPatientPositioningLogic, DrrImageComputationLogic, vtkSlicerDrrImageComputationLogic);

//---------------------------------------------------------------------------
class vtkSlicerPatientPositioningLogic::vtkInternal
{
public:
  vtkInternal(vtkSlicerPatientPositioningLogic* external);
  ~vtkInternal();

  using CoordSys = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;

  vtkSlicerPatientPositioningLogic* External;
  rapidjson::Document* CurrentTreatmentMachineDescription{ nullptr };

  /// Utility function to get element for treatment machine part
  /// \return Json object if found, otherwise null Json object
  rapidjson::Value& GetTreatmentMachinePart(CoordSys partType);
  rapidjson::Value& GetTreatmentMachinePart(std::string partTypeStr);

  std::string GetTreatmentMachinePartFullFilePath(vtkMRMLPatientPositioningNode* parameterNode, std::string partPath);
  std::string GetTreatmentMachineFileNameWithoutExtension(vtkMRMLPatientPositioningNode* parameterNode);
  std::string GetTreatmentMachinePartModelName(vtkMRMLPatientPositioningNode* parameterNode, CoordSys partType);
  std::vector< CoordSys > GetTreatmentMachineParts();
  vtkMRMLModelNode* GetTreatmentMachinePartModelNode(vtkMRMLPatientPositioningNode* parameterNode, CoordSys partType);
  vtkMRMLModelNode* EnsureTreatmentMachinePartModelNode(vtkMRMLPatientPositioningNode* parameterNode, CoordSys partType, bool optional=false);
};

//---------------------------------------------------------------------------
vtkSlicerPatientPositioningLogic::vtkInternal::vtkInternal(vtkSlicerPatientPositioningLogic* external)
{
  this->External = external;
  this->CurrentTreatmentMachineDescription = new rapidjson::Document;
}

//---------------------------------------------------------------------------
vtkSlicerPatientPositioningLogic::vtkInternal::~vtkInternal()
{
  delete this->CurrentTreatmentMachineDescription;
  this->CurrentTreatmentMachineDescription = nullptr;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerPatientPositioningLogic::vtkInternal::GetTreatmentMachinePart(CoordSys type)
{
  if (type >= CoordSys::CoordinateSystemIdentifier_Last)
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachinePart: Invalid part type given " << type);
    return JSON_EMPTY_VALUE;
  }
  std::string typeStr = this->External->GetTreatmentMachinePartTypeAsString(type);
  return this->GetTreatmentMachinePart(typeStr);
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerPatientPositioningLogic::vtkInternal::GetTreatmentMachinePart(std::string typeStr)
{
  if (this->CurrentTreatmentMachineDescription->IsNull())
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachinePart: No treatment machine descriptor file loaded");
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value::MemberIterator partsIt = this->CurrentTreatmentMachineDescription->FindMember("Part");
  if (partsIt == this->CurrentTreatmentMachineDescription->MemberEnd() || !partsIt->value.IsArray())
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachinePart: Failed to find parts array in treatment machine description");
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& partsArray = partsIt->value;

  // Traverse parts and try to find the element with the given part type
  for (rapidjson::SizeType index=0; index < partsArray.Size(); ++index)
  {
    rapidjson::Value& currentObject = partsArray[index];
    if (currentObject.IsObject())
    {
      rapidjson::Value& currentType = currentObject["Type"];
      if (currentType.IsString() && !typeStr.compare(currentType.GetString()))
      {
        return currentObject;
      }
    }
  }

  // Not found
  return JSON_EMPTY_VALUE;
}

//---------------------------------------------------------------------------
std::vector< vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier >
vtkSlicerPatientPositioningLogic::vtkInternal::GetTreatmentMachineParts()
{
  std::vector< CoordSys > parts;
  if (this->CurrentTreatmentMachineDescription->IsNull())
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachinePart: No treatment machine descriptor file loaded");
    return parts;
  }
  rapidjson::Value::MemberIterator partsIt = this->CurrentTreatmentMachineDescription->FindMember("Part");
  if (partsIt == this->CurrentTreatmentMachineDescription->MemberEnd() || !partsIt->value.IsArray())
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachinePart: Failed to find parts array in treatment machine description");
    return parts;
  }
  rapidjson::Value& partsArray = partsIt->value;

  // Traverse parts and try to find the element with the given part type
  for (rapidjson::SizeType index=0; index < partsArray.Size(); ++index)
  {
    rapidjson::Value& currentObject = partsArray[index];
    if (currentObject.IsObject())
    {
      rapidjson::Value& currentType = currentObject["Number"];
      if (currentType.IsInt() && (currentType.GetInt() < CoordSys::CoordinateSystemIdentifier_Last))
      {
        parts.push_back(static_cast< CoordSys >(currentType.GetInt()));
      }
    }
  }
  return parts;
}

//---------------------------------------------------------------------------
std::string vtkSlicerPatientPositioningLogic::vtkInternal::GetTreatmentMachinePartFullFilePath(
  vtkMRMLPatientPositioningNode* parameterNode, std::string partPath)
{
  if (!parameterNode)
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachinePartFullFilePath: Invalid parameter node given");
    return "";
  }

  if (vtksys::SystemTools::FileIsFullPath(partPath))
  {
    // Simply return the path if it is absolute
    return partPath;
  }

  std::string descriptorFileDir = vtksys::SystemTools::GetFilenamePath(parameterNode->GetTreatmentMachineDescriptorFilePath());
  return descriptorFileDir + "/" + partPath;
}

//---------------------------------------------------------------------------
std::string vtkSlicerPatientPositioningLogic::vtkInternal::GetTreatmentMachineFileNameWithoutExtension(vtkMRMLPatientPositioningNode* parameterNode)
{
  if (!parameterNode)
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachineFileNameWithoutExtension: Invalid parameter node given");
    return "";
  }
  if (!parameterNode->GetTreatmentMachineDescriptorFilePath() || strlen(parameterNode->GetTreatmentMachineDescriptorFilePath()) == 0)
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachineFileNameWithoutExtension: Empty treatment machine descriptor file path");
    return "";
  }

  std::string fileName = vtksys::SystemTools::GetFilenameName(parameterNode->GetTreatmentMachineDescriptorFilePath());
  std::string extension = vtksys::SystemTools::GetFilenameExtension(parameterNode->GetTreatmentMachineDescriptorFilePath());
  return fileName.substr(0, fileName.length() - extension.length());
}

//---------------------------------------------------------------------------
std::string vtkSlicerPatientPositioningLogic::vtkInternal::GetTreatmentMachinePartModelName(
  vtkMRMLPatientPositioningNode* parameterNode, CoordSys partType)
{
  if (!parameterNode)
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachinePartModelName: Invalid parameter node given");
    return "";
  }
  std::string machineType = this->GetTreatmentMachineFileNameWithoutExtension(parameterNode);
  return machineType + "_" + this->External->GetTreatmentMachinePartTypeAsString(partType);
}

//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerPatientPositioningLogic::vtkInternal::GetTreatmentMachinePartModelNode(
  vtkMRMLPatientPositioningNode* parameterNode, CoordSys partType)
{
  if (!parameterNode)
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachinePartModelName: Invalid parameter node given");
    return nullptr;
  }
  std::string partName = this->GetTreatmentMachinePartModelName(parameterNode, partType);
  return vtkMRMLModelNode::SafeDownCast(this->External->GetMRMLScene()->GetFirstNodeByName(partName.c_str()));
}

//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerPatientPositioningLogic::vtkInternal::EnsureTreatmentMachinePartModelNode(
  vtkMRMLPatientPositioningNode* parameterNode, CoordSys partType, bool optional/*=false*/)
{
  vtkMRMLScene* scene = this->External->GetMRMLScene();
  if (!scene || !parameterNode)
  {
    vtkErrorWithObjectMacro(this->External, "EnsureTreatmentMachinePartModelNode: Invalid scene or parameter node");
    return nullptr;
  }
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  if (!shNode)
  {
    vtkErrorWithObjectMacro(this->External, "EnsureTreatmentMachinePartModelNode: Failed to access subject hierarchy node");
    return nullptr;
  }

  // Get root SH item
  std::string machineType = this->GetTreatmentMachineFileNameWithoutExtension(parameterNode);
  std::string rootFolderName = machineType + std::string("_Components");
  vtkIdType rootFolderItem = shNode->GetItemChildWithName(shNode->GetSceneItemID(), rootFolderName);
  if (!rootFolderItem)
  {
    // Create subject hierarchy folder so that the treatment machine can be shown/hidden easily
    rootFolderItem = shNode->CreateFolderItem(shNode->GetSceneItemID(), rootFolderName);
  }

  std::string partName = this->GetTreatmentMachinePartModelName(parameterNode, partType);
  vtkMRMLModelNode* partModelNode = this->GetTreatmentMachinePartModelNode(parameterNode, partType);
  if (!partModelNode)
  {
    // Skip model if state is disabled and part is optional
    std::string state = this->External->GetStateForPartType(this->External->GetTreatmentMachinePartTypeAsString(partType));
    if (state == "Disabled")
    {
      if (optional)
      {
        return nullptr;
      }
      else
      {
        vtkWarningWithObjectMacro(this->External, "EnsureTreatmentMachinePartModelNode: State for part "
          << partName << " is set to Disabled but the part is mandatory. Loading anyway.");
      }
    }     
    // Get model file path
    std::string partModelFilePath = this->External->GetFilePathForPartType(
      this->External->GetTreatmentMachinePartTypeAsString(partType));
    if (partModelFilePath == "")
    {
      if (!optional)
      {
        vtkErrorWithObjectMacro(this->External, "EnsureTreatmentMachinePartModelNode: Failed get file path for part "
          << partName << ". This mandatory part may be missing from the descriptor file");
      }
      return nullptr;
    }
    // Load model from file
    partModelFilePath = this->GetTreatmentMachinePartFullFilePath(parameterNode, partModelFilePath);
    if (vtksys::SystemTools::FileExists(partModelFilePath))
    {
      // Create a models logic for convenient loading of components
      vtkNew<vtkSlicerModelsLogic> modelsLogic;
      modelsLogic->SetMRMLScene(scene);
      partModelNode = modelsLogic->AddModel(partModelFilePath.c_str());
      partModelNode->SetName(partName.c_str());
      vtkIdType partItemID = shNode->GetItemByDataNode(partModelNode);
      shNode->SetItemParent(partItemID, rootFolderItem);
    }
    else if (!optional)
    {
      vtkErrorWithObjectMacro(this->External, "EnsureTreatmentMachinePartModelNode: Failed to load " << partName << " model from file " << partModelFilePath);
      return nullptr;
    }
  }
  return partModelNode;
}

//----------------------------------------------------------------------------
vtkSlicerPatientPositioningLogic::vtkSlicerPatientPositioningLogic()
{
  this->Internal = new vtkInternal(this); 

  this->Channel26RobotsLogic = vtkSlicerChannel26Cabin3RobotsTransformLogic::New();

  this->TableTopElbowCollisionDetection = vtkCollisionDetectionFilter::New();
  this->TableTopElbowCollisionDetection->SetCollisionModeToFirstContact();
  this->TableTopShoulderCollisionDetection = vtkCollisionDetectionFilter::New();
  this->TableTopShoulderCollisionDetection->SetCollisionModeToFirstContact();
  this->TableTopBaseRotationCollisionDetection = vtkCollisionDetectionFilter::New();
  this->TableTopBaseRotationCollisionDetection->SetCollisionModeToFirstContact();
  this->TableTopBaseFixedCollisionDetection = vtkCollisionDetectionFilter::New();
  this->TableTopBaseFixedCollisionDetection->SetCollisionModeToFirstContact();
  this->TableTopFixedReferenceCollisionDetection = vtkCollisionDetectionFilter::New();
  this->TableTopFixedReferenceCollisionDetection->SetCollisionModeToFirstContact();

  this->CollimatorPatientCollisionDetection = vtkCollisionDetectionFilter::New();
  this->CollimatorPatientCollisionDetection->SetCollisionModeToFirstContact();

  this->CollimatorTableTopCollisionDetection = vtkCollisionDetectionFilter::New();
  this->CollimatorTableTopCollisionDetection->SetCollisionModeToFirstContact();

  this->AdditionalModelsTableTopCollisionDetection = vtkCollisionDetectionFilter::New();
  this->AdditionalModelsTableTopCollisionDetection->SetCollisionModeToFirstContact();
  this->AdditionalModelsPatientSupportCollisionDetection = vtkCollisionDetectionFilter::New();
  this->AdditionalModelsPatientSupportCollisionDetection->SetCollisionModeToFirstContact();
}

//----------------------------------------------------------------------------
vtkSlicerPatientPositioningLogic::~vtkSlicerPatientPositioningLogic()
{
  if (this->Channel26RobotsLogic)
  {
    this->Channel26RobotsLogic->Delete();
    this->Channel26RobotsLogic = nullptr;
  }

  if (this->Internal)
  {
    delete this->Internal;
    this->Internal = nullptr;
  }
  if (this->TableTopElbowCollisionDetection)
  {
    this->TableTopElbowCollisionDetection->Delete();
    this->TableTopElbowCollisionDetection = nullptr;
  }
  if (this->TableTopShoulderCollisionDetection)
  {
    this->TableTopShoulderCollisionDetection->Delete();
    this->TableTopShoulderCollisionDetection = nullptr;
  }
  if (this->TableTopBaseRotationCollisionDetection)
  {
    this->TableTopBaseRotationCollisionDetection->Delete();
    this->TableTopBaseRotationCollisionDetection = nullptr;
  }
  if (this->TableTopBaseFixedCollisionDetection)
  {
    this->TableTopBaseFixedCollisionDetection->Delete();
    this->TableTopBaseFixedCollisionDetection = nullptr;
  }
  if (this->TableTopFixedReferenceCollisionDetection)
  {
    this->TableTopFixedReferenceCollisionDetection->Delete();
    this->TableTopFixedReferenceCollisionDetection = nullptr;
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
  if (this->AdditionalModelsTableTopCollisionDetection)
  {
    this->AdditionalModelsTableTopCollisionDetection->Delete();
    this->AdditionalModelsTableTopCollisionDetection = nullptr;
  }
  if (this->AdditionalModelsPatientSupportCollisionDetection)
  {
    this->AdditionalModelsPatientSupportCollisionDetection->Delete();
    this->AdditionalModelsPatientSupportCollisionDetection = nullptr;
  }
  if (this->DrrImageComputationLogic)
  {
    this->SetDrrImageComputationLogic(nullptr);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  this->Superclass::SetMRMLSceneInternal(newScene);

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());

  if (this->Channel26RobotsLogic)
  {
    this->Channel26RobotsLogic->SetMRMLScene(newScene);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }

  if (!scene->IsNodeClassRegistered("vtkMRMLPatientPositioningNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLPatientPositioningNode>::New());
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLChannel26GeometryNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLChannel26GeometryNode>::New());
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
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

  if (caller->IsA("vtkMRMLPatientPositioningNode"))
  {
//    vtkMRMLPatientPositioningNode* parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(caller);

    if (event == vtkCommand::ModifiedEvent)
    {
    }
  }
  if (caller->IsA("vtkMRMLChannel26GeometryNode"))
  {
    vtkMRMLChannel26GeometryNode* channel26Geometry = vtkMRMLChannel26GeometryNode::SafeDownCast(caller);
    if (event == vtkCommand::ModifiedEvent)
    {
      this->Channel26RobotsLogic->UpdateRasToTableTopTransform(channel26Geometry);
      this->Channel26RobotsLogic->UpdateRasToTableXrayFlangeTransform(channel26Geometry);
      this->Channel26RobotsLogic->UpdateRasToTableFlangeTransform(channel26Geometry);
      this->Channel26RobotsLogic->UpdateRasToTableWristTransform(channel26Geometry);
      this->UpdateTableTopPlaneNode(channel26Geometry);
      this->UpdateTableTopFiducialNode(channel26Geometry);
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::UpdateFromMRMLScene()
{
  if (this->GetMRMLScene() == nullptr)
  {
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
  }

  if (node->IsA("vtkMRMLPatientPositioningNode"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
  if (node->IsA("vtkMRMLChannel26GeometryNode"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode* vtkSlicerPatientPositioningLogic::CreateTableTopPlaneNode(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkNew<vtkMRMLMarkupsPlaneNode> tableTopPlaneNode;
  this->GetMRMLScene()->AddNode(tableTopPlaneNode);
  tableTopPlaneNode->SetName(TABLETOP_MARKUPS_PLANE_NODE_NAME);
//  tableTopPlaneNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("C26A_") + TABLETOP_MARKUPS_PLANE_NODE_NAME;
//  tableTopPlaneNode->SetSingletonTag(singletonTag.c_str());
//  tableTopPlaneNode->LockedOn();

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkMatrix4x4> patientToRasMatrix;
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }
  patientToRasTransform->GetMatrix(patientToRasMatrix);


  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateTableTopStandPlaneNode: Invalid MRML scene");
    return nullptr;
  }

  if (parameterNode)
  {
    double tableTopCenter[4] = { TableTopCenterFixedReference[0], TableTopCenterFixedReference[1], TableTopCenterFixedReference[2], 1. };
    double tableTopCenterRAS[4] = { };
    double tableTopUp[4] = { TableTopUpFixedReference[0], TableTopUpFixedReference[1], TableTopUpFixedReference[2], 1. };
    double tableTopUpRAS[4] = { };
    double tableTopLeft[4] = { TableTopLeftFixedReference[0], TableTopLeftFixedReference[1], TableTopLeftFixedReference[2], 1. };
    double tableTopLeftRAS[4] = { };
    patientToRasMatrix->MultiplyPoint( tableTopCenter, tableTopCenterRAS);
    patientToRasMatrix->MultiplyPoint( tableTopUp, tableTopUpRAS);
    patientToRasMatrix->MultiplyPoint( tableTopLeft, tableTopLeftRAS);

    tableTopPlaneNode->SetOrigin(tableTopCenterRAS);
    tableTopPlaneNode->SetPlaneBounds( -264.5, 265.5, -1000., 1000.);
    tableTopPlaneNode->SetSize( 530., 2000.);
    tableTopPlaneNode->SetNormal( 0., -1., 0.);
    tableTopPlaneNode->SetSizeMode(vtkMRMLMarkupsPlaneNode::SizeModeAuto);
    tableTopPlaneNode->SetPlaneType(vtkMRMLMarkupsPlaneNode::PlaneType3Points);

    vtkMRMLMarkupsDisplayNode* tableTopPlaneDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(tableTopPlaneNode->GetDisplayNode());
    if (tableTopPlaneDisplayNode)
    {
      tableTopPlaneDisplayNode->SetScaleHandleVisibility(false);
    }
    vtkMRMLTransformNode* transformNode = this->GetChannel26RobotsTransformLogic()->GetTableTopTransform();

    // add transform to fiducial node
    if (transformNode)
    {
      tableTopPlaneNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }

  return tableTopPlaneNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerPatientPositioningLogic::CreateTableTopFiducialNode(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkNew<vtkMRMLMarkupsFiducialNode> tableTopFiducialNode;
  this->GetMRMLScene()->AddNode(tableTopFiducialNode);
  tableTopFiducialNode->SetName(TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME);
//  tableTopFiducialNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("C26A_") + TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME;
//  tableTopFiducialNode->SetSingletonTag(singletonTag.c_str());
//  tableTopFiducialNode->LockedOn();

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkMatrix4x4> patientToRasMatrix;
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }
  patientToRasTransform->GetMatrix(patientToRasMatrix);


  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateTableTopFiducialNode: Invalid MRML scene");
    return nullptr;
  }

  if (parameterNode)
  {
    for (int i = 0; i < 27; ++i)
    {
      // add point to fiducial node (initial position)
      vtkVector3d p( -1. * TableTopHole1FixedReference[0],
        TableTopHole1FixedReference[2],
        TableTopHole1FixedReference[1] + i * 70.);
      vtkVector3d pm( -1. * TableTopMirrorHole1FixedReference[0],
        TableTopMirrorHole1FixedReference[2],
        TableTopMirrorHole1FixedReference[1] + i * 70.);
      std::string name;
      if (!(i % 2))
      {
        name = std::to_string(i / 2 + 1);
      }
      else
      {
        name = std::string(1, 'A' + (i - 1) / 2);
      }
      

      tableTopFiducialNode->AddControlPoint( p, name.c_str());
      tableTopFiducialNode->AddControlPoint( pm, name.c_str());
    }

    vtkMRMLTransformNode* transformNode = this->GetChannel26RobotsTransformLogic()->GetTableTopTransform();

    // add transform to fiducial node
    if (transformNode)
    {
      tableTopFiducialNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }

  return tableTopFiducialNode;
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::UpdateTableTopPlaneNode(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopPlaneNode: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopPlaneNode: Invalid parameter node");
    return;
  }

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkMatrix4x4> patientToRasMatrix;
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }
  patientToRasTransform->GetMatrix(patientToRasMatrix);

  if (scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME))
  {
    vtkMRMLMarkupsPlaneNode* tableTopPlaneNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLETOP_MARKUPS_PLANE_NODE_NAME));
    if (tableTopPlaneNode && tableTopPlaneNode->GetNumberOfControlPoints() == 0)
    {

      double tableTopCenter[4] = { TableTopCenterFixedReference[0], TableTopCenterFixedReference[1], TableTopCenterFixedReference[2], 1. };
      double tableTopCenterRAS[4] = { };
      double tableTopUp[4] = { TableTopUpFixedReference[0], TableTopUpFixedReference[1], TableTopUpFixedReference[2], 1. };
      double tableTopUpRAS[4] = { };
      double tableTopLeft[4] = { TableTopLeftFixedReference[0], TableTopLeftFixedReference[1], TableTopLeftFixedReference[2], 1. };
      double tableTopLeftRAS[4] = { };
      patientToRasMatrix->MultiplyPoint( tableTopCenter, tableTopCenterRAS);
      patientToRasMatrix->MultiplyPoint( tableTopUp, tableTopUpRAS);
      patientToRasMatrix->MultiplyPoint( tableTopLeft, tableTopLeftRAS);

      vtkVector3d plane1( tableTopLeftRAS[0], tableTopLeftRAS[1], tableTopLeftRAS[2]); // Mirror
      vtkVector3d plane2( tableTopUpRAS[0], tableTopUpRAS[1], tableTopUpRAS[2]); // Middle

      tableTopPlaneNode->SetOrigin(tableTopCenterRAS);
      tableTopPlaneNode->AddControlPoint( plane1, "MirrorPlane");
      tableTopPlaneNode->AddControlPoint( plane2, "MiddlePlane");
    }

    // Update markups plane transform node if it's changed    
    vtkMRMLTransformNode* markupsPlaneTransformNode = this->GetChannel26RobotsTransformLogic()->GetTableTopTransform();

    if (markupsPlaneTransformNode)
    {
      tableTopPlaneNode->SetAndObserveTransformNodeID(markupsPlaneTransformNode->GetID());
    }
  }
  else
  {
    this->CreateTableTopPlaneNode(parameterNode);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::UpdateTableTopFiducialNode(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateTableTopFiducialNode: Invalid MRML scene");
    return;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("UpdateTableTopFiducialNode: Invalid parameter node");
    return;
  }

  if (scene->GetFirstNodeByName(TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* tableTopFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME));
    if (tableTopFiducialNode && tableTopFiducialNode->GetNumberOfControlPoints() == 0)
    {
      for (int i = 0; i < 27; ++i)
      {
        // add point to fiducial node (initial position)
        vtkVector3d p( -1. * TableTopHole1FixedReference[0],
          TableTopHole1FixedReference[2],
          TableTopHole1FixedReference[1] + i * 70.);
        vtkVector3d pm( -1. * TableTopMirrorHole1FixedReference[0],
          TableTopMirrorHole1FixedReference[2],
          TableTopMirrorHole1FixedReference[1] + i * 70.);
        std::string name;
        if (!(i % 2))
        {
          name = std::to_string(i / 2 + 1);
        }
        else
        {
          name = std::string(1, 'A' + (i - 1) / 2);
        }
        tableTopFiducialNode->AddControlPoint( p, name.c_str());
        tableTopFiducialNode->AddControlPoint( pm, name.c_str());
      }

      vtkMRMLTransformNode* transformNode = this->GetChannel26RobotsTransformLogic()->GetTableTopTransform();

      // Update markups fiducial transform node if it's changed
      if (transformNode)
      {
        tableTopFiducialNode->SetAndObserveTransformNodeID(transformNode->GetID());
      }
    }
  }
  else
  {
    this->CreateTableTopFiducialNode(parameterNode);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::BuildRobotsTransformHierarchy()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("BuildRobotTableGeometryTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Build TableTop robot hierarchy
  if (this->Channel26RobotsLogic)
  {
    this->Channel26RobotsLogic->BuildRobotsTransformHierarchy();
  }
}

//----------------------------------------------------------------------------
std::vector<vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier>
vtkSlicerPatientPositioningLogic::LoadTreatmentMachineComponents(vtkMRMLPatientPositioningNode* parameterNode)
{
  using CoordSys = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("LoadTreatmentMachineComponents: Invalid scene");
    return std::vector<CoordSys>();
  }
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  if (!shNode)
  {
    vtkErrorMacro("LoadTreatmentMachineComponents: Failed to access subject hierarchy node");
    return std::vector<CoordSys>();
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineDescriptorFilePath())
  {
    vtkErrorMacro("LoadTreatmentMachineComponents: Invalid parameter node");
    return std::vector<CoordSys>();
  }

  // Make sure the transform hierarchy is in place
  this->BuildRobotsTransformHierarchy();

  std::string moduleShareDirectory = this->GetModuleShareDirectory();
  std::string descriptorFilePath(parameterNode->GetTreatmentMachineDescriptorFilePath());
  std::string machineType = this->Internal->GetTreatmentMachineFileNameWithoutExtension(parameterNode);

  // Load treatment machine JSON descriptor file
  FILE *fp = fopen(descriptorFilePath.c_str(), "r");
  if (!fp)
  {
    vtkErrorMacro("LoadTreatmentMachineComponents: Failed to load treatment machine descriptor file '" << descriptorFilePath << "'");
    return std::vector<CoordSys>();
  }
  constexpr size_t size = 1000000;
  std::unique_ptr< char[] > buffer(new char[size]);
  rapidjson::FileReadStream fs(fp, buffer.get(), sizeof(buffer));
  if (this->Internal->CurrentTreatmentMachineDescription->ParseStream(fs).HasParseError())
  {
    vtkErrorMacro("LoadTreatmentMachineComponents: Failed to load treatment machine descriptor file '" << descriptorFilePath << "'");
    fclose(fp);
    return std::vector<CoordSys>();
  }
  fclose(fp);

  // Create subject hierarchy folder so that the treatment machine can be shown/hidden easily
  std::string subjectHierarchyFolderName = machineType + std::string("_Components");
  vtkIdType rootFolderItem = shNode->CreateFolderItem(shNode->GetSceneItemID(), subjectHierarchyFolderName);
  shNode->SetItemAttribute(rootFolderItem, TREATMENT_MACHINE_DESCRIPTOR_FILE_PATH_ATTRIBUTE_NAME, descriptorFilePath);

  std::vector< CoordSys > parts = this->Internal->GetTreatmentMachineParts();
  for (CoordSys part : parts)
  {
    // Load treatment machine models
    // Fixed reference - mandatory
    // Table top - mandatory
    this->Internal->EnsureTreatmentMachinePartModelNode(parameterNode, part);
  }
  // Setup treatment machine model display and transforms
  return this->SetupTreatmentMachineModels(parameterNode);
}

//----------------------------------------------------------------------------
std::vector<vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier>
vtkSlicerPatientPositioningLogic::SetupTreatmentMachineModels(vtkMRMLPatientPositioningNode* parameterNode, bool forceEnableCollisionDetection/*=false*/)
{
  using CoordSys = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return std::vector<CoordSys>();
  }
  vtkMRMLChannel26GeometryNode* channel26GeometryNode = parameterNode->GetChannel26GeometryNode();
  if (!channel26GeometryNode)
  {
    return std::vector<CoordSys>();
  }
  
  std::vector<CoordSys> loadedParts;
  std::map<CoordSys, unsigned int> loadedPartsNumTriangles;
  std::vector<CoordSys> parts = this->Internal->GetTreatmentMachineParts();
  for (CoordSys partIdx : parts)
  {
    std::string partType = this->GetTreatmentMachinePartTypeAsString(partIdx);
    vtkMRMLModelNode* partModel = this->Internal->GetTreatmentMachinePartModelNode(parameterNode, partIdx);
    if (!partModel || !partModel->GetPolyData())
    {
      switch (partIdx)
      {
        case CoordSys::FixedReference:
        case CoordSys::TableTop:
        case CoordSys::TableXrayFlange:
        case CoordSys::TableFlange:
        case CoordSys::TableWrist:
          vtkErrorMacro("SetupTreatmentMachineModels: Unable to access " << partType << " model " << partIdx);
          break;
        default:
          break;
      }
      continue;
    }
    else
    {
      loadedParts.push_back(partIdx);
      loadedPartsNumTriangles[partIdx] = partModel->GetPolyData()->GetNumberOfCells();

      // Set color
      vtkVector3d partColor(this->GetColorForPartType(partType));
      partModel->CreateDefaultDisplayNodes();
      partModel->GetDisplayNode()->SetColor((double)partColor[0] / 255.0, (double)partColor[1] / 255.0, (double)partColor[2] / 255.0);

      // Apply file to RAS transform matrix
      vtkNew<vtkMatrix4x4> fileToRASTransformMatrix;
      if (this->GetFileToRASTransformMatrixForPartType(partType, fileToRASTransformMatrix))
      {
        vtkNew<vtkTransform> fileToRASTransform;
        fileToRASTransform->SetMatrix(fileToRASTransformMatrix);
        vtkNew<vtkTransformPolyDataFilter> transformPolyDataFilter;
        transformPolyDataFilter->SetInputConnection(partModel->GetPolyDataConnection());
        transformPolyDataFilter->SetTransform(fileToRASTransform);
        transformPolyDataFilter->Update();
        vtkNew<vtkPolyData> partPolyDataRAS;
        partPolyDataRAS->DeepCopy(transformPolyDataFilter->GetOutput());
        partModel->SetAndObservePolyData(partPolyDataRAS);
      }
      else
      {
        vtkErrorMacro("SetupTreatmentMachineModels: Failed to set file to RAS matrix for treatment machine part " << partType);
      }
    }

    if (partIdx == CoordSys::TableFlange)
    {
      this->Channel26RobotsLogic->UpdateTableXrayFlangeToTableFlangeTransform(channel26GeometryNode);
      vtkMRMLLinearTransformNode* rasToFlangeTransformNode = this->Channel26RobotsLogic->UpdateRasToTableFlangeTransform(channel26GeometryNode);
      if (rasToFlangeTransformNode)
      {
        partModel->SetAndObserveTransformNodeID(rasToFlangeTransformNode->GetID());
      }
    }
    if (partIdx == CoordSys::TableXrayFlange)
    {
      this->Channel26RobotsLogic->UpdateTableTopToTableXrayFlangeTransform(channel26GeometryNode);
      vtkMRMLLinearTransformNode* rasToFlangeTransformNode = this->Channel26RobotsLogic->UpdateRasToTableXrayFlangeTransform(channel26GeometryNode);
      if (rasToFlangeTransformNode)
      {
        partModel->SetAndObserveTransformNodeID(rasToFlangeTransformNode->GetID());
      }
    }
    else if (partIdx == CoordSys::TableTop)
    {
      this->Channel26RobotsLogic->UpdatePatientToTableTopTransform(channel26GeometryNode);
      vtkMRMLLinearTransformNode* rasToTableTopTransformNode = this->Channel26RobotsLogic->UpdateRasToTableTopTransform(channel26GeometryNode);
      if (rasToTableTopTransformNode)
      {
        partModel->SetAndObserveTransformNodeID(rasToTableTopTransformNode->GetID());
      }
    }
    else if (partIdx == CoordSys::FixedReference)
    {
    }
    else if (partIdx == CoordSys::TableWrist)
    {
    }
  }

  return loadedParts;
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::ShowMarkupsNodes(vtkMRMLPatientPositioningNode* parameterNode, bool show)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("ShowMarkupsNodes: Invalid MRML scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("ShowMarkupsNodes: Invalid parameter set node");
    return;
  }

  std::list<std::string> markupsNames;
  markupsNames.push_back(vtkSlicerPatientPositioningLogic::TABLETOP_MARKUPS_PLANE_NODE_NAME);
  markupsNames.push_back(vtkSlicerPatientPositioningLogic::TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME);

  for (auto markupName : markupsNames)
  {
    // model
    vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(markupName.c_str()) );
    if (!markupsNode)
    {
      vtkErrorMacro("ShowModelsNodes: Unable to access markups: " << markupName.c_str());
      continue;
    }
    markupsNode->GetDisplayNode()->SetVisibility(show);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::ShowModelsNodes(vtkMRMLPatientPositioningNode* parameterNode, bool show)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("ShowModelsNodes: Invalid MRML scene");
    return;
  }
  if (!parameterNode)
  {
    vtkErrorMacro("ShowMarkupsNodes: Invalid parameter set node");
    return;
  }
  std::list<std::string> modelsNames;
  modelsNames.push_back("TableFlange");
  modelsNames.push_back("TableXrayFlange");
  modelsNames.push_back("TableTop");

  for (auto modelName : modelsNames)
  {
    std::string fullName = std::string("Cabin26AGeometry_") + modelName;
    // model
    vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(fullName.c_str()) );
    if (!modelNode)
    {
      vtkErrorMacro("ShowModelsNodes: Unable to access model: " << fullName.c_str());
      continue;
    }
    modelNode->GetDisplayNode()->SetVisibility(show);
  }
}

//---------------------------------------------------------------------------
const char* vtkSlicerPatientPositioningLogic::GetTreatmentMachinePartTypeAsString(
  vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier type)
{
  return this->Channel26RobotsLogic->GetTreatmentMachinePartTypeAsString(type);
}

//---------------------------------------------------------------------------
std::string vtkSlicerPatientPositioningLogic::GetNameForPartType(std::string partType)
{
  rapidjson::Value& partObject = this->Internal->GetTreatmentMachinePart(partType);
  if (partObject.IsNull())
  {
    // The part may not have been included in the description
    return "";
  }

  rapidjson::Value& name = partObject["Name"];
  if (!name.IsString())
  {
    vtkErrorMacro("GetNameForPartType: Invalid treatment machine part name for part " << partType);
    return "";
  }

  return name.GetString();
}

//---------------------------------------------------------------------------
std::string vtkSlicerPatientPositioningLogic::GetFilePathForPartType(std::string partType)
{
  rapidjson::Value& partObject = this->Internal->GetTreatmentMachinePart(partType);
  if (partObject.IsNull())
  {
    // The part may not have been included in the description
    return "";
  }

  rapidjson::Value& filePath = partObject["FilePath"];
  if (!filePath.IsString())
  {
    vtkErrorMacro("GetFilePathForPartType: Invalid treatment machine part file path for part " << partType);
    return "";
  }

  return filePath.GetString();
}

//---------------------------------------------------------------------------
bool vtkSlicerPatientPositioningLogic::GetFileToRASTransformMatrixForPartType(std::string partType, vtkMatrix4x4* fileToPartTransformMatrix)
{
  if (!fileToPartTransformMatrix)
  {
    vtkErrorMacro("GetFileToRASTransformMatrixForPartType: Invalid treatment machine file to RAS matrix for part " << partType);
    return false;
  }

  fileToPartTransformMatrix->Identity();

  rapidjson::Value& partObject = this->Internal->GetTreatmentMachinePart(partType);
  if (partObject.IsNull())
  {
    // The part may not have been included in the description
    return false;
  }

  rapidjson::Value& columnsArray = partObject["FileToRASTransformMatrix"];
  if (!columnsArray.IsArray() || columnsArray.Size() != 4)
  {
    vtkErrorMacro("GetFileToRASTransformMatrixForPartType: Invalid treatment machine file to RAS matrix for part " << partType);
    return false;
  }

  for (rapidjson::SizeType i=0; i<columnsArray.Size(); ++i)
  {
    if (!columnsArray[i].IsArray() || columnsArray[i].Size() != 4)
    {
      vtkErrorMacro("GetFileToRASTransformMatrixForPartType: Invalid treatment machine file to RAS matrix for part " << partType
        << " (problem in row " << i << ")");
      return false;
    }
    for (int j=0; j<4; ++j)
    {
      fileToPartTransformMatrix->SetElement(i, j, columnsArray[i][j].GetDouble());
    }
  }

  return true;
}

//---------------------------------------------------------------------------
vtkVector3d vtkSlicerPatientPositioningLogic::GetColorForPartType(std::string partType)
{
  rapidjson::Value& partObject = this->Internal->GetTreatmentMachinePart(partType);
  if (partObject.IsNull())
  {
    // The part may not have been included in the description
    return vtkVector3d(255, 255, 255);
  }

  rapidjson::Value& colorArray = partObject["Color"];
  if (!colorArray.IsArray() || colorArray.Size() != 3 || !colorArray[0].IsInt())
  {
    vtkErrorMacro("GetFilePathForPartType: Invalid treatment machine color for part " << partType);
    return vtkVector3d(255, 255, 255);
  }

  return vtkVector3d( (unsigned char)colorArray[0].GetInt(),
                      (unsigned char)colorArray[1].GetInt(),
                      (unsigned char)colorArray[2].GetInt() );
}

//---------------------------------------------------------------------------
std::string vtkSlicerPatientPositioningLogic::GetStateForPartType(std::string partType)
{
  rapidjson::Value& partObject = this->Internal->GetTreatmentMachinePart(partType);
  if (partObject.IsNull())
  {
    // The part may not have been included in the description
    return "";
  }

  rapidjson::Value& state = partObject["State"];
  if (!state.IsString())
  {
    vtkErrorMacro("GetStateForPartType: Invalid treatment machine state value type for part " << partType);
    return "";
  }

  std::string stateStr(state.GetString());
  if (state != "Disabled" && state != "Active" && state != "Passive")
  {
    vtkErrorMacro("GetStateForPartType: Invalid treatment machine state for part " << partType
      << ". Valid states are Disabled, Active, or Passive.");
    return "";
  }

  return stateStr;
}

vtkSlicerChannel26Cabin3RobotsTransformLogic* vtkSlicerPatientPositioningLogic::GetChannel26RobotsTransformLogic() const
{
  return this->Channel26RobotsLogic;
}
