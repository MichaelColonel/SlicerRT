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

#ifndef __qSlicerU70RoomGeoModuleWidget_h
#define __qSlicerU70RoomGeoModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerU70RoomGeoModuleExport.h"

class qSlicerU70RoomGeoModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_U70ROOMGEO_EXPORT qSlicerU70RoomGeoModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerU70RoomGeoModuleWidget(QWidget* parent = 0);
  virtual ~qSlicerU70RoomGeoModuleWidget();

  void enter() override;

public slots:
  /// Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);
  void setParameterNode(vtkMRMLNode*);
  /// Process loaded scene
  void onSceneImportedEvent();
  /// Process when scene is closing
  void onSceneClosedEvent();

  /// Update widget GUI from parameter node
  void updateWidgetFromMRML();

  /// Load and unload treatment room machine models and logic 
  void onLoadTreatmentRoomButtonClicked();
  void onUnloadTreatmentRoomButtonClicked();

protected:
  QScopedPointer<qSlicerU70RoomGeoModuleWidgetPrivate> d_ptr;
  /// Initialize the module
  void setup() override;

  /// Run when the module is opened
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerU70RoomGeoModuleWidget);
  Q_DISABLE_COPY(qSlicerU70RoomGeoModuleWidget);
};

#endif
