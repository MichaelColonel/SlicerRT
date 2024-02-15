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

#ifndef __qSlicerPatientPositioningFooBarWidget_h
#define __qSlicerPatientPositioningFooBarWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
#include <ctkAxesWidget.h>

// FooBar Widgets includes
#include "qSlicerPatientPositioningModuleWidgetsExport.h"

class vtkMRMLNode;

class qSlicerPatientPositioningFooBarWidgetPrivate;

class Q_SLICER_MODULE_PATIENTPOSITIONING_WIDGETS_EXPORT qSlicerPatientPositioningFooBarWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QWidget Superclass;
  qSlicerPatientPositioningFooBarWidget(QWidget *parent=0);
  ~qSlicerPatientPositioningFooBarWidget() override;

public slots:
  /// Set DrrImageComputation MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();

  void onCurrentAxisChanged(ctkAxesWidget::Axis);
  void onButtonUpClicked();
  void onButtonDownClicked();
  void onButtonLeftClicked();
  void onButtonRightClicked();

protected slots:

protected:
  QScopedPointer<qSlicerPatientPositioningFooBarWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPatientPositioningFooBarWidget);
  Q_DISABLE_COPY(qSlicerPatientPositioningFooBarWidget);
};

#endif
