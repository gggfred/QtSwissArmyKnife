﻿/*
 * Copyright 2018-2023 Qter(qsaker@qq.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 */
#include <QComboBox>
#include <QMetaEnum>
#include <QDesktopServices>
#include <QLoggingCategory>

#include "SAKCommonCrcInterface.hh"
#include "SAKToolCRCAssistant.hh"
#include "ui_SAKToolCRCAssistant.h"

SAKToolCRCAssistant::SAKToolCRCAssistant(QWidget* parent)
    :QWidget(parent)
    ,mLogCategory("CRCAssistant")
    ,mCrcInterface(new SAKCommonCrcInterface)
    ,mUi(new Ui::SAKToolCRCAssistant)
{
    mUi->setupUi(this);
    mWidthComboBox = mUi->comboBoxWidth;
    mWidthComboBox->addItem(QString::number(8));
    mWidthComboBox->addItem(QString::number(16));
    mWidthComboBox->addItem(QString::number(32));
    mWidthComboBox->setEnabled(false);

    mParameterComboBox = mUi->comboBoxName;
    mParameterComboBox->clear();

    mRefinCheckBox = mUi->checkBoxRefIn;
    mRefoutCheckBox = mUi->checkBoxRefOut;
    mPolyLineEdit = mUi->lineEditPoly;
    mInitLineEdit = mUi->lineEditInit;
    mXorLineEdit = mUi->lineEditXOROUT;

    mRefinCheckBox->setEnabled(false);
    mRefoutCheckBox->setEnabled(false);
    mPolyLineEdit->setReadOnly(true);
    mInitLineEdit->setReadOnly(true);
    mXorLineEdit->setReadOnly(true);

    mHexRadioBt = mUi->radioButtonHex;
    mAsciiRadioBt = mUi->radioButtonASCII;

    mHexCRCOutput = mUi->lineEditOutputHex;
    mBinCRCOutput = mUi->lineEditOutputBin;
    mHexCRCOutput->setReadOnly(true);
    mBinCRCOutput->setReadOnly(true);

    mInputTextEdit = mUi->textEdit;

    mCalculatedBt = mUi->pushButtonCalculate;
    mLabelPolyFormula = mUi->labelPolyFormula;
    mLabelInfo = mUi->labelInfo;
    mLabelInfo->installEventFilter(this);
    mLabelInfo->setCursor(QCursor(Qt::PointingHandCursor));

    initParameterModel();
    connect(mParameterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changedParameterModel(int)));
    connect(mCalculatedBt, SIGNAL(clicked()), this, SLOT(calculate()));
    connect(mInputTextEdit, SIGNAL(textChanged()), this, SLOT(textFormatControl()));
}

SAKToolCRCAssistant::~SAKToolCRCAssistant()
{
    QLoggingCategory category(mLogCategory);
    qCInfo(category) << "Goodbye CRCCalculator";
    delete mCrcInterface;
    delete mUi;
}

void SAKToolCRCAssistant::initParameterModel()
{
    mParameterComboBox->clear();
    QStringList list = mCrcInterface->supportedParameterModels();
    mParameterComboBox->addItems(list);

    QMetaEnum models = QMetaEnum::fromType<SAKCommonCrcInterface::SAKEnumCrcModel>();
    bool ok = false;
    int ret = models.keyToValue(mParameterComboBox->currentText().toLatin1().constData(), &ok);
    SAKCommonCrcInterface::SAKEnumCrcModel model = SAKCommonCrcInterface::CRC_8;
    if (ok){
        model = static_cast<SAKCommonCrcInterface::SAKEnumCrcModel>(ret);
    }

    int bitsWidth = mCrcInterface->bitsWidth(model);
    mWidthComboBox->setCurrentIndex(mWidthComboBox->findText(QString::number(bitsWidth)));
    mLabelPolyFormula->setText(mCrcInterface->friendlyPoly(model));
}

