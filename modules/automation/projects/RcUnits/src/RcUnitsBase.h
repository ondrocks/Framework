// OFFIS Automation Framework
// Copyright (C) 2013-2014 OFFIS e.V.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef RCUNITSBASE_H
#define RCUNITSBASE_H

#include <QObject>
#include "RcUnitsGlobal.h"
#include <core/RcUnitHelp.h>
#include <lolecs/RcExceptions.h>


class Gamepad;
class LolecInterface;
class RcUnitBase;
class HapticInterface;
class MasterTcInvoker;
class GamepadEndpoint;
class HapticBaseEndpoint;

class RCUNITS_EXPORT RcUnitsBase : public QObject
{
    Q_OBJECT
public:
    explicit RcUnitsBase();
    virtual ~RcUnitsBase();

    RcUnitHelp getHelp(const QString &name);
    TelecontrolConfig getTelecontrolConfig(const QString& name);
    QList<QString> unitNames() { return mBaseUnits.keys(); }
    QList<QString> telecontrolableUnitNames();
    QWidget * lolecGui(const QString &name);
    virtual void loadConfig(const QString &filename);
    virtual void releaseConfig();
    QVariant call(const QByteArray &lolec, const QByteArray &method, const QList<QVariant> &params);
    QVariant getConstants(const QByteArray& lolec);

    void activateGamepad(const QString& unit);
    void deactivateGamepad();
    void updateGamepad(const QString& unit, const QString& methodName, double sensitivity, const QList<bool>& inverts);

    void activateHaptic(const QString& unit);
    void deactivateHaptic();
    void updateHaptic(const QString& unit, const QString &methodName, double sensitivity, double forceScaling, const QList<bool> &inverts);
    QWidget* createHapticWidget();

signals:
    void unitsUpdated();
    void telecontrolUpdated(bool gamepadActive, const QString& controlledUnit);
    void telecontrolSensitivityChangeRequested(const QString& unit, bool increase);
    void hapticUpdated(bool hapticActive, const QString& controlledUnit);

private slots:
    void onGamepadButtonPressed(int buttonId, bool pressed);

protected:
    void loadTcMasters(const QString& configFile);
    void loadTcSensitivity(const QString &name, GamepadEndpoint *gamepadEndpoint, HapticBaseEndpoint *hapticEndpoint, const QString& configFile);

    LolecInterface* loadPlugin(const QString& name, QString *errMsg = 0);
    QMap<QString, RcUnitBase*> mBaseUnits;
    QMap<QString, QString> mTypes;
    QString mLolecDir;
    Gamepad* mGamepad;
    QStringList mTelecontrolLolecs;
    QString mCurrentTelecontrolledUnit, mConfigFile, mCurrentHapticUnit;
    HapticInterface* mHaptic;
    QMap<QString, MasterTcInvoker*> mMasterTcInvokers;
    QStringList mUnitsHiddenforTc;
};

#endif // RCUNITSBASE_H
