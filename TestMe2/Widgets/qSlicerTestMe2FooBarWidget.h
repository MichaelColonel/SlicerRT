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

#ifndef __qSlicerTestMe2FooBarWidget_h
#define __qSlicerTestMe2FooBarWidget_h

// Qt includes
#include <QWidget>

// FooBar Widgets includes
#include "qSlicerTestMe2ModuleWidgetsExport.h"

class qSlicerTestMe2FooBarWidgetPrivate;

class Q_SLICER_MODULE_TESTME2_WIDGETS_EXPORT qSlicerTestMe2FooBarWidget
  : public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  qSlicerTestMe2FooBarWidget(QWidget *parent=0);
  ~qSlicerTestMe2FooBarWidget() override;

protected slots:

protected:
  QScopedPointer<qSlicerTestMe2FooBarWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTestMe2FooBarWidget);
  Q_DISABLE_COPY(qSlicerTestMe2FooBarWidget);
};

#endif
