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

#ifndef __qSlicerImagePositioningFooBarWidget_h
#define __qSlicerImagePositioningFooBarWidget_h

// Qt includes
#include <QWidget>
#include <qSlicerWidget.h>

// FooBar Widgets includes
#include "qSlicerImagePositioningModuleWidgetsExport.h"

class qSlicerImagePositioningFooBarWidgetPrivate;
class vtkMRMLScalarVolumeNode;

class Q_SLICER_MODULE_IMAGEPOSITIONING_WIDGETS_EXPORT qSlicerImagePositioningFooBarWidget
  : public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  qSlicerImagePositioningFooBarWidget(QWidget *parent=0);
  ~qSlicerImagePositioningFooBarWidget() override;

  void setXrayNode(vtkMRMLScalarVolumeNode* xrayNode);

protected slots:
  void onMoveUpClicked();
  void onMoveDownClicked();
  void onMoveLeftClicked();
  void onMoveRightClicked();

  void onTranslateSlidersValuesChanged();
  void onTranslateSlidersRangeChanged();

protected:
  QScopedPointer<qSlicerImagePositioningFooBarWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerImagePositioningFooBarWidget);
  Q_DISABLE_COPY(qSlicerImagePositioningFooBarWidget);
};

#endif