void SAKToolCRCAssistant::calculate()
{
    QByteArray inputArray;
    if (mHexRadioBt->isChecked()){
        QString str = mInputTextEdit->toPlainText();
        QStringList strList = str.split(' ');
        char ch;
        for (int i = 0; i < strList.length(); i++){
            if (strList.at(i).isEmpty()){
                continue;
            }
            ch = static_cast<char>(strList.at(i).toInt(Q_NULLPTR, 16));
            inputArray.append(ch);
        }
    }else{
        inputArray = mInputTextEdit->toPlainText().toLatin1();
    }

    if (inputArray.isEmpty()){
        return;
    }

    int bitsWidth = mWidthComboBox->currentText().toInt();
    QMetaEnum models = QMetaEnum::fromType<SAKCommonCrcInterface::SAKEnumCrcModel>();
    bool ok = false;
    int ret = models.keyToValue(mParameterComboBox->currentText().toLatin1().constData(), &ok);
    SAKCommonCrcInterface::SAKEnumCrcModel model = SAKCommonCrcInterface::CRC_8;
    if (ok){
        model = static_cast<SAKCommonCrcInterface::SAKEnumCrcModel>(ret);
    }else{
        Q_ASSERT_X(false, __FUNCTION__, "Unknown crc parameters model!");
    }

    QString crcHexString = "error";
    QString crcBinString = "error";

    if (bitsWidth == 8){
        uint8_t crc = mCrcInterface->crcCalculate<uint8_t>(reinterpret_cast<uint8_t*>(inputArray.data()), static_cast<uint64_t>(inputArray.length()), model);
        crcHexString = QString("0x%1").arg(QString::number(crc, 16), 2, '0');
        crcBinString = QString("%1").arg(QString::number(crc, 2), 8, '0');
    }else if (bitsWidth == 16){
        uint16_t crc = mCrcInterface->crcCalculate<uint16_t>(reinterpret_cast<uint8_t*>(inputArray.data()), static_cast<uint64_t>(inputArray.length()), model);
        crcHexString = QString("0x%1").arg(QString::number(crc, 16), 4, '0');
        crcBinString = QString("%1").arg(QString::number(crc, 2), 16, '0');
    }else if (bitsWidth == 32){
        uint32_t crc = mCrcInterface->crcCalculate<uint32_t>(reinterpret_cast<uint8_t*>(inputArray.data()), static_cast<uint64_t>(inputArray.length()), model);
        crcHexString = QString("0x%1").arg(QString::number(crc, 16), 8, '0');
        crcBinString = QString("%1").arg(QString::number(crc, 2), 32, '0');
    }else {
        qWarning() << "Not supported bits width!";
    }

    mHexCRCOutput->setText(crcHexString);
    mBinCRCOutput->setText(crcBinString);
}

void SAKToolCRCAssistant::textFormatControl()
{
    if (mAsciiRadioBt->isChecked()){
        return;
    }
    disconnect(mInputTextEdit, SIGNAL(textChanged()), this, SLOT(textFormatControl()));

    QString strTemp;
    QString plaintext = mInputTextEdit->toPlainText();
    static QRegularExpression reg("[^0-9a-fA-F]");
    plaintext.remove(reg);
    for (int i = 0; i < plaintext.length(); i++){
        if ((i != 0) && (i % 2 == 0)){
            strTemp.append(QChar(' '));
        }
        strTemp.append(plaintext.at(i));
    }
    mInputTextEdit->setText(strTemp.toUpper());
    mInputTextEdit->moveCursor(QTextCursor::End);

    connect(mInputTextEdit, SIGNAL(textChanged()), this, SLOT(textFormatControl()));
}

void SAKToolCRCAssistant::changedParameterModel(int index)
{
    Q_UNUSED(index)
    QMetaEnum models = QMetaEnum::fromType<SAKCommonCrcInterface::SAKEnumCrcModel>();
    bool ok = false;
    SAKCommonCrcInterface::SAKEnumCrcModel model = SAKCommonCrcInterface::CRC_8;
    int ret = models.keyToValue(mParameterComboBox->currentText().toLatin1().constData(), &ok);
    if (ok){
        model = static_cast<SAKCommonCrcInterface::SAKEnumCrcModel>(ret);
    }else{
        QLoggingCategory category(mLogCategory);
        qCWarning(category) << "Unknown parameter model!";
        Q_ASSERT_X(false, __FUNCTION__, "Unknown parameter model!");
        return;
    }

    int bitsWidth = mCrcInterface->bitsWidth(model);
    mWidthComboBox->setCurrentIndex(mWidthComboBox->findText(QString::number(bitsWidth)));
    mPolyLineEdit->setText(QString("0x%1").arg(QString::number(static_cast<int>(mCrcInterface->poly(model)), 16), bitsWidth/4, '0'));
    mInitLineEdit->setText(QString("0x%1").arg(QString::number(static_cast<int>(mCrcInterface->initialValue(model)), 16), bitsWidth/4, '0'));
    mXorLineEdit->setText(QString("0x%1").arg(QString::number(static_cast<int>(mCrcInterface->xorValue(model)), 16), bitsWidth/4, '0'));
    mRefinCheckBox->setChecked(mCrcInterface->isInputReversal(model));
    mRefoutCheckBox->setChecked(mCrcInterface->isOutputReversal(model));
    mLabelPolyFormula->setText(mCrcInterface->friendlyPoly(model));
}

bool SAKToolCRCAssistant::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick){
        if (watched == mLabelInfo){
            QDesktopServices::openUrl(QUrl(QString("http://www.ip33.com/crc.html")));
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}
