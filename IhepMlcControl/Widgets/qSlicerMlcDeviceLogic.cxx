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
#include "qSlicerMlcDeviceLogic.h"

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

// Command and response buffer size
#define BUFFER 11
 
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerIhepMlcDeviceLogicPrivate
{
  Q_DECLARE_PUBLIC(qSlicerIhepMlcDeviceLogic);
protected:
  qSlicerIhepMlcDeviceLogic* const q_ptr;
public:
  qSlicerIhepMlcDeviceLogicPrivate(qSlicerIhepMlcDeviceLogic& object);
  ~qSlicerIhepMlcDeviceLogicPrivate();
  void loadApplicationSettings();

  unsigned short updateCrc16(unsigned short crc, unsigned char a);
  unsigned short commandCalculateCrc16(const unsigned char* buffer, size_t size);
  bool commandCheckCrc16(const unsigned char* buffer, size_t size);

  /// IhepMlcControl MRML node containing shown parameters
  vtkWeakPointer<vtkMRMLIhepMlcControlNode> ParameterNode;

  QByteArray ResponseBuffer;
  QByteArray InputBuffer;
  QByteArray LastCommand;
  std::queue< QByteArray > CommandQueue;
  QSerialPort* SerialPortMlcLayer{ nullptr };
  QTimer* TimerGetState{ nullptr };
  QTimer* TimerWatchdog{ nullptr };
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcDeviceLogicPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcDeviceLogicPrivate::qSlicerIhepMlcDeviceLogicPrivate(qSlicerIhepMlcDeviceLogic& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcDeviceLogicPrivate::~qSlicerIhepMlcDeviceLogicPrivate() = default;

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
void qSlicerIhepMlcDeviceLogic::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
//  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
//    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;
//  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::getCurrentState()
{
}

//-----------------------------------------------------------------------------
std::queue< QByteArray > qSlicerIhepMlcDeviceLogic::getMotorStateQueue(vtkMRMLTableNode* currentPositionNode,
  vtkMRMLTableNode* nextPositionNode, bool useLayer1, bool useLayer2)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  if (!d->ParameterNode)
  {
  }

  std::queue< QByteArray > queue;
  return queue;
}

//-----------------------------------------------------------------------------
std::queue< QByteArray > qSlicerIhepMlcDeviceLogic::getMotorParametersQueue(vtkMRMLTableNode* currentPositionNode,
  vtkMRMLTableNode* nextPositionNode, bool useLayer1, bool useLayer2)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  if (!d->ParameterNode)
  {
  }

  std::queue< QByteArray > queue;
  return queue;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::serialPortError(QSerialPort::SerialPortError error)
{
  Q_D(qSlicerIhepMlcDeviceLogic);

  if (error == QSerialPort::ReadError)
  {
#if (QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ))
    qDebug() << QObject::tr("An I/O error occurred while reading the data from port %1, error: %2").arg(d->SerialPortMlcLayer->portName()).arg(d->SerialPortMlcLayer->errorString()) << Qt::endl;
#elif (QT_VERSION > QT_VERSION_CHECK( 4, 0, 0 ) && QT_VERSION < QT_VERSION_CHECK( 5, 0, 0 ))
    qDebug() << QObject::tr("An I/O error occurred while reading the data from port %1, error: %2").arg(d->SerialPortMlcLayer->portName()).arg(d->SerialPortMlcLayer->errorString()) << endl;
#endif
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::serialPortDataReady()
{
  Q_D(qSlicerIhepMlcDeviceLogic);

  // Stop watchdog timer because of responce
  d->TimerWatchdog->stop();

  QByteArray portData = d->SerialPortMlcLayer->readAll();

  d->ResponseBuffer.push_back(portData);

  if (d->ResponseBuffer.size() >= BUFFER)
  {
    unsigned char* data = reinterpret_cast<unsigned char*>(d->ResponseBuffer.data());
    if (d->commandCheckCrc16(data, BUFFER))
    {
//      this->log( "[Info] ", "CRC16 OK", Qt::green);
//      this->parseResponceBuffer(data);
      if (d->ResponseBuffer.size() > BUFFER)
      {
        d->ResponseBuffer.remove(0, BUFFER);
      }
      else if (d->ResponseBuffer.size() == BUFFER)
      {
        d->ResponseBuffer.clear();
      }

      d->LastCommand.clear(); // erase last command since it no longer needed
      if (!d->CommandQueue.empty()) // write next command from queue
      {
//        QTimer::singleShot(10, this, SLOT(writeNextCommandFromQueue()));
      }
    }
    else
    {
//      this->log( "[Critical] ", "CRC16 INVALID", Qt::red);

      if (d->ResponseBuffer.size() > BUFFER)
      {
        d->ResponseBuffer.remove(0, BUFFER);
      }
      else if (d->ResponseBuffer.size() == BUFFER)
      {
        d->ResponseBuffer.clear();
      }
      // write last command once again, because CRC16 is invalid

///      unsigned char* dataLast = reinterpret_cast<unsigned char*>(d->LastCommand.data());
///      QString commandMsg = QObject::tr("Last command address: %1, command type: %2") \
///        .arg(dataLast ? dataLast[0] : 255) \
///        .arg(dataLast ? dataLast[1] : 255);
//      this->log( "[Critical] ", commandMsg, Qt::red);
//      QTimer::singleShot(1000, this, SLOT(writeLastCommandOnceAgain()));
    }
  }

  // Start get state timer if command queue is empty
  if (d->CommandQueue.empty() && d->LastCommand.isEmpty())
  {
    d->TimerGetState->start();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::serialPortBytesWritten(qint64 written)
{
  Q_D(qSlicerIhepMlcDeviceLogic);

  if (d->LastCommand.isEmpty()) // last command is empty, take next command
  {
    QByteArray command = d->CommandQueue.front();
    d->LastCommand = command;
    d->CommandQueue.pop();
  }

  bool noResponce = true;
  if (d->LastCommand.size() > 0)
  {
    unsigned char* data = reinterpret_cast<unsigned char*>(d->LastCommand.data());
    switch (data[0])
    {
    case 0: // broadcast mode, no responce, empty last command, write next command in the queue
      d->LastCommand.clear();
      if (!d->CommandQueue.empty())
      {
//        QTimer::singleShot(10, this, SLOT(writeNextCommandFromQueue()));
      }
      break;
    default: // single mode, must be responce
      noResponce = false;
      break;
    }
  }

}

//-----------------------------------------------------------------------------
QSerialPort* qSlicerIhepMlcDeviceLogic::connectDevice(const QString& deviceName)
{
  Q_D(qSlicerIhepMlcDeviceLogic);

  d->SerialPortMlcLayer = new QSerialPort(deviceName, this);

  if (d->SerialPortMlcLayer && d->SerialPortMlcLayer->open(QIODevice::ReadWrite))
  {
    d->SerialPortMlcLayer->setBaudRate(QSerialPort::Baud38400);
    d->SerialPortMlcLayer->setDataBits(QSerialPort::Data8);
    d->SerialPortMlcLayer->setParity(QSerialPort::NoParity);
    d->SerialPortMlcLayer->setStopBits(QSerialPort::OneStop);
    d->SerialPortMlcLayer->setFlowControl(QSerialPort::NoFlowControl);

    while (d->CommandQueue.size())
    {
      d->CommandQueue.pop();
    }

    d->ResponseBuffer.clear();
    d->LastCommand.clear();
    d->InputBuffer.clear();

    connect(d->SerialPortMlcLayer, SIGNAL(readyRead()), this, SLOT(serialPortDataReady()));
    connect(d->SerialPortMlcLayer, SIGNAL(bytesWritten(qint64)), this, SLOT(serialPortBytesWritten(qint64)));
    connect(d->SerialPortMlcLayer, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialPortError(QSerialPort::SerialPortError)));
  }
  else if (d->SerialPortMlcLayer && !d->SerialPortMlcLayer->isOpen())
  {
    delete d->SerialPortMlcLayer;
    d->SerialPortMlcLayer = nullptr;

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
  if (port != d->SerialPortMlcLayer)
  {
    return false;
  }

  if (d->SerialPortMlcLayer && d->SerialPortMlcLayer->isOpen())
  {
    d->SerialPortMlcLayer->flush();
    d->SerialPortMlcLayer->close();
    delete d->SerialPortMlcLayer;
    d->SerialPortMlcLayer = nullptr;
  }
  else if (d->SerialPortMlcLayer && !d->SerialPortMlcLayer->isOpen())
  {
    delete d->SerialPortMlcLayer;
    d->SerialPortMlcLayer = nullptr;
  }

  d->ResponseBuffer.clear();
  d->LastCommand.clear();
  d->InputBuffer.clear();
  d->TimerGetState->stop();
  d->TimerWatchdog->stop();
  return true;
}
