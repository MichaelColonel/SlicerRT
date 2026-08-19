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
#include "qSlicerU70RoomWorklistModuleWidget.h"
#include "ui_qSlicerU70RoomWorklistModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomWorklistModuleWidgetPrivate : public Ui_qSlicerU70RoomWorklistModuleWidget
{
public:
  qSlicerU70RoomWorklistModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomWorklistModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistModuleWidgetPrivate::qSlicerU70RoomWorklistModuleWidgetPrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70RoomWorklistModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistModuleWidget::qSlicerU70RoomWorklistModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70RoomWorklistModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistModuleWidget::~qSlicerU70RoomWorklistModuleWidget() {}

//-----------------------------------------------------------------------------
void qSlicerU70RoomWorklistModuleWidget::setup()
{
  Q_D(qSlicerU70RoomWorklistModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
