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

#ifndef __qSlicerImagePositioningModuleWidget_h
#define __qSlicerImagePositioningModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerImagePositioningModuleExport.h"

// Qt OPC UA includes
#include <QOpcUaClient>
#include <QOpcUaNode>

class qSlicerImagePositioningModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_IMAGEPOSITIONING_EXPORT qSlicerImagePositioningModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerImagePositioningModuleWidget(QWidget *parent=0);
  virtual ~qSlicerImagePositioningModuleWidget();

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

  /// connect to SCADA OPC UA server
  void onConnectClicked();
  void clientConnected();
  void clientDisconnected();
  void clientError(QOpcUaClient::ClientError);
  void clientState(QOpcUaClient::ClientState);
  void onScadaLogicConnected(bool);
  void onSetLocalTimeClicked();
  void onSetErrorMessageClicked();
  void onSetEventMessageClicked();
  
protected:
  QScopedPointer<qSlicerImagePositioningModuleWidgetPrivate> d_ptr;

  void setup() override;
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerImagePositioningModuleWidget);
  Q_DISABLE_COPY(qSlicerImagePositioningModuleWidget);
};

#endif
