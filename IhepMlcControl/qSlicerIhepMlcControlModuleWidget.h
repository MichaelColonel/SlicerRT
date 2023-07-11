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

// SlicerRT IhepMlcControl MRML includes
#include <vtkMRMLIhepMlcControlNode.h>

class qSlicerIhepMlcControlModuleWidgetPrivate;
class vtkMRMLNode;

class QAbstractButton;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_IHEPMLCCONTROL_EXPORT qSlicerIhepMlcControlModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerIhepMlcControlModuleWidget(QWidget *parent=0);
  virtual ~qSlicerIhepMlcControlModuleWidget();
  void enter() override;
  void exit() override;
  /// Display leaf data state in the table widget
  void setLeafData(const vtkMRMLIhepMlcControlNode::LeafData& data);

signals:
  void mlcLayerChanged(vtkMRMLIhepMlcControlNode::LayerType);
  void mlcLayerDeviceDisconnected(vtkMRMLIhepMlcControlNode::LayerType);

public slots:
  /// Scene control
  void setMRMLScene(vtkMRMLScene*) override;
  void setParameterNode(vtkMRMLNode*);
  void onSceneImportedEvent();
  void onSceneClosedEvent();
  /// Parameter and nodes control
  void onParameterNodeChanged(vtkMRMLNode*);
  void onBeamNodeChanged(vtkMRMLNode*);
  void onMlcTableNodeChanged(vtkMRMLNode*);
  /// MLC settings parameters control
  void onParallelBeamToggled(bool toggled);
  void onNumberOfLeafPairsChanged(double);
  void onPairOfLeavesSizeChanged(double);
  void onIsocenterOffsetChanged(double);
  void onDistanceBetweenTwoLayersChanged(double);
  void onOffsetBetweenTwoLayersChanged(double);
  void onMlcLayersButtonClicked(QAbstractButton* button);
  void onMlcOrientationButtonClicked(QAbstractButton* button);
  /// MLC position table control
  void onGenerateMlcBoundaryClicked();
  void onUpdateMlcBoundaryClicked();
  void onSetMlcTableClicked();

  /// Predefined MLC layer position control
  void onMlcLayersPredefinedPositionChanged(vtkMRMLIhepMlcControlNode::PredefinedPositionType);
  void onMlcLayerChanged(vtkMRMLIhepMlcControlNode::LayerType);
  void onMlcLayerChanged(QAbstractButton* button);

  /// Single leaf parameters control
  void onLeafSetParametersClicked();
  void onLeafSetRelativeClicked();
  void onLeafGetStateClicked();
  void onLeafStartClicked();
  void onLeafStopClicked();
  void onLeafAddressChanged(int address);
  void onLeafDirectionButtonClicked(QAbstractButton* button);
  void onLeafStepModeButtonClicked(QAbstractButton* button);
  void onLeafMotorFrequencyIndexChanged(int freq);
  void onLeafStepsChanged(int steps);
  void onLeafResetToggled(bool reset);
  void onLeafEnabledToggled(bool enabled);
  void onLeafDataStepsChanged(int address, int leafDataSteps);

  /// Leaves device control
  void onConnectMlcLayerDevicesClicked();
  void onContinuousStateMonitoringToggled(bool toggled);

  /// Leaves parameters control
  void onLeavesSetParametersClicked();
  void onLeavesSetRelativeParametesClicked();
  void onLeavesGetStateClicked();
  void onLeavesStartBroadcastClicked();
  void onLeavesStopBroadcastClicked();
  void onLeavesOpenBroadcastClicked();

  /// Custom widget layout
  void onSwitchToMlcControlLayoutToggled(bool toggled = true);
  void onSetMlcControlLayout();

  /// Update widget GUI from MLC Control parameters node
  void updateWidgetFromMRML();

signals:
  void writeNextCommand();
  void writeLastCommand();
  void writeCommand(const QByteArray& com, vtkMRMLIhepMlcControlNode::LayerType);
  void writeCommands(const QList< QByteArray >& coms, vtkMRMLIhepMlcControlNode::LayerType);

protected:
  QScopedPointer<qSlicerIhepMlcControlModuleWidgetPrivate> d_ptr;

  void setup() override;
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerIhepMlcControlModuleWidget);
  Q_DISABLE_COPY(qSlicerIhepMlcControlModuleWidget);
};

#endif
