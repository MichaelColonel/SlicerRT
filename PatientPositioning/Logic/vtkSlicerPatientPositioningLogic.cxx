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

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// RapidJSON includes
#include <rapidjson/document.h>     // rapidjson's DOM-style API
#include <rapidjson/filereadstream.h>

// Slicer includes
#include <vtkSlicerModuleLogic.h>
#include <vtkSlicerModelsLogic.h>

//const char* vtkSlicerPatientPositioningLogic::FIXEDREFERENCE_MODEL_NAME = "FixedReference";
//const char* vtkSlicerPatientPositioningLogic::ROBOT_BASE_FIXED_MODEL_NAME = "RobotBaseFixed";
//const char* vtkSlicerPatientPositioningLogic::ROBOT_BASE_ROTATION_MODEL_NAME = "RobotBaseRotation";
//const char* vtkSlicerPatientPositioningLogic::ROBOT_SHOULDER_MODEL_NAME = "RobotShoulder";
//const char* vtkSlicerPatientPositioningLogic::ROBOT_ELBOW_MODEL_NAME = "RobotElbow";
//const char* vtkSlicerPatientPositioningLogic::ROBOT_WRIST_MODEL_NAME = "RobotWrist";
//const char* vtkSlicerPatientPositioningLogic::TABLETOP_MODEL_NAME = "TableTop";

