/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "Painter.h"

#include <QPainter>
#include <QPaintEvent>
#include <QWidget>

#include "landXY.h"

Painter::Painter()
{
    textFont.setPixelSize(20);
    epiFont.setPixelSize(15);
    textPen = QPen(Qt::black);
    backImage.load(":/images/skorea.png");
    backImage = backImage.scaled(SMALL_MAP_WIDTH, SMALL_MAP_HEIGHT, Qt::KeepAspectRatio);

    initPoints();
}

void Painter::initPoints()
{
    points.clear();

    for(int i=0;i<LANDXYCNT_VERSION_1;i++)
    {
        _POINT mypoint;
        mypoint.landX = landXY[i][0];
        mypoint.landY = landXY[i][1];

        points.append(mypoint);
    }
}

void Painter::paint(QPainter *painter, QPaintEvent *event,
                    _BINARY_SMALL_EEWLIST_PACKET myeewp, _BINARY_POINT_PACKET mypoint, _BINARY_PGA_PACKET mystation)
{
    QString chanS;
    if(chanID == 0) chanS = "East/West PGA";
    else if(chanID == 1) chanS = "North/South PGA";
    else if(chanID == 2) chanS = "Up/Down PGA";
    else if(chanID == 3) chanS = "Horizontal PGA";
    else if(chanID == 4) chanS = "Total(3-Axis) PGA";

    isEvent = false;

    if(legendType == 0)
    {
        backImage.load(":/images/skorea.png");
        backImage = backImage.scaled(SMALL_MAP_WIDTH, SMALL_MAP_HEIGHT, Qt::KeepAspectRatio);
    }
    else if(legendType == 1)
    {
        backImage.load(":/images/skorea_without_water.png");
        backImage = backImage.scaled(SMALL_MAP_WIDTH, SMALL_MAP_HEIGHT, Qt::KeepAspectRatio);
    }

    painter->fillRect(event->rect(), backImage);
    painter->save();

    if(myeewp.numEEW != 0)
    {
        for(int i=0;i<myeewp.numEEW;i++)
        {
            _eewInfo = myeewp.eewInfos[i];

            if(dataType == 0)
            {
                if(mypoint.dataTime >= _eewInfo.origintime &&
                        mypoint.dataTime < _eewInfo.origintime + EVENT_DURATION)
                {
                    isEvent = true;
                    break;
                }
            }
            else if(dataType == 1)
            {
                if(mystation.dataTime >= _eewInfo.origintime &&
                        mystation.dataTime < _eewInfo.origintime + EVENT_DURATION)
                {
                    isEvent = true;
                    break;
                }
            }
        }
    }

    if(isEvent)
    {
        if(dataType == 0)
        {
            // if maxPGAList is empty then insert staList into maxPGAList
            if(maxMapZ.isEmpty())
            {
                for(int i=0;i<LANDXYCNT_VERSION_1;i++)
                {
                    maxMapZ.append(mypoint.mapZ[i]);
                }
            }
            else
            {
                for(int i=0;i<LANDXYCNT_VERSION_1;i++)
                {
                    if(mypoint.mapZ[i] >= maxMapZ.at(i))
                        maxMapZ.replace(i, mypoint.mapZ[i]);
                }
            }

            for(int i=0;i<LANDXYCNT_VERSION_1;i++)
            {
                QColor col;
                float value = maxMapZ.at(i);
                if(legendType == 0)
                    col.setRgb(redColor(value), greenColor(value), blueColor(value));
                else if(legendType == 1)
                    col = getGradientColorfromGal(value);
                QPen pen = QPen(col);
                painter->setPen(pen);
                painter->drawPoint(QPoint(points.at(i).landX, points.at(i).landY));
            }
        }
        else if(dataType == 1)
        {
            // if maxPGAList is empty then insert staList into maxPGAList
            if(maxPGAList.isEmpty())
            {
                for(int i=0;i<mystation.numStation;i++)
                {
                    _STATION sta = mystation.staList[i];
                    maxPGAList.append(sta);
                }
            }
            else
            {
                for(int i=0;i<mystation.numStation;i++)
                {
                    _STATION sta = mystation.staList[i];
                    bool needInsert = true;
                    for(int j=0;j<maxPGAList.size();j++)
                    {
                        _STATION maxsta = maxPGAList.at(j);
                        if(QString(sta.netSta).startsWith(maxsta.netSta))
                        {
                            if(sta.pga > maxsta.pga) maxsta.pga[chanID] = sta.pga[chanID];

                            needInsert = false;
                            maxPGAList.replace(j, maxsta);
                            break;
                        }
                    }
                    if(needInsert)
                        maxPGAList.append(sta);
                }
            }

            // draw max pga
            for(int i=0;i<maxPGAList.size();i++)
            {
                _STATION sta = maxPGAList.at(i);
                QColor col;
                if(legendType == 0)
                    col.setRgb(redColor(sta.pga[chanID]), greenColor(sta.pga[chanID]), blueColor(sta.pga[chanID]));
                else if(legendType == 1)
                    col = getGradientColorfromGal(sta.pga[chanID]);
                QBrush brush = QBrush(col);
                painter->setBrush(brush);
                painter->drawEllipse(QPoint(sta.smapX, sta.smapY), 5, 5);
            }
        }

        // draw epicenter
        QBrush brush = QBrush(QColor(Qt::red));
        painter->setBrush(brush);
        painter->setPen(textPen);
        painter->drawEllipse(QPoint(_eewInfo.smapX, _eewInfo.smapY), 8, 8);
        painter->setBrush(Qt::white);
        painter->drawRect(QRect(_eewInfo.smapX - 20, _eewInfo.smapY + 12, 40, 16));
        painter->setFont(epiFont);
        painter->drawText(QRect(_eewInfo.smapX - 25, _eewInfo.smapY + 5, 50, 30), Qt::AlignCenter, "M" + QString::number(_eewInfo.magnitude, 'f', 1));

        int eqFlowTimeSec;

        if(dataType == 0)
            eqFlowTimeSec = mypoint.dataTime - _eewInfo.origintime;
        else if(dataType == 1)
            eqFlowTimeSec = mystation.dataTime - _eewInfo.origintime;

        qreal radiusP = eqFlowTimeSec * P_VEL;
        qreal radiusS = eqFlowTimeSec * S_VEL;
        painter->setPen(Qt::blue);
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(_eewInfo.smapX, _eewInfo.smapY), radiusP, radiusP);
        painter->setPen(Qt::red);
        painter->drawEllipse(QPointF(_eewInfo.smapX, _eewInfo.smapY), radiusS, radiusS);

        painter->setPen(textPen);
        painter->setFont(textFont);

        QDateTime etKST;
        etKST.setTimeSpec(Qt::UTC);
        etKST.setTime_t(_eewInfo.origintime);
        etKST = convertKST(etKST);

        painter->drawText(QRect(500, 95, 300, 25), Qt::AlignRight, "EEW ID:" + QString::number(_eewInfo.eew_evid));
        painter->drawText(QRect(500, 120, 300, 25), Qt::AlignRight, etKST.toString("yyyy-MM-dd hh:mm:ss") + " (KST)");
        painter->drawText(QRect(500, 145, 300, 25), Qt::AlignRight, "M" + QString::number(_eewInfo.magnitude, 'f', 1));

    }
    else
    {
        maxMapZ.clear();
        maxPGAList.clear();

        if(dataType == 0)
        {
            if(mypoint.dataTime != 0)
            {
                for(int i=0;i<LANDXYCNT_VERSION_1;i++)
                {
                    QColor col;
                    if(legendType == 0)
                        col.setRgb(redColor(mypoint.mapZ[i]), greenColor(mypoint.mapZ[i]), blueColor(mypoint.mapZ[i]));
                    else if(legendType == 1)
                        col = getGradientColorfromGal(mypoint.mapZ[i]);
                    QPen pen = QPen(col);
                    painter->setPen(pen);
                    painter->drawPoint(QPoint(points.at(i).landX, points.at(i).landY));
                }
            }
            else
            {
                painter->setPen(textPen);
                painter->setFont(textFont);
                painter->drawText(QRect(0, SMALL_MAP_HEIGHT/2 - 50, SMALL_MAP_WIDTH, 25),
                                  Qt::AlignCenter, "There is no available data");
            }
        }
        else if(dataType == 1)
        {
            if(mystation.dataTime != 0)
            {
                for(int i=0;i<mystation.numStation;i++)
                {
                    _STATION sta = mystation.staList[i];
                    QColor col;
                    if(legendType == 0)
                        col.setRgb(redColor(sta.pga[chanID]), greenColor(sta.pga[chanID]), blueColor(sta.pga[chanID]));
                    else if(legendType == 1)
                        col = getGradientColorfromGal(sta.pga[chanID]);
                    //col.setRgb(redColor(sta.pga), greenColor(sta.pga), blueColor(sta.pga));
                    QBrush brush = QBrush(col);
                    painter->setBrush(brush);
                    painter->drawEllipse(QPoint(sta.smapX, sta.smapY), 5, 5);
                }
            }
            else
            {
                painter->setPen(textPen);
                painter->setFont(textFont);
                painter->drawText(QRect(0, SMALL_MAP_HEIGHT/2 - 50, SMALL_MAP_WIDTH, 25),
                                  Qt::AlignCenter, "There is no available data");
            }
        }
    }

    painter->restore();
    painter->setPen(textPen);
    painter->setFont(textFont);
    painter->drawText(QRect(500, 5, 300, 25), Qt::AlignRight, dataSrc.section(",", 0, 0));
    painter->drawText(QRect(500, 30, 300, 25), Qt::AlignRight, dataSrc.section(",", 1, 1));
    painter->drawText(QRect(500, 55, 300, 25), Qt::AlignRight, chanS);
}

