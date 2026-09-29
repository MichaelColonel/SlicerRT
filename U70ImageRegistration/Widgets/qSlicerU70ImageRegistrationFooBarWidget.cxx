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
#include "qSlicerU70ImageRegistrationFooBarWidget.h"
#include "ui_qSlicerU70ImageRegistrationFooBarWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70ImageRegistrationFooBarWidgetPrivate : public Ui_qSlicerU70ImageRegistrationFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerU70ImageRegistrationFooBarWidget);

protected:
  qSlicerU70ImageRegistrationFooBarWidget* const q_ptr;

public:
  qSlicerU70ImageRegistrationFooBarWidgetPrivate(qSlicerU70ImageRegistrationFooBarWidget& object);
  virtual void setupUi(qSlicerU70ImageRegistrationFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerU70ImageRegistrationFooBarWidgetPrivate::qSlicerU70ImageRegistrationFooBarWidgetPrivate(qSlicerU70ImageRegistrationFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerU70ImageRegistrationFooBarWidgetPrivate::setupUi(qSlicerU70ImageRegistrationFooBarWidget* widget)
{
  this->Ui_qSlicerU70ImageRegistrationFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerU70ImageRegistrationFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationFooBarWidget::qSlicerU70ImageRegistrationFooBarWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerU70ImageRegistrationFooBarWidgetPrivate(*this))
{
  Q_D(qSlicerU70ImageRegistrationFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationFooBarWidget ::~qSlicerU70ImageRegistrationFooBarWidget() {}