const char* vtkSlicerPatientPositioningLogic::ROBOT_BASE_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME = "RobotBaseOriginFiducial";
const char* vtkSlicerPatientPositioningLogic::ROBOT_SHOULDER_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME = "RobotShoulderOriginFiducial";
const char* vtkSlicerPatientPositioningLogic::ROBOT_ELBOW_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME = "RobotElbowOriginFiducial";
const char* vtkSlicerPatientPositioningLogic::ROBOT_WRIST_ORIGIN_MARKUPS_FIDUCIAL_NODE_NAME = "RobotWristOriginFiducial";
const char* vtkSlicerPatientPositioningLogic::ROBOT_TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME = "RobotTableTopOriginFiducial";
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
  rapidjson::Document* CurrentTreatmentMachineDescription{nullptr};

  /// Utility function to get element for treatment machine part
  /// \return Json object if found, otherwise null Json object
  rapidjson::Value& GetTreatmentMachinePart(CoordSys partType);
  rapidjson::Value& GetTreatmentMachinePart(std::string partTypeStr);

  std::string GetTreatmentMachinePartFullFilePath(vtkMRMLChannel25GeometryNode* parameterNode, std::string partPath);
  std::string GetTreatmentMachineFileNameWithoutExtension(vtkMRMLChannel25GeometryNode* parameterNode);
  std::string GetTreatmentMachinePartModelName(vtkMRMLChannel25GeometryNode* parameterNode, CoordSys partType);
  std::vector< CoordSys > GetTreatmentMachineParts();
  vtkMRMLModelNode* GetTreatmentMachinePartModelNode(vtkMRMLChannel25GeometryNode* parameterNode, CoordSys partType);
  vtkMRMLModelNode* EnsureTreatmentMachinePartModelNode(vtkMRMLChannel25GeometryNode* parameterNode, CoordSys partType, bool optional=false);
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
std::vector< vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier > vtkSlicerPatientPositioningLogic::vtkInternal::GetTreatmentMachineParts()
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
  vtkMRMLChannel25GeometryNode* parameterNode, std::string partPath)
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
std::string vtkSlicerPatientPositioningLogic::vtkInternal::GetTreatmentMachineFileNameWithoutExtension(vtkMRMLChannel25GeometryNode* parameterNode)
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
  vtkMRMLChannel25GeometryNode* parameterNode, CoordSys partType)
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
  vtkMRMLChannel25GeometryNode* parameterNode, CoordSys partType)
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
  vtkMRMLChannel25GeometryNode* parameterNode, CoordSys partType, bool optional/*=false*/)
{
  vtkMRMLScene* scene = this->External->GetMRMLScene();
  if (!scene || !parameterNode)
  {
    vtkErrorWithObjectMacro(this->External, "GetTreatmentMachinePartModelName: Invalid scene or parameter node");
    return nullptr;
  }
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  if (!shNode)
  {
    vtkErrorWithObjectMacro(this->External, "LoadTreatmentMachine: Failed to access subject hierarchy node");
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
        vtkWarningWithObjectMacro(this->External, "LoadTreatmentMachine: State for part "
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
        vtkErrorWithObjectMacro(this->External, "LoadTreatmentMachine: Failed get file path for part "
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
      vtkErrorWithObjectMacro(this->External, "LoadTreatmentMachine: Failed to load " << partName << " model from file " << partModelFilePath);
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
void vtkSlicerPatientPositioningLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic
::SetXrayImagesProjection(vtkMRMLPatientPositioningNode* parameterNode, vtkMRMLPatientPositioningNode::XrayProjectionType projection,
  vtkMRMLSliceCompositeNode* sliceCompNode, vtkMRMLSliceNode* sliceNode)
{
  if (!parameterNode || !sliceCompNode || !sliceNode || projection == vtkMRMLPatientPositioningNode::XrayProjectionType_Last)
  {
    return;
  }
  vtkMRMLScalarVolumeNode* drrNode = parameterNode->GetDrrNode(projection);
  vtkMRMLScalarVolumeNode* xrayImageNode = parameterNode->GetXrayImageNode(projection);
  if (drrNode && xrayImageNode)
  {
    sliceCompNode->SetForegroundVolumeID(drrNode->GetID());
    sliceCompNode->SetBackgroundVolumeID(xrayImageNode->GetID());
    sliceCompNode->SetForegroundOpacity(0.5);
  }

  vtkMRMLApplicationLogic* mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
    {
    vtkGenericWarningMacro("vtkSlicerPatientPositioningLogic::SetXrayImagesProjection failed: invalid mrmlApplogic");
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = mrmlAppLogic->GetSliceLogic(sliceNode);
  if (!sliceLogic)
    {
    return;
    }
/*
  vtkMRMLNode* viewNode = nullptr;
  switch (projection)
  {
  case vtkMRMLPatientPositioningNode::Horizontal:
    
    break;
  case vtkMRMLPatientPositioningNode::Vertical:
    break;
  case vtkMRMLPatientPositioningNode::Angle:
    break;
  default:
    break;
  }

  vtkMRMLNode* viewNode = this->External->GetMRMLDisplayableNode();
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(viewNode);
  if (!sliceNode)
    {
    // this displayable manager is not of a slice node
    return nullptr;
    }
  vtkMRMLApplicationLogic* mrmlAppLogic = this->External->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
    {
    vtkGenericWarningMacro("vtkMRMLColorLegendDisplayableManager::vtkInternal::FindSliceCompositeNode failed: invalid mrmlApplogic");
    return nullptr;
    }
  vtkMRMLSliceLogic* sliceLogic = mrmlAppLogic->GetSliceLogic(sliceNode);
  if (!sliceLogic)
    {
    return nullptr;
    }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  return sliceCompositeNode;
*/
}

//---------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkSlicerPatientPositioningLogic
::GetXrayImageRasToIjkMatrixTransformNode(vtkMRMLScalarVolumeNode* xrayImageNode, vtkMRMLRTBeamNode* xrayBeamNode)
{
  if (!xrayBeamNode)
  {
    vtkErrorMacro("GetXrayImageRasToIjkMatrixTransformNode: Invalid beam node");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetXrayImageRasToIjkMatrixTransformNode: Invalid MRML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode;
  if (!scene->GetFirstNodeByName(DRR_TRANSFORM_NODE_NAME))
  {
    transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    transformNode->SetName(DRR_TRANSFORM_NODE_NAME);
    transformNode->SetHideFromEditors(1);
    transformNode->SetSingletonTag("DRR_Transform");
    scene->AddNode(transformNode);
  }
  else
  {
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName(DRR_TRANSFORM_NODE_NAME));
  }

  vtkSmartPointer<vtkMRMLLinearTransformNode> translateNode;
  if (!scene->GetFirstNodeByName(DRR_TRANSLATE_NODE_NAME))
  {
    translateNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    translateNode->SetName(DRR_TRANSLATE_NODE_NAME);
    translateNode->SetHideFromEditors(1);
    translateNode->SetSingletonTag("DRR_Translate");
    scene->AddNode(translateNode);
  }
  else
  {
    translateNode = vtkMRMLLinearTransformNode::SafeDownCast(
      scene->GetFirstNodeByName(DRR_TRANSLATE_NODE_NAME));
  }

  translateNode->SetAndObserveTransformNodeID(transformNode->GetID());
  xrayImageNode->SetAndObserveTransformNodeID(translateNode->GetID());

  return translateNode;
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
}

//----------------------------------------------------------------------------
std::vector<vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier>
vtkSlicerPatientPositioningLogic::LoadTreatmentMachine(vtkMRMLChannel25GeometryNode* parameterNode)
{
  using CoordSys = vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("LoadTreatmentMachine: Invalid scene");
    return std::vector<CoordSys>();
  }
  vtkWarningMacro("LoadTreatmentMachine: 1");
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  if (!shNode)
  {
    vtkErrorMacro("LoadTreatmentMachine: Failed to access subject hierarchy node");
    return std::vector<CoordSys>();
  }
  if (!parameterNode || !parameterNode->GetTreatmentMachineDescriptorFilePath())
  {
    vtkErrorMacro("LoadTreatmentMachine: Invalid parameter node");
    return std::vector<CoordSys>();
  }
  vtkWarningMacro("LoadTreatmentMachine: 2");

  // Make sure the transform hierarchy is in place
  this->BuildRobotTableGeometryTransformHierarchy();

  std::string moduleShareDirectory = this->GetModuleShareDirectory();
  vtkWarningMacro("LoadTreatmentMachine: 1 " << parameterNode->GetTreatmentMachineDescriptorFilePath());
  std::string descriptorFilePath(parameterNode->GetTreatmentMachineDescriptorFilePath());
  std::string machineType = this->Internal->GetTreatmentMachineFileNameWithoutExtension(parameterNode);

  // Load treatment machine JSON descriptor file
  FILE *fp = fopen(descriptorFilePath.c_str(), "r");
  if (!fp)
  {
    vtkErrorMacro("LoadTreatmentMachine: Failed to load treatment machine descriptor file '" << descriptorFilePath << "'");
    return std::vector<CoordSys>();
  }
  constexpr size_t size = 1000000;
  std::unique_ptr< char[] > buffer(new char[size]);
  rapidjson::FileReadStream fs(fp, buffer.get(), sizeof(buffer));
  if (this->Internal->CurrentTreatmentMachineDescription->ParseStream(fs).HasParseError())
  {
    vtkErrorMacro("LoadTreatmentMachine: Failed to load treatment machine descriptor file '" << descriptorFilePath << "'");
    fclose(fp);
    return std::vector<CoordSys>();
  }
  fclose(fp);
  vtkWarningMacro("LoadTreatmentMachine: 3");

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
  vtkWarningMacro("LoadTreatmentMachine: 5");

  // Setup treatment machine model display and transforms
  return this->SetupTreatmentMachineModels(parameterNode);
}

//----------------------------------------------------------------------------
std::vector<vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier>
vtkSlicerPatientPositioningLogic::SetupTreatmentMachineModels(vtkMRMLChannel25GeometryNode* parameterNode, bool forceEnableCollisionDetection/*=false*/)
{
  using CoordSys = vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier;
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
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
    vtkErrorMacro("SetupTreatmentMachineModels: 8 " << partIdx << " " << partType);
    if (!partModel || !partModel->GetPolyData())
    {
      switch (partIdx)
      {
        case CoordSys::FixedReference:
        case CoordSys::TableTop:
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
      vtkErrorMacro("SetupTreatmentMachineModels: Color: " << partColor[0] << " " << partColor[1] << " " << partColor[2]);
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

    if (partIdx == CoordSys::TableTop)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: P1");
      vtkMRMLLinearTransformNode* tableTopToElbowTransformNode =
        this->TableTopRobotLogic->GetTransformNodeBetween(CoordSys::TableTop, CoordSys::Wrist);
      vtkErrorMacro("SetupTreatmentMachineModels: P2");
      if (tableTopToElbowTransformNode)
      {
        vtkErrorMacro("SetupTreatmentMachineModels: P2.5");
        partModel->SetAndObserveTransformNodeID(tableTopToElbowTransformNode->GetID());
      }
      vtkErrorMacro("SetupTreatmentMachineModels: P3");
//      this->GantryTableTopCollisionDetection->SetInputData(1, partModel->GetPolyData());
//      this->CollimatorTableTopCollisionDetection->SetInputData(1, partModel->GetPolyData());
    }
    else if (partIdx == CoordSys::FixedReference)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: P4");
      vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode =
        this->TableTopRobotLogic->GetTransformNodeBetween(CoordSys::FixedReference, CoordSys::RAS);
      vtkErrorMacro("SetupTreatmentMachineModels: P5");
      if (fixedReferenceToRasTransformNode)
      {
        vtkErrorMacro("SetupTreatmentMachineModels: P5.5");
        partModel->SetAndObserveTransformNodeID(fixedReferenceToRasTransformNode->GetID());
      }
      vtkErrorMacro("SetupTreatmentMachineModels: P6");
    }
    else if (partIdx == CoordSys::BaseFixed)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: P4");
      vtkMRMLLinearTransformNode* baseFixedToFixedReferenceTransformNode =
        this->TableTopRobotLogic->GetTransformNodeBetween(CoordSys::BaseFixed, CoordSys::FixedReference);
      vtkErrorMacro("SetupTreatmentMachineModels: P5");
      if (baseFixedToFixedReferenceTransformNode)
      {
        vtkErrorMacro("SetupTreatmentMachineModels: P5.5");
        partModel->SetAndObserveTransformNodeID(baseFixedToFixedReferenceTransformNode->GetID());
      }
      vtkErrorMacro("SetupTreatmentMachineModels: P6");
    }
    else if (partIdx == CoordSys::BaseRotation)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: P4");
      vtkMRMLLinearTransformNode* baseRotationToBaseFixedTransformNode =
        this->TableTopRobotLogic->GetTransformNodeBetween(CoordSys::BaseRotation, CoordSys::BaseFixed);
      vtkErrorMacro("SetupTreatmentMachineModels: P5");
      if (baseRotationToBaseFixedTransformNode)
      {
        vtkErrorMacro("SetupTreatmentMachineModels: P5.5");
        partModel->SetAndObserveTransformNodeID(baseRotationToBaseFixedTransformNode->GetID());
      }
      vtkErrorMacro("SetupTreatmentMachineModels: P6");
    }
    else if (partIdx == CoordSys::Shoulder)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: P4");
      vtkMRMLLinearTransformNode* shoulderToBaseRotationTransformNode =
        this->TableTopRobotLogic->GetTransformNodeBetween(CoordSys::Shoulder, CoordSys::BaseRotation);
      vtkErrorMacro("SetupTreatmentMachineModels: P5");
      if (shoulderToBaseRotationTransformNode)
      {
        vtkErrorMacro("SetupTreatmentMachineModels: P5.5");
        partModel->SetAndObserveTransformNodeID(shoulderToBaseRotationTransformNode->GetID());
      }
      vtkErrorMacro("SetupTreatmentMachineModels: P6");
    }
    else if (partIdx == CoordSys::Shoulder)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: P4");
      vtkMRMLLinearTransformNode* shoulderToBaseRotationTransformNode =
        this->TableTopRobotLogic->GetTransformNodeBetween(CoordSys::Shoulder, CoordSys::BaseRotation);
      vtkErrorMacro("SetupTreatmentMachineModels: P5");
      if (shoulderToBaseRotationTransformNode)
      {
        vtkErrorMacro("SetupTreatmentMachineModels: P5.5");
        partModel->SetAndObserveTransformNodeID(shoulderToBaseRotationTransformNode->GetID());
      }
      vtkErrorMacro("SetupTreatmentMachineModels: P6");
    }
    else if (partIdx == CoordSys::Elbow)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: P4");
      vtkMRMLLinearTransformNode* elbowToShoulderTransformNode =
        this->TableTopRobotLogic->GetTransformNodeBetween(CoordSys::Elbow, CoordSys::Shoulder);
      vtkErrorMacro("SetupTreatmentMachineModels: P5");
      if (elbowToShoulderTransformNode)
      {
        vtkErrorMacro("SetupTreatmentMachineModels: P5.5");
        partModel->SetAndObserveTransformNodeID(elbowToShoulderTransformNode->GetID());
      }
      vtkErrorMacro("SetupTreatmentMachineModels: P6");
    }
    else if (partIdx == CoordSys::Wrist)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: P4");
      vtkMRMLLinearTransformNode* wristToElbowTransformNode =
        this->TableTopRobotLogic->GetTransformNodeBetween(CoordSys::Wrist, CoordSys::Elbow);
      vtkErrorMacro("SetupTreatmentMachineModels: P5");
      if (wristToElbowTransformNode)
      {
        vtkErrorMacro("SetupTreatmentMachineModels: P5.5");
        partModel->SetAndObserveTransformNodeID(wristToElbowTransformNode->GetID());
      }
      vtkErrorMacro("SetupTreatmentMachineModels: P6");
    }
  }