int Painter::redColor(float gal)
{
    int color ;

    // red color value
    if( gal <= 0.0098 )
    {
      color = 191 ;
    }
    else if (gal > 0.0098 && gal <= 0.0392)
    {
      color = gal * (-3265.31) + 223 ;
    }
    else if (gal > 0.0392 && gal <= 0.0784)
    {
      color = 95 ;
    }
    else if (gal > 0.0784 && gal <= 0.098)
    {
      color = gal * 3265.31 - 161 ;
    }
    else if (gal > 0.098 && gal <= 0.98)
    {
      color = gal * 103.82 + 148.497 ;
    }
    else if (gal > 0.98 && gal <= 147)
    {
      color = 255 ;
    }
    else if (gal > 147 && gal <= 245)
    {
      color = -0.00333195 * pow(gal,2) + 0.816327 * gal + 207 ;
    }
    else if (gal > 245)
      color = 207 ;

    return color ;
}

int Painter::greenColor(float gal)
{
    int color ;
    // red color value
    if( gal <= 0.98 )
    {
      color = 255 ;
    }
    else if (gal > 0.98 && gal <= 9.8)
    {
      color = -0.75726 * gal * gal - 0.627943 * gal + 255.448 ;
    }
    else if (gal > 0.98 && gal <= 245)
    {
      color = 0.00432696 * gal * gal - 1.84309 * gal + 192.784 ;
      if(color < 0)
        color = 0 ;
    }
    else if (gal > 245)
      color = 0 ;

    return color ;
}

