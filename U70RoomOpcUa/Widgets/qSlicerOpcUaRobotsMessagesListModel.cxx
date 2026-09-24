/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qSlicerOpcUaRobotsMessagesListModel.h"

#include <bitset>

int qSlicerOpcUaRobotsMessagesListModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
   {
    return 0;
  }
  return messages.size();
}

QVariant qSlicerOpcUaRobotsMessagesListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || index.row() >= messages.size())
  {
    return QVariant();
  }
    
  const ListItem &item = messages.at(index.row());
  QVariant res;
  switch (role)
  {
  case Qt::DecorationRole:
    res = item.icon;   // QIcon works directly with Image in QML
    break;
  case Qt::DisplayRole:
    res = item.text;
    break;
  default:
    break;
  }
  return res;
}

QHash< int, QByteArray > qSlicerOpcUaRobotsMessagesListModel::roleNames() const
{
  return {
    { Qt::DecorationRole, "iconSource" },
    { Qt::DisplayRole, "labelText"  }
  };
}

void qSlicerOpcUaRobotsMessagesListModel::addItem(const QIcon &icon, const QString &text)
{
  beginInsertRows(QModelIndex(), messages.size(), messages.size());
  messages.append({ icon, text });
  endInsertRows();
}

void qSlicerOpcUaRobotsMessagesListModel::updateErrorBits(quint64 errorMsg)
{
  errorMessages = errorMsg;
  this->updateItemsList();
}

void qSlicerOpcUaRobotsMessagesListModel::updateServiceBits(quint64 serviceMsg)
{
  serviceMessages = serviceMsg;
  this->updateItemsList();
}

void qSlicerOpcUaRobotsMessagesListModel::updateMiscBits(quint64 miscMsg)
{
  miscMessages = miscMsg;
  this->updateItemsList();
}

void qSlicerOpcUaRobotsMessagesListModel::updateMessagesBits(quint64 errorMsg, quint64 serviceMsg, quint64 miscMsg)
{
  errorMessages = errorMsg;
  serviceMessages = serviceMsg;
  miscMessages = miscMsg;

  this->updateItemsList();
}

void qSlicerOpcUaRobotsMessagesListModel::updateItemsList()
{
  beginResetModel();
  messages.clear();
  std::bitset< MESSAGES_SIZE > errr(errorMessages);
  std::bitset< MESSAGES_SIZE > serv(serviceMessages);
  std::bitset< MESSAGES_SIZE > misc(miscMessages);
  for (size_t i = 0; i < MESSAGES_SIZE; ++i)
  {
    if (errr.test(i))
    {
     ListItem errMsgItem;
     errMsgItem.icon = QPixmap(":/Icons/red.png");
     errMsgItem.text = QObject::tr("Error %1").arg(i);
     messages.push_back(errMsgItem);
    }
  }
  for (size_t i = 0; i < MESSAGES_SIZE; ++i)
  {
    if (serv.test(i))
    {
     ListItem servMsgItem;
     servMsgItem.icon = QPixmap(":/Icons/green.png");
     servMsgItem.text = QObject::tr("Service %1").arg(i);
     messages.push_back(servMsgItem);
    }
  }
  for (size_t i = 0; i < MESSAGES_SIZE; ++i)
  {
    if (misc.test(i))
    {
     ListItem miscMsgItem;
     miscMsgItem.icon = QPixmap(":/Icons/gray.png");
     miscMsgItem.text = QObject::tr("Misc. %1").arg(i);
     messages.push_back(miscMsgItem);
    }
  }
  endResetModel();
}

void qSlicerOpcUaRobotsMessagesListModel::clear()
{
  beginResetModel();
  messages.clear();
  endResetModel();
}
