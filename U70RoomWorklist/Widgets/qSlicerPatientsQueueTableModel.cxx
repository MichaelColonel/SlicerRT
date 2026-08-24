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

// Qt includes
#include <QDebug>

// U70RoomWorklist Widgets includes
#include "qSlicerPatientsQueueTableModel.h"

// DCMTK includes
#include <dcmtk/dcmnet/scu.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/dcmdata/dcsequen.h>

namespace
{

QString getTagValueAsString(DcmItem* item, const DcmTagKey& tagkey, bool& res)
{
  if (!item)
  {
    qWarning() << Q_FUNC_INFO << "DcmItem is invalid";
    res = false;
    return QString();
  }
  OFString str;
  OFCondition cond;
  if (item->tagExistsWithValue(tagkey))
  {
    cond = item->findAndGetOFString(tagkey, str);
    if (cond.good())
    {
      res = true;
      return QString::fromUtf8(str.c_str());
    }
    else
    {
      qWarning() << Q_FUNC_INFO << "TagKey: " << QString::fromLatin1(tagkey.toString().c_str()) << " value is bad";
    }
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "TagKey: " << QString::fromLatin1(tagkey.toString().c_str()) << " doesn't exist or has no value";
  }
  res = false;
  return QString();
}

QDate getTagValueAsDate(DcmItem* item, const DcmTagKey& tagkey, bool& res)
{
  if (!item)
  {
    qWarning() << Q_FUNC_INFO << "DcmItem is invalid";
    res = false;
    return QDate();
  }
  OFString str;
  OFCondition cond;
  if (item->tagExistsWithValue(tagkey))
  {
    cond = item->findAndGetOFString(tagkey, str);
    if (cond.good())
    {
      res = true;
      QString dateStr = QString::fromUtf8(str.c_str());
      return QDate::fromString(dateStr, QString("yyyyMMdd"));
    }
    else
    {
      qWarning() << Q_FUNC_INFO << "TagKey: " << QString::fromLatin1(tagkey.toString().c_str()) << " value is bad";
    }
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "TagKey: " << QString::fromLatin1(tagkey.toString().c_str()) << " doesn't exist or has no value";
  }
  res = false;
  return QDate();
}

QTime getTagValueAsTime(DcmItem* item, const DcmTagKey& tagkey, bool& res)
{
  if (!item)
  {
    qWarning() << Q_FUNC_INFO << "DcmItem is invalid";
    res = false;
    return QTime();
  }
  OFString str;
  OFCondition cond;
  if (item->tagExistsWithValue(tagkey))
  {
    cond = item->findAndGetOFString(tagkey, str);
    if (cond.good())
    {
      res = true;
      QString timeStr = QString::fromUtf8(str.c_str());
      return QTime::fromString(timeStr, QString("HHmmss"));
    }
    else
    {
      qWarning() << Q_FUNC_INFO << "TagKey: " << QString::fromLatin1(tagkey.toString().c_str()) << " value is bad";
    }
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "TagKey: " << QString::fromLatin1(tagkey.toString().c_str()) << " doesn't exist or has no value";
  }
  res = false;
  return QTime();
}

}

bool ModalityWork::addWorkFromDicomResponse(QRResponse* resp)
{
  if (!resp)
  {
    return false;
  }
  if (!resp->m_dataset)
  {
    return false;
  }
  bool res = false;
  this->PatientName = getTagValueAsString(resp->m_dataset, DCM_PatientName, res);
  this->PatientID = getTagValueAsString(resp->m_dataset, DCM_PatientID, res);
  this->PatientSex = getTagValueAsString(resp->m_dataset, DCM_PatientSex, res);
  this->AccessionNumber = getTagValueAsString(resp->m_dataset, DCM_AccessionNumber, res);
  this->PatientBirthDate = getTagValueAsDate(resp->m_dataset, DCM_PatientBirthDate, res);
  this->StudyInstanceUID = getTagValueAsString(resp->m_dataset, DCM_StudyInstanceUID, res);
  this->RequestedProcedureDescription = getTagValueAsString(resp->m_dataset, DCM_RequestedProcedureDescription, res);
  this->RequestedProcedureID = getTagValueAsString(resp->m_dataset, DCM_RequestedProcedureID, res);

  DcmSequenceOfItems* spsSequence = nullptr;
  DcmItem* spsSequenceItem = nullptr;
  OFCondition cond = resp->m_dataset->findAndGetSequence(DCM_ScheduledProcedureStepSequence, spsSequence);
  if (cond.good() && spsSequence)
  {
    unsigned long items = spsSequence->getNumberOfValues();
    for (unsigned long i = 0; i < items; ++i)
    {
      ModalityWork::ScheduledProcedureStep spsData;

      spsSequenceItem = spsSequence->getItem(i);
      if (spsData.fillDataFromDicomItem(spsSequenceItem))
      {
        this->spsSequence.push_back(spsData);
      }
    }
  }
  return res;
}

