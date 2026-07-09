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

#ifndef __qSlicerPatientsQueueWorklistWidget_h
#define __qSlicerPatientsQueueWorklistWidget_h

// Qt includes
#include <qSlicerWidget.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// PatientsQueueWorklist Widgets includes
#include "qSlicerPatientsQueueModuleWidgetsExport.h"

class vtkMRMLNode;
class vtkSlicerPatientsQueueLogic;
class vtkMRMLScalarVolumeNode;

class qSlicerPatientsQueueWorklistWidgetPrivate;

class QAbstractButton;

class Q_SLICER_MODULE_PATIENTSQUEUE_WIDGETS_EXPORT qSlicerPatientsQueueWorklistWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QWidget Superclass;
  qSlicerPatientsQueueWorklistWidget(QWidget *parent=0);
  ~qSlicerPatientsQueueWorklistWidget() override;

public slots:
  /// Update widget GUI from PatientPositioning parameters node
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qSlicerPatientsQueueWorklistWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPatientsQueueWorklistWidget);
  Q_DISABLE_COPY(qSlicerPatientsQueueWorklistWidget);
};

#endif
