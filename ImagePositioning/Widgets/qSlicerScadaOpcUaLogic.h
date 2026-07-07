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

#ifndef __qSlicerScadaOpcUaLogic_h
#define __qSlicerScadaOpcUaLogic_h

#include "qSlicerImagePositioningModuleWidgetsExport.h"

// SlicerQt includes
#include "qSlicerObject.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerRT ScadaOpcUa MRML includes
#include <vtkMRMLScadaOpcUaNode.h>

// Qt OPC UA includes
#include <QOpcUaClient>
#include <QOpcUaNode>

class vtkMRMLScene;
class vtkMRMLNode;
class qSlicerScadaOpcUaLogicPrivate;

/// \ingroup SlicerRt_QtModules_IhepMlcControl
class Q_SLICER_MODULE_IMAGEPOSITIONING_WIDGETS_EXPORT qSlicerScadaOpcUaLogic :
  public QObject, public virtual qSlicerObject
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QObject Superclass;
  /// Constructor
  explicit qSlicerScadaOpcUaLogic(QObject* parent=nullptr);
  /// Destructor
  ~qSlicerScadaOpcUaLogic() override;
  /// Set the current MRML scene to the widget
  void setMRMLScene(vtkMRMLScene* scene) override;
  bool getClientConnectedFlag() const;
  QOpcUaClient* getClientConnected() const;
  bool setPatientPositioningLocalTime(const QDateTime&);
  bool setPatientPositioningErrorMessage(const QString&);
  bool setPatientPositioningEventMessage(const QString&);
  bool setButtonAmR1LoadToIsoIsPressed(bool);

public slots:
  /// Set ScadaOpcUa MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);

  bool connectToServer(const QString& client, const QString& serverEndpoint);
  bool disconnectFromServer();
  void clientConnected();
  void clientDisconnected();
  void clientError(QOpcUaClient::ClientError);
  void clientState(QOpcUaClient::ClientState);
  // Scada local time
/*
  void onScadaLocalTimeAttributeRead(QOpcUa::NodeAttributes attr);
  void onScadaLocalTimeAttributeChanged(QOpcUa::NodeAttribute attr,
    const QVariant &value);
  void onScadaLocalTimeEnableMonitoringFinished(QOpcUa::NodeAttribute attr,
    QOpcUa::UaStatusCode status);
*/
protected slots:
  /// Called when a node is added to the scene
  void onNodeAdded(vtkObject* scene, vtkObject* nodeObject);

  /// Called when scene import is finished
  void onSceneImportEnded(vtkObject* sceneObject);

signals:
  void opcUaClientConnected(bool);

private slots:

protected:
  virtual void updateLogicFromMRML();
  QScopedPointer<qSlicerScadaOpcUaLogicPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScadaOpcUaLogic);
  Q_DISABLE_COPY(qSlicerScadaOpcUaLogic);
};

#endif
