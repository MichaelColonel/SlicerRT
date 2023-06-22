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

  vtkWeakPointer<vtkMRMLIhepMlcControlNode> ParameterNode;

  QByteArray ResponseBuffer;
  QByteArray InputBuffer;
  QByteArray LastCommand;
  std::queue< QByteArray > CommandQueue;
  QSerialPort* MlcLayerSerialPort{ nullptr };
  QTimer* TimerGetState{ nullptr };
  QTimer* TimerWatchdog{ nullptr };
  vtkMRMLIhepMlcControlNode::LayerType Layer{ vtkMRMLIhepMlcControlNode::Layer_Last };
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcDeviceLogicPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcDeviceLogicPrivate::qSlicerIhepMlcDeviceLogicPrivate(qSlicerIhepMlcDeviceLogic& object)
  : q_ptr(&object)
{
  this->MlcLayerSerialPort = new QSerialPort(&object);
  this->TimerGetState = new QTimer(&object);
  this->TimerWatchdog = new QTimer(&object);
  this->TimerGetState->setInterval(5000); // 5000 ms, 5 seconds
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcDeviceLogicPrivate::~qSlicerIhepMlcDeviceLogicPrivate()
{
  if (this->MlcLayerSerialPort && this->MlcLayerSerialPort->isOpen())
  {
    this->MlcLayerSerialPort->flush();
    this->MlcLayerSerialPort->close();
  }
  delete this->MlcLayerSerialPort;
  this->MlcLayerSerialPort = nullptr;

  this->TimerGetState->stop();
  delete this->TimerGetState;
  this->TimerGetState = nullptr;

  this->TimerWatchdog->stop();
  delete this->TimerWatchdog;
  this->TimerWatchdog = nullptr;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogicPrivate::loadApplicationSettings()
{
  //TODO: Implement if there are application settings (such as default dose engine)
  //      See qSlicerSubjectHierarchyPluginLogicPrivate::loadApplicationSettings
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
void qSlicerIhepMlcDeviceLogic::serialPortBytesWritten(qint64 written)
{
  qDebug() << Q_FUNC_INFO << "Serial port data written: " << written;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::serialPortDataReady()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::serialPortError(QSerialPort::SerialPortError)
{
}

//-----------------------------------------------------------------------------
QSerialPort* qSlicerIhepMlcDeviceLogic::openDevice(const QString& deviceName, vtkMRMLIhepMlcControlNode::LayerType layer)
{
  Q_D(qSlicerIhepMlcDeviceLogic);

  d->MlcLayerSerialPort->setPortName(deviceName);

  if (d->MlcLayerSerialPort->open(QIODevice::ReadWrite))
  {
    d->MlcLayerSerialPort->flush();
    d->MlcLayerSerialPort->setBaudRate(QSerialPort::Baud38400);
    d->MlcLayerSerialPort->setDataBits(QSerialPort::Data8);
    d->MlcLayerSerialPort->setParity(QSerialPort::NoParity);
    d->MlcLayerSerialPort->setStopBits(QSerialPort::OneStop);
    d->MlcLayerSerialPort->setFlowControl(QSerialPort::NoFlowControl);

    while (d->CommandQueue.size())
    {
      d->CommandQueue.pop();
    }

    d->ResponseBuffer.clear();
    d->LastCommand.clear();
    d->InputBuffer.clear();

    QObject::connect(d->MlcLayerSerialPort, SIGNAL(readyRead()), this, SLOT(serialPortDataReady()));
    QObject::connect(d->MlcLayerSerialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(serialPortBytesWritten(qint64)));
    QObject::connect(d->MlcLayerSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialPortError(QSerialPort::SerialPortError)));

    QObject::connect(this, SIGNAL(writeLastCommand()), this, SLOT(writeLastCommandOnceAgain()));
    QObject::connect(this, SIGNAL(writeNextCommand()), this, SLOT(writeNextCommandFromQueue()));

    qDebug() << Q_FUNC_INFO << ": Device port has been opened for device name: " << deviceName;
    return d->MlcLayerSerialPort;
  }
  else
  {
    d->MlcLayerSerialPort->flush();
    QObject::disconnect(d->MlcLayerSerialPort, SIGNAL(readyRead()), this, SLOT(serialPortDataReady()));
    QObject::disconnect(d->MlcLayerSerialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(serialPortBytesWritten(qint64)));
    QObject::disconnect(d->MlcLayerSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialPortError(QSerialPort::SerialPortError)));

    QObject::disconnect(this, SIGNAL(writeLastCommand()), this, SLOT(writeLastCommandOnceAgain()));
    QObject::disconnect(this, SIGNAL(writeNextCommand()), this, SLOT(writeNextCommandFromQueue()));

    while (d->CommandQueue.size())
    {
      d->CommandQueue.pop();
    }
    d->ResponseBuffer.clear();
    d->LastCommand.clear();
    d->InputBuffer.clear();
    qWarning() << Q_FUNC_INFO << ": Unable to open device port for device name: " << deviceName;
    return nullptr;
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::writeNextCommandFromQueue()
{
  Q_D(qSlicerIhepMlcDeviceLogic);
//  if (!d->LastCommand.isEmpty() && d->CommandQueue.empty())
  if (d->CommandQueue.empty())
  {
//    qWarning() << Q_FUNC_INFO << "Last command is not empty, command queue is empty: "
    qWarning() << Q_FUNC_INFO << "Command queue is empty. Last command state: " << (d->LastCommand.isEmpty() ? "is empty" : "not empty");
    if (d->LastCommand.isEmpty())
    {
///      d->TimerGetState->start();
    }
    return;
  }

  d->LastCommand = d->CommandQueue.front();

  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  constexpr int commandSize = buf.size();

  if (d->MlcLayerSerialPort && d->MlcLayerSerialPort->isOpen())
  {
///    qDebug() << Q_FUNC_INFO << "Write data: " << d->LastCommand << ", size: " << d->LastCommand.size();
//    unsigned char* data = reinterpret_cast< unsigned char* >(d->ResponseBuffer.data());
    std::copy_n(d->LastCommand.data(), commandSize, std::begin(buf));
///    qDebug() << Q_FUNC_INFO << ": Buffer: " << d->LastCommand << ", data: " << int(buf[0])
///      << " " << int(buf[1]) << " " << int(buf[2]) << " " << int(buf[3])
///      << " " << int(buf[4]) << " " << int(buf[5]) << " " << int(buf[6])
///      << " " << int(buf[7]) << " " << int(buf[8]) << " " << int(buf[9])
///      << " " << int(buf[10]);
    d->MlcLayerSerialPort->write(d->LastCommand);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::writeLastCommandOnceAgain()
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  if (d->LastCommand.isEmpty())
  {
    qWarning() << Q_FUNC_INFO << "Last command is empty";
    return;
  }

  if (d->MlcLayerSerialPort && d->MlcLayerSerialPort->isOpen())
  {
    qWarning() << Q_FUNC_INFO << "Write last command last again: " << d->LastCommand;
    d->MlcLayerSerialPort->write(d->LastCommand);
  }
}
