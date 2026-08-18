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
#include "qSlicerU70RoomOpcUaModuleWidget.h"
#include "ui_qSlicerU70RoomOpcUaModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomOpcUaModuleWidgetPrivate : public Ui_qSlicerU70RoomOpcUaModuleWidget
{
public:
  qSlicerU70RoomOpcUaModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomOpcUaModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomOpcUaModuleWidgetPrivate::qSlicerU70RoomOpcUaModuleWidgetPrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70RoomOpcUaModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomOpcUaModuleWidget::qSlicerU70RoomOpcUaModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70RoomOpcUaModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomOpcUaModuleWidget::~qSlicerU70RoomOpcUaModuleWidget() {}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModuleWidget::setup()
{
  Q_D(qSlicerU70RoomOpcUaModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
