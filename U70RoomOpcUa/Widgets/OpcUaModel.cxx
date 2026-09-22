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

#include "OpcUaModel.h"
#include "OpcUaTreeItem.h"
#include <QOpcUaClient>
#include <QOpcUaNode>
#include <QIcon>

OpcUaModel::OpcUaModel(QObject *parent)
  :
  QAbstractItemModel(parent)
{
}

void OpcUaModel::setOpcUaClient(QOpcUaClient *client)
{
  this->beginResetModel();
  mOpcUaClient = client;
  if (mOpcUaClient)
  {
    mRootItem.reset(new OpcUaTreeItem(client->node("ns=0;i=84"), this /* model */, nullptr /* parent */));
  }
  else
  {
    mRootItem.reset(nullptr);
  }
  this->endResetModel();
}

QOpcUaClient* OpcUaModel::opcUaClient() const
{
  return mOpcUaClient;
}

QVariant OpcUaModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  auto item = static_cast<OpcUaTreeItem *>(index.internalPointer());

  if (role == Qt::DisplayRole)
  {
    return item->data(index.column());
  }
  else if (role ==  Qt::DecorationRole && index.column() == 0)
  {
    return item->icon(index.column());
  }

  return QVariant();
}

QVariant OpcUaModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }

  if (orientation == Qt::Horizontal)
  {
    if (section == 0)
    {
      return QString("BrowseName");
    }
    else if (section == 1)
    {
      return QString("Value");
    }
    else if (section == 2)
    {
      return QString("NodeClass");
    }
    else if (section == 3)
    {
      return QString("DataType");
    }
    else if (section == 4)
    {
      return QString("NodeId");
    }
    else if (section == 5)
    {
      return QString("DisplayName");
    }
    else if (section == 6)
    {
      return QString("Description");
    }
    else
    {
      return QString("Column %1").arg(section);
    }
  }
  else
  {
    return QString("Row %1").arg(section);
  }
}

QModelIndex OpcUaModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!this->hasIndex(row, column, parent))
  {
    return QModelIndex();
  }

  OpcUaTreeItem *item = nullptr;

  if (!parent.isValid())
  {
    item = mRootItem.get();
  }
  else
  {
    item = static_cast<OpcUaTreeItem*>(parent.internalPointer())->child(row);
  }

  if (item)
  {
    return createIndex(row, column, item);
  }
  else
  {
    return QModelIndex();
  }
}

QModelIndex OpcUaModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return QModelIndex();
  }

  auto childItem = static_cast<OpcUaTreeItem*>(index.internalPointer());
  auto parentItem = childItem->parentItem();

  if (childItem == mRootItem.get() || !parentItem)
  {
    return QModelIndex();
  }

  return createIndex(parentItem->row(), 0, parentItem);
}

int OpcUaModel::rowCount(const QModelIndex& parent) const
{
  OpcUaTreeItem *parentItem;
  if (!mOpcUaClient)
  {
    return 0;
  }

  if (parent.column() > 0)
  {
    return 0;
  }

  if (!parent.isValid())
  {
    return 1; // only one root item
  }
  else
  {
    parentItem = static_cast<OpcUaTreeItem*>(parent.internalPointer());
  }

  if (!parentItem)
  {
    return 0;
  }
  return parentItem->childCount();
}

int OpcUaModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return static_cast<OpcUaTreeItem*>(parent.internalPointer())->columnCount();
  }
  else if (mRootItem)
  {
    return mRootItem->columnCount();
  }
  else
  {
    return 0;
  }
}
