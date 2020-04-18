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

#ifndef __qSlicerLoadableModuleTemplateModuleWidget_h
#define __qSlicerLoadableModuleTemplateModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerLoadableModuleTemplateModuleExport.h"

class qSlicerLoadableModuleTemplateModuleWidgetPrivate;

class vtkImageData;

class vtkMRMLNode;
class vtkMRMLVolumeNode;
class vtkMRMLSegmentationNode;

class vtkSlicerSegmentationsModuleLogic;
class vtkSlicerBeamsModuleLogic;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_LOADABLEMODULETEMPLATE_EXPORT qSlicerLoadableModuleTemplateModuleWidget : public qSlicerAbstractModuleWidget {

  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerLoadableModuleTemplateModuleWidget(QWidget *parent = nullptr);
  virtual ~qSlicerLoadableModuleTemplateModuleWidget();
  virtual void enter();
  void setSegmentationsLogic(vtkSlicerSegmentationsModuleLogic*);
  void setBeamsLogic(vtkSlicerBeamsModuleLogic*);

public slots:
  /// Reimplemented for internal reasons
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// RTPlan changed
  void onRTPlanNodeChanged(vtkMRMLNode* planNode); 
  /// RTBeam or RTIonBeam changed
  void onRTBeamNodeChanged(vtkMRMLNode* beamNode); 
  /// Volume node (slices series) changed
  void onVolumeNodeChanged(vtkMRMLNode* volumeNode);
  /// Segmentation node (RTSTRUCT contours) changed
  void onSegmentationNodeChanged(vtkMRMLNode* segmentationNode);
  /// Target volume name changed
  void onTargetSegmentChanged(const QString& text);

  /// Process Volume and Segmentation Nodes
  void onCalculateOpeningButtonClicked();

  /// Change visibility of the MRML node ID columns
  void setMRMLIDsVisible(bool visible);

protected:
  QScopedPointer<qSlicerLoadableModuleTemplateModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerLoadableModuleTemplateModuleWidget);
  Q_DISABLE_COPY(qSlicerLoadableModuleTemplateModuleWidget);
};

#endif
