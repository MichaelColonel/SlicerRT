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

#include "ScadaOpcUaModel.h"
#include "ScadaOpcUaTreeItem.h"
#include <QOpcUaClient>
#include <QOpcUaNode>
#include <QIcon>

ScadaOpcUaModel::ScadaOpcUaModel(QObject *parent)
  :
  QAbstractItemModel(parent)
{
}

void ScadaOpcUaModel::setOpcUaClient(QOpcUaClient *client, const QString& rootNodeId)
{
  this->beginResetModel();
  mOpcUaClient = client;

  if (mOpcUaClient)
  {
    mRootItem.reset(new ScadaOpcUaTreeItem(client->node(rootNodeId), this /* model */, nullptr /* parent */));
  }
  else
  {
    mRootItem.reset(nullptr);
  }
  this->endResetModel();
}

void ScadaOpcUaModel::browseStarted()
{
  this->mBrowsesStarted++;
  emit browseProgressChanged(this->mBrowsesFinished, this->mBrowsesStarted);
}

void ScadaOpcUaModel::browseFinished()
{
    this->mBrowsesFinished++;
    emit browseProgressChanged(this->mBrowsesFinished, this->mBrowsesStarted);
}

QOpcUaClient* ScadaOpcUaModel::opcUaClient() const
{
  return mOpcUaClient;
}

QVariant ScadaOpcUaModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  auto item = static_cast< ScadaOpcUaTreeItem* >(index.internalPointer());

  if (role == Qt::DisplayRole)
  {
    return item->data(index.column());
  }
  else if (role == Qt::DecorationRole && index.column() == 0)
  {
    return item->icon(index.column());
  }

  return QVariant();
}

QVariant ScadaOpcUaModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }

  if (orientation == Qt::Horizontal)
  {
    if (section == 0)
    {
      return QString(tr("BrowseName"));
    }
    else if (section == 1)
    {
      return QString(tr("Value"));
    }
    else if (section == 2)
    {
      return QString(tr("NodeClass"));
    }
    else if (section == 3)
    {
      return QString(tr("DataType"));
    }
    else if (section == 4)
    {
      return QString(tr("NodeId"));
    }
    else if (section == 5)
    {
      return QString(tr("DisplayName"));
    }
    else if (section == 6)
    {
      return QString(tr("Description"));
    }
    else
    {
      return QString(tr("Column %1")).arg(section);
    }
  }
  else
  {
    return QString(tr("Row %1")).arg(section);
  }
}

QModelIndex ScadaOpcUaModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!this->hasIndex(row, column, parent))
  {
    return QModelIndex();
  }

  ScadaOpcUaTreeItem *item = nullptr;

  if (!parent.isValid())
  {
    item = mRootItem.get();
  }
  else
  {
    item = static_cast< ScadaOpcUaTreeItem* >(parent.internalPointer())->child(row);
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

QModelIndex ScadaOpcUaModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return QModelIndex();
  }

  auto childItem = static_cast< ScadaOpcUaTreeItem* >(index.internalPointer());
  auto parentItem = childItem->parentItem();

  if (childItem == mRootItem.get() || !parentItem)
  {
    return QModelIndex();
  }

  return createIndex(parentItem->row(), 0, parentItem);
}

int ScadaOpcUaModel::rowCount(const QModelIndex& parent) const
{
  ScadaOpcUaTreeItem *parentItem;
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
    parentItem = static_cast<ScadaOpcUaTreeItem*>(parent.internalPointer());
  }

  if (!parentItem)
  {
    return 0;
  }
  return parentItem->childCount();
}

int ScadaOpcUaModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return static_cast<ScadaOpcUaTreeItem*>(parent.internalPointer())->columnCount();
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

void ScadaOpcUaModel::clear()
{
  this->mBrowsesStarted = 0;
  this->mBrowsesFinished = 0;
}
