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

#ifndef __qSlicerDrrImageComputationRtkParametersWidget_h
#define __qSlicerDrrImageComputationRtkParametersWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// DrrImageComputation Widgets includes
#include "qSlicerDrrImageComputationModuleWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLDrrImageComputationNode;
class qSlicerDrrImageComputationRtkParametersWidgetPrivate;

/// \ingroup Slicer_QtModules_DrrImageComputation
class Q_SLICER_MODULE_DRRIMAGECOMPUTATION_WIDGETS_EXPORT qSlicerDrrImageComputationRtkParametersWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QWidget Superclass;
  qSlicerDrrImageComputationRtkParametersWidget(QWidget *parent=0);
  ~qSlicerDrrImageComputationRtkParametersWidget() override;

public slots:
  /// Set DrrImageComputation MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();

protected slots:
  /// Type of RTK forward projection filter
  void onForwardProjectionFilterChanged(int);
  void onForwardProjectionFilterParameterChanged(double);
  void onFillPadValueChanged(double);

protected:
  QScopedPointer<qSlicerDrrImageComputationRtkParametersWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDrrImageComputationRtkParametersWidget);
  Q_DISABLE_COPY(qSlicerDrrImageComputationRtkParametersWidget);
};

#endif