/*
  using CoordSys = vtkSlicerTableTopRobotTransformLogic::CoordinateSystemIdentifier;
  vtkErrorMacro("SetupTreatmentMachineModels: 6");
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Invalid scene");
    return std::vector<CoordSys>();
  }
  vtkErrorMacro("SetupTreatmentMachineModels: 7");
  std::vector<CoordSys> loadedParts;
  std::map<CoordSys, unsigned int> loadedPartsNumTriangles;

  std::string partType1 = this->GetTreatmentMachinePartTypeAsString(CoordSys::FixedReference);
  vtkMRMLModelNode* partModel1 = this->Internal->GetTreatmentMachinePartModelNode(parameterNode, CoordSys::FixedReference);
  
  loadedParts.push_back(CoordSys::FixedReference);
  loadedPartsNumTriangles[CoordSys::FixedReference] = partModel1->GetPolyData()->GetNumberOfCells();

  // Set color
  vtkVector3d partColor1(this->GetColorForPartType(partType1));
  partModel1->CreateDefaultDisplayNodes();
  vtkErrorMacro("SetupTreatmentMachineModels: Color: " << partColor1[0] << " " << partColor1[1] << " " << partColor1[2]);
  partModel1->GetDisplayNode()->SetColor((double)partColor1[0] / 255.0, (double)partColor1[1] / 255.0, (double)partColor1[2] / 255.0);

  std::string partType2 = this->GetTreatmentMachinePartTypeAsString(CoordSys::TableTop);
  vtkMRMLModelNode* partModel2 = this->Internal->GetTreatmentMachinePartModelNode(parameterNode, CoordSys::TableTop);
  
  loadedParts.push_back(CoordSys::TableTop);
  loadedPartsNumTriangles[CoordSys::TableTop] = partModel2->GetPolyData()->GetNumberOfCells();

  // Set color
  vtkVector3d partColor2(this->GetColorForPartType(partType2));
  partModel2->CreateDefaultDisplayNodes();
  vtkErrorMacro("SetupTreatmentMachineModels: Color: " << partColor2[0] << " " << partColor2[1] << " " << partColor2[2]);
  partModel2->GetDisplayNode()->SetColor((double)partColor2[0] / 255.0, (double)partColor2[1] / 255.0, (double)partColor2[2] / 255.0);
*/
/*
  for (int partIdx = CoordSys::FixedReference; partIdx < CoordSys::CoordinateSystemIdentifier_Last; ++partIdx)
  {
    std::string partType = this->GetTreatmentMachinePartTypeAsString(static_cast<CoordSys>(partIdx));
    vtkMRMLModelNode* partModel = this->Internal->GetTreatmentMachinePartModelNode(parameterNode, static_cast<CoordSys>(partIdx));
    vtkErrorMacro("SetupTreatmentMachineModels: 8 " << partIdx << " " << partType);
    if (!partModel || !partModel->GetPolyData())
    {
      switch (partIdx)
      {
        case CoordSys::FixedReference:
        case CoordSys::TableTop:
          vtkErrorMacro("SetupTreatmentMachineModels: Unable to access " << partType << " model " << partIdx);
          break;
        default:
          break;
      }
      continue;
    }
    else
    {
      loadedParts.push_back(static_cast<CoordSys>(partIdx));
      loadedPartsNumTriangles[static_cast<CoordSys>(partIdx)] = partModel->GetPolyData()->GetNumberOfCells();

      // Set color
      vtkVector3d partColor(this->GetColorForPartType(partType));
      partModel->CreateDefaultDisplayNodes();
      vtkErrorMacro("SetupTreatmentMachineModels: Color: " << partColor[0] << " " << partColor[1] << " " << partColor[2]);
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
*/
/*
    // Setup transforms and collision detection
    if (partIdx == Collimator)
    {
      vtkMRMLLinearTransformNode* collimatorToGantryTransformNode =
        this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Collimator, vtkSlicerIECTransformLogic::Gantry);
      partModel->SetAndObserveTransformNodeID(collimatorToGantryTransformNode->GetID());
      this->CollimatorTableTopCollisionDetection->SetInputData(0, partModel->GetPolyData());
      // Patient model is set when calculating collisions, as it can be changed dynamically
      this->CollimatorPatientCollisionDetection->SetInputData(0, partModel->GetPolyData());
    }
    else if (partIdx == Gantry)
    {
      vtkMRMLLinearTransformNode* gantryToFixedReferenceTransformNode =
        this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::Gantry, vtkSlicerIECTransformLogic::FixedReference);
      partModel->SetAndObserveTransformNodeID(gantryToFixedReferenceTransformNode->GetID());
      this->GantryTableTopCollisionDetection->SetInputData(0, partModel->GetPolyData());
      this->GantryPatientSupportCollisionDetection->SetInputData(0, partModel->GetPolyData());
      // Patient model is set when calculating collisions, as it can be changed dynamically
      this->GantryPatientCollisionDetection->SetInputData(0, partModel->GetPolyData());
    }
    else if (partIdx == PatientSupport)
    {
      vtkMRMLLinearTransformNode* patientSupportToPatientSupportRotationTransformNode =
        this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::PatientSupport, vtkSlicerIECTransformLogic::PatientSupportRotation);
      partModel->SetAndObserveTransformNodeID(patientSupportToPatientSupportRotationTransformNode->GetID());
      this->GantryPatientSupportCollisionDetection->SetInputData(1, partModel->GetPolyData());
    }
    else if (partIdx == TableTop)
    {
      vtkMRMLLinearTransformNode* tableTopToTableTopEccentricRotationTransformNode =
        this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::TableTop, vtkSlicerIECTransformLogic::TableTopEccentricRotation);
      partModel->SetAndObserveTransformNodeID(tableTopToTableTopEccentricRotationTransformNode->GetID());
      this->GantryTableTopCollisionDetection->SetInputData(1, partModel->GetPolyData());
      this->CollimatorTableTopCollisionDetection->SetInputData(1, partModel->GetPolyData());
    }
    else if (partIdx == Body)
    {
      vtkMRMLLinearTransformNode* fixedReferenceToRasTransformNode =
        this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::FixedReference, vtkSlicerIECTransformLogic::RAS);
      partModel->SetAndObserveTransformNodeID(fixedReferenceToRasTransformNode->GetID());
    }
    else if (partIdx == ImagingPanelLeft)
    {
      vtkMRMLLinearTransformNode* leftImagingPanelToGantryTransformNode =
        this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::LeftImagingPanel, vtkSlicerIECTransformLogic::Gantry);
      partModel->SetAndObserveTransformNodeID(leftImagingPanelToGantryTransformNode->GetID());
    }
    else if (partIdx == ImagingPanelRight)
    {
      vtkMRMLLinearTransformNode* rightImagingPanelToGantryTransformNode =
        this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::RightImagingPanel, vtkSlicerIECTransformLogic::Gantry);
      partModel->SetAndObserveTransformNodeID(rightImagingPanelToGantryTransformNode->GetID());
    }
    else if (partIdx == FlatPanel)
    {
      vtkMRMLLinearTransformNode* flatPanelToGantryTransformNode =
        this->IECLogic->GetTransformNodeBetween(vtkSlicerIECTransformLogic::FlatPanel, vtkSlicerIECTransformLogic::Gantry);
      partModel->SetAndObserveTransformNodeID(flatPanelToGantryTransformNode->GetID());
    }
    //TODO: ApplicatorHolder, ElectronApplicator?
*/
//  }
/*
  // Disable collision detection if product of number of triangles of the two models is above threshold
  if (loadedPartsNumTriangles[Gantry] * loadedPartsNumTriangles[TableTop] > MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS && !forceEnableCollisionDetection)
  {
    vtkWarningMacro("Too many combined triangles (product = " << loadedPartsNumTriangles[Gantry] * loadedPartsNumTriangles[TableTop]
      << ") detected between gantry and table top. Collision detection may take a very long time.");
    this->GantryTableTopCollisionDetection->SetInputData(0, nullptr);
    this->GantryTableTopCollisionDetection->SetInputData(1, nullptr);
  }
  if (loadedPartsNumTriangles[Gantry] * loadedPartsNumTriangles[PatientSupport] > MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS && !forceEnableCollisionDetection)
  {
    vtkWarningMacro("Too many combined triangles (product = " << loadedPartsNumTriangles[Gantry] * loadedPartsNumTriangles[PatientSupport]
      << ") detected between gantry and patient support. Collision detection may take a very long time.");
    this->GantryPatientSupportCollisionDetection->SetInputData(0, nullptr);
    this->GantryPatientSupportCollisionDetection->SetInputData(1, nullptr);
  }
  if (loadedPartsNumTriangles[Collimator] * loadedPartsNumTriangles[TableTop] > MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS && !forceEnableCollisionDetection)
  {
    vtkWarningMacro("Too many combined triangles (product = " << loadedPartsNumTriangles[Collimator] * loadedPartsNumTriangles[TableTop]
      << ") detected between collimator and table top. Collision detection may take a very long time.");
    this->CollimatorTableTopCollisionDetection->SetInputData(0, nullptr);
    this->CollimatorTableTopCollisionDetection->SetInputData(1, nullptr);
  }

  // Set identity transform for patient (parent transform is taken into account when getting poly data from segmentation)
  vtkNew<vtkTransform> identityTransform;
  identityTransform->Identity();
  this->GantryPatientCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(identityTransform));
  this->CollimatorPatientCollisionDetection->SetTransform(1, vtkLinearTransform::SafeDownCast(identityTransform));
*/
  return loadedParts;
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::LoadTreatmentMachine(vtkMRMLPatientPositioningNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Invalid scene");
    return;
  }
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Invalid parameter node");
    return;
  }

  // Make sure the transform hierarchy is in place
  this->BuildRobotTableGeometryTransformHierarchy();
