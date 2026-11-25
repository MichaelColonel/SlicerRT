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

// U70RoomOpcUa Logic includes
#include "vtkSlicerU70RoomSiemensPlcOpcUaLogic.h"
#include "vtkSlicerU70RoomSiemensPlcOpcUaNodesNames.h"

// MRML includes
#include <vtkMRMLScene.h>

// U70RoomOpcUa MRML includes
#include "vtkMRMLSiemensPlcOpcUaNode.h"

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cstring>
#include <bitset>

// QtOpcUa includes
#include <QOpcUaClient>
#include <QWeakPointer>
#include <QPointer>
#include <QVariant>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QOpcUaNode>
#include <QOpcUaMonitoringParameters>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerU70RoomSiemensPlcOpcUaLogic);

//---------------------------------------------------------------------------
class vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal
{
public:
  vtkInternal(vtkSlicerU70RoomSiemensPlcOpcUaLogic* external);
  ~vtkInternal();
   QOpcUaNode* node(const QString& fullNodeName);
   bool readNodeValueAttribute(const std::string& fullNodeName);
   double getDouble(const QString& fullNodeName, bool& res);
   uint64_t getFlagsFromList(const QString& fullNodeName, bool& res);
   uint32_t getUint32(const QString& fullNodeName, bool& res);
   int32_t getInt32(const QString& fullNodeName, bool& res);
   uint16_t getUint16(const QString& fullNodeName, bool& res);
   bool getBool(const QString& fullNodeName, bool& res);
   int8_t getInt8(const QString& fullNodeName, bool& res);
   uint8_t getUint8(const QString& fullNodeName, bool& res);

   bool updateMRMLNodeFromOpcUaModeAndStatus(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   bool updateMRMLNodeFromOpcUaAutoManualButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   bool updateMRMLNodeFromOpcUaServiceButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   bool updateMRMLNodeFromOpcUaKukaControllersButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   bool updateMRMLNodeFromOpcUaOtherButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   bool updateMRMLNodeFromOpcUaSetCoordinates(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   bool updateMRMLNodeFromOpcUaCurrentCoordinates(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   bool updateMRMLNodeFromOpcUaReflectionCoordinates(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   bool updateMRMLNodeFromOpcUaFlags(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);

  vtkSlicerU70RoomSiemensPlcOpcUaLogic* External{ nullptr };

  struct Entry
  {
    QPointer< QOpcUaNode > Node;        // becomes null automatically if node is deleted
    QMetaObject::Connection ConnChanged;
    QMetaObject::Connection ConnRead;
    QVariant Cache;
  };

  QWeakPointer< QOpcUaClient > Client;
  QHash< QString, Entry > Nodes;
};

//---------------------------------------------------------------------------
QOpcUaNode* vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::node(const QString& fullNodeName)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "node: Siemens PLC OPC UA client is invalid");
    return nullptr;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "node: Siemens PLC OPC UA client is created, but isn't connected");
    return nullptr;
  }

