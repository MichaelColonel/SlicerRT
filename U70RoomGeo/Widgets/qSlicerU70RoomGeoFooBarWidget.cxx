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
#include "qSlicerU70RoomGeoFooBarWidget.h"
#include "ui_qSlicerU70RoomGeoFooBarWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomGeoFooBarWidgetPrivate : public Ui_qSlicerU70RoomGeoFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerU70RoomGeoFooBarWidget);

protected:
  qSlicerU70RoomGeoFooBarWidget* const q_ptr;

public:
  qSlicerU70RoomGeoFooBarWidgetPrivate(qSlicerU70RoomGeoFooBarWidget& object);
  virtual void setupUi(qSlicerU70RoomGeoFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerU70RoomGeoFooBarWidgetPrivate::qSlicerU70RoomGeoFooBarWidgetPrivate(qSlicerU70RoomGeoFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerU70RoomGeoFooBarWidgetPrivate::setupUi(qSlicerU70RoomGeoFooBarWidget* widget)
{
  this->Ui_qSlicerU70RoomGeoFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerU70RoomGeoFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoFooBarWidget::qSlicerU70RoomGeoFooBarWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerU70RoomGeoFooBarWidgetPrivate(*this))
{
  Q_D(qSlicerU70RoomGeoFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoFooBarWidget ::~qSlicerU70RoomGeoFooBarWidget() {}