/*
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

  // FixedReference - mandatory
  std::string fixedReferenceModelSingletonTag = machineType + "_" + FIXEDREFERENCE_MODEL_NAME;
  vtkMRMLModelNode* fixedReferenceModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(fixedReferenceModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (fixedReferenceModelNode && !fixedReferenceModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(fixedReferenceModelNode);
    fixedReferenceModelNode = nullptr;
  }
  if (!fixedReferenceModelNode)
  {
    std::string fixedReferenceModelFilePath = treatmentMachineModelsDirectory + "/" + FIXEDREFERENCE_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(fixedReferenceModelFilePath))
    {
      fixedReferenceModelNode = modelsLogic->AddModel(fixedReferenceModelFilePath.c_str());
    }
    if (fixedReferenceModelNode)
    {
      fixedReferenceModelNode->SetSingletonTag(fixedReferenceModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> fixedReferenceModelHierarchyNode;
      scene->AddNode(fixedReferenceModelHierarchyNode);
      fixedReferenceModelHierarchyNode->SetModelNodeID(fixedReferenceModelNode->GetID());
      fixedReferenceModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      fixedReferenceModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load fixed reference model");
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

  // Table platform for lateral movement - optional
  std::string tablePlatformModelSingletonTag = machineType + "_" + TABLE_PLATFORM_MODEL_NAME;
  vtkMRMLModelNode* tablePlatformModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tablePlatformModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tablePlatformModelNode && !tablePlatformModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tablePlatformModelNode);
    tablePlatformModelNode = nullptr;
  }
  if (!tablePlatformModelNode)
  {
    std::string tablePlatformModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_PLATFORM_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tablePlatformModelFilePath))
    {
      tablePlatformModelNode = modelsLogic->AddModel(tablePlatformModelFilePath.c_str());
    }
    if (tablePlatformModelNode)
    {
      tablePlatformModelNode->SetSingletonTag(tablePlatformModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tablePlatformModelHierarchyNode;
      scene->AddNode(tablePlatformModelHierarchyNode);
      tablePlatformModelHierarchyNode->SetModelNodeID(tablePlatformModelNode->GetID());
      tablePlatformModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tablePlatformModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table platform model for longitudinal movement");
    }
  }

  // Table support for longitudinal movement  - mandatory
  std::string tableSupportModelSingletonTag = machineType + "_" + TABLE_SUPPORT_MODEL_NAME;
  vtkMRMLModelNode* tableSupportModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableSupportModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableSupportModelNode && !tableSupportModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableSupportModelNode);
    tableSupportModelNode = nullptr;
  }
  if (!tableSupportModelNode)
  {
    std::string tableSupportModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_SUPPORT_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableSupportModelFilePath))
    {
      tableSupportModelNode = modelsLogic->AddModel(tableSupportModelFilePath.c_str());
    }
    if (tableSupportModelNode)
    {
      tableSupportModelNode->SetSingletonTag(tableSupportModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableSupportModelHierarchyNode;
      scene->AddNode(tableSupportModelHierarchyNode);
      tableSupportModelHierarchyNode->SetModelNodeID(tableSupportModelNode->GetID());
      tableSupportModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableSupportModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table support model for lateral movement");
    }
  }

  // Table origin - mandatory
  std::string tableOriginModelSingletonTag = machineType + "_" + TABLE_ORIGIN_MODEL_NAME;
  vtkMRMLModelNode* tableOriginModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableOriginModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableOriginModelNode && !tableOriginModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableOriginModelNode);
    tableOriginModelNode = nullptr;
  }
  if (!tableOriginModelNode)
  {
    std::string tableOriginModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_ORIGIN_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableOriginModelFilePath))
    {
      tableOriginModelNode = modelsLogic->AddModel(tableOriginModelFilePath.c_str());
    }
    if (tableOriginModelNode)
    {
      tableOriginModelNode->SetSingletonTag(tableOriginModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableOriginModelHierarchyNode;
      scene->AddNode(tableOriginModelHierarchyNode);
      tableOriginModelHierarchyNode->SetModelNodeID(tableOriginModelNode->GetID());
      tableOriginModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableOriginModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table origin model for vertical movement");
    }
  }

  // Table mirror - mandatory
  std::string tableMirrorModelSingletonTag = machineType + "_" + TABLE_MIRROR_MODEL_NAME;
  vtkMRMLModelNode* tableMirrorModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableMirrorModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableMirrorModelNode && !tableMirrorModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableMirrorModelNode);
    tableMirrorModelNode = nullptr;
  }
  if (!tableMirrorModelNode)
  {
    std::string tableMirrorModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_MIRROR_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableMirrorModelFilePath))
    {
      tableMirrorModelNode = modelsLogic->AddModel(tableMirrorModelFilePath.c_str());
    }
    if (tableMirrorModelNode)
    {
      tableMirrorModelNode->SetSingletonTag(tableMirrorModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableMirrorModelHierarchyNode;
      scene->AddNode(tableMirrorModelHierarchyNode);
      tableMirrorModelHierarchyNode->SetModelNodeID(tableMirrorModelNode->GetID());
      tableMirrorModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableMirrorModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table mirror model for vertical movement");
    }
  }

  // Table middle - mandatory
  std::string tableMiddleModelSingletonTag = machineType + "_" + TABLE_MIDDLE_MODEL_NAME;
  vtkMRMLModelNode* tableMiddleModelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetSingletonNode(tableMiddleModelSingletonTag.c_str(), "vtkMRMLModelNode") );
  if (tableMiddleModelNode && !tableMiddleModelNode->GetPolyData())
  {
    // Remove node if contains empty polydata (e.g. after closing scene), so that it can be loaded again
    scene->RemoveNode(tableMiddleModelNode);
    tableMiddleModelNode = nullptr;
  }
  if (!tableMiddleModelNode)
  {
    std::string tableMiddleModelFilePath = treatmentMachineModelsDirectory + "/" + TABLE_MIDDLE_MODEL_NAME + ".stl";
    if (vtksys::SystemTools::FileExists(tableMiddleModelFilePath))
    {
      tableMiddleModelNode = modelsLogic->AddModel(tableMiddleModelFilePath.c_str());
    }
    if (tableMiddleModelNode)
    {
      tableMiddleModelNode->SetSingletonTag(tableMiddleModelSingletonTag.c_str());
      vtkNew<vtkMRMLModelHierarchyNode> tableMiddleModelHierarchyNode;
      scene->AddNode(tableMiddleModelHierarchyNode);
      tableMiddleModelHierarchyNode->SetModelNodeID(tableMiddleModelNode->GetID());
      tableMiddleModelHierarchyNode->SetParentNodeID(rootModelHierarchyNode->GetID());
      tableMiddleModelHierarchyNode->HideFromEditorsOn();
    }
    else
    {
      vtkErrorMacro("LoadTreatmentMachineModels: Failed to load table middle model for vertical movement");
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

  if (!fixedReferenceModelNode || !fixedReferenceModelNode->GetPolyData() 
    || !tableSupportModelNode || !tableSupportModelNode->GetPolyData() 
    || !tablePlatformModelNode || !tablePlatformModelNode->GetPolyData() 
    || !patientSupportModelNode || !patientSupportModelNode->GetPolyData() 
    || !tableTopModelNode || !tableTopModelNode->GetPolyData()
    || !tableOriginModelNode || !tableOriginModelNode->GetPolyData()
    || !tableMiddleModelNode || !tableMiddleModelNode->GetPolyData()
    || !tableMirrorModelNode || !tableOriginModelNode->GetPolyData())
  {
    vtkErrorMacro("LoadTreatmentMachineModels: Failed to load every mandatory treatment machine component");
    return;
  }

  // Create / Update markups nodes if they already exists 
  // Update markups (Table fiducials, plane node, FixedReference line node)
  this->UpdateTableOriginFiducialNode(parameterNode);
  this->UpdateTableMirrorFiducialNode(parameterNode);
  this->UpdateTableMiddleFiducialNode(parameterNode);
  this->UpdateTableTopPlaneNode(parameterNode);
  this->UpdateFixedReferenceLineNode(parameterNode);
  this->CreateFixedBeamPlanAndNode(parameterNode);
  this->CreateExternalXrayPlanAndNode(parameterNode);
*/
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::ResetModelsToInitialPosition(vtkMRMLPatientPositioningNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("ResetModelsToInitialPosition: Invalid scene");
    return;
  }
  if (!parameterNode/* || !parameterNode->GetTreatmentMachineType() */)
  {
    vtkErrorMacro("ResetModelsToInitialPosition: Invalid parameter node");
    return;
  }
  // Reset all positions into zeros
