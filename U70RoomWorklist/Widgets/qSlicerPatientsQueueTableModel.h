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

#ifndef __qSlicerPatientsQueueTableModel_h
#define __qSlicerPatientsQueueTableModel_h

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

struct Q_SLICER_MODULE_U70ROOMWORKLIST_WIDGETS_EXPORT ModalityWork
{
  bool addWorkFromDicomResponse(QRResponse* retresp);
  void clear();

  QString AccessionNumber;
  QString PatientName;
  QString PatientID;
  QString IssuerOfPatientID;
  QDate PatientBirthDate;
  QString PatientSex;
  QString StudyInstanceUID;
  QString RequestedProcedureDescription;
  QString RequestedProcedureID;
  struct Q_SLICER_MODULE_U70ROOMWORKLIST_WIDGETS_EXPORT ScheduledProcedureStep
  {
    bool fillDataFromDicomItem(DcmItem* spsSequenceItem);
    QString Modality;
    QString StationAeTitle;
    QDateTime StartDateTime;
    QString Description;
    QString ID;
    QString StationName;
    QString Location;
    QString Status;
  };
  QList< ScheduledProcedureStep > spsSequence;
};

class Q_SLICER_MODULE_U70ROOMWORKLIST_WIDGETS_EXPORT qSlicerPatientsQueueTableModel
  : public QAbstractTableModel
{
  Q_OBJECT
public:
  static constexpr int NOF_HEADERS = 7;
  qSlicerPatientsQueueTableModel(QObject *parent = nullptr);
  void setModalityWorkList(const QList< ModalityWork >& mwl);
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
protected:
  QString headerAt(int offset) const;
  QList< ModalityWork > modalityWorkList;
  const QStringList tableHeadersList{
    "Patient\nID",
    "Patient\nName",
    "Patient\nBirth Date",
    "Accession\nNumber",
    "Requested\nProcedure\nID",
    "Requested\nProcedure\nDescription",
    "Study\nInstance\nUID"
  };
};

#endif
