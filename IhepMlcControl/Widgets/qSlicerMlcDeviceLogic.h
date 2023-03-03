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

#ifndef __qSlicerMlcDeviceLogic_h
#define __qSlicerMlcDeviceLogic_h

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

// STD includes
#include <queue>

class vtkMRMLScene;
class vtkMRMLNode;
class vtkMRMLTableNode;
class vtkMRMLRTPlanNode;
class vtkMRMLRTBeamNode;
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

public:
  /// Set the current MRML scene to the widget
  Q_INVOKABLE void setMRMLScene(vtkMRMLScene* scene) override;

signals:
  void progressUpdated(int leafAddress, int externalCount, int internalCount);
  void signalMotorStateChanged(const QByteArray& motorState);

public slots:
  /// Set IhepMlcControl MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  void getCurrentState();
  std::queue< QByteArray > getMotorStateQueue(vtkMRMLTableNode* currentPositionNode, vtkMRMLTableNode* nextPositionNode,
    bool useLayer1 = true, bool useLayer2 = false);
  std::queue< QByteArray > getMotorParametersQueue(vtkMRMLTableNode* currentPositionNode, vtkMRMLTableNode* nextPositionNode,
    bool useLayer1 = true, bool useLayer2 = false);

  QSerialPort* connectDevice(const QString& deviceName);
  bool disconnectDevice(QSerialPort* devicePort);

  void serialPortBytesWritten(qint64);
  void serialPortDataReady();
  void serialPortError(QSerialPort::SerialPortError);

protected slots:
  /// Called when a node is added to the scene
  void onNodeAdded(vtkObject* scene, vtkObject* nodeObject);

  /// Called when scene import is finished
  void onSceneImportEnded(vtkObject* sceneObject);

protected:
  QScopedPointer<qSlicerIhepMlcDeviceLogicPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerIhepMlcDeviceLogic);
  Q_DISABLE_COPY(qSlicerIhepMlcDeviceLogic);
};

#endif
