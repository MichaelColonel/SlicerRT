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
#include "qSlicerMlcPositionFooBarWidget.h"
#include "ui_qSlicerMlcPositionFooBarWidget.h"

#include <array>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_LoadableModuleTemplate
class qSlicerMlcPositionFooBarWidgetPrivate : public Ui_qSlicerMlcPositionFooBarWidget {
  Q_DECLARE_PUBLIC(qSlicerMlcPositionFooBarWidget);
protected:
  qSlicerMlcPositionFooBarWidget* const q_ptr;

public:
  qSlicerMlcPositionFooBarWidgetPrivate(qSlicerMlcPositionFooBarWidget& object);
  virtual void setupUi(qSlicerMlcPositionFooBarWidget*);

};

// --------------------------------------------------------------------------
qSlicerMlcPositionFooBarWidgetPrivate::qSlicerMlcPositionFooBarWidgetPrivate(qSlicerMlcPositionFooBarWidget& object)
  :
  q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void
qSlicerMlcPositionFooBarWidgetPrivate::setupUi(qSlicerMlcPositionFooBarWidget* widget)
{
  this->Ui_qSlicerMlcPositionFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerMlcPositionFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerMlcPositionFooBarWidget::qSlicerMlcPositionFooBarWidget(QWidget* parentWidget)
  :
  Superclass(parentWidget),
  d_ptr(new qSlicerMlcPositionFooBarWidgetPrivate(*this))
{
  Q_D(qSlicerMlcPositionFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerMlcPositionFooBarWidget::~qSlicerMlcPositionFooBarWidget()
{
}
