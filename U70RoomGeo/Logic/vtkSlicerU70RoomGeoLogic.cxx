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

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLMarkupsLineNode.h>
#include <vtkMRMLMarkupsPlaneNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>

#include "vtkMRMLU70RoomGeoNode.h"
#include "vtkMRMLChannel26GeometryNode.h"

// U70RoomGeo Logic includes
#include "vtkSlicerU70RoomGeoLogic.h"
#include "vtkSlicerChannel26Cabin3RobotsGeometryCommon.h"

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

// Modules U70OpsUa MRML
//#include <vtkMRMLScadaOpcUaNode.h>

// SlicerRT includes
#include <vtkSlicerRtCommon.h>

const char* vtkSlicerU70RoomGeoLogic::FIXEDBEAMAXIS_MARKUPS_LINE_NODE_NAME = "FixedBeamAxis";
const char* vtkSlicerU70RoomGeoLogic::FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME = "FixedIsocenter";

const char* vtkSlicerU70RoomGeoLogic::TREATMENT_MACHINE_DESCRIPTOR_FILE_PATH_ATTRIBUTE_NAME = "TreatmentMachineDescriptorFilePath";
unsigned long vtkSlicerU70RoomGeoLogic::MAX_TRIANGLE_NUMBER_PRODUCT_FOR_COLLISIONS = 10E+10;

const char* vtkSlicerU70RoomGeoLogic::TABLETOP_MARKUPS_PLANE_NODE_NAME = "TableTopMarkupsPlane";
const char* vtkSlicerU70RoomGeoLogic::TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME = "TableTopMarkupsFiducial";

namespace
{

rapidjson::Value JSON_EMPTY_VALUE;

const double TableTopUpLeftFixedReference[3] = { -265., 1082.5, 0. }; // table top point A, LPS coordinate system
const double TableTopUpRightFixedReference[3] = { 265., 1082.5, 0. }; // table top point B, LPS coordinate system
const double TableTopDownRightFixedReference[3] = { 265, -1082.5, 0. }; // table top point C, LPS coordinate system
const double TableTopDownLeftFixedReference[3] = { -265., -1082.5, 0. }; // table top point D, LPS coordinate system

const double TableTopHole1FixedReference[3] = { -265., -1037., 0. }; // table top hole "1", LPS coordinate system
const double TableTopMirrorHole1FixedReference[3] = { 265., -1037., 0. }; // table top mirror hole "1", LPS coordinate system

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
vtkStandardNewMacro(vtkSlicerU70RoomGeoLogic);

//---------------------------------------------------------------------------
class vtkSlicerU70RoomGeoLogic::vtkInternal
{
public:
  vtkInternal(vtkSlicerU70RoomGeoLogic* external);
  ~vtkInternal();

  using CoordSys = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;

  vtkSlicerU70RoomGeoLogic* External;
  rapidjson::Document* CurrentTreatmentMachineDescription{ nullptr };

  /// Utility function to get element for treatment machine part
  /// \return Json object if found, otherwise null Json object
  rapidjson::Value& GetTreatmentMachinePart(CoordSys partType);
  rapidjson::Value& GetTreatmentMachinePart(std::string partTypeStr);

