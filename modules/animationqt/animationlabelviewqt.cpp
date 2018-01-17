/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/animationqt/animationlabelviewqt.h>
#include <modules/animation/datastructures/animation.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWheelEvent>
#include <QPainter>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QPushButton>
#include <warn/pop>
#include "animation/datastructures/propertytrack.h"

namespace inviwo {

namespace animation {

class AnimationLabelModelQt : public QStandardItemModel {
public:
    AnimationLabelModelQt(QObject* parent) : QStandardItemModel(parent) {
        setColumnCount(1);
    }

    virtual Qt::ItemFlags flags(const QModelIndex& index) const override {
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
    }
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (role == Qt::SizeHintRole) {
            return QSize(200, 25);
        }
        return QStandardItemModel::data(index, role);
    }
};

AnimationLabelViewQt::AnimationLabelViewQt(AnimationController& controller)
    : QListView(), controller_(controller) {
    setMouseTracking(true);
    setSelectionBehavior(SelectItems);
    setMovement(Snap);
    setDragDropMode(InternalMove);
    setDragDropOverwriteMode(false);

    std::string style = "border: 0px;\n background-color: #323235;";
    setStyleSheet(style.c_str());

    Animation& animation = *controller_.getAnimation();
    animation.addObserver(this);
    model_ = new AnimationLabelModelQt(this);

	// Add Control Track
	{
		QList<QStandardItem*> row;
		auto item = new QStandardItem(QString::fromStdString(animation.getControlTrack().getName()));
		item->setData(QVariant::fromValue(static_cast<void*>(&animation.getControlTrack())), Qt::UserRole + 1);
		row.append(item);
		model_->appendRow(row);
	}

	// Add Other Tracks
    for (size_t i = 0; i < animation.size(); ++i) {
        auto& track = animation[i];
        QList<QStandardItem*> row;
        auto item = new QStandardItem(QString::fromStdString(track.getName()));
        item->setData(QVariant::fromValue(static_cast<void*>(&track)), Qt::UserRole + 1);
        row.append(item);
        model_->appendRow(row);
    }

    setModel(model_);
}

void AnimationLabelViewQt::mousePressEvent(QMouseEvent* e) {
    // Process the Event first, so the correct index is selected
    QListView::mousePressEvent(e);
    // Deselect all processors first
    util::setSelected(
        util::getInviwoApplication()->getProcessorNetwork()->getProcessors(), false);
    for (auto index: selectedIndexes())
    {
        auto item = model_->itemFromIndex(index)->data().value<void*>();
        Track* track = reinterpret_cast<Track*>(item);
        BasePropertyTrack* propertytrack = dynamic_cast<BasePropertyTrack*>(track);
        // Might not have been a BasePropertyTrack
        if (propertytrack)
        {
            auto property = propertytrack->getProperty();
            // Select the processor the selected property belongs to
            Processor* processor = property->getOwner()->getProcessor();
            util::setSelected({ processor }, true);
        }

    }
}

void AnimationLabelViewQt::mouseMoveEvent(QMouseEvent* e) {
    QListView::mouseMoveEvent(e);
}

void AnimationLabelViewQt::mouseReleaseEvent(QMouseEvent* e) {
    QListView::mouseReleaseEvent(e);
}

void AnimationLabelViewQt::onTrackAdded(Track* track) {
    QList<QStandardItem*> row;
    auto item = new QStandardItem(QString::fromStdString(track->getName()));
    item->setData(QVariant::fromValue(static_cast<void*>(track)), Qt::UserRole + 1);
    row.append(item);
    model_->appendRow(row);
}

void AnimationLabelViewQt::onTrackRemoved(Track* track) {
    QModelIndex parent = QModelIndex();
    for (int r = 0; r < model_->rowCount(parent); ++r) {
        QModelIndex index = model_->index(r, 0, parent);
        if (model_->data(index, Qt::UserRole + 1).value<void*>() == static_cast<void*>(track)) {
            model_->removeRow(r, parent);
            break;
        }
    }
}

}  // namespace

}  // namespace
