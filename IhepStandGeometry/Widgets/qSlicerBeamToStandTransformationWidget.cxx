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
#include "qSlicerBeamToStandTransformationWidget.h"
#include "ui_qSlicerBeamToStandTransformationWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IhepStandGeometry
class qSlicerBeamToStandTransformationWidgetPrivate
  : public Ui_qSlicerBeamToStandTransformationWidget
{
  Q_DECLARE_PUBLIC(qSlicerBeamToStandTransformationWidget);
protected:
  qSlicerBeamToStandTransformationWidget* const q_ptr;

public:
  qSlicerBeamToStandTransformationWidgetPrivate(
    qSlicerBeamToStandTransformationWidget& object);
  virtual void setupUi(qSlicerBeamToStandTransformationWidget*);
};

// --------------------------------------------------------------------------
qSlicerBeamToStandTransformationWidgetPrivate::qSlicerBeamToStandTransformationWidgetPrivate(
  qSlicerBeamToStandTransformationWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerBeamToStandTransformationWidgetPrivate::setupUi(qSlicerBeamToStandTransformationWidget* widget)
{
  this->Ui_qSlicerBeamToStandTransformationWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerBeamToStandTransformationWidget methods

//-----------------------------------------------------------------------------
qSlicerBeamToStandTransformationWidget
::qSlicerBeamToStandTransformationWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerBeamToStandTransformationWidgetPrivate(*this) )
{
  Q_D(qSlicerBeamToStandTransformationWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerBeamToStandTransformationWidget::~qSlicerBeamToStandTransformationWidget()
{
}
