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
#include "qSlicerU70RoomWorklistFooBarWidget.h"
#include "ui_qSlicerU70RoomWorklistFooBarWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomWorklistFooBarWidgetPrivate : public Ui_qSlicerU70RoomWorklistFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerU70RoomWorklistFooBarWidget);

protected:
  qSlicerU70RoomWorklistFooBarWidget* const q_ptr;

public:
  qSlicerU70RoomWorklistFooBarWidgetPrivate(qSlicerU70RoomWorklistFooBarWidget& object);
  virtual void setupUi(qSlicerU70RoomWorklistFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerU70RoomWorklistFooBarWidgetPrivate::qSlicerU70RoomWorklistFooBarWidgetPrivate(qSlicerU70RoomWorklistFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerU70RoomWorklistFooBarWidgetPrivate::setupUi(qSlicerU70RoomWorklistFooBarWidget* widget)
{
  this->Ui_qSlicerU70RoomWorklistFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerU70RoomWorklistFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistFooBarWidget::qSlicerU70RoomWorklistFooBarWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerU70RoomWorklistFooBarWidgetPrivate(*this))
{
  Q_D(qSlicerU70RoomWorklistFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistFooBarWidget ::~qSlicerU70RoomWorklistFooBarWidget() {}
