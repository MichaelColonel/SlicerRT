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
#include <cassert>

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
   double getDouble(const QString& fullNodeName, bool& res);
   uint64_t getUint64(const QString& fullNodeName, bool& res);
   uint32_t getUint32(const QString& fullNodeName, bool& res);
   int32_t getInt32(const QString& fullNodeName, bool& res);
   uint16_t getUint16(const QString& fullNodeName, bool& res);
   bool getBool(const QString& fullNodeName, bool& res);
   int8_t getInt8(const QString& fullNodeName, bool& res);
   uint8_t getUint8(const QString& fullNodeName, bool& res);
   bool updateMRMLNodeFromOpcUaAutoManualButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   void updateMRMLNodeFromOpcUaServiceButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   void updateMRMLNodeFromOpcUaKukaControllersButtons(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   void updateMRMLNodeFromOpcUaSetCoordinates(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   bool updateMRMLNodeFromOpcUaCurrentCoordinates(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   void updateMRMLNodeFromOpcUaReflectionCoordinates(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);
   void updateMRMLNodeFromOpcUaFlags(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName);

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
    vtkWarningWithObjectMacro(this->External, "node: Scada OPC UA client is invalid");
    return nullptr;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "node: Scada OPC UA client is created, but isn't connected");
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
uint64_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::vtkInternal::getUint64(const QString& fullNodeName, bool& res)
{
  QSharedPointer< QOpcUaClient > sharedClient = this->Client.toStrongRef();
  if (!sharedClient)
  {
    vtkWarningWithObjectMacro(this->External, "getUint64: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getUint64: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< qulonglong >())
  {
    bool resBool;
    qulonglong v = var.toULongLong(&resBool);
    if (resBool)
    {
      res = true;
      return static_cast< uint64_t >(v);
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
    vtkWarningWithObjectMacro(this->External, "getUint32: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getUint32: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< uint >())
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
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is created, but isn't connected");
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
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< bool >())
  {
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
    vtkWarningWithObjectMacro(this->External, "getInt8: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt8: Scada OPC UA client is created, but isn't connected");
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
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is created, but isn't connected");
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
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is invalid");
    res = false;
  }

  if (sharedClient->state() != QOpcUaClient::Connected)
  {
    vtkWarningWithObjectMacro(this->External, "getInt32: Scada OPC UA client is created, but isn't connected");
    res = false;
  }

  const QVariant& var = this->Nodes.value(fullNodeName).Cache;
  if (var.canConvert< int >())
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
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToHomeButtonIsEnabled())
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
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::ServerConnected(const QSharedPointer< QOpcUaClient >& sharedClient, const QHash< QString, QString >& hashNodesFullNamesIds)
{
  this->UnregisterAllOpcUaNodes();

  this->Internal->Client = sharedClient;

  if (!sharedClient)
  {
    return;
  }
  QHashIterator< QString, QString > i(hashNodesFullNamesIds);
  while (i.hasNext())
  {
    i.next();
    QString nodeFullName = i.key();
//    vtkWarningMacro("ServerConnected: key Full Name: " << nodeFullName.toStdString() << ", node value id: " << i.value().toStdString());
    
    vtkInternal::Entry siemensPlcOpcUaNodeEntry;
    siemensPlcOpcUaNodeEntry.Node = sharedClient->node(i.value());
    if (!siemensPlcOpcUaNodeEntry.Node)
    {
      continue;
    }
    
    QOpcUaNode* node = siemensPlcOpcUaNodeEntry.Node.data();

    node->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));

    siemensPlcOpcUaNodeEntry.ConnChanged = QObject::connect(node,
      &QOpcUaNode::dataChangeOccurred, [this, nodeFullName](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr != QOpcUa::NodeAttribute::Value)
      {
        return;
      }
      this->Internal->Nodes[nodeFullName].Cache = value; // map types as needed
    });

    siemensPlcOpcUaNodeEntry.ConnRead = QObject::connect(node,
      &QOpcUaNode::attributeRead, [this, nodeFullName, node](QOpcUa::NodeAttributes attr)
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
        }
      }
    });
    this->Internal->Nodes.insert(nodeFullName, siemensPlcOpcUaNodeEntry);
  }
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
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A1(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A1();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A2(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A2();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A3(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A3();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A4(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A4();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A5(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A5();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR1A6(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A6();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A1(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A1();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A2(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A2();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A3(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A3();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A4(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A4();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A5(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A5();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
double vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadCurrentCoordsR2A6(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A6();
  return this->Internal->getDouble(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
uint8_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadMode(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  return this->Internal->getUint8(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadStatusRtk(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusRtk();
  return this->Internal->getUint16(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadStatusR1(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  return this->Internal->getUint16(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
uint16_t vtkSlicerU70RoomSiemensPlcOpcUaLogic::ReadStatusR2(bool& res)
{
  std::string nodeName = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode();
  return this->Internal->getUint16(QString::fromStdString(nodeName), res);
}

//---------------------------------------------------------------------------
void vtkSlicerU70RoomSiemensPlcOpcUaLogic::UpdateMRMLNodeFromOpcUaNodeValue(vtkMRMLSiemensPlcOpcUaNode* mrmlSiemensPlcNode, const std::string& fullNodeName)
{
  bool res = false;
  if (!mrmlSiemensPlcNode)
  {
    return;
  }

  if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetErrorsFlags())
  {
    uint64_t errMsgFlags = this->Internal->getUint64(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetErrorMessages(errMsgFlags);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceFlags())
  {
    uint64_t servMsgFlags = this->Internal->getUint64(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetServiceMessages(servMsgFlags);
    }
  }
  else if (fullNodeName == vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMessagesFlags())
  {
    uint64_t msgFlags = this->Internal->getUint64(QString::fromStdString(fullNodeName), res);
    if (res)
    {
      mrmlSiemensPlcNode->SetMiscMessages(msgFlags);
    }
  }
  res = this->Internal->updateMRMLNodeFromOpcUaCurrentCoordinates(mrmlSiemensPlcNode, fullNodeName);
}
