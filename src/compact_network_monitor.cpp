/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "compact_network_monitor.h"
#include <DApplicationHelper>
#include <DHiDPIHelper>
#include <DPalette>
#include <QApplication>
#include <QDebug>
#include <QPainter>

#include "constant.h"
#include "dthememanager.h"
#include "smooth_curve_generator.h"
#include "utils.h"

DWIDGET_USE_NAMESPACE

using namespace Utils;

CompactNetworkMonitor::CompactNetworkMonitor(QWidget *parent)
    : QWidget(parent)
{
    int statusBarMaxWidth = Utils::getStatusBarMaxWidth();
    setFixedWidth(statusBarMaxWidth);
    setFixedHeight(160);

    pointsNumber = int(statusBarMaxWidth / 5.4);

    downloadSpeeds = new QList<double>();
    for (int i = 0; i < pointsNumber; i++) {
        downloadSpeeds->append(0);
    }

    uploadSpeeds = new QList<double>();
    for (int i = 0; i < pointsNumber; i++) {
        uploadSpeeds->append(0);
    }
}

CompactNetworkMonitor::~CompactNetworkMonitor()
{
    delete downloadSpeeds;
    delete uploadSpeeds;
}

void CompactNetworkMonitor::updateStatus(long tRecvBytes, long tSentBytes, float tRecvKbs,
                                         float tSentKbs)
{
    totalRecvBytes = tRecvBytes;
    totalSentBytes = tSentBytes;
    totalRecvKbs = tRecvKbs;
    totalSentKbs = tSentKbs;

    // Init download path.
    downloadSpeeds->append(totalRecvKbs);

    if (downloadSpeeds->size() > pointsNumber) {
        downloadSpeeds->pop_front();
    }

    QList<QPointF> downloadPoints;

    double downloadMaxHeight = 0;
    for (int i = 0; i < downloadSpeeds->size(); i++) {
        if (downloadSpeeds->at(i) > downloadMaxHeight) {
            downloadMaxHeight = downloadSpeeds->at(i);
        }
    }

    for (int i = 0; i < downloadSpeeds->size(); i++) {
        if (downloadMaxHeight < downloadRenderMaxHeight) {
            downloadPoints.append(QPointF(i * 5, downloadSpeeds->at(i)));
        } else {
            downloadPoints.append(QPointF(
                i * 5, downloadSpeeds->at(i) * downloadRenderMaxHeight / downloadMaxHeight));
        }
    }

    downloadPath = SmoothCurveGenerator::generateSmoothCurve(downloadPoints);

    // Init upload path.
    uploadSpeeds->append(totalSentKbs);

    if (uploadSpeeds->size() > pointsNumber) {
        uploadSpeeds->pop_front();
    }

    QList<QPointF> uploadPoints;

    double uploadMaxHeight = 0;
    for (int i = 0; i < uploadSpeeds->size(); i++) {
        if (uploadSpeeds->at(i) > uploadMaxHeight) {
            uploadMaxHeight = uploadSpeeds->at(i);
        }
    }

    for (int i = 0; i < uploadSpeeds->size(); i++) {
        if (uploadMaxHeight < uploadRenderMaxHeight) {
            uploadPoints.append(QPointF(i * 5, uploadSpeeds->at(i)));
        } else {
            uploadPoints.append(
                QPointF(i * 5, uploadSpeeds->at(i) * uploadRenderMaxHeight / uploadMaxHeight));
        }
    }

    uploadPath = SmoothCurveGenerator::generateSmoothCurve(uploadPoints);

    repaint();
}

