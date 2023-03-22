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

// MLC device includes
#include "qSlicerIhepMlcDeviceLogic.h"

// Beams includes
#include "vtkMRMLRTBeamNode.h"
#include "vtkMRMLRTIonBeamNode.h"
#include "vtkMRMLRTIhepIonBeamNode.h"
#include "vtkMRMLRTPlanNode.h"

// SlicerRT IhepMlcControl MRML includes
#include <vtkMRMLIhepMlcControlNode.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLSubjectHierarchyConstants.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLTableNode.h>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// VTK includes
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QTimer>

// STD includes
#include <queue>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerIhepMlcDeviceLogicPrivate
{
  Q_DECLARE_PUBLIC(qSlicerIhepMlcDeviceLogic);
protected:
  qSlicerIhepMlcDeviceLogic* const q_ptr;
public:
  static constexpr size_t BUFFER = 11;
  qSlicerIhepMlcDeviceLogicPrivate(qSlicerIhepMlcDeviceLogic& object);
  ~qSlicerIhepMlcDeviceLogicPrivate();
  void loadApplicationSettings();

  unsigned short updateCrc16(unsigned short crc, unsigned char a);
  unsigned short commandCalculateCrc16(const unsigned char* buffer, size_t size);
  bool commandCheckCrc16(const unsigned char* buffer, size_t size);

  QByteArray ResponseBuffer;
  QByteArray InputBuffer;
  QByteArray LastCommand;
  std::queue< QByteArray > CommandQueue;
  QSerialPort* SerialPortMlcLayer{ nullptr };
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcDeviceLogicPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcDeviceLogicPrivate::qSlicerIhepMlcDeviceLogicPrivate(qSlicerIhepMlcDeviceLogic& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcDeviceLogicPrivate::~qSlicerIhepMlcDeviceLogicPrivate()
{
  if (this->SerialPortMlcLayer && this->SerialPortMlcLayer->isOpen())
  {
    this->SerialPortMlcLayer->flush();
    this->SerialPortMlcLayer->close();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogicPrivate::loadApplicationSettings()
{
  //TODO: Implement if there are application settings (such as default dose engine)
  //      See qSlicerSubjectHierarchyPluginLogicPrivate::loadApplicationSettings
}

//-----------------------------------------------------------------------------
unsigned short qSlicerIhepMlcDeviceLogicPrivate::updateCrc16(unsigned short crc, unsigned char a)
{
  crc ^= a;
  for (int i = 0; i < CHAR_BIT; ++i)
  {
    if (crc & 1)
    {
      crc = (crc >> 1) ^ 0xA001;
    }
    else
    {
      crc = (crc >> 1);
    }
  }

  return crc;
}

//-----------------------------------------------------------------------------
unsigned short qSlicerIhepMlcDeviceLogicPrivate::commandCalculateCrc16(const unsigned char* buffer, size_t size)
{
  unsigned short crc = 0xFFFF;
  for (size_t i = 0; i < size; ++i)
  {
    crc = this->updateCrc16(crc, buffer[i]);
  }

  return crc;
}

//-----------------------------------------------------------------------------
bool qSlicerIhepMlcDeviceLogicPrivate::commandCheckCrc16(const unsigned char* buffer, size_t size)
{
  if (size != BUFFER)
  {
    return false;
  }
  unsigned short dataCRC16 = this->commandCalculateCrc16(buffer, BUFFER - 2); // calculated CRC16 check sum for buffer data
  unsigned short comCRC16 = (buffer[10] << CHAR_BIT) | buffer[9]; // check sum from buffer
  return (dataCRC16 == comCRC16);
}

//-----------------------------------------------------------------------------
// qSlicerIhepMlcDeviceLogic methods

//----------------------------------------------------------------------------
qSlicerIhepMlcDeviceLogic::qSlicerIhepMlcDeviceLogic(QObject* parent)
  : QObject(parent)
{
}

//----------------------------------------------------------------------------
qSlicerIhepMlcDeviceLogic::~qSlicerIhepMlcDeviceLogic() = default;

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::setMRMLScene(vtkMRMLScene* scene)
{
  this->qSlicerObject::setMRMLScene(scene);

  // Connect scene node added event so that the new subject hierarchy nodes can be claimed by a plugin
  qvtkReconnect( scene, vtkMRMLScene::NodeAddedEvent, this, SLOT( onNodeAdded(vtkObject*,vtkObject*) ) );
  // Connect scene import ended event so that subject hierarchy nodes can be created for supported data nodes if missing (backwards compatibility)
  qvtkReconnect( scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportEnded(vtkObject*) ) );
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::onNodeAdded(vtkObject* sceneObject, vtkObject* nodeObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
  {
    return;
  }

//  if (nodeObject->IsA("vtkMRMLRTPlanNode"))
//  {
    // Observe dose engine changed event so that default beam parameters
    // can be added for the newly selected engine in the beams contained by the plan
//    vtkMRMLRTPlanNode* planNode = vtkMRMLRTPlanNode::SafeDownCast(nodeObject);
//    qvtkConnect( planNode, vtkMRMLRTPlanNode::DoseEngineChanged, this, SLOT( applyDoseEngineInPlan(vtkObject*) ) );
//  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::onSceneImportEnded(vtkObject* sceneObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
  {
    return;
  }

  // Traverse all plan nodes in the scene and observe dose engine changed event so that default
  // beam parameters can be added for the newly selected engine in the beams contained by the plan
//  std::vector<vtkMRMLNode*> planNodes;
//  scene->GetNodesByClass("vtkMRMLRTPlanNode", planNodes);
//  for (std::vector<vtkMRMLNode*>::iterator planNodeIt = planNodes.begin(); planNodeIt != planNodes.end(); ++planNodeIt)
//  {
//    vtkMRMLNode* planNode = (*planNodeIt);
//    qvtkConnect( planNode, vtkMRMLRTPlanNode::DoseEngineChanged, this, SLOT( applyDoseEngineInPlan(vtkObject*) ) );
//  }
}

//-----------------------------------------------------------------------------
QSerialPort* qSlicerIhepMlcDeviceLogic::connectDevice(QSerialPort* serialPort)
{
  Q_D(qSlicerIhepMlcDeviceLogic);

//  qWarning() << Q_FUNC_INFO << "Port name: " << deviceName;
//  if (!d->SerialPortMlcLayer)
//  {
//    qWarning() << Q_FUNC_INFO << "MLC port is invalid";
//    return nullptr;
//  }
//  d->SerialPortMlcLayer->setPortName(deviceName);

  qWarning() << Q_FUNC_INFO << "1";

  if (serialPort->open(QIODevice::ReadWrite))
  {
    qWarning() << Q_FUNC_INFO << "3";

    serialPort->setBaudRate(QSerialPort::Baud38400);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    while (d->CommandQueue.size())
    {
      d->CommandQueue.pop();
    }

    d->ResponseBuffer.clear();
    d->LastCommand.clear();
    d->InputBuffer.clear();
    d->SerialPortMlcLayer = serialPort;
  }
  else
  {
    d->ResponseBuffer.clear();
    d->LastCommand.clear();
    d->InputBuffer.clear();
  }
  return d->SerialPortMlcLayer;
}

//-----------------------------------------------------------------------------
bool qSlicerIhepMlcDeviceLogic::disconnectDevice(QSerialPort* port)
{
  Q_D(qSlicerIhepMlcDeviceLogic);

  if (!port)
  {
    qWarning() << Q_FUNC_INFO << "Serial port is invalid";
    return false;
  }

  if (port != d->SerialPortMlcLayer)
  {
    qWarning() << Q_FUNC_INFO << "Internal and external serial ports don't match";
    return false;
  }

  if (d->SerialPortMlcLayer->isOpen())
  {
    d->SerialPortMlcLayer->flush();
    d->SerialPortMlcLayer->close();
  }

  while (d->CommandQueue.size())
  {
    d->CommandQueue.pop();
  }

  d->ResponseBuffer.clear();
  d->LastCommand.clear();
  d->InputBuffer.clear();

  return true;
}
