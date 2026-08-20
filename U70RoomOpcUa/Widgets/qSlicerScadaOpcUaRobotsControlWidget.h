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

#ifndef __qSlicerScadaOpcUaRobotsControlWidget_h
#define __qSlicerScadaOpcUaRobotsControlWidget_h

// Qt includes
#include <QWidget>
#include <QSharedPointer>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// ScadaOpcUaRobotsControl Widgets includes
#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

class qSlicerScadaOpcUaRobotsControlWidgetPrivate;
class qSlicerScadaOpcUaLogic;

class vtkMRMLNode;

class Q_SLICER_MODULE_U70ROOMOPCUA_WIDGETS_EXPORT qSlicerScadaOpcUaRobotsControlWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
    
public:
  typedef QWidget Superclass;
  qSlicerScadaOpcUaRobotsControlWidget(QWidget *parent=0);
  ~qSlicerScadaOpcUaRobotsControlWidget() override;

public slots:
  /// Set PatientPositioning MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  /// Set PatientPositioning logic
  void setScadaOpcUaLogic(const QSharedPointer< qSlicerScadaOpcUaLogic >& opcUaLogic);
  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();
  /// Display table top angles

protected:
  QScopedPointer<qSlicerScadaOpcUaRobotsControlWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScadaOpcUaRobotsControlWidget);
  Q_DISABLE_COPY(qSlicerScadaOpcUaRobotsControlWidget);
};

#endif
