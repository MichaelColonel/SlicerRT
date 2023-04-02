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

// Qt includes
#include <QDebug>
#include <QTimer>
#include <QRadioButton>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

#include "qSlicerIhepMlcControlModuleWidget.h"
#include "ui_qSlicerIhepMlcControlModuleWidget.h"

#include "qSlicerIhepMlcControlLayoutWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLTableNode.h>

// SlicerRT MRML Beams includes
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>

// SlicerRT MRML IhepMlcControl includes
#include "vtkMRMLIhepMlcControlNode.h"

// Logic includes
#include "vtkSlicerIhepMlcControlLogic.h"

// STD includes
#include <cstring>
#include <queue>
#include <bitset>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerIhepMlcControlModuleWidgetPrivate: public Ui_qSlicerIhepMlcControlModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerIhepMlcControlModuleWidget);
protected:
  qSlicerIhepMlcControlModuleWidget* const q_ptr;
public:
  qSlicerIhepMlcControlModuleWidgetPrivate(qSlicerIhepMlcControlModuleWidget &object);
  virtual ~qSlicerIhepMlcControlModuleWidgetPrivate();
  vtkSlicerIhepMlcControlLogic* logic() const;

  bool connectDevice(const QString& deviceName);
  bool disconnectDevice();

  bool processResponseBufferToLeafData(vtkMRMLIhepMlcControlNode::LeafData& leafData);

  void writeNextCommandFromQueue();
  void writeLastCommandOnceAgain();
  QByteArray getParametersCommandByAddress(int address);
  QByteArray getStateCommandByAddress(int address);
  QByteArray getStartCommandByAddress(int address);
  QByteArray getStopCommandByAddress(int address);
  QByteArray getStartBroadcastCommand();
  QByteArray getStopBroadcastCommand();
  QByteArray getOpenBroadcastCommand();
  QByteArray getParametersCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData);
  QByteArray getStateCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData);
  QByteArray getStartCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData);
  QByteArray getStopCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData);
  QList< QByteArray > getParametersCommandsByLayer(vtkMRMLIhepMlcControlNode::LayerType);
  QList< QByteArray > getStateCommandsByLayer(vtkMRMLIhepMlcControlNode::LayerType);
  QList< QByteArray > getParametersCommandsByLayerSide(vtkMRMLIhepMlcControlNode::LayerType,vtkMRMLIhepMlcControlNode::SideType);
  QList< QByteArray > getStateCommandsByLayerSide(vtkMRMLIhepMlcControlNode::LayerType,vtkMRMLIhepMlcControlNode::SideType);
  QList< QByteArray > getParametersCommands();
  QList< QByteArray > getStateCommands();

  qSlicerIhepMlcControlLayoutWidget* MlcControlWidget{ nullptr };
  int PreviousLayoutId{ 0 };
  int MlcCustomLayoutId{ 507 };
  vtkWeakPointer<vtkMRMLIhepMlcControlNode> ParameterNode;

  QSerialPort* MlcLayer1SerialPort{ nullptr };
  QSerialPort* MlcLayer2SerialPort{ nullptr };

  QByteArray ResponseBuffer;
  QByteArray InputBuffer;
  QByteArray LastCommand;
  std::queue< QByteArray > CommandQueue;
  QTimer* TimerGetState{ nullptr };
  QTimer* TimerWatchdog{ nullptr };
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::qSlicerIhepMlcControlModuleWidgetPrivate(qSlicerIhepMlcControlModuleWidget &object)
  :
  q_ptr(&object)
{
  this->MlcLayer1SerialPort = new QSerialPort(&object);
  this->MlcLayer2SerialPort = new QSerialPort(&object);
  this->TimerGetState = new QTimer(&object);
  this->TimerWatchdog = new QTimer(&object);
  this->TimerGetState->setInterval(5000); // 5000 ms, 5 seconds
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::~qSlicerIhepMlcControlModuleWidgetPrivate()
{
  if (this->MlcLayer1SerialPort && this->MlcLayer1SerialPort->isOpen())
  {
    this->MlcLayer1SerialPort->flush();
    this->MlcLayer1SerialPort->close();
  }
  delete this->MlcLayer1SerialPort;
  this->MlcLayer1SerialPort = nullptr;

  if (this->MlcLayer2SerialPort && this->MlcLayer2SerialPort->isOpen())
  {
    this->MlcLayer2SerialPort->flush();
    this->MlcLayer2SerialPort->close();
  }
  delete this->MlcLayer2SerialPort;
  this->MlcLayer2SerialPort = nullptr;

  this->TimerGetState->stop();
  delete this->TimerGetState;
  this->TimerGetState = nullptr;
  this->TimerWatchdog->stop();
  delete this->TimerWatchdog;
  this->TimerWatchdog = nullptr;
}

//-----------------------------------------------------------------------------
vtkSlicerIhepMlcControlLogic* qSlicerIhepMlcControlModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerIhepMlcControlModuleWidget);
  return vtkSlicerIhepMlcControlLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getParametersCommandByAddress(int address)
{
  if (!this->ParameterNode || address == 0)
  {
    return QByteArray();
  }
  if (this->ParameterNode->GetNumberOfLeafPairs() * 2 >= address)
  {
    return QByteArray();
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  if (this->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    buf[0] = static_cast<unsigned char>(address); // address
    buf[1] = 0; // set leaf parameters
    std::bitset< CHAR_BIT > v(leafData.Frequency); // frequency code
    v.set(4, leafData.Mode); // step mode, halfStep == true, fullStep == false
    v.set(5, leafData.Direction); // clockwise direction == true, counter clockwise == false
    v.set(6, leafData.Reset); // reset state
    v.set(7, leafData.Enabled); // enable state
    buf[2] = static_cast<unsigned char>(v.to_ulong());
    buf[3] = 0; // reserved
    buf[5] = leafData.Steps >> CHAR_BIT;
    buf[4] = leafData.Steps & 0xFF;
    unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
    buf[9] = chcksumm & 0xFF;
    buf[10] = chcksumm >> CHAR_BIT;

    return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
  }
  return QByteArray();
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getStateCommandByAddress(int address)
{
  if (!this->ParameterNode || address == 0)
  {
    return QByteArray();
  }
  if (address >= this->ParameterNode->GetNumberOfLeafPairs() * 2)
  {
    return QByteArray();
  }
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;

  buf[0] = static_cast<unsigned char>(address); // address
  buf[1] = 1; // get leaf state
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;

  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getStartCommandByAddress(int address)
{
  if (!this->ParameterNode || address == 0)
  {
    return QByteArray();
  }
  if (this->ParameterNode->GetNumberOfLeafPairs() * 2 >= address)
  {
    return QByteArray();
  }
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;

  buf[0] = static_cast<unsigned char>(address); // address
  buf[1] = 2; // start leaf command
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;

  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getStopCommandByAddress(int address)
{
  if (!this->ParameterNode || address == 0)
  {
    return QByteArray();
  }
  if (this->ParameterNode->GetNumberOfLeafPairs() * 2 >= address)
  {
    return QByteArray();
  }

  vtkMRMLIhepMlcControlNode::CommandBufferType buf;

  buf[0] = static_cast<unsigned char>(address); // address
  buf[1] = 3; // stop leaf command
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;

  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getStartBroadcastCommand()
{
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;

  buf[0] = 0; // broadcast
  buf[1] = 9; // start command
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;

  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getStopBroadcastCommand()
{
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;

  buf[0] = 0; // broadcast
  buf[1] = 6; // stop command
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;

  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getOpenBroadcastCommand()
{
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;

  buf[0] = 0; // broadcast
  buf[1] = 5; // open command
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;

  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcControlModuleWidgetPrivate::getParametersCommandsByLayer(vtkMRMLIhepMlcControlNode::LayerType layer)
{
  Q_Q(const qSlicerIhepMlcControlModuleWidget);
  QList< QByteArray > list;
  if (!this->ParameterNode || layer == vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    return list;
  }
  std::vector<int> addresses;
  this->ParameterNode->GetAddressesByLayer(addresses, layer);

  for (int address : addresses)
  {
    QByteArray command = this->getParametersCommandByAddress(address);
    if (command.size())
    {
      list.push_back(command);
    }
  }
  return list;
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcControlModuleWidgetPrivate::getStateCommandsByLayer(vtkMRMLIhepMlcControlNode::LayerType layer)
{
  Q_Q(const qSlicerIhepMlcControlModuleWidget);
  QList< QByteArray > list;
  if (!this->ParameterNode || layer == vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    return list;
  }
  std::vector<int> addresses;
  this->ParameterNode->GetAddressesByLayer(addresses, layer);
  qDebug() << Q_FUNC_INFO << ": Size of list for state command for addresses: " << addresses.size();
  for (int address : addresses)
  {
    qDebug() << Q_FUNC_INFO << ": State command for address: " << address;
    QByteArray command = this->getStateCommandByAddress(address);
    if (command.size())
    {
      list.push_back(command);
    }
  }
  return list;
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcControlModuleWidgetPrivate::getParametersCommands()
{
  Q_Q(const qSlicerIhepMlcControlModuleWidget);
  QList< QByteArray > list;
  if (!this->ParameterNode)
  {
    return list;
  }

  for (const auto& pairOfLeavesData : this->ParameterNode->GetPairOfLeavesMap())
  {
    const vtkMRMLIhepMlcControlNode::PairOfLeavesData& pairOfLeaves = pairOfLeavesData.second;
    const vtkMRMLIhepMlcControlNode::LeafData& side1 = pairOfLeaves.first;
    const vtkMRMLIhepMlcControlNode::LeafData& side2 = pairOfLeaves.second;
    QByteArray arr1 = this->getParametersCommandFromLeafData(side1);
    QByteArray arr2 = this->getParametersCommandFromLeafData(side2);
    if (arr1.size() > 0)
    {
      list.push_back(arr1);
    }
    if (arr2.size() > 0)
    {
      list.push_back(arr2);
    }
  }
  return list;
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcControlModuleWidgetPrivate::getStateCommands()
{
  Q_Q(const qSlicerIhepMlcControlModuleWidget);
  QList< QByteArray > list;
  if (!this->ParameterNode)
  {
    return list;
  }

  for (const auto& pairOfLeavesData : this->ParameterNode->GetPairOfLeavesMap())
  {
    const vtkMRMLIhepMlcControlNode::PairOfLeavesData& pairOfLeaves = pairOfLeavesData.second;
    const vtkMRMLIhepMlcControlNode::LeafData& side1 = pairOfLeaves.first;
    const vtkMRMLIhepMlcControlNode::LeafData& side2 = pairOfLeaves.second;
    QByteArray arr1 = this->getStateCommandFromLeafData(side1);
    QByteArray arr2 = this->getStateCommandFromLeafData(side2);
    if (arr1.size() > 0)
    {
      list.push_back(arr1);
    }
    if (arr2.size() > 0)
    {
      list.push_back(arr2);
    }
  }
  return list;
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getParametersCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData)
{
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  buf[0] = static_cast<unsigned char>(leafData.Address); // address
  buf[1] = 0; // set leaf parameters
  std::bitset< CHAR_BIT > v(leafData.Frequency); // frequency code
  v.set(4, leafData.Mode); // step mode, halfStep == true, fullStep == false
  v.set(5, leafData.Direction); // clockwise direction == true, counter clockwise == false
  v.set(6, leafData.Reset); // reset state
  v.set(7, leafData.Enabled); // enable state
  buf[2] = static_cast<unsigned char>(v.to_ulong());
  buf[3] = 0; // reserved
  buf[5] = leafData.Steps >> CHAR_BIT;
  buf[4] = leafData.Steps & 0xFF;
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;
  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getStateCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData)
{
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  buf[0] = static_cast<unsigned char>(leafData.Address); // address
  buf[1] = 1; // get leaf state
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;
  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getStartCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData)
{
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  buf[0] = static_cast<unsigned char>(leafData.Address); // address
  buf[1] = 2; // start leaf
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;
  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcControlModuleWidgetPrivate::getStopCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData)
{
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  buf[0] = static_cast<unsigned char>(leafData.Address); // address
  buf[1] = 3; // stop leaf
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;
  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidgetPrivate::writeNextCommandFromQueue()
{
  if (!this->LastCommand.isEmpty() && this->CommandQueue.empty())
  {
    qWarning() << Q_FUNC_INFO << "Last command is not empty, command queue is empty: " << this->LastCommand.isEmpty() << ' ' << this->CommandQueue.empty();
    return;
  }

  QByteArray command = this->CommandQueue.front();

  if (this->MlcLayer1SerialPort && this->MlcLayer1SerialPort->isOpen())
  {
    qDebug() << Q_FUNC_INFO << "Write data: " << command << ", size: " << command.size();
    this->MlcLayer1SerialPort->write(command);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidgetPrivate::writeLastCommandOnceAgain()
{
  if (this->LastCommand.isEmpty())
  {
    qWarning() << Q_FUNC_INFO << "Last command is empty";
    return;
  }

  if (this->MlcLayer1SerialPort && this->MlcLayer1SerialPort->isOpen())
  {
    this->MlcLayer1SerialPort->write(this->LastCommand);
  }
}

//std::queue< QByteArray > getParametersCommandsForLayer(vtkMRMLIhepMlcControlNode::LayerType);
//std::queue< QByteArray > getParametersCommands();

//-----------------------------------------------------------------------------
bool qSlicerIhepMlcControlModuleWidgetPrivate::connectDevice(const QString& deviceName)
{
  Q_Q(qSlicerIhepMlcControlModuleWidget);

  this->MlcLayer1SerialPort->setPortName(deviceName);

  if (this->MlcLayer1SerialPort->open(QIODevice::ReadWrite))
  {
    this->MlcLayer1SerialPort->setBaudRate(QSerialPort::Baud38400);
    this->MlcLayer1SerialPort->setDataBits(QSerialPort::Data8);
    this->MlcLayer1SerialPort->setParity(QSerialPort::NoParity);
    this->MlcLayer1SerialPort->setStopBits(QSerialPort::OneStop);
    this->MlcLayer1SerialPort->setFlowControl(QSerialPort::NoFlowControl);

    while (this->CommandQueue.size())
    {
      this->CommandQueue.pop();
    }

    this->ResponseBuffer.clear();
    this->LastCommand.clear();
    this->InputBuffer.clear();

    QObject::connect(this->MlcLayer1SerialPort, SIGNAL(readyRead()), q, SLOT(serialPortDataReady()));
    QObject::connect(this->MlcLayer1SerialPort, SIGNAL(bytesWritten(qint64)), q, SLOT(serialPortBytesWritten(qint64)));
    QObject::connect(this->MlcLayer1SerialPort, SIGNAL(error(QSerialPort::SerialPortError)), q, SLOT(serialPortError(QSerialPort::SerialPortError)));
    QObject::connect(this->TimerGetState, SIGNAL(timeout()), q, SLOT(onMlcStateTimeoutExpired()));

//    this->TimerGetState->start();
    qDebug() << Q_FUNC_INFO << ": Device port has been opened for device name: " << deviceName;
    return true;
  }
  else
  {
    QObject::disconnect(this->MlcLayer1SerialPort, SIGNAL(readyRead()), q, SLOT(serialPortDataReady()));
    QObject::disconnect(this->MlcLayer1SerialPort, SIGNAL(bytesWritten(qint64)), q, SLOT(serialPortBytesWritten(qint64)));
    QObject::disconnect(this->MlcLayer1SerialPort, SIGNAL(error(QSerialPort::SerialPortError)), q, SLOT(serialPortError(QSerialPort::SerialPortError)));
    QObject::disconnect(this->TimerGetState, SIGNAL(timeout()), q, SLOT(onMlcStateTimeoutExpired()));

    while (this->CommandQueue.size())
    {
      this->CommandQueue.pop();
    }
    this->ResponseBuffer.clear();
    this->LastCommand.clear();
    this->InputBuffer.clear();
    qWarning() << Q_FUNC_INFO << ": Unable to open device port for device name: " << deviceName;
    return false;
  }
  return false;
}

//-----------------------------------------------------------------------------
bool qSlicerIhepMlcControlModuleWidgetPrivate::disconnectDevice()
{
  Q_Q(qSlicerIhepMlcControlModuleWidget);

  this->TimerGetState->stop();

  if (this->MlcLayer1SerialPort && this->MlcLayer1SerialPort->isOpen())
  {
    this->MlcLayer1SerialPort->flush();
    this->MlcLayer1SerialPort->close();
  }

  while (this->CommandQueue.size())
  {
    this->CommandQueue.pop();
  }
  this->ResponseBuffer.clear();
  this->LastCommand.clear();
  this->InputBuffer.clear();
  return true;
}

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidget::qSlicerIhepMlcControlModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerIhepMlcControlModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidget::~qSlicerIhepMlcControlModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setup()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  d->MlcControlWidget = new qSlicerIhepMlcControlLayoutWidget;

  qSlicerSingletonViewFactory* viewFactory = new qSlicerSingletonViewFactory();
  viewFactory->setWidget(d->MlcControlWidget);
  viewFactory->setTagName("MlcControlLayout");

  const char* layoutString = \
    "<layout type=\"vertical\">" \
    " <item>" \
    "  <MlcControlLayout></MlcControlLayout>" \
    " </item>" \
    "</layout>";

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->registerViewFactory(viewFactory);
  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
  {
    layoutNode->AddLayoutDescription( d->MlcCustomLayoutId, layoutString);
  }

  // MRMLNodeComboBoxes
  QObject::connect( d->MRMLNodeComboBox_Beam, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onBeamNodeChanged(vtkMRMLNode*)));
  QObject::connect( d->MRMLNodeComboBox_MlcTable, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onMlcTableNodeChanged(vtkMRMLNode*)));

  // Buttons
  QObject::connect( d->PushButton_SetMlcTable, SIGNAL(clicked()),
    this, SLOT(onSetMlcTableClicked()));
  QObject::connect( d->PushButton_SwitchLayout, SIGNAL(toggled(bool)),
    this, SLOT(onSwitchToMlcControlLayoutToggled(bool)));
  QObject::connect( d->CheckBox_ParallelBeam, SIGNAL(toggled(bool)),
    this, SLOT(onParallelBeamToggled(bool)));
  QObject::connect( d->PushButton_GenerateMlcBoundary, SIGNAL(clicked()),
    this, SLOT(onGenerateMlcBoundaryClicked()));
  QObject::connect( d->PushButton_UpdateMlcBoundary, SIGNAL(clicked()),
    this, SLOT(onUpdateMlcBoundaryClicked()));
  QObject::connect( d->PushButton_ConnectMlcDevice, SIGNAL(clicked()),
    this, SLOT(onConnectMlcLayersButtonClicked()));
  QObject::connect( d->PushButton_SetLeafParameters, SIGNAL(clicked()),
    this, SLOT(onLeafSetParametersClicked()));
  QObject::connect( d->PushButton_GetLeafState, SIGNAL(clicked()),
    this, SLOT(onLeafGetStateClicked()));
  QObject::connect( d->PushButton_StartLeaf, SIGNAL(clicked()),
    this, SLOT(onLeafStartClicked()));
  QObject::connect( d->PushButton_StopLeaf, SIGNAL(clicked()),
    this, SLOT(onLeafStopClicked()));
  QObject::connect( d->PushButton_StartLeaves, SIGNAL(clicked()),
    this, SLOT(onLeavesStartBroadcastClicked()));
  QObject::connect( d->PushButton_StopLeaves, SIGNAL(clicked()),
    this, SLOT(onLeavesStopBroadcastClicked()));
  QObject::connect( d->PushButton_OpenLeaves, SIGNAL(clicked()),
    this, SLOT(onLeavesOpenBroadcastClicked()));
  QObject::connect( d->PushButton_SetLeavesParameters, SIGNAL(clicked()),
    this, SLOT(onLeavesSetParametersClicked()));
  QObject::connect( d->PushButton_GetLeavesState, SIGNAL(clicked()),
    this, SLOT(onLeavesGetStateClicked()));

  // Sliders
  QObject::connect( d->SliderWidget_NumberOfLeavesPairs, SIGNAL(valueChanged(double)),
    this, SLOT(onNumberOfLeafPairsChanged(double)));
  QObject::connect( d->SliderWidget_PairOfLeavesBoundarySize, SIGNAL(valueChanged(double)),
    this, SLOT(onPairOfLeavesSizeChanged(double)));
  QObject::connect( d->SliderWidget_IsocenterOffset, SIGNAL(valueChanged(double)),
    this, SLOT(onIsocenterOffsetChanged(double)));
  QObject::connect( d->SliderWidget_DistanceBetweenLayers, SIGNAL(valueChanged(double)),
    this, SLOT(onDistanceBetweenTwoLayersChanged(double)));
  QObject::connect( d->SliderWidget_LayersOffset, SIGNAL(valueChanged(double)),
    this, SLOT(onOffsetBetweenTwoLayersChanged(double)));

  QObject::connect( d->HorizontalSlider_LeafAddress, SIGNAL(valueChanged(int)),
    this, SLOT(onLeafAddressChanged(int)));
  QObject::connect( d->HorizontalSlider_LeafSteps, SIGNAL(valueChanged(int)),
    this, SLOT(onLeafStepsChanged(int)));

  // GroupBox
  QObject::connect( d->ButtonGroup_MlcLayers, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onMlcLayersButtonClicked(QAbstractButton*)));
  QObject::connect( d->ButtonGroup_MlcOrientation, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onMlcOrientationButtonClicked(QAbstractButton*)));
  QObject::connect( d->ButtonGroup_LeafDirection, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onLeafDirectionButtonClicked(QAbstractButton*)));
  QObject::connect( d->ButtonGroup_LeafStepMode, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onLeafStepModeButtonClicked(QAbstractButton*)));

  // CheckBox
  QObject::connect( d->CheckBox_MotorReset, SIGNAL(toggled(bool)),
    this, SLOT(onLeafResetToggled(bool)));
  QObject::connect( d->CheckBox_MotorEnable, SIGNAL(toggled(bool)),
    this, SLOT(onLeafEnabledToggled(bool)));

  // Leaf data signal
  QObject::connect( this, SIGNAL(leafDataChanged(const vtkMRMLIhepMlcControlNode::LeafData&)),
    d->MlcControlWidget, SLOT(onLeafDataChanged(const vtkMRMLIhepMlcControlNode::LeafData&)));

  QObject::connect( d->MlcControlWidget, SIGNAL(leafAddressStepsMovementChanged(int, int)),
    this, SLOT(onLeafAddressStepsMovementChanged(int,int)));

  // Select predefined shape as square
  QTimer::singleShot(0, d->MlcControlWidget, [=](){ d->MlcControlWidget->onMlcPredefinedIndexChanged(3); } );
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::exit()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->Superclass::exit();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(d->PreviousLayoutId);
}


//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::enter()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->Superclass::enter();

  this->onEnter();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  d->PreviousLayoutId = layoutManager->layout();
  layoutManager->setLayout(d->MlcCustomLayoutId);
  QTimer::singleShot(100, this, SLOT(onSetMlcControlLayout()));
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSwitchToMlcControlLayoutToggled(bool toggled)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(toggled ? d->MlcCustomLayoutId : d->PreviousLayoutId);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSetMlcControlLayout()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(d->MlcCustomLayoutId);
  QSignalBlocker blocker1(d->PushButton_SwitchLayout);
  d->PushButton_SwitchLayout->setChecked(true);
}


//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->Superclass::setMRMLScene(scene);

  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onSceneClosedEvent()));

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    if (d->MRMLNodeComboBox_ParameterSet->currentNode())
    {
      this->setParameterNode(d->MRMLNodeComboBox_ParameterSet->currentNode());
    }
    else if (vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLIhepMlcControlNode"))
    {
      this->setParameterNode(node);
    }
    else
    {
      vtkNew<vtkMRMLIhepMlcControlNode> newNode;
      std::string nodeName = this->mrmlScene()->GenerateUniqueName("IHEP_MLC_VRBS");
      newNode->SetName(nodeName.c_str());
      newNode->SetSingletonTag("IHEP_MLC");
      this->mrmlScene()->AddNode(newNode);
      this->setParameterNode(newNode);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setParameterNode(vtkMRMLNode *node)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->MRMLNodeComboBox_ParameterSet->setCurrentNode(node);

  // Set parameter node to children widgets (MlcControlWidget)
  d->MlcControlWidget->setParameterNode(node);

  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( parameterNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->ParameterNode = parameterNode;

  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  if (parameterNode)
  {
    if (!parameterNode->GetBeamNode())
    {
      vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_Beam->currentNode());
//      qvtkConnect( beamNode, vtkMRMLRTBeamNode::BeamTransformModified, this, SLOT(updateNormalAndVupVectors())); // update beam transform and geo
      parameterNode->SetAndObserveBeamNode(beamNode);
    }
  }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onBeamNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  if (beamNode)
  {
    d->ParameterNode->SetAndObserveBeamNode(beamNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcTableNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLTableNode* tableNode = vtkMRMLTableNode::SafeDownCast(node);
  d->ParameterNode->GetBeamNode()->SetAndObserveMultiLeafCollimatorTableNode(tableNode);
  d->PushButton_GenerateMlcBoundary->setEnabled(true);
  d->PushButton_UpdateMlcBoundary->setEnabled(tableNode);
  d->PushButton_SetMlcTable->setEnabled(tableNode);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  // Enable widgets
//  d->CheckBox_ShowDrrMarkups->setEnabled(parameterNode);
//  d->CollapsibleButton_ReferenceInput->setEnabled(parameterNode);
//  d->CollapsibleButton_GeometryBasicParameters->setEnabled(parameterNode);
//  d->PlastimatchParametersWidget->setEnabled(parameterNode);
//  d->PushButton_ComputeDrr->setEnabled(parameterNode);

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (!parameterNode->GetBeamNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid referenced parameter's beam node";
    d->PushButton_GenerateMlcBoundary->setEnabled(false);
    return;
  }

  // Update widgets from parameter node
  d->CheckBox_ParallelBeam->setChecked(parameterNode->GetParallelBeam());
  d->MRMLNodeComboBox_Beam->setCurrentNode(parameterNode->GetBeamNode());

  d->PushButton_GenerateMlcBoundary->setEnabled(true);

  vtkMRMLTableNode* mlcTableNode = parameterNode->GetBeamNode()->GetMultiLeafCollimatorTableNode();
  d->MRMLNodeComboBox_MlcTable->setCurrentNode(mlcTableNode);
  d->MlcControlWidget->setMlcTableNode(mlcTableNode);
  d->PushButton_UpdateMlcBoundary->setEnabled(mlcTableNode);
  d->PushButton_SetMlcTable->setEnabled(mlcTableNode);

  switch (parameterNode->GetOrientation())
  {
  case vtkMRMLIhepMlcControlNode::X:
    d->RadioButton_MLCX->setChecked(true);
    d->RadioButton_MLCY->setChecked(false);
    break;
  case vtkMRMLIhepMlcControlNode::Y:
    d->RadioButton_MLCX->setChecked(false);
    d->RadioButton_MLCY->setChecked(true);
    break;
  default:
    break;
  }
  switch (parameterNode->GetLayers())
  {
  case vtkMRMLIhepMlcControlNode::OneLayer:
    d->RadioButton_OneLayer->setChecked(true);
    d->RadioButton_TwoLayers->setChecked(false);
    break;
  case vtkMRMLIhepMlcControlNode::TwoLayers:
    d->RadioButton_OneLayer->setChecked(false);
    d->RadioButton_TwoLayers->setChecked(true);
    break;
  default:
    break;
  }
  d->SliderWidget_NumberOfLeavesPairs->setValue(parameterNode->GetNumberOfLeafPairs());
  d->SliderWidget_PairOfLeavesBoundarySize->setValue(parameterNode->GetPairOfLeavesSize());
  d->SliderWidget_IsocenterOffset->setValue(parameterNode->GetIsocenterOffset());
  d->SliderWidget_DistanceBetweenLayers->setValue(parameterNode->GetDistanceBetweenTwoLayers());
  d->SliderWidget_DistanceBetweenLayers->setValue(parameterNode->GetDistanceBetweenTwoLayers());
  d->SliderWidget_LayersOffset->setValue(parameterNode->GetOffsetBetweenTwoLayers());
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSceneClosedEvent()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onEnter()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid scene";
    return;
  }

  // First check the logic if it has a parameter node
  if (!d->logic())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid logic";
    return;
  }

  vtkMRMLIhepMlcControlNode* parameterNode = nullptr; 
  // Try to find one in the scene
  if (vtkMRMLNode* node = this->mrmlScene()->GetFirstNodeByClass("vtkMRMLIhepMlcControlNode"))
  {
    parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);
  }

  if (parameterNode && parameterNode->GetBeamNode())
  {
    // First thing first: update normal and vup vectors for parameter node
    // in case observed beam node transformation has been modified
///    d->logic()->UpdateNormalAndVupVectors(parameterNode);
  }

  // Create DRR markups nodes
///  d->logic()->CreateMarkupsNodes(parameterNode);

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onParameterNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  vtkMRMLIhepMlcControlNode* parameterNode = vtkMRMLIhepMlcControlNode::SafeDownCast(node);

  if (!parameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  this->setParameterNode(parameterNode);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onParallelBeamToggled(bool toggled)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetParallelBeam(toggled);
  qDebug() << Q_FUNC_INFO << ": MLC parallel beam " << toggled;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcLayersButtonClicked(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
  if (rButton && rButton == d->RadioButton_OneLayer)
  {
    d->ParameterNode->SetLayers(vtkMRMLIhepMlcControlNode::OneLayer);
  }
  else if (rButton && rButton == d->RadioButton_TwoLayers)
  {
    d->ParameterNode->SetLayers(vtkMRMLIhepMlcControlNode::TwoLayers);
  }
  else
  {
  }
  qDebug() << Q_FUNC_INFO << ": MLC layers " << d->ParameterNode->GetLayers();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcOrientationButtonClicked(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
  if (rButton && rButton == d->RadioButton_MLCX)
  {
    d->ParameterNode->SetOrientation(vtkMRMLIhepMlcControlNode::X);
  }
  else if (rButton && rButton == d->RadioButton_MLCY)
  {
    d->ParameterNode->SetOrientation(vtkMRMLIhepMlcControlNode::Y);
  }
  else
  {
  }
  qDebug() << Q_FUNC_INFO << ": MLC orientation " << d->ParameterNode->GetOrientation();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onNumberOfLeafPairsChanged(double numberOfLeaves)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetNumberOfLeafPairs(static_cast<int>(numberOfLeaves));
  qDebug() << Q_FUNC_INFO << ": number of leaves " << numberOfLeaves;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onPairOfLeavesSizeChanged(double size)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetPairOfLeavesSize(size);
  qDebug() << Q_FUNC_INFO << ": pair of leaves size " << size;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onIsocenterOffsetChanged(double offset)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetIsocenterOffset(offset);
  qDebug() << Q_FUNC_INFO << ": isocenter offset distance " << offset;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onDistanceBetweenTwoLayersChanged(double distance)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetDistanceBetweenTwoLayers(distance);
  qDebug() << Q_FUNC_INFO << ": distance between layers " << distance;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onOffsetBetweenTwoLayersChanged(double distance)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetOffsetBetweenTwoLayers(distance);
  qDebug() << Q_FUNC_INFO << ": layer offset distance " << distance;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onGenerateMlcBoundaryClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  if (vtkMRMLTableNode* tableNode = d->logic()->CreateMlcTableNodeBoundaryData(d->ParameterNode))
  {
    vtkMRMLRTBeamNode* beamNode = d->ParameterNode->GetBeamNode();
    if (d->logic()->SetBeamParentForMlcTableNode(beamNode, tableNode))
    {
      qDebug() << Q_FUNC_INFO << ": Table created";
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onUpdateMlcBoundaryClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (!d->ParameterNode->GetBeamNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid RTBeam node";
    return;
  }

  if (vtkMRMLTableNode* tableNode = d->ParameterNode->GetBeamNode()->GetMultiLeafCollimatorTableNode())
  {
    tableNode->RemoveAllColumns();
    if (d->logic()->UpdateMlcTableNodeBoundaryData(d->ParameterNode, tableNode))
    {
      qDebug() << Q_FUNC_INFO << ": Table updated";
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSetMlcTableClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (!d->ParameterNode->GetBeamNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid RTBeam node";
    return;
  }

  if (vtkMRMLTableNode* tableNode = d->ParameterNode->GetBeamNode()->GetMultiLeafCollimatorTableNode())
  {
    d->MlcControlWidget->setMlcTableNode(tableNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onConnectMlcLayersButtonClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  // if devices are connected disconnect them
  if (d->MlcLayer1SerialPort->isOpen() && d->PushButton_ConnectMlcDevice->text() == tr("Disconnect"))
  {
    d->disconnectDevice();
    d->PushButton_ConnectMlcDevice->setText(tr("Connect"));
    return;
  }

  // connect devices
  if (d->ParameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::OneLayer || d->ParameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::TwoLayers)
  {
    QString portName = d->LineEdit_DeviceLayer1->text();
    if (d->connectDevice(portName))
    {
      ; // connected
    }
  }
  if (d->ParameterNode->GetLayers() == vtkMRMLIhepMlcControlNode::TwoLayers)
  {
    ;
  }
  if (d->MlcLayer1SerialPort->isOpen())
  {
    d->PushButton_ConnectMlcDevice->setText(tr("Disconnect"));
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::serialPortError(QSerialPort::SerialPortError error)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  if (error == QSerialPort::ReadError)
  {
#if (QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ))
    qDebug() << QObject::tr("An I/O error occurred while reading the data from port %1, error: %2").arg(d->MlcLayer1SerialPort->portName()).arg(d->MlcLayer1SerialPort->errorString()) << Qt::endl;
#elif (QT_VERSION > QT_VERSION_CHECK( 4, 0, 0 ) && QT_VERSION < QT_VERSION_CHECK( 5, 0, 0 ))
    qDebug() << QObject::tr("An I/O error occurred while reading the data from port %1, error: %2").arg(d->MlcLayer1SerialPort->portName()).arg(d->MlcLayer1SerialPort->errorString()) << endl;
#endif
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::serialPortDataReady()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);

  // Stop watchdog timer because of responce
  d->TimerWatchdog->stop();

  QByteArray portData = d->MlcLayer1SerialPort->readAll();

  d->ResponseBuffer.append(portData);

  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  const int commandSize = buf.size();
  if (d->ResponseBuffer.size() >= commandSize)
  {
//    unsigned char* data = reinterpret_cast< unsigned char* >(d->ResponseBuffer.data());
    std::copy_n(d->ResponseBuffer.data(), commandSize, std::begin(buf));
    qDebug() << Q_FUNC_INFO << ": Buffer: " << d->ResponseBuffer << ", data: " << int(buf[0])
      << " " << int(buf[1]) << " " << int(buf[2]) << " " << int(buf[3])
      << " " << int(buf[4]) << " " << int(buf[5]) << " " << int(buf[6])
      << " " << int(buf[7]) << " " << int(buf[8]) << " " << int(buf[9])
      << " " << int(buf[10]);
    if (vtkMRMLIhepMlcControlNode::CommandCheckCrc16(buf))
    {
      qDebug() << Q_FUNC_INFO << "Command data is OK!";
      vtkMRMLIhepMlcControlNode::LeafData leafData;
      vtkMRMLIhepMlcControlNode::ProcessCommandBufferToLeafData(buf, leafData);
      emit leafDataChanged(leafData);

      if (d->ResponseBuffer.size() > commandSize)
      {
        d->ResponseBuffer.remove(0, commandSize);
      }
      else if (d->ResponseBuffer.size() == commandSize)
      {
        d->ResponseBuffer.clear();
      }

      d->LastCommand.clear(); // erase last command since it no longer needed
      if (!d->CommandQueue.empty()) // write next command from queue
      {
//        QTimer::singleShot(10, d, SLOT(writeNextCommandFromQueue()));
        d->writeNextCommandFromQueue();
      }
    }
    else
    {
      qWarning() << Q_FUNC_INFO << "Command data is INVALID!";
      if (d->ResponseBuffer.size() > commandSize)
      {
        d->ResponseBuffer.remove(0, commandSize);
      }
      else if (d->ResponseBuffer.size() == commandSize)
      {
        d->ResponseBuffer.clear();
      }
      // write last command once again, because CRC16 is invalid
//      QTimer::singleShot(1000, d, SLOT(writeLastCommandOnceAgain()));
      d->writeLastCommandOnceAgain();
    }
  }

  // Start get state timer if command queue is empty
  if (d->CommandQueue.empty() && d->LastCommand.isEmpty())
  {
//    d->TimerGetState->start();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::serialPortBytesWritten(qint64 written)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  qDebug() << Q_FUNC_INFO << "Serial port data written: " << written << " bytes";
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onMlcStateTimeoutExpired()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->onLeavesGetStateClicked();
  qDebug() << Q_FUNC_INFO << "MLC state timeout";
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafAddressChanged(int address)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  int pos, range;
  if (d->MlcControlWidget && d->MlcControlWidget->getLeafDataByAddress(address, range, pos))
  {
    d->HorizontalSlider_LeafSteps->setRange(0, range);
    d->HorizontalSlider_LeafSteps->setValue(pos);
    d->SpinBox_LeafSteps->setRange(0, range);
    d->SpinBox_LeafSteps->setValue(pos);
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  if (d->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    (leafData.Direction) ? d->RadioButton_Clockwise->setChecked(true) : d->RadioButton_CounterClockwise->setChecked(true);
    d->ComboBox_MotorFrequency->setCurrentIndex(leafData.Frequency);
    (leafData.Mode) ? d->RadioButton_HalfStep->setChecked(true) : d->RadioButton_FullStep->setChecked(true);
    d->CheckBox_MotorReset->setChecked(leafData.Reset);
    d->CheckBox_MotorEnable->setChecked(leafData.Enabled);
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafStepModeButtonClicked(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
    if (rButton && rButton == d->RadioButton_FullStep)
    {
      leafData.Mode = false;
    }
    else if (rButton && rButton == d->RadioButton_HalfStep)
    {
      leafData.Mode = true;
    }
    else
    {
      return;
    }
    if (d->ParameterNode->SetLeafDataByAddress(leafData, address))
    {
      d->MlcControlWidget->setLeafData(leafData);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafDirectionButtonClicked(QAbstractButton* button)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    QRadioButton* rButton = qobject_cast<QRadioButton*>(button);
    if (rButton && rButton == d->RadioButton_Clockwise)
    {
      leafData.Direction = false;
    }
    else if (rButton && rButton == d->RadioButton_CounterClockwise)
    {
      leafData.Direction = true;
    }
    else
    {
      return;
    }
    if (d->ParameterNode->SetLeafDataByAddress(leafData, address))
    {
      d->MlcControlWidget->setLeafData(leafData);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafStepsChanged(int steps)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    leafData.Steps = steps;
    if (d->ParameterNode->SetLeafDataByAddress(leafData, address))
    {
      d->MlcControlWidget->setLeafData(leafData);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafResetToggled(bool reset)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    leafData.Reset = reset;
    if (d->ParameterNode->SetLeafDataByAddress(leafData, address))
    {
      d->MlcControlWidget->setLeafData(leafData);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafEnabledToggled(bool enabled)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  if (d->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    leafData.Enabled = enabled;
    if (d->ParameterNode->SetLeafDataByAddress(leafData, address))
    {
      d->MlcControlWidget->setLeafData(leafData);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafSetParametersClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  QByteArray com;
  if (d->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    com = d->getParametersCommandFromLeafData(leafData);
  }
  if (com.size())
  {
    d->TimerGetState->stop();
    unsigned char* buf = reinterpret_cast< unsigned char* >(com.data());
    qDebug() << Q_FUNC_INFO << "Set parameters command data: " << int(buf[0]) << " "
      << " " << int(buf[1]) << " " << " " << int(buf[2]) << " " << " " << int(buf[3]) << " "
      << " " << int(buf[4]) << " " << " " << int(buf[5]) << " " << " " << int(buf[6]) << " "
      << " " << int(buf[7]) << " " << " " << int(buf[8]) << " " << " " << int(buf[9]) << " "
      << " " << int(buf[10]);

    d->CommandQueue.push(com);
    d->writeNextCommandFromQueue();
  }
  qDebug() << Q_FUNC_INFO << "Leaf address: " << leafData.Address;
  qDebug() << Q_FUNC_INFO << "Leaf steps: " << leafData.Steps;
  qDebug() << Q_FUNC_INFO << "Leaf range: " << leafData.Range;
  qDebug() << Q_FUNC_INFO << "Leaf side: " << leafData.Side;
  qDebug() << Q_FUNC_INFO << "Leaf layer: " << leafData.Layer;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafGetStateClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }

  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  QByteArray com;
  if (d->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    com = d->getStateCommandFromLeafData(leafData);
  }
  if (com.size())
  {
    d->TimerGetState->stop();
    d->CommandQueue.push(com);
    d->writeNextCommandFromQueue();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafStartClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }

  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  QByteArray com;
  if (d->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    com = d->getStartCommandFromLeafData(leafData);
  }
  if (com.size())
  {
    d->TimerGetState->stop();
    d->CommandQueue.push(com);
    d->writeNextCommandFromQueue();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafStopClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }

  vtkMRMLIhepMlcControlNode::LeafData leafData;
  int address = d->HorizontalSlider_LeafAddress->value();
  QByteArray com;
  if (d->ParameterNode->GetLeafDataByAddress(leafData, address))
  {
    com = d->getStopCommandFromLeafData(leafData);
  }
  if (com.size())
  {
    d->TimerGetState->stop();
    d->CommandQueue.push(com);
    d->writeNextCommandFromQueue();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeafAddressStepsMovementChanged(int address, int movementSteps)
{
  qDebug() << Q_FUNC_INFO << "Address: " << address << " movement steps: " << movementSteps;

  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }

  if (d->logic()->UpdateMlcTableNodePositionData(d->ParameterNode, address, movementSteps))
  {
    qDebug() << Q_FUNC_INFO << "MLC table modified";
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesSetParametersClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  QList< QByteArray > mlcLayer1StateCommands = d->getParametersCommandsByLayer(vtkMRMLIhepMlcControlNode::Layer1);
  if (mlcLayer1StateCommands.size())
  {
    d->TimerGetState->stop();
    for (const auto& leafStateCommand : mlcLayer1StateCommands)
    {
      d->CommandQueue.push(leafStateCommand);
    }
    d->writeNextCommandFromQueue();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesGetStateClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  QList< QByteArray > mlcLayer1StateCommands = d->getStateCommandsByLayer(vtkMRMLIhepMlcControlNode::Layer1);
  qDebug() << Q_FUNC_INFO << ": Layer1 MLC state size: " << mlcLayer1StateCommands.size();
  if (mlcLayer1StateCommands.size())
  {
    d->TimerGetState->stop();
    for (const auto& leafStateCommand : mlcLayer1StateCommands)
    {
      qDebug() << Q_FUNC_INFO << "Leaf State command: " << leafStateCommand;
      d->CommandQueue.push(leafStateCommand);
    }
    d->writeNextCommandFromQueue();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesStartBroadcastClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }

  QByteArray com = d->getStartBroadcastCommand();
  if (com.size())
  {
    d->TimerGetState->stop();
    d->CommandQueue.push(com);
    d->writeNextCommandFromQueue();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesStopBroadcastClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }

  QByteArray com = d->getStopBroadcastCommand();
  if (com.size())
  {
    d->TimerGetState->stop();
    d->CommandQueue.push(com);
    d->writeNextCommandFromQueue();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onLeavesOpenBroadcastClicked()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  if (!d->ParameterNode)
  {
    return;
  }

  QByteArray com = d->getOpenBroadcastCommand();
  if (com.size())
  {
    d->TimerGetState->stop();
    d->CommandQueue.push(com);
    d->writeNextCommandFromQueue();
  }
}
