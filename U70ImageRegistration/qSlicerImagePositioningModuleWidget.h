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

#ifndef __qSlicerImagePositioningModuleWidget_h
#define __qSlicerImagePositioningModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerImagePositioningModuleExport.h"

class qSlicerImagePositioningModuleWidgetPrivate;
class vtkMRMLNode; 
class vtkMRMLScalarVolumeNode;

class Q_SLICER_QTMODULES_IMAGEPOSITIONING_EXPORT qSlicerImagePositioningModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerImagePositioningModuleWidget(QWidget *parent=0);
  virtual ~qSlicerImagePositioningModuleWidget();

public slots:

  void setMRMLScene(vtkMRMLScene*) override;
  void setParameterNode(vtkMRMLNode*);
  void onRetrieveDICOMClicked();
  void onSetViewClicked();
  void onSetCustomLayoutClicked();
 // void onHorizontalOrientationClicked();
 // void onVerticalOrientationClicked();
  void onDrrImageNodeChanged(vtkMRMLNode* drrImageNode);
  void onXrayImageNodeChanged(vtkMRMLNode* xrayImageNode);
//  void onDrrOpacityChanged(double* opacity);
  void onXrayOpacityChanged(double opacity);
  void setSliceOrientation();
  void onMoveUpClicked();
  void onMoveDownClicked();
  void onMoveLeftClicked();
  void onMoveRightClicked();
  void onRotateClockwiseClicked();
  void onRotateCounterClockwiseClicked();
  void onResetTransformClicked();
  void onHorizontalTransformChanged(double value);
  void onVerticalTransformChanged(double value);
  void onRotationTransformChanged(double value);
  void onScaleXrayImageClicked();
  void onShowEdgesXrayToggled(bool checked);
  void onShowEdgesDrrToggled(bool checked);

  void updateWidgetFromMRML();
protected:
  QScopedPointer<qSlicerImagePositioningModuleWidgetPrivate> d_ptr;
  void setup() override;
  void setXrayNode(vtkMRMLScalarVolumeNode* xrayNode);
  void updateXrayTransformFrom2DControls();
  void sync2DControlsFromActiveOrientation();

private:
  Q_DECLARE_PRIVATE(qSlicerImagePositioningModuleWidget);
  Q_DISABLE_COPY(qSlicerImagePositioningModuleWidget);
};

#endif
