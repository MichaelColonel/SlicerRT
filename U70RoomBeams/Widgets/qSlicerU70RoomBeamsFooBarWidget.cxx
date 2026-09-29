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
#include "qSlicerU70RoomBeamsFooBarWidget.h"
#include "ui_qSlicerU70RoomBeamsFooBarWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomBeamsFooBarWidgetPrivate : public Ui_qSlicerU70RoomBeamsFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerU70RoomBeamsFooBarWidget);

protected:
  qSlicerU70RoomBeamsFooBarWidget* const q_ptr;

public:
  qSlicerU70RoomBeamsFooBarWidgetPrivate(qSlicerU70RoomBeamsFooBarWidget& object);
  virtual void setupUi(qSlicerU70RoomBeamsFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerU70RoomBeamsFooBarWidgetPrivate::qSlicerU70RoomBeamsFooBarWidgetPrivate(qSlicerU70RoomBeamsFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerU70RoomBeamsFooBarWidgetPrivate::setupUi(qSlicerU70RoomBeamsFooBarWidget* widget)
{
  this->Ui_qSlicerU70RoomBeamsFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerU70RoomBeamsFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomBeamsFooBarWidget::qSlicerU70RoomBeamsFooBarWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerU70RoomBeamsFooBarWidgetPrivate(*this))
{
  Q_D(qSlicerU70RoomBeamsFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerU70RoomBeamsFooBarWidget ::~qSlicerU70RoomBeamsFooBarWidget() {}
