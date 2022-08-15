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

#ifndef __qSlicerIhepMlcControlFooBarWidget_h
#define __qSlicerIhepMlcControlFooBarWidget_h

// Qt includes
#include <QWidget>

// FooBar Widgets includes
#include "qSlicerIhepMlcControlModuleWidgetsExport.h"

class qSlicerIhepMlcControlFooBarWidgetPrivate;

/// \ingroup Slicer_QtModules_IhepMlcControl
class Q_SLICER_MODULE_IHEPMLCCONTROL_WIDGETS_EXPORT qSlicerIhepMlcControlFooBarWidget
  : public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  qSlicerIhepMlcControlFooBarWidget(QWidget *parent=0);
  ~qSlicerIhepMlcControlFooBarWidget() override;

protected slots:

protected:
  QScopedPointer<qSlicerIhepMlcControlFooBarWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerIhepMlcControlFooBarWidget);
  Q_DISABLE_COPY(qSlicerIhepMlcControlFooBarWidget);
};

#endif
