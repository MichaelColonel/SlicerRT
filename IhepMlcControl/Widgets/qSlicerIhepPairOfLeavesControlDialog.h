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

#ifndef __qSlicerIhepPairOfLeavesControlDialog_h
#define __qSlicerIhepPairOfLeavesControlDialog_h

// Qt includes
#include <QDialog>

// Widget includes
#include "qSlicerIhepMlcControlModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class QAbstractButton;
class qSlicerIhepPairOfLeavesControlDialogPrivate;

/// \ingroup Slicer_QtModules_IhepMlcControl
class Q_SLICER_MODULE_IHEPMLCCONTROL_WIDGETS_EXPORT qSlicerIhepPairOfLeavesControlDialog : public QDialog
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QDialog Superclass;
  explicit qSlicerIhepPairOfLeavesControlDialog(QWidget *parent = nullptr);
  explicit qSlicerIhepPairOfLeavesControlDialog(int side1Address, int side2Address,
    int side1Max, int side1CurrentPosition, int side1RequiredPosition,
    int side2Max, int side2CurrentPosition, int side2RequiredPosition,
    QWidget *parent = nullptr);

  ~qSlicerIhepPairOfLeavesControlDialog() override;
  void getSidePositions(int& side1, int& side2);
  void getSide1Range(int& min, int& max);
  void getSide2Range(int& min, int& max);

public slots:
  void onLeafPositionInPairChanged(bool, bool);
  void onRequiredPositionChanged(QAbstractButton*);
  void onSide1RequiredValueChanged(double);
  void onSide2RequiredValueChanged(double);
  void onSide1LeafRangeChanged(int);
  void onLeafPairPositionChanged(int, int);
  void onSide2LeafRangeChanged(int);
  void onSide1CurrentValueChanged(int);
  void onSide2CurrentValueChanged(int);
  void onSide1GapChanged(int);
  void onSide2GapChanged(int);

private:
  void fillLeavesControlWidgetContainer(int side1Address, int side2Address,
    int side1Max, int side1CurrentPosition, int side1RequiredPosition,
    int side2Max, int side2CurrentPosition, int side2RequiredPosition);

protected:
  QScopedPointer<qSlicerIhepPairOfLeavesControlDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerIhepPairOfLeavesControlDialog);
  Q_DISABLE_COPY(qSlicerIhepPairOfLeavesControlDialog);

//  Ui::LeavesPairControl *ui;
//  AbstractLeavesWidget* leavesPairWidget{ nullptr };
//  std::pair<int, int> initialPosition;
};

#endif
