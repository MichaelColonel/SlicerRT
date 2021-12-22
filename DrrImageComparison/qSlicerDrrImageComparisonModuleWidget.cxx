/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerDrrImageComparisonModuleWidget.h"
#include "ui_qSlicerDrrImageComparisonModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerDrrImageComparisonModuleWidgetPrivate: public Ui_qSlicerDrrImageComparisonModuleWidget
{
public:
  qSlicerDrrImageComparisonModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerDrrImageComparisonModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonModuleWidgetPrivate::qSlicerDrrImageComparisonModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerDrrImageComparisonModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonModuleWidget::qSlicerDrrImageComparisonModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerDrrImageComparisonModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonModuleWidget::~qSlicerDrrImageComparisonModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComparisonModuleWidget::setup()
{
  Q_D(qSlicerDrrImageComparisonModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComparisonModuleWidget::enter()
{
  Q_D(qSlicerDrrImageComparisonModuleWidget);
  this->Superclass::enter();
  qDebug() << Q_FUNC_INFO << "module widget enter";
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComparisonModuleWidget::exit()
{
  Q_D(qSlicerDrrImageComparisonModuleWidget);

  this->Superclass::exit();
  qDebug() << Q_FUNC_INFO << "module widget exit";
}