int Painter::blueColor(float gal)
{
    int color ;

    // red color value
    if( gal <= 0.0098 )
    {
      color = 255 ;
    }
    else if (gal > 0.0098 && gal <= 0.098)
    {
      color = -19799.2 * gal * gal + 538.854 * gal + 260.429 ;
    }
    else if (gal > 0.098 && gal <= 0.98)
    {
      color = -35.4966 * gal * gal - 65.8163 * gal + 116.264 ;
    }
    else if (gal > 0.98 && gal <= 3.92)
    {
      color = -5.10204 * gal + 20 ;
    }
    else if (gal > 3.92)
    {
      color = 0 ;
    }

    if(color > 255)
      color = 255 ;

    return color ;
}

QColor Painter::getGradientColorfromGal(float value)
{
    //float pg = value / 980 * 100; // convert gal to %g
    float pg = value;

    QList<float> pgaRange;
    //pgaRange << 0 << 0.1 << 0.3 << 0.5 << 2.4 << 6.7 << 13 << 24 << 44 << 83;
    pgaRange << 0 << 0.98 << 2.94 << 4.90 << 23.52 << 65.66 << 127.40 << 235.20 << 431.20 << 813.40;
    QList<QColor> pgaColor;
    pgaColor << QColor("#FFFFFF") << QColor("#A5DDF9") << QColor("#92D050") << QColor("#FFFF00")
             << QColor("#FFC000") << QColor("#FF0000") << QColor("#A32777") << QColor("#632523")
             << QColor("#4C2600") << QColor("#000000");

    int index;

    for(int i=1;i<9;i++)
    {
        if(pg < 0.98)
        {
            index = 0;
            break;
        }
        else if(pg > 813.40)
        {
            index = 9;
            return pgaColor.at(index);
            break;
        }
        else
        {
            if(pg >= pgaRange.at(i) && pg < pgaRange.at(i+1))
            {
                index = i;
                break;
            }
        }
    }

    double k = pgaRange.at(index+1) - pgaRange.at(index);
    double k2 = pgaRange.at(index+1) - pg;
    float key = 100 - (k2 / k * 100); // get percent
    float ratio = key / 100;

    QColor startC = pgaColor.at(index + 1);
    QColor endC = pgaColor.at(index);

    int r = (int)(ratio*startC.red() + (1-ratio)*endC.red());
    int g = (int)(ratio*startC.green() + (1-ratio)*endC.green());
    int b = (int)(ratio*startC.blue() + (1-ratio)*endC.blue());

    return QColor::fromRgb(r, g, b);
}
