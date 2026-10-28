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

#ifndef __qSlicerU70RoomOpcUaModuleWidget_h
#define __qSlicerU70RoomOpcUaModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerU70RoomOpcUaModuleExport.h"

// Qt OPC UA includes
#include <QOpcUaClient>
#include <QOpcUaNode>

// MRML includes
#include "vtkMRMLSiemensPlcOpcUaNode.h"

class qSlicerU70RoomOpcUaModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_U70ROOMOPCUA_EXPORT qSlicerU70RoomOpcUaModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerU70RoomOpcUaModuleWidget(QWidget *parent=0);
  virtual ~qSlicerU70RoomOpcUaModuleWidget();

  void enter() override;

public slots:
  void onShowSiemensPlcControlsClicked();
  /// Set the current MRML scene to the widget
  void setMRMLScene(vtkMRMLScene*) override;
  void onOpcUaLogicModified();

  /// Process loaded scene
  void onSceneImportedEvent();
  /// Process closing scene
  void onSceneClosedEvent();
  /// Update the entire widget based on the current parameter node
  void updateWidgetFromMRML();

  /// connect to Siemens PLC OPC UA server
  void siemensConnectToServer();
  void siemensFindServers();
  void siemensFindServersComplete(const QVector<QOpcUaApplicationDescription> &servers, QOpcUa::UaStatusCode statusCode);
  void siemensGetEndpoints();
  void siemensGetEndpointsComplete(const QVector<QOpcUaEndpointDescription> &endpoints, QOpcUa::UaStatusCode statusCode);
  void siemensClientConnected();
  void siemensClientDisconnected();
  void siemensNamespacesArrayUpdated(const QStringList &namespaceArray);
  void siemensClientError(QOpcUaClient::ClientError);
  void siemensClientState(QOpcUaClient::ClientState);
  void siemensShowOpcUaErrorDialog(QOpcUaErrorState *errorState);

signals:
  void siemensPlcOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::ModeType);

protected:
  QScopedPointer<qSlicerU70RoomOpcUaModuleWidgetPrivate> d_ptr;

  void setup() override;
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerU70RoomOpcUaModuleWidget);
  Q_DISABLE_COPY(qSlicerU70RoomOpcUaModuleWidget);
};

#endif
