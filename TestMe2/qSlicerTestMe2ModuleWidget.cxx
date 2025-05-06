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
#include "qSlicerTestMe2ModuleWidget.h"
#include "ui_qSlicerTestMe2ModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerTestMe2ModuleWidgetPrivate: public Ui_qSlicerTestMe2ModuleWidget
{
public:
  qSlicerTestMe2ModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTestMe2ModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerTestMe2ModuleWidgetPrivate::qSlicerTestMe2ModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTestMe2ModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTestMe2ModuleWidget::qSlicerTestMe2ModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerTestMe2ModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerTestMe2ModuleWidget::~qSlicerTestMe2ModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTestMe2ModuleWidget::setup()
{
  Q_D(qSlicerTestMe2ModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

void qSlicerTestMe2ModuleWidget::onCreateButton();
{
  logic->createPoint();
}
