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

#ifndef __qSlicerU70RoomWorklistModuleWidget_h
#define __qSlicerU70RoomWorklistModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerU70RoomWorklistModuleExport.h"

class qSlicerU70RoomWorklistModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_U70ROOMWORKLIST_EXPORT qSlicerU70RoomWorklistModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerU70RoomWorklistModuleWidget(QWidget* parent = 0);
  virtual ~qSlicerU70RoomWorklistModuleWidget();

public slots:
  void onSetCustomLayoutClicked();
  void onCheckConnectionClicked();
  void onWorklistQueryClicked();

protected:
  QScopedPointer<qSlicerU70RoomWorklistModuleWidgetPrivate> d_ptr;

  void setup() override;
  void enter() override;
  void exit() override;

private:
  void onEnter();

  Q_DECLARE_PRIVATE(qSlicerU70RoomWorklistModuleWidget);
  Q_DISABLE_COPY(qSlicerU70RoomWorklistModuleWidget);
};

#endif
