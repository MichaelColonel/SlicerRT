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
#include "qSlicerScalarBarFooBarWidget.h"
#include "ui_qSlicerScalarBarFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ScalarBar
class qSlicerScalarBarFooBarWidgetPrivate
  : public Ui_qSlicerScalarBarFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerScalarBarFooBarWidget);
protected:
  qSlicerScalarBarFooBarWidget* const q_ptr;

public:
  qSlicerScalarBarFooBarWidgetPrivate(
    qSlicerScalarBarFooBarWidget& object);
  virtual void setupUi(qSlicerScalarBarFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerScalarBarFooBarWidgetPrivate
::qSlicerScalarBarFooBarWidgetPrivate(
  qSlicerScalarBarFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerScalarBarFooBarWidgetPrivate
::setupUi(qSlicerScalarBarFooBarWidget* widget)
{
  this->Ui_qSlicerScalarBarFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerScalarBarFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerScalarBarFooBarWidget
::qSlicerScalarBarFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerScalarBarFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerScalarBarFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerScalarBarFooBarWidget
::~qSlicerScalarBarFooBarWidget()
{
}