  return this->Nodes.value(fullNodeName).Node;
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getDouble(const QString& fullNodeName, bool& res)
{
  int32_t v = this->getInt32(fullNodeName, res);
  return double(v) / 1000.;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::readNodeValueAttribute(const std::string& fullNodeName)
{
  QOpcUaNode* node = this->node(QString::fromStdString(fullNodeName));
  if (node)
  {
    return node->readAttributes(QOpcUa::NodeAttribute::Value);
  }
  return false;
}

//---------------------------------------------------------------------------
uint64_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getFlagsFromList(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getFlagsFromList: Siemens PLC OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getFlagsFromList: Siemens PLC OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& variant = this->Nodes.value(fullNodeName).Cache;

  if (variant.canConvert< QVariantList >())
  {
    std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > flags;
    QVariantList variantList = variant.toList(); // Get the attribute from the cache
    if (variantList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
    {
      int i = 0;
      for (const QVariant& flag : variantList)
      {
        flags.set(i, flag.toBool());
        ++i;
      }
      res = true;
      return static_cast< uint64_t >(flags.to_ullong());
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
uint32_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getUint32(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getUint32: Siemens PLC OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getUint32: Siemens PLC OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< uint32_t >())
  {
    bool resBool;
    uint v = var.toUInt(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< uint32_t >(v);
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
int32_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getInt32(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Siemens PLC OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Siemens PLC OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< int32_t >())
  {
    bool resBool;
    int v = var.toInt(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< int32_t >(v);
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getBool(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getBool: Siemens PLC OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getBool: Siemens PLC OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< bool >())
  {
    res = true;
    return var.toBool();
  }
  return false;
}

//---------------------------------------------------------------------------
int8_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getInt8(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getInt8: Siemens PLC OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt8: Siemens PLC OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< int8_t >())
  {
    bool resBool;
    int v = var.toInt(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< int8_t >(v);
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
uint8_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getUint8(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Siemens PLC OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Siemens PLC OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< uint8_t >())
  {
    bool resBool;
    int v = var.toInt(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< uint8_t >(v);
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getUint16(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Siemens PLC OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Siemens PLC OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< uint16_t >())
  {
    bool resBool;
    int v = var.toInt(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< uint16_t >(v);
    }
  }
  return 0;
}

//---------------------------------------------------------------------------
vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::vtkInternal(vtkSlicerU70RoomSiemensPlcOpcUaLogic* external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::~vtkInternal()
{
}

bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::updateMRMLNodeFromOpcUaModeAndStatus(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = false;
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode())
  {
    uint8_t modeState = this->getUint8(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      vtkErrorWithObjectMacro(this->External, "UpdateMRMLNodeFromOpcUaNodeValue: Mode: " << int(modeState));
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetMode(static_cast< vtkMRMLSiemensPlcOpcUaNode::ModeType >(modeState));
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusRtk())
  {
    uint16_t rtkState = this->getUint16(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      vtkErrorWithObjectMacro(this->External, "UpdateMRMLNodeFromOpcUaNodeValue: RTK State: " << rtkState);
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetStateRtk(rtkState);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusR1())
  {
    uint16_t r1State = this->getUint16(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetStateR1(r1State);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusR2())
  {
    uint16_t r2State = this->getUint16(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetStateR2(r2State);
    }
  }
  return res;
}

bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::updateMRMLNodeFromOpcUaCurrentCoordinates(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = false;
  // R1 == Tabletop robot
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A1())
  {
    int32_t r1a1 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r1axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR1(r1axis);
      r1axis[0] = r1a1;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR1(r1axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A2())
  {
    int32_t r1a2 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r1axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR1(r1axis);
      r1axis[1] = r1a2;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR1(r1axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A3())
  {
    int32_t r1a3 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r1axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR1(r1axis);
      r1axis[2] = r1a3;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR1(r1axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A4())
  {
    int32_t r1a4 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r1axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR1(r1axis);
      r1axis[3] = r1a4;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR1(r1axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A5())
  {
    int32_t r1a5 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r1axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR1(r1axis);
      r1axis[4] = r1a5;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR1(r1axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A6())
  {
    int32_t r1a6 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r1axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR1(r1axis);
      r1axis[5] = r1a6;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR1(r1axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR1X())
  {
    int32_t kuka_x = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR1(kuka_coods);
      kuka_coods[0] = kuka_x;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR1Y())
  {
    int32_t kuka_y = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR1(kuka_coods);
      kuka_coods[1] = kuka_y;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR1Z())
  {
    int32_t kuka_z = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR1(kuka_coods);
      kuka_coods[2] = kuka_z;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR1A())
  {
    int32_t kuka_a = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR1(kuka_coods);
      kuka_coods[3] = kuka_a;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR1B())
  {
    int32_t kuka_b = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR1(kuka_coods);
      kuka_coods[4] = kuka_b;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR1C())
  {
    int32_t kuka_c = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR1(kuka_coods);
      kuka_coods[5] = kuka_c;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR1(kuka_coods);
      }
    }
  }

  // R2 == C-arm robot
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A1())
  {
    int32_t r2a1 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r2axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR2(r2axis);
      r2axis[0] = r2a1;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR2(r2axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A2())
  {
    int32_t r2a2 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r2axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR2(r2axis);
      r2axis[1] = r2a2;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR2(r2axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A3())
  {
    int32_t r2a3 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r2axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR2(r2axis);
      r2axis[2] = r2a3;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR2(r2axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A4())
  {
    int32_t r2a4 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r2axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR2(r2axis);
      r2axis[3] = r2a4;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR2(r2axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A5())
  {
    int32_t r2a5 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r2axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR2(r2axis);
      r2axis[4] = r2a5;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR2(r2axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A6())
  {
    int32_t r2a6 = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t r2axis[6] = {};
      mrmlSiemensPlcNode->GetCurrentAxisCoordsR2(r2axis);
      r2axis[5] = r2a6;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentAxisCoordsR2(r2axis);
      }
      mrmlSiemensPlcNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR2X())
  {
    int32_t kuka_x = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR2(kuka_coods);
      kuka_coods[0] = kuka_x;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR2(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR2Y())
  {
    int32_t kuka_y = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR2(kuka_coods);
      kuka_coods[1] = kuka_y;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR2(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR2Z())
  {
    int32_t kuka_z = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR1(kuka_coods);
      kuka_coods[2] = kuka_z;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR2A())
  {
    int32_t kuka_a = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR2(kuka_coods);
      kuka_coods[3] = kuka_a;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR2(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR2B())
  {
    int32_t kuka_b = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR2(kuka_coods);
      kuka_coods[4] = kuka_b;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR2(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentCoordR2C())
  {
    int32_t kuka_c = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetCurrentCoordsR2(kuka_coods);
      kuka_coods[5] = kuka_c;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCurrentCoordsR2(kuka_coods);
      }
    }
  }
  return res;
}

bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::updateMRMLNodeFromOpcUaReflectionCoordinates(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = false;
  // R1 == Tabletop robot

  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR1X())
  {
    int32_t kuka_x = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR1(kuka_coods);
      kuka_coods[0] = kuka_x;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR1Y())
  {
    int32_t kuka_y = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR1(kuka_coods);
      kuka_coods[1] = kuka_y;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR1Z())
  {
    int32_t kuka_z = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR1(kuka_coods);
      kuka_coods[2] = kuka_z;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR1A())
  {
    int32_t kuka_a = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR1(kuka_coods);
      kuka_coods[3] = kuka_a;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR1B())
  {
    int32_t kuka_b = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR1(kuka_coods);
      kuka_coods[4] = kuka_b;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR1(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR1C())
  {
    int32_t kuka_c = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR1(kuka_coods);
      kuka_coods[5] = kuka_c;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR1(kuka_coods);
      }
    }
  }

  // R2 == C-arm robot
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR2X())
  {
    int32_t kuka_x = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR2(kuka_coods);
      kuka_coods[0] = kuka_x;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR2(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR2Y())
  {
    int32_t kuka_y = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR2(kuka_coods);
      kuka_coods[1] = kuka_y;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR2(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR2Z())
  {
    int32_t kuka_z = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR2(kuka_coods);
      kuka_coods[2] = kuka_z;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR2(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR2A())
  {
    int32_t kuka_a = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR2(kuka_coods);
      kuka_coods[3] = kuka_a;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR2(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR2B())
  {
    int32_t kuka_b = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR2(kuka_coods);
      kuka_coods[4] = kuka_b;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR2(kuka_coods);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReflectionTargetCoordsR2C())
  {
    int32_t kuka_c = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      int32_t kuka_coods[6] = {};
      mrmlSiemensPlcNode->GetConfirmedCoordsR2(kuka_coods);
      kuka_coods[5] = kuka_c;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetConfirmedCoordsR2(kuka_coods);
      }
    }
  }
  return res;
}

bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::updateMRMLNodeFromOpcUaAutoManualButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = false;
  // R1 == Tabletop robot
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1LoadToIsoButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR1LoadToIso(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR1LoadToIso(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1LoadToIsoButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR1LoadToIso(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR1LoadToIso(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToLoadButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR1ToLoad(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR1ToLoad(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToLoadButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR1ToLoad(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR1ToLoad(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToNewCoordsButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR1ToNewCoords(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR1ToNewCoords(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToNewCoordsButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR1ToNewCoords(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR1ToNewCoords(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1R2EmergencyEvacuationButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualEmergencyEvac(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualEmergencyEvac(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1R2EmergencyEvacuationButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualEmergencyEvac(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualEmergencyEvac(buttonEnabledPressed);
      }
    }
  }
  // R2 == C-arm
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToHomeButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR2ToHome(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR2ToHome(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToHomeButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR2ToHome(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR2ToHome(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane1ButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR2ToPlane1(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR2ToPlane1(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane1ButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR2ToPlane1(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR2ToPlane1(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane2ButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR2ToPlane2(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR2ToPlane2(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane2ButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualR2ToPlane2(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualR2ToPlane2(buttonEnabledPressed);
      }
    }
  }

  // Other AM_MM buttons
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualSetTabletopOrientationButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualApplyTabletopPosition(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualApplyTabletopPosition(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualSetTabletopOrientationButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualApplyTabletopPosition(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualApplyTabletopPosition(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualApplyNewCoordsButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualApplyNewCoords(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualApplyNewCoords(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualApplyNewCoordsButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetAutoManualApplyNewCoords(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetAutoManualApplyNewCoords(buttonEnabledPressed);
      }
    }
  }

  return res;
}

bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::updateMRMLNodeFromOpcUaServiceButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = false;
  // R1 == Tabletop robot
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1BreakTestButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1BreakTest(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1BreakTest(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1BreakTestButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1BreakTest(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1BreakTest(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1MasterReferenceTestButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1MasterReferenceTest(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1MasterReferenceTest(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1MasterReferenceTestButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1MasterReferenceTest(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1MasterReferenceTest(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1LoadButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1Load(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1Load(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1LoadButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1Load(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1Load(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1ServicePosition1ButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1Position1(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1Position1(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1ServicePosition1ButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1Position1(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1Position1(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1ServicePosition2ButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1Position2(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1Position2(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1ServicePosition2ButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1Position2(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1Position2(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1ServicePosition3ButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1Position3(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1Position3(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR1ServicePosition3ButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR1Position3(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR1Position3(buttonEnabledPressed);
      }
    }
  }

  // R2 == C-arm
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2BreakTestButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2BreakTest(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2BreakTest(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2BreakTestButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2BreakTest(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2BreakTest(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2MasterReferenceTestButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2MasterReferenceTest(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2MasterReferenceTest(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2MasterReferenceTestButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2MasterReferenceTest(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2MasterReferenceTest(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2HomeButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2Home(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2Home(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2HomeButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2Home(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2Home(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2ServicePosition1ButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2Position1(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2Position1(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2ServicePosition1ButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2Position1(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2Position1(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2ServicePosition2ButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2Position2(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2Position2(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2ServicePosition2ButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2Position2(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2Position2(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2ServicePosition3ButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2Position3(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2Position3(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceR2ServicePosition3ButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetServiceR2Position3(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetServiceR2Position3(buttonEnabledPressed);
      }
    }
  }

  return res;
}

bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::updateMRMLNodeFromOpcUaKukaControllersButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = false;
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetKukaControllersAllowT1ButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetKukaAllowT1(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetKukaAllowT1(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetKukaControllersAllowT1ButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetKukaAllowT1(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetKukaAllowT1(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetKukaControllersAllowXrayButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetKukaAllowXray(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetKukaAllowXray(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetKukaControllersAllowXrayButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetKukaAllowXray(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetKukaAllowXray(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetKukaControllersAllowBeamButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetKukaAllowBeam(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetKukaAllowBeam(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetKukaControllersAllowBeamButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetKukaAllowBeam(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetKukaAllowBeam(buttonEnabledPressed);
      }
    }
  }

  return res;
}

bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::updateMRMLNodeFromOpcUaOtherButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  // ResetErrors button
  bool res = false;
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetResetErrorsButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetResetErrors(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetResetErrors(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetResetErrorsButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetResetErrors(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetResetErrors(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMakeXrayButtonIsEnabled())
  {
    bool enabled = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetMakeXray(buttonEnabledPressed);
      buttonEnabledPressed[0] = enabled;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetMakeXray(buttonEnabledPressed);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMakeXrayButtonIsPressed())
  {
    bool pressed = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      bool buttonEnabledPressed[2] = {};
      mrmlSiemensPlcNode->GetMakeXray(buttonEnabledPressed);
      buttonEnabledPressed[1] = pressed;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetMakeXray(buttonEnabledPressed);
      }
    }
  }

  return res;
}

bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::updateMRMLNodeFromOpcUaSetCoordinates(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = false;
  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordX())
  {
    int32_t coord_x = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      vtkErrorWithObjectMacro(this->External, "UpdateMRMLNodeFromOpcUaNodeValue: Set new coord X: " << coord_x);
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetCoordsX(coord_x);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordY())
  {
    int32_t coord_y = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      vtkErrorWithObjectMacro(this->External, "UpdateMRMLNodeFromOpcUaNodeValue: Set new coord Y: " << coord_y);
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetCoordsY(coord_y);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordZ())
  {
    int32_t coord_z = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      vtkErrorWithObjectMacro(this->External, "UpdateMRMLNodeFromOpcUaNodeValue: Set new coord Z: " << coord_z);
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetCoordsZ(coord_z);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordA())
  {
    int32_t angle_a = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      vtkErrorWithObjectMacro(this->External, "UpdateMRMLNodeFromOpcUaNodeValue: Set new angle A: " << angle_a);
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetCoordsA(angle_a);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordB())
  {
    int32_t angle_b = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      vtkErrorWithObjectMacro(this->External, "UpdateMRMLNodeFromOpcUaNodeValue: Set new angle B: " << angle_b);
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetCoordsB(angle_b);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordC())
  {
    int32_t angle_c = this->getInt32(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      vtkErrorWithObjectMacro(this->External, "UpdateMRMLNodeFromOpcUaNodeValue: Set new angle C: " << angle_c);
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetCoordsC(angle_c);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetTabletopOrientation())  // Other coordinates as a Patient (tabletop) position/orientation and
  {
    uint8_t tabletopPosition = this->getUint8(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      vtkErrorWithObjectMacro(this->External, "UpdateMRMLNodeFromOpcUaNodeValue: Tabletop orientation: " << int(tabletopPosition));
      MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
      mrmlSiemensPlcNode->SetTableTopPosition(tabletopPosition);
    }
  }
  return res;
}

bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::updateMRMLNodeFromOpcUaFlags(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = false;

  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetErrorsFlags())
  {
    uint64_t errMsgFlags = this->getFlagsFromList(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetErrorMessages(errMsgFlags);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceFlags())
  {
    uint64_t servMsgFlags = this->getFlagsFromList(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetServiceMessages(servMsgFlags);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMessagesFlags())
  {
    uint64_t msgFlags = this->getFlagsFromList(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetMiscMessages(msgFlags);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCoordinatesAcceptedR1Flag())
  {
    bool flag = this->getBool(QString::fromStdString(fullNodeName), res);
    bool acceptedCoordsFlags[4] = {};
    if (res)
    {
      mrmlSiemensPlcNode->GetCoordsAccepted(acceptedCoordsFlags);
      acceptedCoordsFlags[0] = flag;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCoordsAccepted(acceptedCoordsFlags);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCoordinatesNotAcceptedR1Flag())
  {
    bool flag = this->getBool(QString::fromStdString(fullNodeName), res);
    bool acceptedCoordsFlags[4] = {};
    if (res)
    {
      mrmlSiemensPlcNode->GetCoordsAccepted(acceptedCoordsFlags);
      acceptedCoordsFlags[1] = flag;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCoordsAccepted(acceptedCoordsFlags);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCoordinatesAcceptedR2Flag())
  {
    bool flag = this->getBool(QString::fromStdString(fullNodeName), res);
    bool acceptedCoordsFlags[4] = {};
    if (res)
    {
      mrmlSiemensPlcNode->GetCoordsAccepted(acceptedCoordsFlags);
      acceptedCoordsFlags[2] = flag;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCoordsAccepted(acceptedCoordsFlags);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCoordinatesNotAcceptedR2Flag())
  {
    bool flag = this->getBool(QString::fromStdString(fullNodeName), res);
    bool acceptedCoordsFlags[4] = {};
    if (res)
    {
      mrmlSiemensPlcNode->GetCoordsAccepted(acceptedCoordsFlags);
      acceptedCoordsFlags[3] = flag;
      {
        MRMLNodeModifyBlocker block(mrmlSiemensPlcNode);
        mrmlSiemensPlcNode->SetCoordsAccepted(acceptedCoordsFlags);
      }
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetComingHomeHoldButtonPressedFlag())
  {
    bool flag = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetHoldButtonPressedForMovingFlag(flag);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReadyForXray1Flag())
  {
    bool flag = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetRobotsReadyForXray1(flag);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReadyForXray2Flag())
  {
    bool flag = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetRobotsReadyForXray2(flag);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetReadyForBeamFlag())
  {
    bool flag = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetRobotsReadyForBeam(flag);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetEmergencyEvacuationFlag())
  {
    bool flag = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetEmergencyEvacSignal(flag);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetPatientOnTheTableFlag())
  {
    bool flag = this->getBool(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetPatientOnTableTop(flag);
    }
  }
  return res;
}

//----------------------------------------------------------------------------
vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkSlicerU70RoomSiemensPlcOpcUaLogic()
{
  this->Internal = new vtkInternal(this); 
}

//----------------------------------------------------------------------------
vtkSlicerU70RoomSiemensPlcOpcUaLogic::~vtkSlicerU70RoomSiemensPlcOpcUaLogic()
{
  if (this->Internal)
  {
    delete this->Internal;
    this->Internal = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }

  if (!scene->IsNodeClassRegistered("vtkMRMLSiemensPlcOpcUaNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLSiemensPlcOpcUaNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::UpdateFromMRMLScene()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
std::list< QOpcUaNode* > vtkSlicerU70RoomSiemensPlcOpcUaLogic::ServerConnected(const QSharedPointer< QOpcUaClient >& sharedClient, const QHash< QString, QString >& hashNodesFullNamesIds)
{
  std::list< QOpcUaNode* > connectedNodesList;

  this->UnregisterAllOpcUaNodes();

  this->Internal->Client = sharedClient;

  if (!sharedClient)
  {
    return connectedNodesList;
  }

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return connectedNodesList;
  }

  vtkMRMLSiemensPlcOpcUaNode* siemensOpcUaNode = nullptr;
  if (vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLSiemensPlcOpcUaNode"))
  {
    siemensOpcUaNode = vtkMRMLSiemensPlcOpcUaNode::SafeDownCast(node);
  }

  QHashIterator< QString, QString > i(hashNodesFullNamesIds);
  std::list< std::string > fullNameNodesList = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSiemensPlcFullNodeNamesList();

  while (i.hasNext())
  {
    i.next();
    QString nodeFullName = i.key();
  
    bool flag = false;
    for (const auto& builtinFullNodeName : fullNameNodesList)
    {
      if (nodeFullName == QString::fromStdString(builtinFullNodeName))
      {
        flag = true;
        break;
      }
    }

    vtkInternal::Entry siemensPlcOpcUaNodeEntry;
    siemensPlcOpcUaNodeEntry.Node = sharedClient->node(i.value());
    if (!flag || !siemensPlcOpcUaNodeEntry.Node)
    {
      continue;
    }

    QOpcUaNode* node = siemensPlcOpcUaNodeEntry.Node.data();

    connectedNodesList.push_back(node);

    node->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

    siemensPlcOpcUaNodeEntry.ConnChanged = QObject::connect(node,
      &QOpcUaNode::dataChangeOccurred, [this, nodeFullName, siemensOpcUaNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr != QOpcUa::NodeAttribute::Value)
      {
        return;
      }
      this->Internal->Nodes[nodeFullName].Cache = value; // map types as needed
      std::string stdNodeName = nodeFullName.toStdString();
      if (siemensOpcUaNode && !nodeFullName.isEmpty())
      {
        char* ptr = new char[stdNodeName.size() + 1];
        std::memset(ptr, 0, stdNodeName.size() + 1);
        stdNodeName.copy(ptr, stdNodeName.size());
        siemensOpcUaNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::ValueChangedEvent, static_cast< void* >(ptr));
      }
    });

    siemensPlcOpcUaNodeEntry.ConnRead = QObject::connect(node,
      &QOpcUaNode::attributeRead, [this, nodeFullName, node, siemensOpcUaNode](QOpcUa::NodeAttributes attr)
    {
      if (!node)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = node->attribute(QOpcUa::NodeAttribute::Value);
          this->Internal->Nodes[nodeFullName].Cache = value; // map types as needed
          std::string stdNodeName = nodeFullName.toStdString();
          if (siemensOpcUaNode && !nodeFullName.isEmpty())
          {
            char* ptr = new char[stdNodeName.size() + 1];
            std::memset(ptr, 0, stdNodeName.size() + 1);
            stdNodeName.copy(ptr, stdNodeName.size());
            siemensOpcUaNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::ValueReadEvent, static_cast< void* >(ptr));
          }
        }
      }
    });
    this->Internal->Nodes.insert(nodeFullName, siemensPlcOpcUaNodeEntry);
  }
  return connectedNodesList;
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::UnregisterOpcUaNode(const std::string& nodeFullName)
{
  auto it = this->Internal->Nodes.find(QString::fromStdString(nodeFullName));
  if (it == this->Internal->Nodes.end())
  {
    return;
  }
  QObject::disconnect(it.value().ConnChanged);
  QObject::disconnect(it.value().ConnRead);
  this->Internal->Nodes.erase(it);
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::UnregisterAllOpcUaNodes()
{
  for (auto& kv : this->Internal->Nodes)
  {
    QObject::disconnect(kv.ConnChanged);
    QObject::disconnect(kv.ConnRead);
  }
  this->Internal->Nodes.clear();
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR1A1(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A1();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A1()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A1();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR1A2(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A2();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A2()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A2();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR1A3(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A3();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A3()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A3();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR1A4(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A4();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A4()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A4();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR1A5(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A5();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A5()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A5();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR1A6(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A6();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A6()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A6();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR2A1(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A1();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A1()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A1();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR2A2(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A2();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A2()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A2();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR2A3(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A3();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A3()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A3();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR2A4(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A4();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A4()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A4();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR2A5(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A5();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A5()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A5();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetCurrentCoordsR2A6(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A6();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A6()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A6();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
uint8_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetMode(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  return this->Internal->getUint8(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadMode()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteMode(vtkMRMLSiemensPlcOpcUaNode::ModeType mode)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(mode), QOpcUa::Types::Byte);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteTabletopOrientation(vtkMRMLSiemensPlcOpcUaNode::TabletopOrientation ttOrientation)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetTabletopOrientation();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(ttOrientation), QOpcUa::Types::Byte);
  }
  return false;
}

//---------------------------------------------------------------------------
uint8_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetTabletopOrientation(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetTabletopOrientation();
  return this->Internal->getUint8(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadTabletopOrientation()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetTabletopOrientation();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetStatusRtk(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusRtk();
  return this->Internal->getUint16(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadStatusRtk()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusRtk();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetStatusR1(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  return this->Internal->getUint16(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadStatusR1()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusR1();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetStatusR2(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  return this->Internal->getUint16(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadStatusR2()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusR2();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteAutoManualR1LoadToIsoPressed(bool pressed)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1LoadToIsoButtonIsPressed();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(pressed), QOpcUa::Types::Boolean);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR1LoadToIsoPressed(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1LoadToIsoButtonIsPressed();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR1LoadToIsoPressed()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1LoadToIsoButtonIsPressed();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR1LoadToIsoEnabled(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1LoadToIsoButtonIsEnabled();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR1LoadToIsoEnabled()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1LoadToIsoButtonIsEnabled();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteAutoManualR1ToLoadPressed(bool pressed)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToLoadButtonIsPressed();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(pressed), QOpcUa::Types::Boolean);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR1ToLoadPressed(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToLoadButtonIsPressed();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR1ToLoadPressed()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToLoadButtonIsPressed();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR1ToLoadEnabled(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToLoadButtonIsEnabled();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR1ToLoadEnabled()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToLoadButtonIsEnabled();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteAutoManualR1ToNewCoordsPressed(bool pressed)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToNewCoordsButtonIsPressed();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(pressed), QOpcUa::Types::Boolean);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR1ToNewCoordsPressed(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToNewCoordsButtonIsPressed();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR1ToNewCoordsPressed()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToNewCoordsButtonIsPressed();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR1ToNewCoordsEnabled(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToNewCoordsButtonIsEnabled();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR1ToNewCoordsEnabled()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToNewCoordsButtonIsEnabled();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteAutoManualR2ToHomePressed(bool pressed)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToHomeButtonIsPressed();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(pressed), QOpcUa::Types::Boolean);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR2ToHomePressed(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToHomeButtonIsPressed();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR2ToHomePressed()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToHomeButtonIsPressed();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR2ToHomeEnabled(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToHomeButtonIsEnabled();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR2ToHomeEnabled()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToHomeButtonIsEnabled();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteAutoManualR2ToPlane1Pressed(bool pressed)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane1ButtonIsPressed();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(pressed), QOpcUa::Types::Boolean);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR2ToPlane1Pressed(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane1ButtonIsPressed();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR2ToPlane1Pressed()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane1ButtonIsPressed();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR2ToPlane1Enabled(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane1ButtonIsEnabled();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR2ToPlane1Enabled()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane1ButtonIsEnabled();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteAutoManualR2ToPlane2Pressed(bool pressed)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane2ButtonIsPressed();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(pressed), QOpcUa::Types::Boolean);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR2ToPlane2Pressed(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane2ButtonIsPressed();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR2ToPlane2Pressed()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane2ButtonIsPressed();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualR2ToPlane2Enabled(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane2ButtonIsEnabled();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualR2ToPlane2Enabled()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane2ButtonIsEnabled();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteAutoManualEmergencyEvacuationPressed(bool pressed)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1R2EmergencyEvacuationButtonIsPressed();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(pressed), QOpcUa::Types::Boolean);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualEmergencyEvacuationPressed(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1R2EmergencyEvacuationButtonIsPressed();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualEmergencyEvacuationPressed()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1R2EmergencyEvacuationButtonIsPressed();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualEmergencyEvacuationEnabled(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1R2EmergencyEvacuationButtonIsEnabled();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualEmergencyEvacuationEnabled()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1R2EmergencyEvacuationButtonIsEnabled();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteAutoManualApplyTabletopOrientationPressed(bool pressed)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualSetTabletopOrientationButtonIsPressed();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(pressed), QOpcUa::Types::Boolean);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualApplyTabletopOrientationPressed(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualSetTabletopOrientationButtonIsPressed();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualApplyTabletopOrientationPressed()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualSetTabletopOrientationButtonIsPressed();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualApplyTabletopOrientationEnabled(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualSetTabletopOrientationButtonIsEnabled();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualApplyTabletopOrientationEnabled()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualSetTabletopOrientationButtonIsEnabled();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteAutoManualApplyNewCoordsPressed(bool pressed)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualApplyNewCoordsButtonIsPressed();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(pressed), QOpcUa::Types::Boolean);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualApplyNewCoordsPressed(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualApplyNewCoordsButtonIsPressed();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualApplyNewCoordsPressed()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualApplyNewCoordsButtonIsPressed();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetAutoManualApplyNewCoordsEnabled(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualApplyNewCoordsButtonIsEnabled();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadAutoManualApplyNewCoordsEnabled()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualApplyNewCoordsButtonIsEnabled();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteResetErrorsPressed(bool pressed)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetResetErrorsButtonIsPressed();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(pressed), QOpcUa::Types::Boolean);
  }
  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetResetErrorsPressed(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetResetErrorsButtonIsPressed();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadResetErrorsPressed()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetResetErrorsButtonIsPressed();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetResetErrorsEnabled(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetResetErrorsButtonIsEnabled();
  return this->Internal->getBool(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadResetErrorsEnabled()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetResetErrorsButtonIsEnabled();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteNewCoordX(int32_t coordX)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordX();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(coordX), QOpcUa::Types::Int32);
  }
  return false;
}

//---------------------------------------------------------------------------
int32_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetNewCoordX(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordX();
  return this->Internal->getInt32(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadNewCoordX()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordX();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteNewCoordY(int32_t coordY)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordY();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(coordY), QOpcUa::Types::Int32);
  }
  return false;
}

//---------------------------------------------------------------------------
int32_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetNewCoordY(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordY();
  return this->Internal->getInt32(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadNewCoordY()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordY();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteNewCoordZ(int32_t coordZ)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordZ();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(coordZ), QOpcUa::Types::Int32);
  }
  return false;
}

//---------------------------------------------------------------------------
int32_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetNewCoordZ(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordZ();
  return this->Internal->getInt32(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadNewCoordZ()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordZ();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteNewCoordAngleA(int32_t angleA)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordA();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(angleA), QOpcUa::Types::Int32);
  }
  return false;
}

//---------------------------------------------------------------------------
int32_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetNewCoordAngleA(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordA();
  return this->Internal->getInt32(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadNewCoordAngleA()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordA();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteNewCoordAngleB(int32_t angleB)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordB();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(angleB), QOpcUa::Types::Int32);
  }
  return false;
}

//---------------------------------------------------------------------------
int32_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetNewCoordAngleB(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordB();
  return this->Internal->getInt32(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadNewCoordAngleB()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordB();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::WriteNewCoordAngleC(int32_t angleC)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordC();
  QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodeName));
  if (node)
  {
    return node->writeValueAttribute(QVariant(angleC), QOpcUa::Types::Int32);
  }
  return false;
}

//---------------------------------------------------------------------------
int32_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetNewCoordAngleC(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordC();
  return this->Internal->getInt32(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadNewCoordAngleC()
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSetCoordC();
  return this->Internal->readNodeValueAttribute(nodeName);
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::UpdateMRMLNodeFromOpcUaNodeValue(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = true;
  if (!mrmlSiemensPlcNode)
  {
    return;
  }

  res &= this->Internal->updateMRMLNodeFromOpcUaModeAndStatus(mrmlSiemensPlcNode, fullNodeName);
  res &= this->Internal->updateMRMLNodeFromOpcUaFlags(mrmlSiemensPlcNode, fullNodeName);
  res &= this->Internal->updateMRMLNodeFromOpcUaSetCoordinates(mrmlSiemensPlcNode, fullNodeName);
  res &= this->Internal->updateMRMLNodeFromOpcUaCurrentCoordinates(mrmlSiemensPlcNode, fullNodeName);
  res &= this->Internal->updateMRMLNodeFromOpcUaReflectionCoordinates(mrmlSiemensPlcNode, fullNodeName);
  res &= this->Internal->updateMRMLNodeFromOpcUaAutoManualButtons(mrmlSiemensPlcNode, fullNodeName);
  res &= this->Internal->updateMRMLNodeFromOpcUaServiceButtons(mrmlSiemensPlcNode, fullNodeName);
  res &= this->Internal->updateMRMLNodeFromOpcUaKukaControllersButtons(mrmlSiemensPlcNode, fullNodeName);
  res &= this->Internal->updateMRMLNodeFromOpcUaOtherButtons(mrmlSiemensPlcNode, fullNodeName);
}

//---------------------------------------------------------------------------
bool vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadNodesValues(const std::list< std::string >& fullNamesList)
{
  bool result = true;
  for (const auto& nodefullName : fullNamesList)
  {
    QOpcUaNode* node = this->Internal->node(QString::fromStdString(nodefullName));
    if (!node)
    {
      result = false;
      continue;
    }
    result &= node->readAttributes(QOpcUa::NodeAttribute::Value);
  }
  return result;
}

//---------------------------------------------------------------------------
std::vector< std::string > vtkSlicerU70RoomSiemensPlcOpcUaLogic::GetRegisteredNodeIds()
{
  std::vector< std::string > ids;
  for (const auto& nodeEntry : this->Internal->Nodes)
  {
    if (nodeEntry.Node)
    {
      QString nodeID = nodeEntry.Node->nodeId();
      ids.push_back(nodeID.toStdString());
    }
  }
  return ids;
}
