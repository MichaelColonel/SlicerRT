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

#ifndef __qSlicerScalarBarModuleWidget_h
#define __qSlicerScalarBarModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerScalarBarModuleExport.h"

class qSlicerScalarBarModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_SCALARBAR_EXPORT qSlicerScalarBarModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerScalarBarModuleWidget(QWidget* parent = nullptr);
  ~qSlicerScalarBarModuleWidget() override;

  void exit() override;
  void enter() override;

public slots:
  void setMRMLScene(vtkMRMLScene*) override;
  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();

  void onSceneImportedEvent();
  void onSceneClosedEvent();

  void onScalarVolumeNodeChanged(vtkMRMLNode*);
  void onAddColorBarDisplayNodeClicked();
  void onShowColorBar2DToggled(bool);
  void onShowColorBar3DToggled(bool);
  void onColocrBarPositionIndexChanged(int);

  void onLogicModified();

protected:
  QScopedPointer<qSlicerScalarBarModuleWidgetPrivate> d_ptr;

  void setup() override;
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerScalarBarModuleWidget);
  Q_DISABLE_COPY(qSlicerScalarBarModuleWidget);
};

#endif
