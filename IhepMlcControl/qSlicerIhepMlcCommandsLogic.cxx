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
#include "qSlicerIhepMlcCommandsLogic.h"

// Qt includes
#include <QDebug>
#include <QTimer>

// STD includes
#include <queue>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerIhepMlcCommandsLogicPrivate
{
  Q_DECLARE_PUBLIC(qSlicerIhepMlcCommandsLogic);
protected:
  qSlicerIhepMlcCommandsLogic* const q_ptr;
public:
  static constexpr size_t BUFFER = 11;
  qSlicerIhepMlcCommandsLogicPrivate(qSlicerIhepMlcCommandsLogic& object);
  ~qSlicerIhepMlcCommandsLogicPrivate();

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
// qSlicerIhepMlcCommandsLogicPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcCommandsLogicPrivate::qSlicerIhepMlcCommandsLogicPrivate(qSlicerIhepMlcCommandsLogic& object)
  : q_ptr(&object)
{
  this->SerialPortMlcLayer = new QSerialPort(&object);
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcCommandsLogicPrivate::~qSlicerIhepMlcCommandsLogicPrivate()
{
  if (this->SerialPortMlcLayer->isOpen())
  {
    this->SerialPortMlcLayer->flush();
    this->SerialPortMlcLayer->close();
  }
  delete this->SerialPortMlcLayer;
  this->SerialPortMlcLayer = nullptr;
}

//-----------------------------------------------------------------------------
unsigned short qSlicerIhepMlcCommandsLogicPrivate::updateCrc16(unsigned short crc, unsigned char a)
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
unsigned short qSlicerIhepMlcCommandsLogicPrivate::commandCalculateCrc16(const unsigned char* buffer, size_t size)
{
  unsigned short crc = 0xFFFF;
  for (size_t i = 0; i < size; ++i)
  {
    crc = this->updateCrc16(crc, buffer[i]);
  }

  return crc;
}

//-----------------------------------------------------------------------------
bool qSlicerIhepMlcCommandsLogicPrivate::commandCheckCrc16(const unsigned char* buffer, size_t size)
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
// qSlicerIhepMlcCommandsLogic methods

//----------------------------------------------------------------------------
qSlicerIhepMlcCommandsLogic::qSlicerIhepMlcCommandsLogic(QObject* parent)
  : QObject(parent)
{
}

//----------------------------------------------------------------------------
qSlicerIhepMlcCommandsLogic::~qSlicerIhepMlcCommandsLogic() = default;

//-----------------------------------------------------------------------------
QSerialPort* qSlicerIhepMlcCommandsLogic::connectDevice(const QString& deviceName)
{
  Q_D(qSlicerIhepMlcCommandsLogic);

  this->disconnectDevice(d->SerialPortMlcLayer);

  qWarning() << Q_FUNC_INFO << "Port name: " << deviceName;

  d->SerialPortMlcLayer->setPortName(deviceName);

  qWarning() << Q_FUNC_INFO << "1";

  if (d->SerialPortMlcLayer->open(QIODevice::ReadWrite))
  {
    qWarning() << Q_FUNC_INFO << "3";

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
bool qSlicerIhepMlcCommandsLogic::disconnectDevice(QSerialPort* port)
{
  Q_D(qSlicerIhepMlcCommandsLogic);

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
