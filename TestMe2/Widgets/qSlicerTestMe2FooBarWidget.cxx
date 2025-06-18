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
#include "qSlicerTestMe2FooBarWidget.h"
#include "ui_qSlicerTestMe2FooBarWidget.h"

//-----------------------------------------------------------------------------
class qSlicerTestMe2FooBarWidgetPrivate
  : public Ui_qSlicerTestMe2FooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerTestMe2FooBarWidget);
protected:
  qSlicerTestMe2FooBarWidget* const q_ptr;

public:
  qSlicerTestMe2FooBarWidgetPrivate(
    qSlicerTestMe2FooBarWidget& object);
  virtual void setupUi(qSlicerTestMe2FooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerTestMe2FooBarWidgetPrivate
::qSlicerTestMe2FooBarWidgetPrivate(
  qSlicerTestMe2FooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerTestMe2FooBarWidgetPrivate
::setupUi(qSlicerTestMe2FooBarWidget* widget)
{
  this->Ui_qSlicerTestMe2FooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerTestMe2FooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerTestMe2FooBarWidget
::qSlicerTestMe2FooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerTestMe2FooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerTestMe2FooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerTestMe2FooBarWidget
::~qSlicerTestMe2FooBarWidget()
{
}
