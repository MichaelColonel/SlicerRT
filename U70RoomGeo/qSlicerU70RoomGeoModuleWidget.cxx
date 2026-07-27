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
#include "qSlicerU70RoomGeoModuleWidget.h"
#include "ui_qSlicerU70RoomGeoModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomGeoModuleWidgetPrivate : public Ui_qSlicerU70RoomGeoModuleWidget
{
public:
  qSlicerU70RoomGeoModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomGeoModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModuleWidgetPrivate::qSlicerU70RoomGeoModuleWidgetPrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70RoomGeoModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModuleWidget::qSlicerU70RoomGeoModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70RoomGeoModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModuleWidget::~qSlicerU70RoomGeoModuleWidget() {}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModuleWidget::setup()
{
  Q_D(qSlicerU70RoomGeoModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
