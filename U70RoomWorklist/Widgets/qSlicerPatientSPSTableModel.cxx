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
    if (index.row() >= this->modalityWorkAndScheduledProcedureStepList.spsSequence.size())
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
