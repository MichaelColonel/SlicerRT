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
#include "qSlicerPatientPositioningFooBarWidget.h"
#include "ui_qSlicerPatientPositioningFooBarWidget.h"

//-----------------------------------------------------------------------------
class qSlicerPatientPositioningFooBarWidgetPrivate
  : public Ui_qSlicerPatientPositioningFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerPatientPositioningFooBarWidget);
protected:
  qSlicerPatientPositioningFooBarWidget* const q_ptr;

public:
  qSlicerPatientPositioningFooBarWidgetPrivate(
    qSlicerPatientPositioningFooBarWidget& object);
  virtual void setupUi(qSlicerPatientPositioningFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerPatientPositioningFooBarWidgetPrivate
::qSlicerPatientPositioningFooBarWidgetPrivate(
  qSlicerPatientPositioningFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerPatientPositioningFooBarWidgetPrivate
::setupUi(qSlicerPatientPositioningFooBarWidget* widget)
{
  this->Ui_qSlicerPatientPositioningFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPatientPositioningFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerPatientPositioningFooBarWidget
::qSlicerPatientPositioningFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerPatientPositioningFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerPatientPositioningFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerPatientPositioningFooBarWidget
::~qSlicerPatientPositioningFooBarWidget()
{
}
