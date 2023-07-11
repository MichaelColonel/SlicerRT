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

#ifndef __qSlicerIhepMlcDeviceLogic_h
#define __qSlicerIhepMlcDeviceLogic_h

#include "qSlicerIhepMlcControlModuleWidgetsExport.h"

// SlicerQt includes
#include "qSlicerObject.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Qt includes
#include <QObject>
#include <QByteArray>
#include <QtSerialPort/QSerialPort>

// SlicerRT IhepMlcControl MRML includes
#include <vtkMRMLIhepMlcControlNode.h>

class vtkMRMLScene;
class vtkMRMLNode;
class qSlicerIhepMlcDeviceLogicPrivate;

/// \ingroup SlicerRt_QtModules_IhepMlcControl
class Q_SLICER_MODULE_IHEPMLCCONTROL_WIDGETS_EXPORT qSlicerIhepMlcDeviceLogic :
  public QObject, public virtual qSlicerObject
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QObject Superclass;
  /// Constructor
  explicit qSlicerIhepMlcDeviceLogic(QObject* parent=nullptr);
  /// Destructor
  ~qSlicerIhepMlcDeviceLogic() override;

  /// Set the current MRML scene to the widget
  void setMRMLScene(vtkMRMLScene* scene) override;
  QSerialPort* openDevice(const QString& deviceName, vtkMRMLIhepMlcControlNode::LayerType layer);
  bool closeDevice(const QSerialPort*);

  QByteArray getParametersCommandByAddress(int address);
  QByteArray getRelativeParametersCommandByAddress(int address);
  QByteArray getStateCommandByAddress(int address);
  QByteArray getStartCommandByAddress(int address);
  QByteArray getStopCommandByAddress(int address);

  QByteArray getStartBroadcastCommand();
  QByteArray getStopBroadcastCommand();
  QByteArray getOpenBroadcastCommand();

  QList< QByteArray > getParametersCommands(const std::vector<int>& addresses);
  QList< QByteArray > getRelativeParametersCommands(const std::vector<int>& addresses);
  QList< QByteArray > getStateCommands(const std::vector<int>& addresses);
  QList< QByteArray > getStopCommands(const std::vector<int>& addresses);
  QList< QByteArray > getStartCommands(const std::vector<int>& addresses);

public slots:
  /// Set IhepMlcControl MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  /// Serial port
  void serialPortBytesWritten(qint64);
  void serialPortDataReady();
  void serialPortError(QSerialPort::SerialPortError);

  void addCommandToQueue(const QByteArray& com);
  void addCommandsToQueue(const QList< QByteArray >& coms);
  void enableStateMonitoring(bool);

protected slots:
  /// Called when a node is added to the scene
  void onNodeAdded(vtkObject* scene, vtkObject* nodeObject);

  /// Called when scene import is finished
  void onSceneImportEnded(vtkObject* sceneObject);

signals:
  void writeNextCommand();
  void writeLastCommand();
  void leafPositionChanged(int address,
    vtkMRMLIhepMlcControlNode::LayerType layer,
    vtkMRMLIhepMlcControlNode::SideType side,
    int requiredPosition, int currentPosition);
  void leafSwitchChanged(int address,
    vtkMRMLIhepMlcControlNode::LayerType layer,
    vtkMRMLIhepMlcControlNode::SideType side,
    bool switchIsPressed);
  void leafStateCommandBufferChanged(const vtkMRMLIhepMlcControlNode::CommandBufferType& stateBuffer);

private slots:
  void writeNextCommandFromQueue();
  void writeLastCommandOnceAgain();

protected:
  virtual void updateLogicFromMRML();
  QScopedPointer<qSlicerIhepMlcDeviceLogicPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerIhepMlcDeviceLogic);
  Q_DISABLE_COPY(qSlicerIhepMlcDeviceLogic);
};

#endif
