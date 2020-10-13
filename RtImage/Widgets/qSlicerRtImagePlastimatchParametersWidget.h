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

#ifndef __qSlicerRtImagePlastimatchParametersWidget_h
#define __qSlicerRtImagePlastimatchParametersWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// RtImage Widgets includes
#include "qSlicerRtImageModuleWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLRTImageNode;
class qSlicerRtImagePlastimatchParametersWidgetPrivate;

/// \ingroup Slicer_QtModules_RtImage
class Q_SLICER_MODULE_RTIMAGE_WIDGETS_EXPORT qSlicerRtImagePlastimatchParametersWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QWidget Superclass;
  qSlicerRtImagePlastimatchParametersWidget(QWidget *parent=0);
  virtual ~qSlicerRtImagePlastimatchParametersWidget();

public slots:
  /// Set RT Image MRML node (Parameter node)
  void setRtImageNode(vtkMRMLNode* node);
  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();

protected slots:
  /// Exponential mapping flag
  void onUseExponentialMappingToggled(bool);
  /// Autoscale flag
  void onAutoscalePixelsRangeToggled(bool);
  /// Type of reconstruct algorithm
  void onReconstructionAlgorithmChanged(int);
  /// Type of computation threading
  void onThreadingChanged(int);
  /// Type Hounsfield Units conversion
  void onHUConversionChanged(int);
  void onAutoscaleIntensityRangeChanged(double, double);

protected:
  QScopedPointer<qSlicerRtImagePlastimatchParametersWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerRtImagePlastimatchParametersWidget);
  Q_DISABLE_COPY(qSlicerRtImagePlastimatchParametersWidget);
};

#endif
