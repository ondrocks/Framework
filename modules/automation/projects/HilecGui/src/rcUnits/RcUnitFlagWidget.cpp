// OFFIS Automation Framework
// Copyright (C) 2013 OFFIS e.V.
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


#include "RcUnitFlagWidget.h"
#include "ui_RcUnitFlagWidget.h"

#include "../HilecSingleton.h"

RcUnitFlagWidget::RcUnitFlagWidget(const RcUnitHelp &help) :
    ui(new Ui::RcUnitFlagWidget)
{
    ui->setupUi(this);
    mHelp = help;
    ui->groupBox->setTitle(help.unitName);
    if(help.type != RobotRcUnitType)
        ui->stopButton->hide();
    else
        ui->stopButton->setEnabled(help.hwConnected);
    if(help.type == BaseRcUnitType)
        ui->connectButton->hide();
    else
        ui->connectButton->setChecked(help.hwConnected);

    foreach(const RcFlagDefinition& def, help.flags)
    {
        QLineEdit* edit = new QLineEdit();
        edit->setMinimumWidth(100);
        edit->setAlignment(Qt::AlignRight);
        ui->formLayout->addRow(def.name, edit);
        mLineEdits << edit;
    }
    connect(this, SIGNAL(acquire(QString)), HilecSingleton::hilec(), SLOT(callRcUnitAcquire(QString)),Qt::QueuedConnection);
    connect(this, SIGNAL(release(QString)), HilecSingleton::hilec(), SLOT(callRcUnitRelease(QString)),Qt::QueuedConnection);
    connect(this, SIGNAL(stop(QString)), HilecSingleton::hilec(), SLOT(callRcUnitStop(QString)),Qt::QueuedConnection);
}

RcUnitFlagWidget::~RcUnitFlagWidget()
{
    delete ui;
}

void RcUnitFlagWidget::updateFlags(const QVariantList &flags)
{
    for(int i=0;i<mLineEdits.size(); i++)
    {
        QVariant data = flags.value(i);
        RcFlagDefinition def = mHelp.flags.value(i);
        QLineEdit* line = mLineEdits[i];
        QString content;
        if(data.canConvert(QVariant::Double))
            content = QString::number(data.toDouble(), 'g', def.decimalPlaces);
        else
            content = data.toString();
        if(!def.unit.isEmpty())
            content += " " + def.unit;
        line->setText(content);
    }
}

void RcUnitFlagWidget::unitStatusChanged(bool acquired)
{
    ui->connectButton->setChecked(acquired);
    ui->stopButton->setEnabled(acquired);
    ui->connectButton->setEnabled(true);
}

void RcUnitFlagWidget::on_connectButton_clicked(bool checked)
{
    ui->connectButton->setEnabled(false);
    if(checked)
        emit acquire(mHelp.unitName);
    else
        emit release(mHelp.unitName);
}

void RcUnitFlagWidget::on_stopButton_clicked()
{

}
