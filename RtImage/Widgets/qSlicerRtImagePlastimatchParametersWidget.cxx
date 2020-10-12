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

// PlastimatchParameters Widgets includes
#include "qSlicerRtImagePlastimatchParametersWidget.h"
#include "ui_qSlicerRtImagePlastimatchParametersWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_RtImage
class qSlicerRtImagePlastimatchParametersWidgetPrivate
  : public Ui_qSlicerRtImagePlastimatchParametersWidget
{
  Q_DECLARE_PUBLIC(qSlicerRtImagePlastimatchParametersWidget);
protected:
  qSlicerRtImagePlastimatchParametersWidget* const q_ptr;

public:
  qSlicerRtImagePlastimatchParametersWidgetPrivate(
    qSlicerRtImagePlastimatchParametersWidget& object);
  virtual void setupUi(qSlicerRtImagePlastimatchParametersWidget*);
};

// --------------------------------------------------------------------------
qSlicerRtImagePlastimatchParametersWidgetPrivate
::qSlicerRtImagePlastimatchParametersWidgetPrivate(
  qSlicerRtImagePlastimatchParametersWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidgetPrivate
::setupUi(qSlicerRtImagePlastimatchParametersWidget* widget)
{
  this->Ui_qSlicerRtImagePlastimatchParametersWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerRtImagePlastimatchParametersWidget methods

//-----------------------------------------------------------------------------
qSlicerRtImagePlastimatchParametersWidget
::qSlicerRtImagePlastimatchParametersWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerRtImagePlastimatchParametersWidgetPrivate(*this) )
{
  Q_D(qSlicerRtImagePlastimatchParametersWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerRtImagePlastimatchParametersWidget
::~qSlicerRtImagePlastimatchParametersWidget()
{
}
