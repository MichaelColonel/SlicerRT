/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerIhepMlcControlLayoutWidget_h
#define __qSlicerIhepMlcControlLayoutWidget_h

// Qt includes
#include <QWidget>

// Layout Widget includes
#include "qSlicerIhepMlcControlModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkMRMLNode;

class QAbstractButton;
class qSlicerIhepMlcControlLayoutWidgetPrivate;

/// \ingroup Slicer_QtModules_IhepMlcControl
class Q_SLICER_MODULE_IHEPMLCCONTROL_WIDGETS_EXPORT qSlicerIhepMlcControlLayoutWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef QWidget Superclass;
  qSlicerIhepMlcControlLayoutWidget(QWidget *parent=0);
  ~qSlicerIhepMlcControlLayoutWidget() override;

  void fillLeavesControlContainer(int pairOfLeavesIndex);

public slots:
  /// Set IhepMlcControl MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  /// Update widget GUI from IhepMlcControl node and observed RTBeam and Table nodes
  void updateWidgetFromMRML();

  void onMlcLayerChanged(QAbstractButton* radioButton);

  // Pair of leaves slots:
  void onPairOfLeavesDoubleClicked();
  void onPairOfLeavesSideValuesChanged(int,int);
  void onPairOfLeavesSide1RangeChanged(int,int);
  void onPairOfLeavesSide2RangeChanged(int,int);
  void onPairOfLeavesAddressChanged(bool,bool);
  void onPairOfLeavesSize2ValueChanged(int);
  void onPairOfLeavesSize1ValueChanged(int);

  // Buttons
  void onSetPredefinedMlcPositionsClicked();
  void onApplyPredefinedMlcPositionsClicked();
  void onOpenMlcClicked();
  void onCloseMlcClicked();

protected:
  QScopedPointer<qSlicerIhepMlcControlLayoutWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerIhepMlcControlLayoutWidget);
  Q_DISABLE_COPY(qSlicerIhepMlcControlLayoutWidget);
};

#endif