void CompactNetworkMonitor::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();
    // TODO: change color
    textColor = palette.color(DPalette::Text);
    summaryColor = palette.color(DPalette::Text);

    // Draw background grid.
    painter.setRenderHint(QPainter::Antialiasing, false);
    QPen framePen;
    painter.setOpacity(0.1);
    framePen.setColor(QColor(textColor));
    framePen.setWidth(1);
    painter.setPen(framePen);

    int penSize = 1;
    int gridX = rect().x() + penSize;
    int gridY = rect().y() + gridRenderOffsetY + gridPaddingTop;
    int gridWidth = rect().width() - gridPaddingRight - penSize * 2;
    int gridHeight = downloadRenderMaxHeight + uploadRenderMaxHeight + waveformRenderPadding;

    QPainterPath framePath;
    framePath.addRect(QRect(gridX, gridY, gridWidth, gridHeight));
    painter.drawPath(framePath);

    // Draw grid.
    QPen gridPen;
    QVector<qreal> dashes;
    qreal space = 3;
    dashes << 5 << space;
    painter.setOpacity(0.05);
    gridPen.setColor(QColor(textColor));
    gridPen.setWidth(1);
    gridPen.setDashPattern(dashes);
    painter.setPen(gridPen);

    int gridLineX = gridX;
    while (gridLineX < gridX + gridWidth - gridSize) {
        gridLineX += gridSize;
        painter.drawLine(gridLineX, gridY + 1, gridLineX, gridY + gridHeight - 1);
    }
    int gridLineY = gridY;
    while (gridLineY < gridY + gridHeight - gridSize) {
        gridLineY += gridSize;
        painter.drawLine(gridX + 1, gridLineY, gridX + gridWidth - 1, gridLineY);
    }
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw network summary.
    setFontSize(painter, downloadRenderSize);
    QFontMetrics fm = painter.fontMetrics();

    QString downloadTitle = QString("%1 %2").arg(tr("Download")).arg(formatBandwidth(totalRecvKbs));
    QString downloadContent =
        QString("%1 %2").arg(tr("Total")).arg(formatByteCount(totalRecvBytes));
    QString uploadTitle = QString("%1 %2").arg(tr("Upload")).arg(formatBandwidth(totalSentKbs));
    QString uploadContent = QString("%1 %2").arg(tr("Total")).arg(formatByteCount(totalSentBytes));
    int titleWidth = std::max(fm.width(downloadTitle), fm.width(uploadTitle));

    painter.setOpacity(1);
    painter.setPen(QPen(QColor(downloadColor)));
    painter.setBrush(QBrush(QColor(downloadColor)));
    painter.drawEllipse(QPointF(rect().x() + pointerRenderPaddingX,
                                rect().y() + downloadRenderPaddingY + pointerRenderPaddingY),
                        pointerRadius, pointerRadius);

    setFontSize(painter, downloadRenderSize);
    painter.setPen(QPen(QColor(summaryColor)));
    painter.drawText(QRect(rect().x() + downloadRenderPaddingX, rect().y() + downloadRenderPaddingY,
                           fm.width(downloadTitle), rect().height()),
                     Qt::AlignLeft | Qt::AlignTop, downloadTitle);

    setFontSize(painter, downloadRenderSize);
    painter.setPen(QPen(QColor(summaryColor)));
    painter.drawText(
        QRect(rect().x() + downloadRenderPaddingX + titleWidth + textPadding,
              rect().y() + downloadRenderPaddingY, fm.width(downloadContent), rect().height()),
        Qt::AlignLeft | Qt::AlignTop, downloadContent);

    painter.setPen(QPen(QColor(uploadColor)));
    painter.setBrush(QBrush(QColor(uploadColor)));
    painter.drawEllipse(QPointF(rect().x() + pointerRenderPaddingX,
                                rect().y() + uploadRenderPaddingY + pointerRenderPaddingY),
                        pointerRadius, pointerRadius);

    setFontSize(painter, uploadRenderSize);
    painter.setPen(QPen(QColor(summaryColor)));
    painter.drawText(QRect(rect().x() + uploadRenderPaddingX, rect().y() + uploadRenderPaddingY,
                           fm.width(uploadTitle), rect().height()),
                     Qt::AlignLeft | Qt::AlignTop, uploadTitle);

    setFontSize(painter, uploadRenderSize);
    painter.setPen(QPen(QColor(summaryColor)));
    painter.drawText(
        QRect(rect().x() + uploadRenderPaddingX + titleWidth + textPadding,
              rect().y() + uploadRenderPaddingY, fm.width(uploadContent), rect().height()),
        Qt::AlignLeft | Qt::AlignTop, uploadContent);

    painter.translate((rect().width() - pointsNumber * 5) / 2 - 7,
                      downloadWaveformsRenderOffsetY + gridPaddingTop);
    painter.scale(1, -1);

    qreal devicePixelRatio = qApp->devicePixelRatio();
    qreal networkCurveWidth = 1.2;
    if (devicePixelRatio > 1) {
        networkCurveWidth = 2;
    }
    painter.setPen(QPen(QColor(downloadColor), networkCurveWidth));
    painter.setBrush(QBrush());
    painter.drawPath(downloadPath);

    painter.translate(0, uploadWaveformsRenderOffsetY);
    painter.scale(1, -1);

    painter.setPen(QPen(QColor(uploadColor), networkCurveWidth));
    painter.setBrush(QBrush());
    painter.drawPath(uploadPath);
}
