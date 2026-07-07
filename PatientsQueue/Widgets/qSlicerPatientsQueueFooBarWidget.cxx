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

// FooBar Widgets includes
#include "qSlicerPatientsQueueFooBarWidget.h"
#include "ui_qSlicerPatientsQueueFooBarWidget.h"

//-----------------------------------------------------------------------------
class qSlicerPatientsQueueFooBarWidgetPrivate
  : public Ui_qSlicerPatientsQueueFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerPatientsQueueFooBarWidget);
protected:
  qSlicerPatientsQueueFooBarWidget* const q_ptr;

public:
  qSlicerPatientsQueueFooBarWidgetPrivate(
    qSlicerPatientsQueueFooBarWidget& object);
  virtual void setupUi(qSlicerPatientsQueueFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerPatientsQueueFooBarWidgetPrivate
::qSlicerPatientsQueueFooBarWidgetPrivate(
  qSlicerPatientsQueueFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerPatientsQueueFooBarWidgetPrivate
::setupUi(qSlicerPatientsQueueFooBarWidget* widget)
{
  this->Ui_qSlicerPatientsQueueFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPatientsQueueFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerPatientsQueueFooBarWidget
::qSlicerPatientsQueueFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerPatientsQueueFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerPatientsQueueFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerPatientsQueueFooBarWidget
::~qSlicerPatientsQueueFooBarWidget()
{
}