void ModalityWork::clear()
{
}

bool ModalityWork::ScheduledProcedureStep::fillDataFromDicomItem(DcmItem* spsSequenceItem)
{
  if (!spsSequenceItem)
  {
    return false;
  }

  bool res = false;
  this->Modality = getTagValueAsString(spsSequenceItem, DCM_Modality, res);
  this->StationAeTitle = getTagValueAsString(spsSequenceItem, DCM_ScheduledStationAETitle, res);

  QDate startdate = getTagValueAsDate(spsSequenceItem, DCM_ScheduledProcedureStepStartDate, res);
  if (res)
  {
    this->StartDateTime.setDate(startdate);
  }

  QTime starttime = getTagValueAsTime(spsSequenceItem, DCM_ScheduledProcedureStepStartTime, res);
  if (res)
  {
    this->StartDateTime.setTime(starttime);
  }

  this->ID = getTagValueAsString(spsSequenceItem, DCM_ScheduledProcedureStepID, res);
  this->StationName = getTagValueAsString(spsSequenceItem, DCM_ScheduledStationName, res);
  this->Description = getTagValueAsString(spsSequenceItem, DCM_ScheduledProcedureStepDescription, res);
  this->Location = getTagValueAsString(spsSequenceItem, DCM_ScheduledProcedureStepLocation, res);
  this->Status = getTagValueAsString(spsSequenceItem, DCM_ScheduledProcedureStepStatus, res);

  return res;
}

qSlicerPatientsQueueTableModel::qSlicerPatientsQueueTableModel(QObject *parent) : QAbstractTableModel(parent)
{

}

int qSlicerPatientsQueueTableModel::rowCount(const QModelIndex &) const
{
  return this->modalityWorkList.size();
}

int qSlicerPatientsQueueTableModel::columnCount(const QModelIndex &) const
{
  return NOF_HEADERS;
}

QVariant qSlicerPatientsQueueTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }
  if (role == Qt::TextAlignmentRole)
  {
    return int(Qt::AlignHCenter | Qt::AlignVCenter);
  }
  if (role == Qt::DisplayRole)
  {
    if (index.row() >= this->modalityWorkList.size())
    {
      return QVariant();
    }
    if (index.column() >= NOF_HEADERS)
    {
      return QVariant();
    }
/*    double time = -1.;
    BeamProfileArray prof{ 0., 0., -1., -1.};
    int i = 0;
    for (auto iter = this->beamProfileMap.begin(); iter != this->beamProfileMap.end(); ++iter, ++i)
    {
      if (i == index.row())
      {
        time = iter->first;
        prof = iter->second;
      }
    }
*/
    QVariant res;
/*
    switch (index.column())
    {
    case 0:
      res = QString::number(time);
      break;
    case 1:
      {
        if (prof[2] < 0)
        {
          res = tr("No beam");
        }
        else
        {
          res = QObject::tr("%1").arg(-1. * prof[0], 5, 'f', 1, QChar(' ')); // position X
        }
      }
      break;
    case 2:
      {
        if (prof[2] < 0)
        {
          res = tr("No beam");
        }
        else
        {
          res = QObject::tr("%1").arg(prof[1], 5, 'f', 1, QChar(' ')); // position Y
        }
      }
      break;
    case 3:
      {
        if (prof[2] < 0)
        {
          res = tr("No beam");
        }
        else
        {
          res = QObject::tr("%1").arg(6. * prof[2], 5, 'f', 1, QChar(' ')); // Diameter Dx (== 6sigma)
        }
      }
      break;
    case 4:
      {
        if (prof[2] < 0)
        {
          res = tr("No beam");
        }
        else
        {
          res = QObject::tr("%1").arg(6. * prof[3], 5, 'f', 1, QChar(' ')); // Diameter Dy (== 6sigma)
        }
      }
      break;
    default:
      res = QVariant();
      break;
    }
*/
    return res;
  }
  return QVariant();
}

QVariant qSlicerPatientsQueueTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    return headerAt(section);
  }
  return QVariant();
}

QString qSlicerPatientsQueueTableModel::headerAt(int offset) const
{
  if (offset >= 0 && offset < tableHeadersList.size())
  {
    return tableHeadersList.at(offset);
  }
  return QString();
}

void qSlicerPatientsQueueTableModel::setModalityWorkList(const QList< ModalityWork >& mwl)
{
  this->beginResetModel();
  this->modalityWorkList = mwl;
  this->endResetModel();
}
