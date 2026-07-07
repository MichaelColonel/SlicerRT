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
#include "qSlicerPatientsQueueModuleWidget.h"
#include "ui_qSlicerPatientsQueueModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerPatientsQueueModuleWidgetPrivate: public Ui_qSlicerPatientsQueueModuleWidget
{
public:
  qSlicerPatientsQueueModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPatientsQueueModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPatientsQueueModuleWidgetPrivate::qSlicerPatientsQueueModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPatientsQueueModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPatientsQueueModuleWidget::qSlicerPatientsQueueModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPatientsQueueModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerPatientsQueueModuleWidget::~qSlicerPatientsQueueModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientsQueueModuleWidget::setup()
{
  Q_D(qSlicerPatientsQueueModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
