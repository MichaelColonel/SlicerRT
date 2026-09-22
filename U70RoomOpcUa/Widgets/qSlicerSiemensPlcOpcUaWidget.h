/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerSiemensPlcOpcUaWidget_h
#define __qSlicerSiemensPlcOpcUaWidget_h

// Qt includes
#include <QWidget>
#include <QSharedPointer>
#include <QOpcUaNode>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// U70RoomOpcUaModuke Widgets includes
#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

#include <vtkMRMLSiemensPlcOpcUaNode.h>

class qSlicerSiemensPlcOpcUaWidgetPrivate;
class QOpcUaClient;

class vtkMRMLNode;

class Q_SLICER_MODULE_U70ROOMOPCUA_WIDGETS_EXPORT qSlicerSiemensPlcOpcUaWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
    
public:
  typedef QWidget Superclass;
  qSlicerSiemensPlcOpcUaWidget(QWidget *parent=0);
  ~qSlicerSiemensPlcOpcUaWidget() override;

public slots:
  /// Set SiemensPlcOpcUa MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  /// Set SiemensPlcOpcUa client
  void setSiemensPlcOpcUaClient(const QSharedPointer< QOpcUaClient >& opcUaClient);
  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();
  /// Display table top angles
  void onParseServerInterfacesClicked();
  void onOpcUaClientConnected();
  void onOpcUaClientDisconnected();

  // Siemens PLC OPC UA slots (values, buttons, etc)
  void onOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::ModeType mode);
  void onAutoManualR1LoadToIsoPressed();
  void onAutoManualR1LoadToIsoReleased();

  void onAutoManualR1ToLoadPressed();
  void onAutoManualR1ToLoadReleased();

  void onServerInterfacesRead(QOpcUa::NodeAttributes attr);

protected:
  QScopedPointer<qSlicerSiemensPlcOpcUaWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSiemensPlcOpcUaWidget);
  Q_DISABLE_COPY(qSlicerSiemensPlcOpcUaWidget);
};

#endif
