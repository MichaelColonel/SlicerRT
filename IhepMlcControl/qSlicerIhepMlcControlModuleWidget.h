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

#ifndef __qSlicerIhepMlcControlModuleWidget_h
#define __qSlicerIhepMlcControlModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerIhepMlcControlModuleExport.h"

// Qt includes
#include <QtSerialPort/QSerialPort>

class qSlicerIhepMlcControlModuleWidgetPrivate;
class vtkMRMLNode;

class QAbstractButton;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_IHEPMLCCONTROL_EXPORT qSlicerIhepMlcControlModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerIhepMlcControlModuleWidget(QWidget *parent=0);
  virtual ~qSlicerIhepMlcControlModuleWidget();
  void enter() override;
  void exit() override;

public slots:
  void setMRMLScene(vtkMRMLScene*) override;
  void setParameterNode(vtkMRMLNode*);
  void onSceneImportedEvent();
  void onSceneClosedEvent();

  void onParameterNodeChanged(vtkMRMLNode*);
  void onBeamNodeChanged(vtkMRMLNode*);
  void onMlcTableNodeChanged(vtkMRMLNode*);

  void onParallelBeamToggled(bool toggled);
  void onNumberOfLeafPairsChanged(double);
  void onPairOfLeavesSizeChanged(double);
  void onIsocenterOffsetChanged(double);
  void onDistanceBetweenTwoLayersChanged(double);
  void onOffsetBetweenTwoLayersChanged(double);

  void onMlcLayersButtonClicked(QAbstractButton* button);
  void onMlcOrientationButtonClicked(QAbstractButton* button);
  void onGenerateMlcBoundaryClicked();
  void onUpdateMlcBoundaryClicked();

  // Custom widget layout
  void onSwitchToMlcControlLayoutToggled(bool toggled = true);
  void onSetMlcControlLayout();

  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();

  void serialPortBytesWritten(qint64);
  void serialPortDataReady();
  void serialPortError(QSerialPort::SerialPortError);

protected:
  QScopedPointer<qSlicerIhepMlcControlModuleWidgetPrivate> d_ptr;

  void setup() override;
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerIhepMlcControlModuleWidget);
  Q_DISABLE_COPY(qSlicerIhepMlcControlModuleWidget);
};

#endif
