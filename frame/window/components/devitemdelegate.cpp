/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     zhaoyingzhen <zhaoyingzhen@uniontech.com>
 *
 * Maintainer: zhaoyingzhen <zhaoyingzhen@uniontech.com>
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
#include "devitemdelegate.h"

#include <QtMath>
#include <QPainter>
#include <QPainterPath>

#include <DFontSizeManager>

#define RADIUS_VALUE 10
#define ITEM_SPACE 10
#define ICON_WIDTH 16
#define ICON_HEIGHT 16
#define TEXT_RECT_HEIGHT 20
#define ITEM_HEIGHT 36
#define INDICATOR_SHADOW_OFFSET 10

DevItemDelegate::DevItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void DevItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    painter->setRenderHint(QPainter::Antialiasing);
    QVariant var = index.data(StaticDataRole);
    DevItemData itemData = var.value<DevItemData>();
    QRect rect = option.rect;
    QPen pen;
    pen.setWidth(2);

    // 鼠标悬停
    if (option.state.testFlag(QStyle::State_MouseOver)) {
        pen.setColor(QColor("#EBECED"));
        painter->setPen(pen);
        painter->setBrush(QColor("#EBECED"));
        painter->drawRoundedRect(rect, RADIUS_VALUE, RADIUS_VALUE);
    }

    // 选中背景（连接上和选中）
    int result = index.data(ResultDataRole).toInt();
    if (option.state.testFlag(QStyle::State_Selected) && result == Connected) {
        pen.setColor(QColor("#0081FF"));
        painter->setPen(pen);
        painter->setBrush(QColor("#0081FF"));
        painter->drawRoundedRect(rect, RADIUS_VALUE, RADIUS_VALUE);
    } else {
        // 绘制默认背景
        pen.setColor(QColor("#EBECED"));
        painter->setPen(pen);
        painter->setBrush(QColor("#EBECED"));
        painter->drawRoundedRect(rect, RADIUS_VALUE, RADIUS_VALUE);
    }

    bool selected = (option.state.testFlag(QStyle::State_Selected) && result == Connected);

    // 绘制Icon
    QString imagePath = selected ? itemData.checkedIconPath : itemData.iconPath;
    QRect iconRect = QRect(rect.left() + ITEM_SPACE, rect.top() + rect.height() / 2 - ICON_HEIGHT / 2,
                           ICON_WIDTH, ICON_HEIGHT);
    painter->drawImage(iconRect, QImage(imagePath));

    // 绘制text
    QFont font = Dtk::Widget::DFontSizeManager::instance()->t4();
    painter->setFont(font);
    pen.setColor(selected ? Qt::white : Qt::black);
    painter->setPen(pen);

    int textRectWidth = rect.width() - ITEM_SPACE - iconRect.width() - iconRect.width() - ITEM_SPACE;
    QRect textRect = QRect(iconRect.right() + ITEM_SPACE, rect.top() + 2,
                           textRectWidth, rect.height());

    QFontMetrics fm(font);
    QString itemText = fm.elidedText(itemData.text, Qt::ElideRight, textRectWidth);
    painter->drawText(textRect, itemText);

    switch (result) {
        case ResultState::Connected:
            drawResultState(painter, rect);
            break;
        case ResultState::Connecting:
            drawWaitingState(painter, rect, index.data(DegreeDataRole).toInt());
            break;
        default:
            break;
    }
}

QSize DevItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QSize(option.rect.width(), ITEM_HEIGHT);
}

void DevItemDelegate::drawWaitingState(QPainter *painter, const QRect &rect, int degree) const
{
    int left = rect.width() - ITEM_SPACE;
    int top = rect.top() + rect.height() / 2 - ICON_HEIGHT / 2;
    QRect newRect(left, top, ICON_WIDTH, ICON_HEIGHT);

    painter->setRenderHint(QPainter::Antialiasing, true);
    QList<QList<QColor>> indicatorColors;
    for (int i = 0; i < 3; i++)
        indicatorColors << createDefaultIndicatorColorList(QColor("#0081FF"));

    double radius = 16 * 0.66;
    auto center = QRectF(newRect).center();
    auto indicatorRadius = radius / 2 / 2 * 1.1;
    auto indicatorDegreeDelta = 360 / indicatorColors.count();

    for (int i = 0; i < indicatorColors.count(); ++i) {
        QList<QColor> colors = indicatorColors.value(i);
        for (int j = 0; j < colors.count(); ++j) {
            double degreeCurrent = degree - j * INDICATOR_SHADOW_OFFSET + indicatorDegreeDelta * i;
            auto x = (radius - indicatorRadius) * qCos(qDegreesToRadians(degreeCurrent));
            auto y = (radius - indicatorRadius) * qSin(qDegreesToRadians(degreeCurrent));

            x = center.x() + x;
            y = center.y() + y;
            auto tl = QPointF(x - 1 * indicatorRadius, y - 1 * indicatorRadius);
            QRectF rf(tl.x(), tl.y(), indicatorRadius * 2, indicatorRadius * 2);

            QPainterPath path;
            path.addEllipse(rf);

            painter->fillPath(path, colors.value(j));
        }
    }
}

void DevItemDelegate::drawResultState(QPainter *painter, const QRect &rect) const
{
    // 绘制对勾,14x12
    int left = rect.width() - ITEM_SPACE;
    int top = rect.top() + rect.height() / 2 - 6;

    QPainterPath path;
    path.moveTo(left, top + 6);
    path.lineTo(left + 4, top + 11);
    path.lineTo(left + 12, top + 1);

    painter->drawPath(path);
}

QList<QColor> DevItemDelegate::createDefaultIndicatorColorList(QColor color) const
{
    QList<QColor> colors;
    QList<int> opacitys;
    opacitys << 100 << 30 << 15 << 10 << 5 << 4 << 3 << 2 << 1;
    for (int i = 0; i < opacitys.count(); ++i) {
        color.setAlpha(255 * opacitys.value(i) / 100);
        colors << color;
    }

    return colors;
}
