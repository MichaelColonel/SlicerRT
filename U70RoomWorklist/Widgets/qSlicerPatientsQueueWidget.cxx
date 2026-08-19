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

// Qt includes
#include <QDebug>

// U70RoomWorklist Widgets includes
#include "qSlicerPatientsQueueWidget.h"
#include "ui_qSlicerPatientsQueueWidget.h"

#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qMRMLSliceWidget.h>
#include <qSlicerSubjectHierarchyFolderPlugin.h>
#include <qSlicerSubjectHierarchyPluginHandler.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDView.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkVector.h>
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
class qSlicerPatientsQueueWidgetPrivate : public Ui_qSlicerPatientsQueueWidget
{
  Q_DECLARE_PUBLIC(qSlicerPatientsQueueWidget)
protected:
  qSlicerPatientsQueueWidget* const q_ptr;

public:
  qSlicerPatientsQueueWidgetPrivate(qSlicerPatientsQueueWidget& object);
  virtual void setupUi(qSlicerPatientsQueueWidget*);
};

// --------------------------------------------------------------------------
qSlicerPatientsQueueWidgetPrivate::qSlicerPatientsQueueWidgetPrivate(
  qSlicerPatientsQueueWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerPatientsQueueWidgetPrivate::setupUi(
  qSlicerPatientsQueueWidget* widget)
{
  this->Ui_qSlicerPatientsQueueWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPatientsQueueWorklistWidget methods

//-----------------------------------------------------------------------------
qSlicerPatientsQueueWidget::qSlicerPatientsQueueWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerPatientsQueueWidgetPrivate(*this) )
{
  Q_D(qSlicerPatientsQueueWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerPatientsQueueWidget::~qSlicerPatientsQueueWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientsQueueWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerPatientsQueueWidget);
}
