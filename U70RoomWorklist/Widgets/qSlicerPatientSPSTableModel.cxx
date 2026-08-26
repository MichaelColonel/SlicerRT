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
#include "qSlicerPatientSPSTableModel.h"

qSlicerPatientSPSTableModel::qSlicerPatientSPSTableModel(QObject *parent) : QAbstractTableModel(parent)
{

}

int qSlicerPatientSPSTableModel::rowCount(const QModelIndex &) const
{
  return this->modalityWorkAndScheduledProcedureStepList.spsSequence.size();
}

int qSlicerPatientSPSTableModel::columnCount(const QModelIndex &) const
{
  return NOF_HEADERS;
}

QVariant qSlicerPatientSPSTableModel::data(const QModelIndex& index, int role) const
{
  const ModalityWork& modalityWork = this->modalityWorkAndScheduledProcedureStepList;

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
    if (index.row() >= modalityWork.spsSequence.size())
    {
      return QVariant();
    }
    if (index.column() >= NOF_HEADERS)
    {
      return QVariant();
    }

    const ModalityWork::ScheduledProcedureStep& sps = modalityWork.spsSequence.at(index.row());

    QVariant res;

    switch (index.column())
    {
    case 0:
      res = modalityWork.RequestedProcedureID;
      break;
    case 1:
      res = sps.Modality;
      break;
    case 2:
      res = sps.StationAeTitle;
      break;
    case 3:
      res = sps.StationName;
      break;
    case 4:
      res = sps.StartDateTime.toString("dd.MM.yyyy hh:mm:ss");
      break;
    case 5:
      res = sps.ID;
      break;
    case 6:
      res = sps.Description;
      break;
    case 7:
      res = sps.Location;
      break;
    case 8:
      res = sps.Status;
      break;
    default:
      res = QVariant();
      break;
    }
    return res;
  }
  return QVariant();
}

QVariant qSlicerPatientSPSTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    return headerAt(section);
  }
  return QVariant();
}

QString qSlicerPatientSPSTableModel::headerAt(int offset) const
{
  if (offset >= 0 && offset < tableHeadersList.size())
  {
    return tableHeadersList.at(offset);
  }
  return QString();
}

void qSlicerPatientSPSTableModel::setScheduledProcedureStepList(const ModalityWork& mwl)
{
  this->beginResetModel();
  this->modalityWorkAndScheduledProcedureStepList = mwl;
  this->endResetModel();
}
