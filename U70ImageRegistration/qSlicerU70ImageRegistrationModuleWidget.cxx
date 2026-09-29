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
#include "qSlicerU70ImageRegistrationModuleWidget.h"
#include "ui_qSlicerU70ImageRegistrationModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70ImageRegistrationModuleWidgetPrivate : public Ui_qSlicerU70ImageRegistrationModuleWidget
{
public:
  qSlicerU70ImageRegistrationModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70ImageRegistrationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModuleWidgetPrivate::qSlicerU70ImageRegistrationModuleWidgetPrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70ImageRegistrationModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModuleWidget::qSlicerU70ImageRegistrationModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70ImageRegistrationModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModuleWidget::~qSlicerU70ImageRegistrationModuleWidget() {}

//-----------------------------------------------------------------------------
void qSlicerU70ImageRegistrationModuleWidget::setup()
{
  Q_D(qSlicerU70ImageRegistrationModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