//  parameterNode->ResetModelsToInitialPositions();

  // Initial transforms for the models (if any)
//  this->InitialSetupTransformTranslations(parameterNode);
}

//----------------------------------------------------------------------------
void vtkSlicerPatientPositioningLogic::SetupTreatmentMachine(vtkMRMLPatientPositioningNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if (!scene)
  {
    vtkErrorMacro("SetupTreatmentMachineModels: Invalid scene");
    return;
  }
/*
  //TODO: Store treatment machine component color and other properties in JSON

  // Display all pieces of the treatment room and sets each piece a color to provide realistic representation
  using IHEP = vtkSlicerIhepStandGeometryTransformLogic::CoordinateSystemIdentifier;

  // Transform IHEP stand models (IEC Patient) to RAS
  vtkNew<vtkTransform> patientToRasTransform;
  patientToRasTransform->Identity();
  patientToRasTransform->RotateX(-90.);
  if (!parameterNode->GetPatientHeadFeetRotation())
  {
    patientToRasTransform->RotateZ(180.);
  }

  // Table top - mandatory
  // Transform path: RAS -> Patient -> TableTop
  vtkNew<vtkTransform> rasToTableTopTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTop, 
    rasToTableTopTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopTransform"))
    {
      rasToTableTopTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopTransform"));
    }
    else
    {
      rasToTableTopTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopTransformNode->SetName("RasToTableTopTransform");
      rasToTableTopTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopTransform";
      rasToTableTopTransformNode->SetSingletonTag(singletonTag.c_str());

      scene->AddNode(rasToTableTopTransformNode);
    }
    if (rasToTableTopTransformNode)
    {
      rasToTableTopTransformNode->SetAndObserveTransformToParent(rasToTableTopTransform);
    }

    vtkMRMLModelNode* tableTopModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLETOP_MODEL_NAME) );
    if (!tableTopModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top model");
      return;
    }
    if (rasToTableTopTransformNode)
    {
      tableTopModel->SetAndObserveTransformNodeID(rasToTableTopTransformNode->GetID());
      tableTopModel->CreateDefaultDisplayNodes();
      tableTopModel->GetDisplayNode()->SetColor(0, 0, 0);
    }
  }

  // Table top origin - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin
  vtkNew<vtkTransform> rasToTableTopOriginTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopOrigin, 
    rasToTableTopOriginTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopOriginTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopOriginTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopOriginTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopOriginTransform"))
    {
      rasToTableTopOriginTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopOriginTransform"));
    }
    else
    {
      rasToTableTopOriginTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopOriginTransformNode->SetName("RasToTableTopOriginTransform");
      rasToTableTopOriginTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopOriginTransform";
      rasToTableTopOriginTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToTableTopOriginTransformNode);
    }
    if (rasToTableTopOriginTransformNode)
    {
      rasToTableTopOriginTransformNode->SetAndObserveTransformToParent(rasToTableTopOriginTransform);
    }

    vtkMRMLModelNode* tableTopOriginModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLE_ORIGIN_MODEL_NAME) );
    if (!tableTopOriginModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table origin model");
      return;
    }
    if (rasToTableTopOriginTransformNode)
    {
      tableTopOriginModel->SetAndObserveTransformNodeID(rasToTableTopOriginTransformNode->GetID());
      tableTopOriginModel->CreateDefaultDisplayNodes();
      tableTopOriginModel->GetDisplayNode()->SetColor(0.3, 0.3, 0.3);
    }
  }

  // Table mirror - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopMirror
  vtkNew<vtkTransform> rasToTableTopMirrorTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopMirror, 
    rasToTableTopMirrorTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopMirrorTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopMirrorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopMirrorTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopMirrorTransform"))
    {
      rasToTableTopMirrorTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopMirrorTransform"));
    }
    else
    {
      rasToTableTopMirrorTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopMirrorTransformNode->SetName("RasToTableTopMirrorTransform");
      rasToTableTopMirrorTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopMirrorTransform";
      rasToTableTopMirrorTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToTableTopMirrorTransformNode);
    }
    if (rasToTableTopMirrorTransformNode)
    {
      rasToTableTopMirrorTransformNode->SetAndObserveTransformToParent(rasToTableTopMirrorTransform);
    }

    vtkMRMLModelNode* tableTopMirrorModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLE_MIRROR_MODEL_NAME) );
    if (!tableTopMirrorModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table mirror model");
      return;
    }
    if (rasToTableTopMirrorTransformNode)
    {
      tableTopMirrorModel->SetAndObserveTransformNodeID(rasToTableTopMirrorTransformNode->GetID());
      tableTopMirrorModel->CreateDefaultDisplayNodes();
      tableTopMirrorModel->GetDisplayNode()->SetColor(0.3, 0.3, 0.3);
    }
  }

  // Table top middle - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopMiddle
  vtkNew<vtkTransform> rasToTableTopMiddleTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopMiddle, 
    rasToTableTopMiddleTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopMiddleTransform->Concatenate(patientToRasTransform);

    // Find RasToTableMiddleTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopMiddleTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopMiddleTransform"))
    {
      rasToTableTopMiddleTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopMiddleTransform"));
    }
    else
    {
      rasToTableTopMiddleTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopMiddleTransformNode->SetName("RasToTableTopMiddleTransform");
      rasToTableTopMiddleTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopMiddleTransform";
      rasToTableTopMiddleTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToTableTopMiddleTransformNode);
    }
    if (rasToTableTopMiddleTransformNode)
    {
      rasToTableTopMiddleTransformNode->SetAndObserveTransformToParent(rasToTableTopMiddleTransform);
    }

    vtkMRMLModelNode* tableTopMiddleModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(TABLE_MIDDLE_MODEL_NAME) );
    if (!tableTopMiddleModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table middle model");
      return;
    }
    if (rasToTableTopMiddleTransformNode)
    {
      tableTopMiddleModel->SetAndObserveTransformNodeID(rasToTableTopMiddleTransformNode->GetID());
      tableTopMiddleModel->CreateDefaultDisplayNodes();
      tableTopMiddleModel->GetDisplayNode()->SetColor(0.3, 0.3, 0.3);
    }
  }

  // Table top support movement model - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin -> TableTopSupport
  vtkNew<vtkTransform> rasToTableTopSupportTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TableTopSupport, 
    rasToTableTopSupportTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTableTopSupportTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopInferiorSuperiorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTableTopSupportTransformNode;
    if (scene->GetFirstNodeByName("RasToTableTopSupportTransform"))
    {
      rasToTableTopSupportTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTableTopSupportTransform"));
    }
    else
    {
      rasToTableTopSupportTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTableTopSupportTransformNode->SetName("RasToTableTopSupportTransform");
      rasToTableTopSupportTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTableTopSupportTransform";
      rasToTableTopSupportTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToTableTopSupportTransformNode);
    }

    vtkMRMLModelNode* tableTopSupportModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_SUPPORT_MODEL_NAME) );
    if (!tableTopSupportModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table top support model");
      return;
    }
    if (rasToTableTopSupportTransformNode)
    {
      rasToTableTopSupportTransformNode->SetAndObserveTransformToParent(rasToTableTopSupportTransform);
      tableTopSupportModel->SetAndObserveTransformNodeID(rasToTableTopSupportTransformNode->GetID());
      tableTopSupportModel->CreateDefaultDisplayNodes();
      tableTopSupportModel->GetDisplayNode()->SetColor(0.95, 0.95, 0.95);
    }
  }

  // Table platform model - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin -> TableTopSupport -> TablePlatform
  vtkNew<vtkTransform> rasToTablePlatformTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::TablePlatform, 
    rasToTablePlatformTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToTablePlatformTransform->Concatenate(patientToRasTransform);

    // Find RasToTableTopInferiorSuperiorTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToTablePlatformTransformNode;
    if (scene->GetFirstNodeByName("RasToTablePlatformTransform"))
    {
      rasToTablePlatformTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToTablePlatformTransform"));
    }
    else
    {
      rasToTablePlatformTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToTablePlatformTransformNode->SetName("RasToTablePlatformTransform");
      rasToTablePlatformTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToTablePlatformTransform";
      rasToTablePlatformTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToTablePlatformTransformNode);
    }

    vtkMRMLModelNode* tablePlatformModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(TABLE_PLATFORM_MODEL_NAME) );
    if (!tablePlatformModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access table platform model");
      return;
    }
    if (rasToTablePlatformTransformNode)
    {
      rasToTablePlatformTransformNode->SetAndObserveTransformToParent(rasToTablePlatformTransform);
      tablePlatformModel->SetAndObserveTransformNodeID(rasToTablePlatformTransformNode->GetID());
      tablePlatformModel->CreateDefaultDisplayNodes();
      tablePlatformModel->GetDisplayNode()->SetColor(0.75, 0.75, 0.75);
    }
  }

  // Patient support - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin -> TableTopSupport -> TablePlatform -> PatientSupport
  vtkNew<vtkTransform> rasToPatientSupportTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::PatientSupport, 
    rasToPatientSupportTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToPatientSupportTransform->Concatenate(patientToRasTransform);

    // Find RasToFixedReferenceTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToPatientSupportTransformNode;
    if (scene->GetFirstNodeByName("RasToPatientSupportTransform"))
    {
      rasToPatientSupportTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToPatientSupportTransform"));
    }
    else
    {
      rasToPatientSupportTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToPatientSupportTransformNode->SetName("RasToPatientSupportTransform");
      rasToPatientSupportTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToPatientSupportTransform";
      rasToPatientSupportTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToPatientSupportTransformNode);
    }
    if (rasToPatientSupportTransformNode)
    {
      rasToPatientSupportTransformNode->SetAndObserveTransformToParent(rasToPatientSupportTransform);
    }

    vtkMRMLModelNode* patientSupportModel = vtkMRMLModelNode::SafeDownCast(
      scene->GetFirstNodeByName(PATIENTSUPPORT_MODEL_NAME) );
    if (!patientSupportModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access patient support model");
      return;
    }
    if (rasToPatientSupportTransformNode)
    {
      rasToPatientSupportTransformNode->SetAndObserveTransformToParent(rasToPatientSupportTransform);
      patientSupportModel->SetAndObserveTransformNodeID(rasToPatientSupportTransformNode->GetID());
      patientSupportModel->CreateDefaultDisplayNodes();
      patientSupportModel->GetDisplayNode()->SetColor(0.55, 0.55, 0.55);
    }
  }

  // Fixed Reference - mandatory
  // Transform path: RAS -> Patient -> TableTop -> TableTopOrigin -> TableTopSupport -> TablePlatform -> PatientSupport -> FixedReference
  vtkNew<vtkTransform> rasToFixedReferenceTransform;
  if (this->IhepLogic->GetTransformBetween( IHEP::RAS, IHEP::FixedReference, 
    rasToFixedReferenceTransform, false))
  {
    // Transform to RAS, set transform to node, transform the model
    rasToFixedReferenceTransform->Concatenate(patientToRasTransform);

    // Find RasToFixedReferenceTransform or create it
    vtkSmartPointer<vtkMRMLLinearTransformNode> rasToFixedReferenceTransformNode;
    if (scene->GetFirstNodeByName("RasToFixedReferenceTransform"))
    {
      rasToFixedReferenceTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(
        scene->GetFirstNodeByName("RasToFixedReferenceTransform"));
    }
    else
    {
      rasToFixedReferenceTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
      rasToFixedReferenceTransformNode->SetName("RasToFixedReferenceTransform");
      rasToFixedReferenceTransformNode->SetHideFromEditors(1);
      std::string singletonTag = std::string("IHEP_") + "RasToFixedReferenceTransform";
      rasToFixedReferenceTransformNode->SetSingletonTag(singletonTag.c_str());
      scene->AddNode(rasToFixedReferenceTransformNode);
    }
    if (rasToFixedReferenceTransformNode)
    {
      rasToFixedReferenceTransformNode->SetAndObserveTransformToParent(rasToFixedReferenceTransform);
    }

    vtkMRMLModelNode* fixedReferenceModel = vtkMRMLModelNode::SafeDownCast(
      this->GetMRMLScene()->GetFirstNodeByName(FIXEDREFERENCE_MODEL_NAME) );
    if (!fixedReferenceModel)
    {
      vtkErrorMacro("SetupTreatmentMachineModels: Unable to access fixed reference model");
      return;
    }
    if (rasToFixedReferenceTransformNode)
    {
      fixedReferenceModel->SetAndObserveTransformNodeID(rasToFixedReferenceTransformNode->GetID());
      fixedReferenceModel->CreateDefaultDisplayNodes();
      fixedReferenceModel->GetDisplayNode()->SetColor(0.7, 0.65, 0.65);
    }
  }
*/
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
