﻿/******************************************************************************
 * Copyright 2018-2023 Qter(qsaker@qq.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 *****************************************************************************/
#include <QDebug>
#include <QEventLoop>
#include <QApplication>

#include "SAKTestDebuggerDevice.hh"
#include "SAKCommonDataStructure.hh"

SAKTestDebuggerDevice::SAKTestDebuggerDevice(QSettings *settings,
                                             const QString &settingsGroup,
                                             QWidget *uiParent,
                                             QObject *parent)
    :SAKDebuggerDevice(settings, settingsGroup, uiParent, parent)
    ,mReadDataTimerId(-1)
    ,mWriteDateTimerId(-1)
{
    connect(this, &SAKTestDebuggerDevice::finished, this, [=](){
        if (mReadDataTimerId != -1) {
            killTimer(this->mReadDataTimerId);
            this->mReadDataTimerId = -1;
        }

        if (mWriteDateTimerId != -1) {
            killTimer(this->mWriteDateTimerId);
            this->mWriteDateTimerId = -1;
        }
    });
}

SAKTestDebuggerDevice::~SAKTestDebuggerDevice()
{

}

void SAKTestDebuggerDevice::generateReadData(bool start, int interval)
{
    if (mReadDataTimerId != -1) {
        killTimer(mReadDataTimerId);
        mReadDataTimerId = -1;
    }

    if (start) {
        mReadDataTimerId = startTimer(interval > 50 ? interval : 50);
    }
}

void SAKTestDebuggerDevice::generateWriteData(bool start, int interval)
{
    if (mWriteDateTimerId != -1) {
        killTimer(mWriteDateTimerId);
        mWriteDateTimerId = -1;
    }

    if (start) {
        mWriteDateTimerId = startTimer(interval > 50 ? interval : 50);
    }
}

bool SAKTestDebuggerDevice::initialize()
{
    auto ctx = parametersContext()
            .value<SAKCommonDataStructure::SAKStructTestParametersContext>();
    bool openFailed = ctx.openFailed;
    QString msg = ctx.errorString;

    if (openFailed) {
        emit errorOccurred(msg);
    }

    return !openFailed;
}

SAKDebuggerDevice::ReadContextVector SAKTestDebuggerDevice::read()
{
    typedef SAKCommonDataStructure::SAKStructTestParametersContext Parameters;
    auto paras = parametersContext().value<Parameters>();
    ReadContext ctx;
    ctx.bytes = paras.rxData.toUtf8();
    ctx.flag = QString("self");
    return ReadContextVector() << ctx;
}

SAKDebuggerDevice::WriteContext
SAKTestDebuggerDevice::write(const QByteArray &bytes)
{
    WriteContext ctx;
    ctx.bytes = bytes;
    ctx.flag = QString("self");
    return ctx;
}

void SAKTestDebuggerDevice::uninitialize()
{
    // Nothing to do.
}

void SAKTestDebuggerDevice::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mReadDataTimerId) {
        emit readyRead(SAKDebuggerDevice::SAKDeviceProtectedSignal());
    } else if (event->timerId() == mWriteDateTimerId) {
        typedef SAKCommonDataStructure::SAKStructTestParametersContext Parameters;
        auto paras = parametersContext().value<Parameters>();
        writeBytes(paras.txData.toUtf8());
    }

    SAKDebuggerDevice::timerEvent(event);
}
