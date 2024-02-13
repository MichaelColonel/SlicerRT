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

  // Buttons
  connect( d->PushButton_Up, SIGNAL(clicked()), this, SLOT(onButtonUpClicked()));
  connect( d->PushButton_Down, SIGNAL(clicked()), this, SLOT(onButtonDownClicked()));
  connect( d->PushButton_Left, SIGNAL(clicked()), this, SLOT(onButtonLeftClicked()));
  connect( d->PushButton_Right, SIGNAL(clicked()), this, SLOT(onButtonRightClicked()));
  connect( d->AxesWidget, SIGNAL(currentAxisChanged(ctkAxesWidget::Axis)), this, SLOT(onCurrentAxisChanged(ctkAxesWidget::Axis)));
}

//-----------------------------------------------------------------------------
qSlicerPatientPositioningFooBarWidget
::~qSlicerPatientPositioningFooBarWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFooBarWidget::onCurrentAxisChanged(ctkAxesWidget::Axis axis)
{
  switch (axis)
  {
  case ctkAxesWidget::Left:
    this->onButtonLeftClicked();
    break;
  case ctkAxesWidget::Right:
    this->onButtonRightClicked();
    break;
  case ctkAxesWidget::Superior:
    this->onButtonUpClicked();
    break;
  case ctkAxesWidget::Inferior:
    this->onButtonDownClicked();
    break;
  default:
    break;
  }  
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFooBarWidget::onButtonUpClicked()
{
  qDebug() << Q_FUNC_INFO << "Up";
}
//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFooBarWidget::onButtonDownClicked()
{
  qDebug() << Q_FUNC_INFO << "Down";
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFooBarWidget::onButtonLeftClicked()
{
  qDebug() << Q_FUNC_INFO << "Left";
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFooBarWidget::onButtonRightClicked()
{
  qDebug() << Q_FUNC_INFO << "Right";
}
