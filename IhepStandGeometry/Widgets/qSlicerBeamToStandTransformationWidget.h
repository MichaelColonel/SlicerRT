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

#ifndef __qSlicerBeamToStandTransformationWidget_h
#define __qSlicerBeamToStandTransformationWidget_h

#include <qSlicerWidget.h>

// IhepStandGeometry Widgets includes
#include "qSlicerIhepStandGeometryModuleWidgetsExport.h"

class qSlicerBeamToStandTransformationWidgetPrivate;

/// \ingroup Slicer_QtModules_IhepStandGeometry
class Q_SLICER_MODULE_IHEPSTANDGEOMETRY_WIDGETS_EXPORT qSlicerBeamToStandTransformationWidget
  : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerBeamToStandTransformationWidget(QWidget *parent=0);
  virtual ~qSlicerBeamToStandTransformationWidget();

protected slots:

protected:
  QScopedPointer<qSlicerBeamToStandTransformationWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerBeamToStandTransformationWidget);
  Q_DISABLE_COPY(qSlicerBeamToStandTransformationWidget);
};

#endif