  std::string GetTreatmentMachinePartFullFilePath(vtkMRMLU70RoomGeoNode* parameterNode, std::string partPath);
  std::string GetTreatmentMachineFileNameWithoutExtension(vtkMRMLU70RoomGeoNode* parameterNode);
  std::string GetTreatmentMachinePartModelName(vtkMRMLU70RoomGeoNode* parameterNode, CoordSys partType);
  std::vector< CoordSys > GetTreatmentMachineParts();
  vtkMRMLModelNode* GetTreatmentMachinePartModelNode(vtkMRMLU70RoomGeoNode* parameterNode, CoordSys partType);
  vtkMRMLModelNode* EnsureTreatmentMachinePartModelNode(vtkMRMLU70RoomGeoNode* parameterNode, CoordSys partType, bool optional=false);
};

//---------------------------------------------------------------------------
vtkSlicerU70RoomGeoLogic::vtkInternal::vtkInternal(vtkSlicerU70RoomGeoLogic* external)
{
  this->External = external;
  this->CurrentTreatmentMachineDescription = new rapidjson::Document;
}

//---------------------------------------------------------------------------
vtkSlicerU70RoomGeoLogic::vtkInternal::~vtkInternal()
{
  delete this->CurrentTreatmentMachineDescription;
  this->CurrentTreatmentMachineDescription = nullptr;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerU70RoomGeoLogic::vtkInternal::GetTreatmentMachinePart(CoordSys type)
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
rapidjson::Value& vtkSlicerU70RoomGeoLogic::vtkInternal::GetTreatmentMachinePart(std::string typeStr)
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
vtkSlicerU70RoomGeoLogic::vtkInternal::GetTreatmentMachineParts()
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
std::string vtkSlicerU70RoomGeoLogic::vtkInternal::GetTreatmentMachinePartFullFilePath(
  vtkMRMLU70RoomGeoNode* parameterNode, std::string partPath)
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
std::string vtkSlicerU70RoomGeoLogic::vtkInternal::GetTreatmentMachineFileNameWithoutExtension(vtkMRMLU70RoomGeoNode* parameterNode)
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
std::string vtkSlicerU70RoomGeoLogic::vtkInternal::GetTreatmentMachinePartModelName(
  vtkMRMLU70RoomGeoNode* parameterNode, CoordSys partType)
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
vtkMRMLModelNode* vtkSlicerU70RoomGeoLogic::vtkInternal::GetTreatmentMachinePartModelNode(
  vtkMRMLU70RoomGeoNode* parameterNode, CoordSys partType)
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
vtkMRMLModelNode* vtkSlicerU70RoomGeoLogic::vtkInternal::EnsureTreatmentMachinePartModelNode(
  vtkMRMLU70RoomGeoNode* parameterNode, CoordSys partType, bool optional/*=false*/)
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
vtkSlicerU70RoomGeoLogic::vtkSlicerU70RoomGeoLogic()
{
  this->Internal = new vtkInternal(this); 

  this->Channel26Cabin3RobotsLogic = vtkSlicerChannel26Cabin3RobotsTransformLogic::New();
}

//----------------------------------------------------------------------------
vtkSlicerU70RoomGeoLogic::~vtkSlicerU70RoomGeoLogic()
{
  if (this->Channel26Cabin3RobotsLogic)
  {
    this->Channel26Cabin3RobotsLogic->Delete();
    this->Channel26Cabin3RobotsLogic = nullptr;
  }

  if (this->Internal)
  {
    delete this->Internal;
    this->Internal = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerU70RoomGeoLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomGeoLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  this->Superclass::SetMRMLSceneInternal(newScene);

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());

  if (this->Channel26Cabin3RobotsLogic)
  {
    this->Channel26Cabin3RobotsLogic->SetMRMLScene(newScene);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerU70RoomGeoLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }

  if (!scene->IsNodeClassRegistered("vtkMRMLU70RoomGeoNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLU70RoomGeoNode>::New());
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLChannel26GeometryNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLChannel26GeometryNode>::New());
  }
}

//----------------------------------------------------------------------------
void vtkSlicerU70RoomGeoLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
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

  using CoordSys = vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier;
/*
  if (caller->IsA("vtkMRMLScadaOpcUaNode"))
  {
    vtkMRMLScadaOpcUaNode* parameterNode = vtkMRMLScadaOpcUaNode::SafeDownCast(caller);
    if (event == vtkCommand::ModifiedEvent)
    {
      vtkErrorMacro("ProcessMRMLNodesEvents: SCADA OPC-UA node is modified: " << parameterNode->GetSysTime());
    }
  }
*/
  if (caller->IsA("vtkMRMLU70RoomGeoNode"))
  {
  }
  if (caller->IsA("vtkMRMLChannel26GeometryNode"))
  {
    vtkMRMLChannel26GeometryNode* channel26Geometry = vtkMRMLChannel26GeometryNode::SafeDownCast(caller);
    if (event == vtkCommand::ModifiedEvent)
    {
      this->Channel26Cabin3RobotsLogic->UpdateFrameToRasHierarchy(channel26Geometry, CoordSys::TableTop);

      this->UpdateTableTopPlaneNode(channel26Geometry);
      this->UpdateTableTopFiducialNode(channel26Geometry);

      {
        double pos[3] = { 1220., 0., 0. };
        double res[3];
        if (this->Channel26Cabin3RobotsLogic->GetTransformForPointBetweenFrames(CoordSys::TableRobotElbowWrist,
          CoordSys::TableRobotBaseFixed, pos, res))
        {
          vtkErrorMacro("ProcessMRMLNodesEvents: ElbowWrist = [1220., 0., 0.] " << res[0] << ' ' << res[1] << ' ' << res[2]);
        }
        pos[0] = 0.;
        pos[2] = 0.;
        if (this->Channel26Cabin3RobotsLogic->GetTransformForPointBetweenFrames(CoordSys::TableRobotWrist,
          CoordSys::TableRobotBaseFixed, pos, res))
        {
          vtkErrorMacro("ProcessMRMLNodesEvents: RobotWrist = [0., 0., 0.] " << res[0] << ' ' << res[1] << ' ' << res[2]);
        }
        pos[0] = 240.;
        if (this->Channel26Cabin3RobotsLogic->GetTransformForPointBetweenFrames(CoordSys::TableRobotFlange,
          CoordSys::TableRobotBaseFixed, pos, res))
        {
          vtkErrorMacro("ProcessMRMLNodesEvents: RobotFlange = [240., 0., 0.] " << res[0] << ' ' << res[1] << ' ' << res[2]);
        }
        pos[0] = 1420.;
        if (this->Channel26Cabin3RobotsLogic->GetTransformForPointBetweenFrames(CoordSys::CarmRobotElbowWrist,
          CoordSys::CarmRobotBaseFixed, pos, res))
        {
          vtkErrorMacro("ProcessMRMLNodesEvents: CarmElbowWrist = [1420., 0., 0.] " << res[0] << ' ' << res[1] << ' ' << res[2]);
        }
        pos[0] = 0.;
        pos[1] = 0.;
        pos[2] = 0.;
        if (this->Channel26Cabin3RobotsLogic->GetTransformForPointBetweenFrames(CoordSys::FixedReference,
          CoordSys::RAS, pos, res))
        {
          vtkErrorMacro("ProcessMRMLNodesEvents: FixedReference->RAS = [0., 0., 0.] " << res[0] << ' ' << res[1] << ' ' << res[2]);
        }
        if (this->Channel26Cabin3RobotsLogic->GetTransformForPointBetweenFrames(CoordSys::TableTop,
          CoordSys::FixedReference, pos, res))
        {
          vtkErrorMacro("ProcessMRMLNodesEvents: TableTop->FixedReference = [0., 0., 0.] " << res[0] << ' ' << res[1] << ' ' << res[2]);
        }
      }
/*
      {
        double pos[3];
        double res[3];
        pos[0] = 0.;
        pos[1] = 0.;
        pos[2] = 0.;
        if (this->Channel26RobotsLogic->GetTransformForPointBetweenFrames(CoordSys::TableFlange,
          CoordSys::FixedReference, pos, res))
        {
          vtkErrorMacro("ProcessMRMLNodesEvents: 0 in TableFlange = [0., 0., 0.] " << pos[0] << ' ' << pos[1] << ' ' << pos[2]);
          vtkErrorMacro("ProcessMRMLNodesEvents: TableFlange->FixedReference = [0., 0., 0.] " << res[0] << ' ' << res[1] << ' ' << res[2]);
        }
        if (this->Channel26RobotsLogic->GetTransformForPointBetweenFrames(CoordSys::TableTop,
          CoordSys::FixedReference, pos, res))
        {
          vtkErrorMacro("ProcessMRMLNodesEvents: 0 in TableTop = [0., 0., 0.] " << pos[0] << ' ' << pos[1] << ' ' << pos[2]);
          vtkErrorMacro("ProcessMRMLNodesEvents: TableTop->FixedReference = [0., 0., 0.] " << res[0] << ' ' << res[1] << ' ' << res[2]);
        }
      }
*/
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomGeoLogic::UpdateFromMRMLScene()
{
  if (this->GetMRMLScene() == nullptr)
  {
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomGeoLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
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
  if (node->IsA("vtkMRMLScadaOpcUaNode"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomGeoLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode* vtkSlicerU70RoomGeoLogic::CreateTableTopPlaneNode(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkNew<vtkMRMLMarkupsPlaneNode> tableTopPlaneNode;
  this->GetMRMLScene()->AddNode(tableTopPlaneNode);
  tableTopPlaneNode->SetName(TABLETOP_MARKUPS_PLANE_NODE_NAME);
//  tableTopPlaneNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("C26C3_") + TABLETOP_MARKUPS_PLANE_NODE_NAME;
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
    patientToRasMatrix->MultiplyPoint(tableTopCenter, tableTopCenterRAS);
    patientToRasMatrix->MultiplyPoint(tableTopUp, tableTopUpRAS);
    patientToRasMatrix->MultiplyPoint(tableTopLeft, tableTopLeftRAS);

    using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
    tableTopPlaneNode->SetOrigin(tableTopCenterRAS);
    tableTopPlaneNode->SetPlaneBounds(-0.5 * CoordPos::TABLE_TOP_WIDTH, 0.5 * CoordPos::TABLE_TOP_WIDTH,
      -0.5 * CoordPos::TABLE_TOP_LENGTH, 0.5 * CoordPos::TABLE_TOP_LENGTH);
    tableTopPlaneNode->SetSize(CoordPos::TABLE_TOP_WIDTH, CoordPos::TABLE_TOP_LENGTH);
    tableTopPlaneNode->SetNormal(0., -1., 0.);
    tableTopPlaneNode->SetSizeMode(vtkMRMLMarkupsPlaneNode::SizeModeAuto);
    tableTopPlaneNode->SetPlaneType(vtkMRMLMarkupsPlaneNode::PlaneType3Points);

    vtkMRMLMarkupsDisplayNode* tableTopPlaneDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(tableTopPlaneNode->GetDisplayNode());
    if (tableTopPlaneDisplayNode)
    {
      tableTopPlaneDisplayNode->SetScaleHandleVisibility(false);
    }
    vtkMRMLTransformNode* transformNode = this->GetChannel26RobotsTransformLogic()->GetTableTopPlaneCorrectionTransform();

    // add transform to fiducial node
    if (transformNode)
    {
      tableTopPlaneNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }

  return tableTopPlaneNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerU70RoomGeoLogic::CreateTableTopFiducialNode(vtkMRMLChannel26GeometryNode* parameterNode)
{
  vtkNew<vtkMRMLMarkupsFiducialNode> tableTopFiducialNode;
  this->GetMRMLScene()->AddNode(tableTopFiducialNode);
  tableTopFiducialNode->SetName(TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME);
//  tableTopFiducialNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("C26C3_") + TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME;
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
    using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;
    for (int i = 0; i < CoordPos::TABLE_TOP_NUMBER_OF_MARKERS; ++i)
    {
      // add point to fiducial node (initial position)
      vtkVector3d p( -1. * TableTopHole1FixedReference[0],
        TableTopHole1FixedReference[2],
        TableTopHole1FixedReference[1] + i * CoordPos::TABLE_TOP_MARKERS_STEP);
      vtkVector3d pm( -1. * TableTopMirrorHole1FixedReference[0],
        TableTopMirrorHole1FixedReference[2],
        TableTopMirrorHole1FixedReference[1] + i * CoordPos::TABLE_TOP_MARKERS_STEP);
      std::string name;
      if (!(i % 2))
      {
        name = std::to_string(i / 2 + 1);
      }
      else
      {
        name = std::string(1, 'A' + (i - 1) / 2);
      }
      

      tableTopFiducialNode->AddControlPoint(p, name.c_str());
      tableTopFiducialNode->AddControlPoint(pm, name.c_str());
    }

    vtkMRMLTransformNode* transformNode = this->GetChannel26RobotsTransformLogic()->GetTableTopPlaneCorrectionTransform();

    // add transform to fiducial node
    if (transformNode)
    {
      tableTopFiducialNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }

  return tableTopFiducialNode;
}

//----------------------------------------------------------------------------
void vtkSlicerU70RoomGeoLogic::UpdateTableTopPlaneNode(vtkMRMLChannel26GeometryNode* parameterNode)
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
    vtkMRMLTransformNode* markupsPlaneTransformNode = this->GetChannel26RobotsTransformLogic()->GetTableTopPlaneCorrectionTransform();

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
void vtkSlicerU70RoomGeoLogic::UpdateTableTopFiducialNode(vtkMRMLChannel26GeometryNode* parameterNode)
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

      vtkMRMLTransformNode* transformNode = this->GetChannel26RobotsTransformLogic()->GetTableTopPlaneCorrectionTransform();

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

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerU70RoomGeoLogic::CreateBeamAxisLineNode(vtkMRMLU70RoomGeoNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateCabin3BeamAxisLineNode: Invalid MRML scene");
    return nullptr;
  }

  // line markups node
  if (scene->GetFirstNodeByName(FIXEDBEAMAXIS_MARKUPS_LINE_NODE_NAME))
  {
    return vtkMRMLMarkupsLineNode::SafeDownCast(scene->GetFirstNodeByName(FIXEDBEAMAXIS_MARKUPS_LINE_NODE_NAME));
  }
    
  vtkMRMLMarkupsLineNode* lineMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsLineNode"));
  lineMarkupsNode->SetName(FIXEDBEAMAXIS_MARKUPS_LINE_NODE_NAME);
//  std::string singletonTag = std::string("C26C3_") + FIXEDBEAMAXIS_MARKUPS_LINE_NODE_NAME;
  lineMarkupsNode->LockedOn();

  if (parameterNode)
  {
    // add points to line node
    vtkVector3d p0( -4000., 0., 0.); // Begin
    vtkVector3d p1( 4000., 0., 0.); // End

    lineMarkupsNode->AddControlPoint( p0, "Cabin3BeamAxisBegin");
    lineMarkupsNode->AddControlPoint( p1, "Cabin3BeamAxisEnd");

    vtkMRMLTransformNode* transformNode = this->GetChannel26RobotsTransformLogic()->GetFixedReferenceTransform();

    // add transform to fiducial node
    if (transformNode)
    {
      lineMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }
    parameterNode->SetAndObserveBeamAxisLineNode(lineMarkupsNode);
  }

  return lineMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerU70RoomGeoLogic::CreateIsocenterFiducialNode(vtkMRMLU70RoomGeoNode* parameterNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateCabin3IsocenterFiducialNode: Invalid MRML scene");
    return nullptr;
  }

  // Fixed isocenter fiducial markups node
  if (scene->GetFirstNodeByName(FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME))
  {
    return vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->GetFirstNodeByName(FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME));
  }

  vtkMRMLMarkupsFiducialNode* pointMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
  pointMarkupsNode->SetName(FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME);
  std::string singletonTag = std::string("C26C3_") + FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME;
  if (parameterNode)
  {
    vtkVector3d pFixedIsocenter( 0., 0., 0.); // Isocenter in origin of FixedReference frame
    pointMarkupsNode->AddControlPoint( pFixedIsocenter, "FixedIsocenter");

    vtkMRMLTransformNode* transformNode = this->GetChannel26RobotsTransformLogic()->GetFixedReferenceTransform();

    // add transform to fiducial node
    if (transformNode)
    {
      pointMarkupsNode->SetAndObserveTransformNodeID(transformNode->GetID());
    }

    parameterNode->SetAndObserveIsocenterFiducialNode(pointMarkupsNode);
  }

  return pointMarkupsNode;
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomGeoLogic::BuildRobotsTransformHierarchy()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("BuildRobotTableGeometryTransformHierarchy: Invalid MRML scene");
    return;
  }

  // Build TableTop robot hierarchy
  if (this->Channel26Cabin3RobotsLogic)
  {
    this->Channel26Cabin3RobotsLogic->BuildRobotsTransformHierarchy();
  }
}

//----------------------------------------------------------------------------
std::vector<vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier>
vtkSlicerU70RoomGeoLogic::LoadTreatmentMachineComponents(vtkMRMLU70RoomGeoNode* parameterNode)
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
vtkSlicerU70RoomGeoLogic::SetupTreatmentMachineModels(vtkMRMLU70RoomGeoNode* parameterNode, bool forceEnableCollisionDetection/*=false*/)
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
        case CoordSys::TableFlange:
        case CoordSys::TableRobotFlange:
        case CoordSys::TableRobotWrist:
        case CoordSys::TableRobotElbowWrist:
        case CoordSys::TableRobotElbowShoulder:
        case CoordSys::TableRobotShoulder:
        case CoordSys::TableRobotBaseRotation:
        case CoordSys::TableRobotBaseRotationDisk:
        case CoordSys::TableRobotBaseFixed:
        case CoordSys::CarmRobotBaseFixed:
        case CoordSys::CarmRobotBaseRotation:
        case CoordSys::CarmRobotShoulder:
        case CoordSys::CarmRobotElbowShoulder:
        case CoordSys::CarmRobotElbowWrist:
        case CoordSys::CarmRobotWrist:
        case CoordSys::CarmRobotFlange:
        case CoordSys::Carm:
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

    vtkMRMLLinearTransformNode* partFrameToRasTransformNode = nullptr;
    switch (partIdx)
    {
      case CoordSys::TableTop:
        this->Channel26Cabin3RobotsLogic->UpdatePatientToTableTopTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateTableTopToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::TableFlange:
        this->Channel26Cabin3RobotsLogic->UpdateTableTopToTableFlangeTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateTableFlangeToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::TableRobotFlange:
        this->Channel26Cabin3RobotsLogic->UpdateTableFlangeToTableRobotFlangeTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateTableRobotFlangeToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::TableRobotWrist:
        this->Channel26Cabin3RobotsLogic->UpdateTableRobotFlangeToTableRobotWristTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateTableRobotWristToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::TableRobotElbowWrist:
        this->Channel26Cabin3RobotsLogic->UpdateTableRobotWristToTableRobotElbowWristTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateTableRobotElbowWristToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::TableRobotElbowShoulder:
        this->Channel26Cabin3RobotsLogic->UpdateTableRobotElbowWristToTableRobotElbowShoulderTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateTableRobotElbowShoulderToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::TableRobotShoulder:
        this->Channel26Cabin3RobotsLogic->UpdateTableRobotElbowShoulderToTableRobotShoulderTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateTableRobotShoulderToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::TableRobotBaseRotationDisk:
        this->Channel26Cabin3RobotsLogic->UpdateTableRobotShoulderToTableRobotBaseRotationTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateTableRobotBaseRotationDiskToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::TableRobotBaseRotation:
        this->Channel26Cabin3RobotsLogic->UpdateTableRobotShoulderToTableRobotBaseRotationTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateTableRobotBaseRotationToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::TableRobotBaseFixed:
        this->Channel26Cabin3RobotsLogic->UpdateTableRobotBaseRotationToTableRobotBaseFixedTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateTableRobotBaseFixedToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::FixedReference:
        this->Channel26Cabin3RobotsLogic->UpdateTableRobotBaseFixedToFixedReferenceTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateFixedReferenceToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::CarmRobotBaseFixed:
        this->Channel26Cabin3RobotsLogic->UpdateCarmRobotBaseFixedToFixedReferenceTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateCarmRobotBaseFixedToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::CarmRobotBaseRotation:
        this->Channel26Cabin3RobotsLogic->UpdateCarmRobotBaseRotationToCarmRobotBaseFixedTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateCarmRobotBaseRotationToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::CarmRobotShoulder:
        this->Channel26Cabin3RobotsLogic->UpdateCarmRobotShoulderToCarmRobotBaseRotationTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateCarmRobotShoulderToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::CarmRobotElbowShoulder:
        this->Channel26Cabin3RobotsLogic->UpdateCarmRobotShoulderToCarmRobotBaseRotationTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateCarmRobotElbowShoulderToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::CarmRobotElbowWrist:
        this->Channel26Cabin3RobotsLogic->UpdateCarmRobotElbowWristToCarmRobotElbowShoulderTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateCarmRobotElbowWristToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::CarmRobotWrist:
        this->Channel26Cabin3RobotsLogic->UpdateCarmRobotWristToCarmRobotElbowWristTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateCarmRobotWristToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::CarmRobotFlange:
        this->Channel26Cabin3RobotsLogic->UpdateCarmRobotFlangeToCarmRobotWristTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateCarmRobotFlangeToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::Carm:
        this->Channel26Cabin3RobotsLogic->UpdateCarmToCarmRobotFlangeTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateCarmToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::CarmXrayBeam:
        this->Channel26Cabin3RobotsLogic->UpdateCarmXrayBeamToCarmTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateCarmXrayBeamToRasTransform(channel26GeometryNode);
        break;
      case CoordSys::CarmXrayDetector:
        this->Channel26Cabin3RobotsLogic->UpdateCarmXrayDetectorToCarmTransform(channel26GeometryNode);
        partFrameToRasTransformNode = this->Channel26Cabin3RobotsLogic->UpdateCarmXrayDetectorToRasTransform(channel26GeometryNode);
        break;
      default:
        break;
    }
    if (partFrameToRasTransformNode)
    {
      partModel->SetAndObserveTransformNodeID(partFrameToRasTransformNode->GetID());
    }
  }

  return loadedParts;
}

//----------------------------------------------------------------------------
void vtkSlicerU70RoomGeoLogic::ShowMarkupsNodes(vtkMRMLU70RoomGeoNode* parameterNode, bool show)
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
  markupsNames.push_back(vtkSlicerU70RoomGeoLogic::TABLETOP_MARKUPS_PLANE_NODE_NAME);
  markupsNames.push_back(vtkSlicerU70RoomGeoLogic::TABLETOP_MARKUPS_FIDUCIAL_NODE_NAME);
  markupsNames.push_back(vtkSlicerU70RoomGeoLogic::FIXEDBEAMAXIS_MARKUPS_LINE_NODE_NAME);
  markupsNames.push_back(vtkSlicerU70RoomGeoLogic::FIXEDISOCENTER_MARKUPS_FIDUCIAL_NODE_NAME);

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
void vtkSlicerU70RoomGeoLogic::ShowModelsNodes(vtkMRMLU70RoomGeoNode* parameterNode, bool show)
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
  modelsNames.push_back("FixedReference");
  modelsNames.push_back("CarmRobotBaseFixed");
  modelsNames.push_back("TableRobotBaseFixed");
  modelsNames.push_back("CarmRobotBaseRotation");
  modelsNames.push_back("TableRobotBaseRotation");
  modelsNames.push_back("TableRobotBaseRotationDisk");
  modelsNames.push_back("CarmRobotShoulder");
  modelsNames.push_back("TableRobotShoulder");
  modelsNames.push_back("CarmRobotElbowShoulder");
  modelsNames.push_back("TableRobotElbowShoulder");
  modelsNames.push_back("CarmRobotElbowWrist");
  modelsNames.push_back("TableRobotElbowWrist");
  modelsNames.push_back("CarmRobotWrist");
  modelsNames.push_back("TableRobotWrist");
  modelsNames.push_back("CarmRobotFlange");
  modelsNames.push_back("TableRobotFlange");
  modelsNames.push_back("TableFlange");
  modelsNames.push_back("TableTop");
  modelsNames.push_back("Carm");
  modelsNames.push_back("CarmXrayBeam");
  modelsNames.push_back("CarmXrayDetector");

  for (auto modelName : modelsNames)
  {
    const char* treatmentMachineType = parameterNode->GetTreatmentMachineType();
    if (!treatmentMachineType)
    {
      vtkErrorMacro("ShowModelsNodes: Invalid treatment machine type");
      continue;
    }
    std::string fullName = std::string(treatmentMachineType) + std::string("_") + modelName;
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
const char* vtkSlicerU70RoomGeoLogic::GetTreatmentMachinePartTypeAsString(
  vtkSlicerChannel26Cabin3RobotsTransformLogic::CoordinateSystemIdentifier type)
{
  return this->Channel26Cabin3RobotsLogic->GetTreatmentMachinePartTypeAsString(type);
}

//---------------------------------------------------------------------------
std::string vtkSlicerU70RoomGeoLogic::GetNameForPartType(std::string partType)
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
std::string vtkSlicerU70RoomGeoLogic::GetFilePathForPartType(std::string partType)
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
bool vtkSlicerU70RoomGeoLogic::GetFileToRASTransformMatrixForPartType(std::string partType, vtkMatrix4x4* fileToPartTransformMatrix)
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
vtkVector3d vtkSlicerU70RoomGeoLogic::GetColorForPartType(std::string partType)
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
std::string vtkSlicerU70RoomGeoLogic::GetStateForPartType(std::string partType)
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

vtkSlicerChannel26Cabin3RobotsTransformLogic* vtkSlicerU70RoomGeoLogic::GetChannel26RobotsTransformLogic() const
{
  return this->Channel26Cabin3RobotsLogic;
}
