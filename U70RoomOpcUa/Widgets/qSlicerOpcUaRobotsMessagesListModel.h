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

#ifndef __qSlicerOpcUaRobotsMessagesListModel_h
#define __qSlicerOpcUaRobotsMessagesListModel_h

#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

// SlicerQt includes
#include "qSlicerObject.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerRT ScadaOpcUa MRML includes
#include <vtkMRMLScadaOpcUaNode.h>

// Qt
#include <QAbstractListModel>
#include <QIcon>
#include <QString>
#include <QVector>

// FooBar Widgets includes
#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

class Q_SLICER_MODULE_U70ROOMOPCUA_WIDGETS_EXPORT qSlicerOpcUaRobotsMessagesListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  static constexpr size_t MESSAGES_SIZE = 64;
  struct ListItem
  {
    QIcon icon;
    QString text;
  };

  explicit qSlicerOpcUaRobotsMessagesListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  // Public API to fill/modify the model
  void addItem(const QIcon &icon, const QString &text);
  void updateErrorBits(quint64 errorMessages);
  void updateServiceBits(quint64 serviceMessages);
  void updateMiscBits(quint64 miscMessages);
  void updateMessagesBits(quint64 errorMessages, quint64 serviceMessages, quint64 miscMessages);
  void clear();

private:
  static constexpr const char* const ERROR_MESSAGES[MESSAGES_SIZE] = {
    QT_TR_NOOP("PROFISAFE module error in the robot"),
    QT_TR_NOOP("PROFISAFE module error in the robot"),
    QT_TR_NOOP("Error in robot R1: USER_SAFE signal lost during program execution"),
    QT_TR_NOOP("Error in robot R2: USER_SAFE signal lost during program execution"),
    QT_TR_NOOP("Emergency shutdown of one of the power circuit breakers in the switchgear"),
    QT_TR_NOOP("One of the power circuit breakers in the switchgear is disconnected"),
    QT_TR_NOOP("Switchgear switch error"),
    QT_TR_NOOP("Error in the F-DI8/16x24VDC_1 discrete input module"),
    QT_TR_NOOP("Error in the discrete input module F-DI8/16x24VDC_2"),
    QT_TR_NOOP("Error in the discrete output module F-DQ4x24VDC_1"),
    QT_TR_NOOP("Error in the discrete output module F-DQ4x24VDC_2"),
    QT_TR_NOOP("Error in the discrete output module F-DQ4x24VDC_3"),
    QT_TR_NOOP("Emergency stop"),
    QT_TR_NOOP("KUKA R1: Mushroom button pressed on the KUKA remote"),
    QT_TR_NOOP("KUKA R2: Mushroom button pressed on the KUKA remote"),
    QT_TR_NOOP("KUKA R1: ALARM_STOP = FALSE"),
    QT_TR_NOOP("KUKA R2: ALARM_STOP = FALSE"),
    QT_TR_NOOP("KUKA R1: Robot error (details on the KUKA remote control)"),
    QT_TR_NOOP("KUKA R2: Robot error (details on the KUKA remote control)"),
    QT_TR_NOOP("KUKA R1: Continuing the robot program execution is unsafe because the robot is not on the trajectory (ON_PATH signal is missing)"),
    QT_TR_NOOP("KUKA R2: Continuing the robot program execution is unsafe because the robot is not on the trajectory (ON_PATH signal is missing)"),
    QT_TR_NOOP("KUKA R1: To start the automatic operation mode, robot R1 (deck) must be returned to the initial position."),
    QT_TR_NOOP("KUKA R2: To start the automatic operation mode, the R2 robot (C‑arm) must be set to the initial position."),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)")
  };

  static constexpr const char* const SERVICE_MESSAGES[MESSAGES_SIZE] = {
    QT_TR_NOOP("A brake test is required for robot R1 (Table top)"),
    QT_TR_NOOP("A brake test is required for robot R2 (C‑arm)"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages")
  };

  static constexpr const char* const MESSAGES[MESSAGES_SIZE] = {
    QT_TR_NOOP("To operate in automatic mode, the robots must be switched to EXT mode."),
    QT_TR_NOOP("To operate in manual control mode with a PC workstation, both robots must be switched to EXT mode."),
    QT_TR_NOOP("KUKA R1: To run this program, the robot with the table must be moved to the home position."),
    QT_TR_NOOP("KUKA R2: To run this program, the robot with the table must be moved to the home position."),
    QT_TR_NOOP("To operate in service mode, both robots must be switched to EXT mode."),
    QT_TR_NOOP("Robot R1 is not ready for service mode operation."),
    QT_TR_NOOP("Robot R2 is not ready for service mode operation."),
    QT_TR_NOOP("To continue the program, hold down the motion confirmation button."),
    QT_TR_NOOP("KUKA R1 (Diagnostic message): No permission to continue the program."),
    QT_TR_NOOP("KUKA R2 (Diagnostic message): No permission to continue the program."),
    QT_TR_NOOP("Robot movement is impossible due to an unsafe collimator position."),
    QT_TR_NOOP("To operate in the KUKA remote control mode, both robots must be switched to T1 mode."),
    QT_TR_NOOP("It is required to cancel the program in ROBOT R1 (Table top)"),
    QT_TR_NOOP("It is required to cancel the program in ROBOT R2 (C‑arm)"),
    QT_TR_NOOP("It is required to move robot R1 (with a deck) to the HOME position"),
    QT_TR_NOOP("It is required to move robot R2 (with an C‑arm) to the HOME position"),
    QT_TR_NOOP("The evacuation program has been successfully completed"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages")
  };

  void updateItemsList();
  QVector< ListItem > messages;
  quint64 errorMessages;
  quint64 serviceMessages;
  quint64 miscMessages;
};

#endif
