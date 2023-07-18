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
#include <QQueue>

// STD includes
#include <cstring>
#include <bitset>

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
  bool processResponseBufferToLeafData(vtkMRMLIhepMlcControlNode::LeafData& leafData);
  QByteArray getParametersCommandByAddress(int address);
  QByteArray getRelativeParametersCommandByAddress(int address);
  QByteArray getStateCommandByAddress(int address);
  QByteArray getStartCommandByAddress(int address);
  QByteArray getStopCommandByAddress(int address);
  QByteArray getStartBroadcastCommand();
  QByteArray getStopBroadcastCommand();
  QByteArray getOpenBroadcastCommand();
  QByteArray getRelativeParametersCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData);
  QByteArray getParametersCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData);
  QByteArray getStateCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData);
  QByteArray getStartCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData);
  QByteArray getStopCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData);
  bool checkAddressCommandResponseOk(const vtkMRMLIhepMlcControlNode::CommandBufferType& buf) const;
  bool checkAddressCommandResponseInvalid(const vtkMRMLIhepMlcControlNode::CommandBufferType& buf) const;

  QList< QByteArray > getParametersCommands(const std::vector<int>& addresses);
  QList< QByteArray > getRelativeParametersCommands(const std::vector<int>& addresses);
  QList< QByteArray > getStateCommands(const std::vector<int>& addresses);
  QList< QByteArray > getStartCommands(const std::vector<int>& addresses);
  QList< QByteArray > getStopCommands(const std::vector<int>& addresses);

  QList< QByteArray > getParametersCommands();
  QList< QByteArray > getStateCommands();

  vtkWeakPointer<vtkMRMLIhepMlcControlNode> ParameterNode;

  QByteArray ResponseBuffer;
  QByteArray InputBuffer;
  QByteArray LastCommand;
  QQueue< QByteArray > CommandQueue;
  QSerialPort* MlcLayerSerialPort{ nullptr };
  QTimer* TimerCommandQueue{ nullptr };
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
  this->TimerCommandQueue = new QTimer(&object);
  this->TimerWatchdog = new QTimer(&object);
  this->TimerCommandQueue->setInterval(50); // 50 ms
  this->TimerWatchdog->setInterval(10000); // 10000 ms, 10 seconds
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcDeviceLogicPrivate::~qSlicerIhepMlcDeviceLogicPrivate()
{
  this->TimerCommandQueue->stop();
  delete this->TimerCommandQueue;
  this->TimerCommandQueue = nullptr;

  this->TimerWatchdog->stop();
  delete this->TimerWatchdog;
  this->TimerWatchdog = nullptr;

  if (this->MlcLayerSerialPort && this->MlcLayerSerialPort->isOpen())
  {
    this->MlcLayerSerialPort->flush();
    this->MlcLayerSerialPort->close();
  }
  delete this->MlcLayerSerialPort;
  this->MlcLayerSerialPort = nullptr;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogicPrivate::loadApplicationSettings()
{
  //TODO: Implement if there are application settings (such as default dose engine)
  //      See qSlicerSubjectHierarchyPluginLogicPrivate::loadApplicationSettings
}

//-----------------------------------------------------------------------------
bool qSlicerIhepMlcDeviceLogicPrivate::checkAddressCommandResponseOk(const vtkMRMLIhepMlcControlNode::CommandBufferType& buf) const
{
  if (!this->LastCommand.isEmpty())
  {
    int lastCommandAddress = this->LastCommand[0];
    int resporseBufferAddress = static_cast< int >(buf[0]);
    return (vtkMRMLIhepMlcControlNode::CommandCheckCrc16(buf) && (lastCommandAddress == resporseBufferAddress));
  }
  return false;
}

//-----------------------------------------------------------------------------
bool qSlicerIhepMlcDeviceLogicPrivate::checkAddressCommandResponseInvalid(const vtkMRMLIhepMlcControlNode::CommandBufferType& buf) const
{
  if (!this->LastCommand.isEmpty())
  {
    int lastCommandAddress = this->LastCommand[0];
    int resporseBufferAddress = static_cast< int >(buf[0]);
    return !(vtkMRMLIhepMlcControlNode::CommandCheckCrc16(buf) && (lastCommandAddress == resporseBufferAddress));
  }
  return false;
}


//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getParametersCommandByAddress(int address)
{
  if (!this->ParameterNode || address == 0)
  {
    return QByteArray();
  }
  if (address > this->ParameterNode->GetNumberOfLeafPairs() * 2)
  {
    return QByteArray();
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  if (this->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, this->Layer))
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
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getRelativeParametersCommandByAddress(int address)
{
  if (!this->ParameterNode || address == 0)
  {
    return QByteArray();
  }
  if (address > this->ParameterNode->GetNumberOfLeafPairs() * 2)
  {
    return QByteArray();
  }
  vtkMRMLIhepMlcControlNode::LeafData leafData;
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  if (this->ParameterNode->GetLeafDataByAddressInLayer(leafData, address, this->Layer))
  {
    buf[0] = static_cast<unsigned char>(address); // address
    buf[1] = 0; // set leaf parameters
    std::bitset< CHAR_BIT > v(leafData.Frequency); // frequency code
    v.set(4, leafData.Mode); // step mode, halfStep == true, fullStep == false
    int movement = leafData.GetRelativeMovement();
    if (movement > 0)
    {
      v.set(5, true); // clockwise (from the switch)
    }
    else if (movement < 0)
    {
      v.set(5, false); // counter clockwise (to the switch)
    }
    v.set(6, leafData.Reset); // reset state
    v.set(7, leafData.Enabled); // enable state
    buf[2] = static_cast<unsigned char>(v.to_ulong());
    buf[3] = 0; // reserved
    buf[5] = std::abs(movement) >> CHAR_BIT;
    buf[4] = std::abs(movement) & 0xFF;
    unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
    buf[9] = chcksumm & 0xFF;
    buf[10] = chcksumm >> CHAR_BIT;

    return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
  }
  return QByteArray();
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getStateCommandByAddress(int address)
{
  if (!this->ParameterNode || address == 0)
  {
    return QByteArray();
  }
  if (address > this->ParameterNode->GetNumberOfLeafPairs() * 2)
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
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getStartCommandByAddress(int address)
{
  if (!this->ParameterNode || address == 0)
  {
    return QByteArray();
  }
  if (address > this->ParameterNode->GetNumberOfLeafPairs() * 2)
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
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getStopCommandByAddress(int address)
{
  if (!this->ParameterNode || address == 0)
  {
    return QByteArray();
  }
  if (address > this->ParameterNode->GetNumberOfLeafPairs() * 2)
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
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getStartBroadcastCommand()
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
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getStopBroadcastCommand()
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
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getOpenBroadcastCommand()
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
QList< QByteArray > qSlicerIhepMlcDeviceLogicPrivate::getParametersCommands(const std::vector< int >& addrs)
{
  Q_Q(const qSlicerIhepMlcDeviceLogic);
  QList< QByteArray > list;
  if (!this->ParameterNode || this->Layer == vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    return list;
  }
  
  std::vector<int> addresses;
  if (addrs.empty())
  {
    this->ParameterNode->GetAddressesByLayer(addresses, this->Layer);
  }
  else
  {
    addresses = addrs;
  }
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
QList< QByteArray > qSlicerIhepMlcDeviceLogicPrivate::getRelativeParametersCommands(const std::vector< int >& addrs)
{
  Q_Q(const qSlicerIhepMlcDeviceLogic);
  QList< QByteArray > list;
  if (!this->ParameterNode || this->Layer == vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    return list;
  }
  std::vector<int> addresses;
  if (addrs.empty())
  {
    this->ParameterNode->GetAddressesByLayer(addresses, this->Layer);
  }
  else
  {
    addresses = addrs;
  }
  for (int address : addresses)
  {
    QByteArray command = this->getRelativeParametersCommandByAddress(address);
    if (command.size())
    {
      list.push_back(command);
    }
  }
  return list;
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcDeviceLogicPrivate::getStateCommands(const std::vector< int >& addrs)
{
  Q_Q(const qSlicerIhepMlcDeviceLogic);
  QList< QByteArray > list;
  if (!this->ParameterNode || this->Layer == vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    return list;
  }
  std::vector<int> addresses;
  if (addrs.empty())
  {
    this->ParameterNode->GetAddressesByLayer(addresses, this->Layer);
  }
  else
  {
    addresses = addrs;
  }
  for (int address : addresses)
  {
    QByteArray command = this->getStateCommandByAddress(address);
    if (command.size())
    {
      list.push_back(command);
    }
  }
  return list;
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcDeviceLogicPrivate::getStopCommands(const std::vector< int >& addrs)
{
  Q_Q(const qSlicerIhepMlcDeviceLogic);
  QList< QByteArray > list;
  if (!this->ParameterNode || this->Layer == vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    return list;
  }
  std::vector<int> addresses;
  if (addrs.empty())
  {
    this->ParameterNode->GetAddressesByLayer(addresses, this->Layer);
  }
  else
  {
    addresses = addrs;
  }
  for (int address : addresses)
  {
    QByteArray command = this->getStopCommandByAddress(address);
    if (command.size())
    {
      list.push_back(command);
    }
  }
  return list;
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcDeviceLogicPrivate::getStartCommands(const std::vector< int >& addrs)
{
  Q_Q(const qSlicerIhepMlcDeviceLogic);
  QList< QByteArray > list;
  if (!this->ParameterNode || this->Layer == vtkMRMLIhepMlcControlNode::Layer_Last)
  {
    return list;
  }
  std::vector<int> addresses;
  if (addrs.empty())
  {
    this->ParameterNode->GetAddressesByLayer(addresses, this->Layer);
  }
  else
  {
    addresses = addrs;
  }
  for (int address : addresses)
  {
    QByteArray command = this->getStartCommandByAddress(address);
    if (command.size())
    {
      list.push_back(command);
    }
  }
  return list;
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcDeviceLogicPrivate::getParametersCommands()
{
  Q_Q(const qSlicerIhepMlcDeviceLogic);
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
QList< QByteArray > qSlicerIhepMlcDeviceLogicPrivate::getStateCommands()
{
  Q_Q(const qSlicerIhepMlcDeviceLogic);
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
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getParametersCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData)
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
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getRelativeParametersCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData)
{
  if (!this->ParameterNode)
  {
    return QByteArray();
  }
  if (leafData.Address > this->ParameterNode->GetNumberOfLeafPairs() * 2)
  {
    return QByteArray();
  }
  vtkMRMLIhepMlcControlNode::CommandBufferType buf;

  buf[0] = static_cast<unsigned char>(leafData.Address); // address
  buf[1] = 0; // set leaf parameters
  std::bitset< CHAR_BIT > v(leafData.Frequency); // frequency code
  v.set(4, leafData.Mode); // step mode, halfStep == true, fullStep == false
  int movement = leafData.GetRelativeMovement();//this->ParameterNode->GetRelativeMovementByAddress(leafData.Address);
  if (movement > 0)
  {
    v.set(5, true); // clockwise (from the switch)
  }
  else if (movement < 0)
  {
    v.set(5, false); // counter clockwise (to the switch)
  }
  v.set(6, leafData.Reset); // reset state
  v.set(7, leafData.Enabled); // enable state
  buf[2] = static_cast<unsigned char>(v.to_ulong());
  buf[3] = 0; // reserved
  buf[5] = std::abs(movement) >> CHAR_BIT;
  buf[4] = std::abs(movement) & 0xFF;
  unsigned short chcksumm = vtkMRMLIhepMlcControlNode::CommandCalculateCrc16(buf);
  buf[9] = chcksumm & 0xFF;
  buf[10] = chcksumm >> CHAR_BIT;

  return QByteArray(reinterpret_cast< char* >(buf.data()), buf.size());
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getStateCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData)
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
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getStartCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData)
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
QByteArray qSlicerIhepMlcDeviceLogicPrivate::getStopCommandFromLeafData(const vtkMRMLIhepMlcControlNode::LeafData& leafData)
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
// qSlicerIhepMlcDeviceLogic methods

//----------------------------------------------------------------------------
qSlicerIhepMlcDeviceLogic::qSlicerIhepMlcDeviceLogic(QObject* parent)
  : Superclass(parent)
  , d_ptr( new qSlicerIhepMlcDeviceLogicPrivate(*this) )
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

  if (nodeObject->IsA("vtkMRMLIhepMlcControlNode"))
  {
    // Observe MLC control node changes
//    vtkMRMLIhepMlcControlNode* mlcNode = vtkMRMLIhepMlcControlNode::SafeDownCast(nodeObject);
//    qvtkConnect( mlcNode, vtkMRMLIhepMlcControlNode::Modified, this, SLOT( applyDoseEngineInPlan(vtkObject*) ) );
  }
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
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT( updateLogicFromMRML() ) );

  d->ParameterNode = parameterNode;
  this->updateLogicFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::serialPortError(QSerialPort::SerialPortError error)
{
  Q_D(qSlicerIhepMlcDeviceLogic);

  if (error == QSerialPort::ReadError)
  {
#if (QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ))
    qDebug() << QObject::tr("An I/O error occurred while reading the data from port %1, error: %2").arg(d->MlcLayerSerialPort->portName()).arg(d->MlcLayerSerialPort->errorString()) << Qt::endl;
#elif (QT_VERSION > QT_VERSION_CHECK( 4, 0, 0 ) && QT_VERSION < QT_VERSION_CHECK( 5, 0, 0 ))
    qDebug() << QObject::tr("An I/O error occurred while reading the data from port %1, error: %2").arg(d->MlcLayerSerialPort->portName()).arg(d->MlcLayerSerialPort->errorString()) << endl;
#endif
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::serialPortDataReady()
{
  Q_D(qSlicerIhepMlcDeviceLogic);

  qDebug() << Q_FUNC_INFO << "Data is ready to read, bytes available: " << d->MlcLayerSerialPort->bytesAvailable();
  // Stop timers because of responce
  d->TimerCommandQueue->stop();
  d->TimerWatchdog->stop();

//  QByteArray portData = d->MlcLayerSerialPort->readAll();
  if (d->MlcLayerSerialPort->bytesAvailable() > 0)
  {
    char* newData = new char[d->MlcLayerSerialPort->bytesAvailable() + 1];
    qint64 res = d->MlcLayerSerialPort->read(newData, d->MlcLayerSerialPort->bytesAvailable());
    if (res > 0)
    {
      qDebug() << Q_FUNC_INFO << "Bytes have been read: " << res;
      d->ResponseBuffer.append(QByteArray(newData, res));
    }
    delete [] newData;
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "No data, or an error!";
    return;
  }

//  QByteArray portData = d->MlcLayerSerialPort->read(d->MlcLayerSerialPort->bytesAvailable());

//  d->ResponseBuffer.append(portData);

  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  constexpr int commandSize = buf.size();
  if (d->ResponseBuffer.size() >= commandSize)
  {
    std::copy_n(d->ResponseBuffer.data(), commandSize, std::begin(buf));
    qDebug() << Q_FUNC_INFO << ": Buffer: " << d->ResponseBuffer << ", data: " << int(buf[0])
      << " " << int(buf[1]) << " " << int(buf[2]) << " " << int(buf[3])
      << " " << int(buf[4]) << " " << int(buf[5]) << " " << int(buf[6])
      << " " << int(buf[7]) << " " << int(buf[8]) << " " << int(buf[9])
      << " " << int(buf[10]);
    if (d->checkAddressCommandResponseOk(buf))
    {
      d->CommandQueue.dequeue();
      d->LastCommand.clear(); // erase last command since it no longer needed
      qDebug() << Q_FUNC_INFO << "Command data is OK!";
      vtkMRMLIhepMlcControlNode::LeafData leafData;

      vtkMRMLIhepMlcControlNode::ProcessCommandBufferToLeafData(buf, leafData);

      int key = -1;
      vtkMRMLIhepMlcControlNode::SideType side = vtkMRMLIhepMlcControlNode::Side_Last;
      int res = d->ParameterNode->GetLeafOffsetByAddressInLayer(leafData.Address, key, side, d->Layer);
      if (res != -1)
      {
        leafData.Side = side;
        leafData.Layer = d->Layer;
      }

      if (vtkMRMLIhepMlcControlNode::CommandBufferIsStateCommand(buf))
      {
        qDebug() << Q_FUNC_INFO << "State buffer OK, update leaf position!";
        if (!leafData.SwitchState)
        {
          emit leafPositionChanged(leafData.Address, leafData.Layer, leafData.Side, leafData.GetActualCurrentPosition());
        }
        else
        {
          emit leafSwitchChanged(leafData.Address, leafData.Layer, leafData.Side, leafData.SwitchState);
        }
        if (res != -1)
        {
          emit leafStateCommandBufferChanged(buf, d->Layer, side);
        }
      }

      if (d->ResponseBuffer.size() > commandSize)
      {
        d->ResponseBuffer.remove(0, commandSize);
      }
      else if (d->ResponseBuffer.size() == commandSize)
      {
        d->ResponseBuffer.clear();
      }
      if (!d->CommandQueue.isEmpty()) // write next command from queue
      {
///        emit writeNextCommand();
        d->TimerCommandQueue->start();
      }
    }
    else if (d->checkAddressCommandResponseInvalid(buf))
    {
      d->CommandQueue.dequeue();
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
      emit writeLastCommand();
    }
    else
    {
      qCritical() << Q_FUNC_INFO << "Impossible state! Emit last command once again";
    }
  }

  // Start get state timer if command queue is empty
  if (d->CommandQueue.isEmpty() && d->LastCommand.isEmpty())
  {
///    d->TimerGetState->start();
    d->TimerCommandQueue->stop();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::serialPortBytesWritten(qint64 written)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  d->TimerCommandQueue->stop();
  qDebug() << Q_FUNC_INFO << "Serial port data written: " << written << " bytes, command queue size: " << d->CommandQueue.size();
  if (d->CommandQueue.size() && d->LastCommand.size())
  {
    int address = d->LastCommand[0];
    if (!address)
    {
      qDebug() << Q_FUNC_INFO << "Pop last command from command queue because broadcast without answer.";
      d->CommandQueue.dequeue();
      d->LastCommand.clear();
///      emit writeNextCommand();
///      d->TimerCommandQueue->start();
    }
    else
    {
      qDebug() << Q_FUNC_INFO << "Address: " << address;
    }
  }
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

    while (!d->CommandQueue.isEmpty())
    {
      d->CommandQueue.dequeue();
    }

    d->ResponseBuffer.clear();
    d->LastCommand.clear();
    d->InputBuffer.clear();

    QObject::connect(d->MlcLayerSerialPort, SIGNAL(readyRead()), this, SLOT(serialPortDataReady()));
    QObject::connect(d->MlcLayerSerialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(serialPortBytesWritten(qint64)));
    QObject::connect(d->MlcLayerSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialPortError(QSerialPort::SerialPortError)));

    QObject::connect(d->TimerCommandQueue, SIGNAL(timeout()), this, SLOT(writeNextCommandFromQueue()));
    QObject::connect(this, SIGNAL(writeLastCommand()), this, SLOT(writeLastCommandOnceAgain()));
    QObject::connect(this, SIGNAL(writeNextCommand()), this, SLOT(writeNextCommandFromQueue()));

    qDebug() << Q_FUNC_INFO << ": Device port has been opened for device name: " << deviceName;
    d->Layer = layer;
    return d->MlcLayerSerialPort;
  }
  else
  {
    d->MlcLayerSerialPort->flush();
    QObject::disconnect(d->MlcLayerSerialPort, SIGNAL(readyRead()), this, SLOT(serialPortDataReady()));
    QObject::disconnect(d->MlcLayerSerialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(serialPortBytesWritten(qint64)));
    QObject::disconnect(d->MlcLayerSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialPortError(QSerialPort::SerialPortError)));

    QObject::disconnect(d->TimerCommandQueue, SIGNAL(timeout()), this, SLOT(writeNextCommandFromQueue()));
    QObject::disconnect(this, SIGNAL(writeLastCommand()), this, SLOT(writeLastCommandOnceAgain()));
    QObject::disconnect(this, SIGNAL(writeNextCommand()), this, SLOT(writeNextCommandFromQueue()));

    while (!d->CommandQueue.isEmpty())
    {
      d->CommandQueue.dequeue();
    }
    d->ResponseBuffer.clear();
    d->LastCommand.clear();
    d->InputBuffer.clear();
    qWarning() << Q_FUNC_INFO << ": Unable to open device port for device name: " << deviceName;
    d->Layer = vtkMRMLIhepMlcControlNode::Layer_Last;
    return nullptr;
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
bool qSlicerIhepMlcDeviceLogic::closeDevice(const QSerialPort* port)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  if (!port || (port != d->MlcLayerSerialPort))
  {
    return false;
  }
  if (port == d->MlcLayerSerialPort && d->MlcLayerSerialPort->isOpen())
  {
    d->MlcLayerSerialPort->flush();
    d->MlcLayerSerialPort->close();

    QObject::disconnect(d->MlcLayerSerialPort, SIGNAL(readyRead()), this, SLOT(serialPortDataReady()));
    QObject::disconnect(d->MlcLayerSerialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(serialPortBytesWritten(qint64)));
    QObject::disconnect(d->MlcLayerSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialPortError(QSerialPort::SerialPortError)));

    QObject::disconnect(d->TimerCommandQueue, SIGNAL(timeout()), this, SLOT(writeNextCommandFromQueue()));
    QObject::disconnect(this, SIGNAL(writeLastCommand()), this, SLOT(writeLastCommandOnceAgain()));
    QObject::disconnect(this, SIGNAL(writeNextCommand()), this, SLOT(writeNextCommandFromQueue()));

    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::writeNextCommandFromQueue()
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  if (d->CommandQueue.isEmpty())
  {
    qWarning() << Q_FUNC_INFO << "Command queue is empty. Last command state: " << (d->LastCommand.isEmpty() ? "is empty" : "not empty");
    if (d->LastCommand.isEmpty())
    {
///      d->TimerCommandQueue->stop();
///      d->TimerCommandQueue->start();
    }
    return;
  }

  d->LastCommand = d->CommandQueue.head();

  vtkMRMLIhepMlcControlNode::CommandBufferType buf;
  constexpr int commandSize = buf.size();

  if (d->MlcLayerSerialPort && d->MlcLayerSerialPort->isOpen())
  {
    qDebug() << Q_FUNC_INFO << "Write data: " << d->LastCommand << ", size: " << d->LastCommand.size();
    std::copy_n(d->LastCommand.data(), commandSize, std::begin(buf));
    if (d->MlcLayerSerialPort->write(d->LastCommand) == d->LastCommand.size())
    {
      qDebug() << Q_FUNC_INFO << "Next command from queue has been written.";
    }
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
    qDebug() << Q_FUNC_INFO << "Write last command last again: " << d->LastCommand;
    if (d->MlcLayerSerialPort->write(d->LastCommand) == d->LastCommand.size())
    {
      qDebug() << Q_FUNC_INFO << "Last command has been written once again.";
    }
  }
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getParametersCommandByLeafData(const vtkMRMLIhepMlcControlNode::LeafData& data)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getParametersCommandFromLeafData(data);
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getRelativeParametersCommandByLeafData(const vtkMRMLIhepMlcControlNode::LeafData& data)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getRelativeParametersCommandFromLeafData(data);
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getStateCommandByLeafData(const vtkMRMLIhepMlcControlNode::LeafData& data)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStateCommandFromLeafData(data);
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getStartCommandByLeafData(const vtkMRMLIhepMlcControlNode::LeafData& data)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStartCommandFromLeafData(data);
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getStopCommandByLeafData(const vtkMRMLIhepMlcControlNode::LeafData& data)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStopCommandFromLeafData(data);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::addCommandToQueue(const QByteArray& com)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  d->CommandQueue.enqueue(com);
  d->TimerCommandQueue->start();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::addCommandsToQueue(const QList< QByteArray >& coms)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  for (const QByteArray& com : coms)
  {
    d->CommandQueue.enqueue(com);
  }
  d->TimerCommandQueue->start();
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getParametersCommandByAddress(int address)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getParametersCommandByAddress(address);
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getRelativeParametersCommandByAddress(int address)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getRelativeParametersCommandByAddress(address);
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getStateCommandByAddress(int address)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStateCommandByAddress(address);
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getStartCommandByAddress(int address)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStartCommandByAddress(address);
}
  
//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getStopCommandByAddress(int address)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStopCommandByAddress(address);
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getStartBroadcastCommand()
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStartBroadcastCommand();
}  
//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getStopBroadcastCommand()
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStopBroadcastCommand();
}

//-----------------------------------------------------------------------------
QByteArray qSlicerIhepMlcDeviceLogic::getOpenBroadcastCommand()
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getOpenBroadcastCommand();
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcDeviceLogic::getParametersCommands(const std::vector<int>& addresses)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getParametersCommands(addresses);
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcDeviceLogic::getRelativeParametersCommands(const std::vector<int>& addresses)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getRelativeParametersCommands(addresses);
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcDeviceLogic::getStateCommands(const std::vector<int>& addresses)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStateCommands(addresses);
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcDeviceLogic::getStopCommands(const std::vector<int>& addresses)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStopCommands(addresses);
}

//-----------------------------------------------------------------------------
QList< QByteArray > qSlicerIhepMlcDeviceLogic::getStartCommands(const std::vector<int>& addresses)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  return d->getStartCommands(addresses);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::updateLogicFromMRML()
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  qDebug() << Q_FUNC_INFO << "Update logic from MRML";
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcDeviceLogic::enableStateMonitoring(bool enableMonitoring)
{
  Q_D(qSlicerIhepMlcDeviceLogic);
  qDebug() << Q_FUNC_INFO << ": state " << enableMonitoring;
  d->TimerCommandQueue->start();
}

