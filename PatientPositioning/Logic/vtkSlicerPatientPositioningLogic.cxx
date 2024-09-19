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
#include <vtkMRMLCameraNode.h>

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

const char* vtkSlicerPatientPositioningLogic::FIXEDREFERENCE_MARKUPS_LINE_NODE_NAME = "FixedBeamAxis";

const char* vtkSlicerPatientPositioningLogic::DRR_TRANSFORM_NODE_NAME = "DrrPatientPositioningTransform";
const char* vtkSlicerPatientPositioningLogic::DRR_TRANSLATE_NODE_NAME = "DrrPatientPositioningTranslate";

const char* vtkSlicerPatientPositioningLogic::TREATMENT_MACHINE_DESCRIPTOR_FILE_PATH_ATTRIBUTE_NAME = "TreatmentMachineDescriptorFilePath";
unsigned long vtkSlicerPatientPositioningLogic::MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS = 10E+10;

namespace
{
rapidjson::Value JSON_EMPTY_VALUE;
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPatientPositioningLogic);

//---------------------------------------------------------------------------
class vtkSlicerPatientPositioningLogic::vtkInternal
{
public:
  vtkInternal(vtkSlicerPatientPositioningLogic* external);
  ~vtkInternal();

  using CoordSys = vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier;
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
std::vector< vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier >
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
        parts.push_back(static_cast<CoordSys>(currentType.GetInt()));
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

