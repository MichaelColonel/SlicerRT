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
#include "qSlicerImagePositioningFooBarWidget.h"
#include "ui_qSlicerImagePositioningFooBarWidget.h"

//-----------------------------------------------------------------------------
class qSlicerImagePositioningFooBarWidgetPrivate
  : public Ui_qSlicerImagePositioningFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerImagePositioningFooBarWidget);
protected:
  qSlicerImagePositioningFooBarWidget* const q_ptr;

public:
  qSlicerImagePositioningFooBarWidgetPrivate(
    qSlicerImagePositioningFooBarWidget& object);
  virtual void setupUi(qSlicerImagePositioningFooBarWidget*);

};

// --------------------------------------------------------------------------
qSlicerImagePositioningFooBarWidgetPrivate
::qSlicerImagePositioningFooBarWidgetPrivate(
  qSlicerImagePositioningFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerImagePositioningFooBarWidgetPrivate
::setupUi(qSlicerImagePositioningFooBarWidget* widget)
{
  this->Ui_qSlicerImagePositioningFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerImagePositioningFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerImagePositioningFooBarWidget
::qSlicerImagePositioningFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerImagePositioningFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerImagePositioningFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerImagePositioningFooBarWidget
::~qSlicerImagePositioningFooBarWidget()
{
}

void qSlicerImagePositioningFooBarWidget::onMoveUpClicked()
{
}

void qSlicerImagePositioningFooBarWidget::onMoveDownClicked()
{
}

void qSlicerImagePositioningFooBarWidget::onMoveLeftClicked()
{
}

void qSlicerImagePositioningFooBarWidget::onMoveRightClicked()
{
}
