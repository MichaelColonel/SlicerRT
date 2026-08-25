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

#ifndef __qSlicerPatientSPSTableModel_h
#define __qSlicerPatientSPSTableModel_h

// Qt includes
#include <QAbstractTableModel>
#include <QDate>
#include <QDateTime>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// DCMTK includes
#include <dcmtk/dcmnet/scu.h>
#include <dcmtk/dcmdata/dcitem.h>

// U70RoomWorklist Widgets includes
#include "qSlicerU70RoomWorklistModuleWidgetsExport.h"
#include "qSlicerPatientsQueueTableModel.h"

class Q_SLICER_MODULE_U70ROOMWORKLIST_WIDGETS_EXPORT qSlicerPatientSPSTableModel
  : public QAbstractTableModel
{
  Q_OBJECT
public:
  static constexpr int NOF_HEADERS = 9;
  qSlicerPatientSPSTableModel(QObject *parent = nullptr);
  void setScheduledProcedureStepList(const ModalityWork& mwAndSpsList);
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
protected:
  QString headerAt(int offset) const;
  ModalityWork modalityWorkAndScheduledProcedureStepList;
  const QStringList tableHeadersList{
    "Requested\nProcedure\nID",
    "Modality",
    "Scheduled\nStation\nAE Title",
    "Scheduled\nStation\nName",
    "SPS Start\nDate & Time",
    "SPS ID",
    "SPS Description",
    "SPS Location",
    "SPS Status"
  };
};

#endif
