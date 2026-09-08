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
  void onSetCustomLayoutClicked();
  /// Set the current MRML scene to the widget
  void setMRMLScene(vtkMRMLScene*) override;
  void onScadaOpcUaLogicModified();

  /// Process loaded scene
  void onSceneImportedEvent();
  /// Process closing scene
  void onSceneClosedEvent();
  /// Update the entire widget based on the current parameter node
  void updateWidgetFromMRML();

  /// connect to Siemens PLC OPC UA server
  void connectToServer();
  void findServers();
  void findServersComplete(const QVector<QOpcUaApplicationDescription> &servers, QOpcUa::UaStatusCode statusCode);
  void getEndpoints();
  void getEndpointsComplete(const QVector<QOpcUaEndpointDescription> &endpoints, QOpcUa::UaStatusCode statusCode);
  void clientConnected();
  void clientDisconnected();
  void namespacesArrayUpdated(const QStringList &namespaceArray);
  void clientError(QOpcUaClient::ClientError);
  void clientState(QOpcUaClient::ClientState);
  void showErrorDialog(QOpcUaErrorState *errorState);

  void ErrorMessagesRead(QOpcUa::NodeAttributes attr);
  void ErrorMessagesChanged(QOpcUa::NodeAttribute attr, const QVariant &value);

  void ServiceMessagesRead(QOpcUa::NodeAttributes attr);
  void ServiceMessagesChanged(QOpcUa::NodeAttribute attr, const QVariant &value);

  void MiscMessagesRead(QOpcUa::NodeAttributes attr);
  void MiscMessagesChanged(QOpcUa::NodeAttribute attr, const QVariant &value);

  void ServerInterfacesRead(QOpcUa::NodeAttributes attr);

  void onGetErrorMessagesClicked();
  void onGetServiceMessagesClicked();
  void onGetMiscMessagesClicked();
  void onGetServerInterfacesClicked();
  
protected:
  QScopedPointer<qSlicerU70RoomOpcUaModuleWidgetPrivate> d_ptr;

  void setup() override;
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerU70RoomOpcUaModuleWidget);
  Q_DISABLE_COPY(qSlicerU70RoomOpcUaModuleWidget);
};

#endif
