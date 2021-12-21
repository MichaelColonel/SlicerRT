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
#include "qSlicerDrrImageComparisonFooBarWidget.h"
#include "ui_qSlicerDrrImageComparisonFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_DrrImageComparison
class qSlicerDrrImageComparisonFooBarWidgetPrivate
  : public Ui_qSlicerDrrImageComparisonFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerDrrImageComparisonFooBarWidget);
protected:
  qSlicerDrrImageComparisonFooBarWidget* const q_ptr;

public:
  qSlicerDrrImageComparisonFooBarWidgetPrivate(
    qSlicerDrrImageComparisonFooBarWidget& object);
  virtual void setupUi(qSlicerDrrImageComparisonFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerDrrImageComparisonFooBarWidgetPrivate
::qSlicerDrrImageComparisonFooBarWidgetPrivate(
  qSlicerDrrImageComparisonFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerDrrImageComparisonFooBarWidgetPrivate
::setupUi(qSlicerDrrImageComparisonFooBarWidget* widget)
{
  this->Ui_qSlicerDrrImageComparisonFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerDrrImageComparisonFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonFooBarWidget
::qSlicerDrrImageComparisonFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerDrrImageComparisonFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerDrrImageComparisonFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonFooBarWidget
::~qSlicerDrrImageComparisonFooBarWidget()
{
}
