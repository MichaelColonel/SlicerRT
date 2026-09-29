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

#ifndef __qSlicerU70RoomBeamsFooBarWidget_h
#define __qSlicerU70RoomBeamsFooBarWidget_h

// Qt includes
#include <QWidget>

// FooBar Widgets includes
#include "qSlicerU70RoomBeamsModuleWidgetsExport.h"

class qSlicerU70RoomBeamsFooBarWidgetPrivate;

class Q_SLICER_MODULE_U70ROOMBEAMS_WIDGETS_EXPORT qSlicerU70RoomBeamsFooBarWidget : public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  qSlicerU70RoomBeamsFooBarWidget(QWidget* parent = 0);
  ~qSlicerU70RoomBeamsFooBarWidget() override;

protected slots:

protected:
  QScopedPointer<qSlicerU70RoomBeamsFooBarWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerU70RoomBeamsFooBarWidget);
  Q_DISABLE_COPY(qSlicerU70RoomBeamsFooBarWidget);
};

#endif