  this->TableTopRobotLogic = vtkSlicerTableTopRobotTransformLogic::New();
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
  if (this->TableTopRobotLogic)
  {
    this->TableTopRobotLogic->Delete();
    this->TableTopRobotLogic = nullptr;
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

  this->TableTopRobotLogic->SetMRMLScene(newScene);
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
  if (!scene->IsNodeClassRegistered("vtkMRMLChannel25GeometryNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLChannel25GeometryNode>::New());
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
  if (caller->IsA("vtkMRMLChannel25GeometryNode"))
  {
    vtkMRMLChannel25GeometryNode* channel25Geometry = vtkMRMLChannel25GeometryNode::SafeDownCast(caller);
    if (event == vtkCommand::ModifiedEvent)
    {
      this->TableTopRobotLogic->UpdateRasToTableTopTransform(channel25Geometry);
      this->TableTopRobotLogic->UpdateRasToFlangeTransform(channel25Geometry);
      this->TableTopRobotLogic->UpdateRasToWristTransform(channel25Geometry);
      this->TableTopRobotLogic->UpdateRasToElbowTransform(channel25Geometry);
      this->TableTopRobotLogic->UpdateRasToShoulderTransform(channel25Geometry);
      this->TableTopRobotLogic->UpdateRasToBaseRotationTransform(channel25Geometry);
      this->TableTopRobotLogic->UpdateRasToBaseFixedTransform(channel25Geometry);
      this->TableTopRobotLogic->UpdateRasToFixedReferenceTransform(channel25Geometry);
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
  if (node->IsA("vtkMRMLChannel25GeometryNode"))
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

//---------------------------------------------------------------------------
vtkVector3d vtkSlicerPatientPositioningLogic::GetFixedBeamAxisTranslation(vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier vtkNotUsed(fromFrame))
{
  return vtkVector3d(0., 0., 0.);
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::BuildRobotTableGeometryTransformHierarchy()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("BuildRobotTableGeometryTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Build IHEP hierarchy
  this->TableTopRobotLogic->BuildTableRobotTransformHierarchy();
  this->TableTopRobotLogic->ResetToInitialPositions();
}

//----------------------------------------------------------------------------
std::vector<vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier>
vtkSlicerPatientPositioningLogic::LoadTreatmentMachineComponents(vtkMRMLPatientPositioningNode* parameterNode)
{
  using CoordSys = vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier;

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
  this->BuildRobotTableGeometryTransformHierarchy();

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

  std::vector<CoordSys> parts = this->Internal->GetTreatmentMachineParts();
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
std::vector<vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier>
vtkSlicerPatientPositioningLogic::SetupTreatmentMachineModels(vtkMRMLPatientPositioningNode* parameterNode, bool forceEnableCollisionDetection/*=false*/)
{
  using CoordSys = vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier;

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return std::vector<CoordSys>();
  }
  vtkMRMLChannel25GeometryNode* channel25GeoNode = parameterNode->GetChannel25GeometryNode();
  if (!channel25GeoNode)
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
        case CoordSys::Flange:
        case CoordSys::BaseFixed:
        case CoordSys::BaseRotation:
        case CoordSys::Shoulder:
        case CoordSys::Elbow:
        case CoordSys::Wrist:
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

    if (partIdx == CoordSys::FixedReference)
    {
      this->TableTopRobotLogic->UpdateBaseFixedToFixedReferenceTransform(channel25GeoNode);
      vtkMRMLLinearTransformNode* rasToFixedReferenceTransformNode = this->TableTopRobotLogic->UpdateRasToFixedReferenceTransform(channel25GeoNode);
      if (rasToFixedReferenceTransformNode)
      {
        this->TableTopFixedReferenceCollisionDetection->SetInputData(1, partModel->GetPolyData());
        partModel->SetAndObserveTransformNodeID(rasToFixedReferenceTransformNode->GetID());
      }
    }
    else if (partIdx == CoordSys::BaseFixed)
    {
      this->TableTopRobotLogic->UpdateBaseRotationToBaseFixedTransform(channel25GeoNode);
      vtkMRMLLinearTransformNode* rasToBaseFixedTransformNode = this->TableTopRobotLogic->UpdateRasToBaseFixedTransform(channel25GeoNode);
      if (rasToBaseFixedTransformNode)
      {
        this->TableTopBaseFixedCollisionDetection->SetInputData(1, partModel->GetPolyData());
        partModel->SetAndObserveTransformNodeID(rasToBaseFixedTransformNode->GetID());
      }
    }
    else if (partIdx == CoordSys::BaseRotation)
    {
      this->TableTopRobotLogic->UpdateShoulderToBaseRotationTransform(channel25GeoNode);
      vtkMRMLLinearTransformNode* rasToBaseRotationTransformNode = this->TableTopRobotLogic->UpdateRasToBaseRotationTransform(channel25GeoNode);
      if (rasToBaseRotationTransformNode)
      {
        this->TableTopBaseRotationCollisionDetection->SetInputData(1, partModel->GetPolyData());
        partModel->SetAndObserveTransformNodeID(rasToBaseRotationTransformNode->GetID());
      }
    }
    else if (partIdx == CoordSys::Shoulder)
    {
      this->TableTopRobotLogic->UpdateElbowToShoulderTransform(channel25GeoNode);
      vtkMRMLLinearTransformNode* rasToShoulderTransformNode = this->TableTopRobotLogic->UpdateRasToShoulderTransform(channel25GeoNode);
      if (rasToShoulderTransformNode)
      {
        this->TableTopShoulderCollisionDetection->SetInputData(1, partModel->GetPolyData());
        partModel->SetAndObserveTransformNodeID(rasToShoulderTransformNode->GetID());
      }
    }
    else if (partIdx == CoordSys::Elbow)
    {
      this->TableTopRobotLogic->UpdateWristToElbowTransform(channel25GeoNode);
      vtkMRMLLinearTransformNode* rasToElbowTransformNode = this->TableTopRobotLogic->UpdateRasToElbowTransform(channel25GeoNode);
      if (rasToElbowTransformNode)
      {
        this->TableTopElbowCollisionDetection->SetInputData(1, partModel->GetPolyData());
        partModel->SetAndObserveTransformNodeID(rasToElbowTransformNode->GetID());
      }
    }
    else if (partIdx == CoordSys::Wrist)
    {
      this->TableTopRobotLogic->UpdateFlangeToWristTransform(channel25GeoNode);
      vtkMRMLLinearTransformNode* rasToWristTransformNode = this->TableTopRobotLogic->UpdateRasToWristTransform(channel25GeoNode);
      if (rasToWristTransformNode)
      {
        partModel->SetAndObserveTransformNodeID(rasToWristTransformNode->GetID());
      }
    }
    else if (partIdx == CoordSys::Flange)
    {
      this->TableTopRobotLogic->UpdateTableTopToFlangeTransform(channel25GeoNode);
      vtkMRMLLinearTransformNode* rasToFlangeTransformNode = this->TableTopRobotLogic->UpdateRasToFlangeTransform(channel25GeoNode);
      if (rasToFlangeTransformNode)
      {
        partModel->SetAndObserveTransformNodeID(rasToFlangeTransformNode->GetID());
      }
    }
    else if (partIdx == CoordSys::TableTop)
    {
      this->TableTopRobotLogic->UpdatePatientToTableTopTransform(channel25GeoNode);
      vtkMRMLLinearTransformNode* rasToTableTopTransformNode = this->TableTopRobotLogic->UpdateRasToTableTopTransform(channel25GeoNode);
      if (rasToTableTopTransformNode)
      {
        this->TableTopFixedReferenceCollisionDetection->SetInputData(0, partModel->GetPolyData());
        this->TableTopElbowCollisionDetection->SetInputData(0, partModel->GetPolyData());
        this->TableTopShoulderCollisionDetection->SetInputData(0, partModel->GetPolyData());
        this->TableTopBaseRotationCollisionDetection->SetInputData(0, partModel->GetPolyData());
        this->TableTopBaseFixedCollisionDetection->SetInputData(0, partModel->GetPolyData());

        partModel->SetAndObserveTransformNodeID(rasToTableTopTransformNode->GetID());
      }
    }
  }
  this->TableTopRobotLogic->UpdatePatientToTableTopTransform(channel25GeoNode);
  this->TableTopRobotLogic->UpdateTableTopToFlangeTransform(channel25GeoNode);
  this->TableTopRobotLogic->UpdateFlangeToWristTransform(channel25GeoNode);
  this->TableTopRobotLogic->UpdateWristToElbowTransform(channel25GeoNode);
  this->TableTopRobotLogic->UpdateElbowToShoulderTransform(channel25GeoNode);
  this->TableTopRobotLogic->UpdateShoulderToBaseRotationTransform(channel25GeoNode);
  this->TableTopRobotLogic->UpdateBaseRotationToBaseFixedTransform(channel25GeoNode);
  this->TableTopRobotLogic->UpdateBaseFixedToFixedReferenceTransform(channel25GeoNode);

  // Disable collision detection if product of number of triangles of the two models is above threshold
  if (loadedPartsNumTriangles[CoordSys::FixedReference] * loadedPartsNumTriangles[CoordSys::TableTop] > MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS && !forceEnableCollisionDetection)
  {
    vtkWarningMacro("Too many combined triangles (product = " << loadedPartsNumTriangles[CoordSys::FixedReference] * loadedPartsNumTriangles[CoordSys::TableTop]
      << ") detected between FixedReference and TableTop. Collision detection may take a very long time.");
    this->TableTopFixedReferenceCollisionDetection->SetInputData(0, nullptr);
    this->TableTopFixedReferenceCollisionDetection->SetInputData(1, nullptr);
  }
  if (loadedPartsNumTriangles[CoordSys::Elbow] * loadedPartsNumTriangles[CoordSys::TableTop] > MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS && !forceEnableCollisionDetection)
  {
    vtkWarningMacro("Too many combined triangles (product = " << loadedPartsNumTriangles[CoordSys::Elbow] * loadedPartsNumTriangles[CoordSys::TableTop]
      << ") detected between Elbow and TableTop. Collision detection may take a very long time.");
    this->TableTopElbowCollisionDetection->SetInputData(0, nullptr);
    this->TableTopElbowCollisionDetection->SetInputData(1, nullptr);
  }
  if (loadedPartsNumTriangles[CoordSys::Shoulder] * loadedPartsNumTriangles[CoordSys::TableTop] > MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS && !forceEnableCollisionDetection)
  {
    vtkWarningMacro("Too many combined triangles (product = " << loadedPartsNumTriangles[CoordSys::Shoulder] * loadedPartsNumTriangles[CoordSys::TableTop]
      << ") detected between Shoulder and TableTop. Collision detection may take a very long time.");
    this->TableTopShoulderCollisionDetection->SetInputData(0, nullptr);
    this->TableTopShoulderCollisionDetection->SetInputData(1, nullptr);
  }
  if (loadedPartsNumTriangles[CoordSys::BaseRotation] * loadedPartsNumTriangles[CoordSys::TableTop] > MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS && !forceEnableCollisionDetection)
  {
    vtkWarningMacro("Too many combined triangles (product = " << loadedPartsNumTriangles[CoordSys::BaseRotation] * loadedPartsNumTriangles[CoordSys::TableTop]
      << ") detected between BaseRotation and TableTop. Collision detection may take a very long time.");
    this->TableTopBaseRotationCollisionDetection->SetInputData(0, nullptr);
    this->TableTopBaseRotationCollisionDetection->SetInputData(1, nullptr);
  }
  if (loadedPartsNumTriangles[CoordSys::BaseFixed] * loadedPartsNumTriangles[CoordSys::TableTop] > MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS && !forceEnableCollisionDetection)
  {
    vtkWarningMacro("Too many combined triangles (product = " << loadedPartsNumTriangles[CoordSys::BaseFixed] * loadedPartsNumTriangles[CoordSys::TableTop]
      << ") detected between BaseRotation and TableTop. Collision detection may take a very long time.");
    this->TableTopBaseFixedCollisionDetection->SetInputData(0, nullptr);
    this->TableTopBaseFixedCollisionDetection->SetInputData(1, nullptr);
  }

/*
  // Set identity transform for patient (parent transform is taken into account when getting poly data from segmentation)
  vtkNew<vtkTransform> identityTransform;
  identityTransform->Identity();
  this->GantryPatientCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(identityTransform));
  this->CollimatorPatientCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(identityTransform));
*/
  return loadedParts;
}

//-----------------------------------------------------------------------------
std::string vtkSlicerPatientPositioningLogic::CheckForCollisions(vtkMRMLPatientPositioningNode* parameterNode, bool collisionDetectionEnabled)
{
  if (!parameterNode)
  {
    vtkErrorMacro("CheckForCollisions: Invalid parameter set node");
    return "Invalid parameters";
  }
  if (!collisionDetectionEnabled)
  {
    return "";
  }

  std::string statusString = "";

  // Get transforms used in the collision detection filters
  vtkMRMLLinearTransformNode* rasToTableTopTransformNode = this->TableTopRobotLogic->GetTableTopTransform();
  vtkMRMLLinearTransformNode* rasToFixedRerefenceTransformNode = this->TableTopRobotLogic->GetFixedReferenceTransform();
  vtkMRMLLinearTransformNode* rasToElbowTransformNode = this->TableTopRobotLogic->GetElbowTransform();
  vtkMRMLLinearTransformNode* rasToShoulderTransformNode = this->TableTopRobotLogic->GetShoulderTransform();
  vtkMRMLLinearTransformNode* rasToBaseFixedTransformNode = this->TableTopRobotLogic->GetBaseFixedTransform();
  vtkMRMLLinearTransformNode* rasToBaseRotationTransformNode = this->TableTopRobotLogic->GetBaseRotationTransform();

  if ( !rasToTableTopTransformNode || !rasToFixedRerefenceTransformNode
    || !rasToElbowTransformNode || !rasToShoulderTransformNode
    || !rasToBaseFixedTransformNode || !rasToBaseRotationTransformNode)
  {
    statusString = "Failed to access TableTopRobot transform nodes";
    vtkErrorMacro("CheckForCollisions: " + statusString);
    return statusString;
  }

  // Get transforms to parent, make sure they are linear
  vtkLinearTransform* rasToTableTopTransform = vtkLinearTransform::SafeDownCast(rasToTableTopTransformNode->GetTransformToParent());
  vtkLinearTransform* rasToFixedReferenceTransform = vtkLinearTransform::SafeDownCast(rasToFixedRerefenceTransformNode->GetTransformToParent());
  vtkLinearTransform* rasToElbowTransform = vtkLinearTransform::SafeDownCast(rasToElbowTransformNode->GetTransformToParent());
  vtkLinearTransform* rasToShoulderTransform = vtkLinearTransform::SafeDownCast(rasToShoulderTransformNode->GetTransformToParent());
  vtkLinearTransform* rasToBaseFixedTransform = vtkLinearTransform::SafeDownCast(rasToBaseFixedTransformNode->GetTransformToParent());
  vtkLinearTransform* rasToBaseRotationTransform = vtkLinearTransform::SafeDownCast(rasToBaseRotationTransformNode->GetTransformToParent());

  if ( !rasToTableTopTransform || !rasToFixedReferenceTransform
    || !rasToElbowTransform || !rasToShoulderTransform
    || !rasToBaseFixedTransform || !rasToBaseRotationTransform)
  {
    statusString = "Non-linear transform detected";
    vtkErrorMacro("CheckForCollisions: " + statusString);
    return statusString;
  }

  using CoordSys = vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier;

  // Get states of the treatment machine parts involved
  std::string TableTopState = this->GetStateForPartType(this->GetTreatmentMachinePartTypeAsString(CoordSys::TableTop));
  std::string FixedReferenceState = this->GetStateForPartType(this->GetTreatmentMachinePartTypeAsString(CoordSys::FixedReference));
  std::string ElbowState = this->GetStateForPartType(this->GetTreatmentMachinePartTypeAsString(CoordSys::Elbow));
  std::string ShoulderState = this->GetStateForPartType(this->GetTreatmentMachinePartTypeAsString(CoordSys::Shoulder));
  std::string BaseFixedState = this->GetStateForPartType(this->GetTreatmentMachinePartTypeAsString(CoordSys::BaseFixed));
  std::string BaseRotationState = this->GetStateForPartType(this->GetTreatmentMachinePartTypeAsString(CoordSys::BaseRotation));

  // If number of contacts between pieces of treatment room is greater than 0, the collision between which pieces
  // will be set to the output string and returned by the function.
  if (TableTopState == "Active" && FixedReferenceState == "Active" && this->TableTopFixedReferenceCollisionDetection->GetInputData(0))
  {
    this->TableTopFixedReferenceCollisionDetection->SetTransform(0, rasToTableTopTransform);
    this->TableTopFixedReferenceCollisionDetection->SetTransform(1, rasToFixedReferenceTransform);
    this->TableTopFixedReferenceCollisionDetection->Update();
    if (this->TableTopFixedReferenceCollisionDetection->GetNumberOfContacts() > 0)
    {
      statusString = statusString + "Collision between FixedReference and TableTop\n";
    }
  }

  if (TableTopState == "Active" && ElbowState == "Active" && this->TableTopElbowCollisionDetection->GetInputData(0))
  {
    this->TableTopElbowCollisionDetection->SetTransform(0, rasToTableTopTransform);
    this->TableTopElbowCollisionDetection->SetTransform(1, rasToElbowTransform);
    this->TableTopElbowCollisionDetection->Update();
    if (this->TableTopElbowCollisionDetection->GetNumberOfContacts() > 0)
    {
      statusString = statusString + "Collision between Elbow and TableTop\n";
    }
  }

  if (TableTopState == "Active" && ShoulderState == "Active" && this->TableTopShoulderCollisionDetection->GetInputData(0))
  {
    this->TableTopShoulderCollisionDetection->SetTransform(0, rasToTableTopTransform);
    this->TableTopShoulderCollisionDetection->SetTransform(1, rasToShoulderTransform);
    this->TableTopShoulderCollisionDetection->Update();
    if (this->TableTopShoulderCollisionDetection->GetNumberOfContacts() > 0)
    {
      statusString = statusString + "Collision between Shoulder and TableTop\n";
    }
  }

  if (TableTopState == "Active" && BaseRotationState == "Active" && this->TableTopBaseRotationCollisionDetection->GetInputData(0))
  {
    this->TableTopBaseRotationCollisionDetection->SetTransform(0, rasToTableTopTransform);
    this->TableTopBaseRotationCollisionDetection->SetTransform(1, rasToBaseRotationTransform);
    this->TableTopBaseRotationCollisionDetection->Update();
    if (this->TableTopBaseRotationCollisionDetection->GetNumberOfContacts() > 0)
    {
      statusString = statusString + "Collision between BaseRotation and TableTop\n";
    }
  }

  if (TableTopState == "Active" && BaseFixedState == "Active" && this->TableTopBaseFixedCollisionDetection->GetInputData(0))
  {
    this->TableTopBaseFixedCollisionDetection->SetTransform(0, rasToTableTopTransform);
    this->TableTopBaseFixedCollisionDetection->SetTransform(1, rasToBaseFixedTransform);
    this->TableTopBaseFixedCollisionDetection->Update();
    if (this->TableTopBaseFixedCollisionDetection->GetNumberOfContacts() > 0)
    {
      statusString = statusString + "Collision between BaseFixed and TableTop\n";
    }
  }

  return statusString;
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::LoadTreatmentMachine(vtkMRMLPatientPositioningNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("LoadTreatmentMachine: Invalid scene");
    return;
  }
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
  {
    vtkErrorMacro("LoadTreatmentMachine: Invalid parameter node");
    return;
  }

  // Make sure the transform hierarchy is in place
  this->BuildRobotTableGeometryTransformHierarchy();
}

//---------------------------------------------------------------------------
const char* vtkSlicerPatientPositioningLogic::GetTreatmentMachinePartTypeAsString(vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier type)
{
  return this->TableTopRobotLogic->GetTreatmentMachinePartTypeAsString(type);
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

vtkSlicerTableTopRobotTransformLogic* vtkSlicerPatientPositioningLogic::GetTableTopRobotTransformLogic() const
{
  return TableTopRobotLogic;
}
