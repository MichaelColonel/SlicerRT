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
#include "qSlicerU70RoomBeamsModuleWidget.h"
#include "ui_qSlicerU70RoomBeamsModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomBeamsModuleWidgetPrivate : public Ui_qSlicerU70RoomBeamsModuleWidget
{
public:
  qSlicerU70RoomBeamsModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomBeamsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomBeamsModuleWidgetPrivate::qSlicerU70RoomBeamsModuleWidgetPrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70RoomBeamsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomBeamsModuleWidget::qSlicerU70RoomBeamsModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70RoomBeamsModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomBeamsModuleWidget::~qSlicerU70RoomBeamsModuleWidget() {}

//-----------------------------------------------------------------------------
void qSlicerU70RoomBeamsModuleWidget::setup()
{
  Q_D(qSlicerU70RoomBeamsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
