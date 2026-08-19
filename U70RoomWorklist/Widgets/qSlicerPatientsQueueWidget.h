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

#ifndef __qSlicerPatientsQueueWidget_h
#define __qSlicerPatientsQueueWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// U70RoomWorklist Widgets includes
#include "qSlicerU70RoomWorklistModuleWidgetsExport.h"

class qSlicerPatientsQueueWidgetPrivate;

class Q_SLICER_MODULE_U70ROOMWORKLIST_WIDGETS_EXPORT qSlicerPatientsQueueWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QWidget Superclass;
  qSlicerPatientsQueueWidget(QWidget *parent=0);
  ~qSlicerPatientsQueueWidget() override;

public slots:
  /// Update widget GUI from parameters node
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qSlicerPatientsQueueWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPatientsQueueWidget);
  Q_DISABLE_COPY(qSlicerPatientsQueueWidget);
};

#endif
