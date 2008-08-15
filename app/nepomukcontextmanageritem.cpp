// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/*
Gwenview: an image viewer
Copyright 2008 Aurélien Gâteau <aurelien.gateau@free.fr>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
// Self
#include "nepomukcontextmanageritem.moc"

// Qt
#include <QShortcut>
#include <QSignalMapper>

// KDE
#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>

// Nepomuk
#include <nepomuk/kratingwidget.h>

// Local
#include "contextmanager.h"
#include "sidebar.h"
#include "ui_nepomuksidebaritem.h"
#include "ui_semanticinfodialog.h"
#include <lib/semanticinfo/abstractsemanticinfobackend.h>
#include <lib/semanticinfo/semanticinfodirmodel.h>
#include <lib/semanticinfo/sorteddirmodel.h>

namespace Gwenview {


struct SemanticInfoDialog : public QDialog, public Ui_SemanticInfoDialog {
	SemanticInfoDialog(QWidget* parent)
	: QDialog(parent) {
		setupUi(this);
	}
};


struct SemanticInfoContextManagerItemPrivate : public Ui_SemanticInfoSideBarItem {
	SemanticInfoContextManagerItem* that;
	SideBar* mSideBar;
	SideBarGroup* mGroup;
	KActionCollection* mActionCollection;
	QPointer<SemanticInfoDialog> mSemanticInfoDialog;
	TagInfo mTagInfo;
	QAction* mEditTagsAction;

	void setupShortcuts() {
		Q_ASSERT(mSideBar);
		QSignalMapper* mapper = new QSignalMapper(that);
		for (int rating=0; rating <= 5; ++rating) {
			QShortcut* shortcut = new QShortcut(mSideBar);
			shortcut->setKey(Qt::Key_0 + rating);
			QObject::connect(shortcut, SIGNAL(activated()), mapper, SLOT(map()) );
			mapper->setMapping(shortcut, rating);
		}
		QObject::connect(mapper, SIGNAL(mapped(int)), mRatingWidget, SLOT(setRating(int)) );
		QObject::connect(mapper, SIGNAL(mapped(int)), that, SLOT(slotRatingChanged(int)) );
	}


	void updateTagLabel() {
		if (that->contextManager()->selection().isEmpty()) {
			mTagLabel->clear();
			return;
		}

		AbstractSemanticInfoBackEnd* backEnd = that->contextManager()->dirModel()->semanticInfoBackEnd();

		TagInfo::ConstIterator
			it = mTagInfo.constBegin(),
			end = mTagInfo.constEnd();
		QStringList labels;
		for (; it!=end; ++it) {
			SemanticInfoTag tag = it.key();
			QString label = backEnd->labelForTag(tag);
			if (!it.value()) {
				// Tag is not present for all urls
				label += '*';
			}
			labels << label;
		}

		QString editLink = i18n("Edit");
		QString text = labels.join(", ") + QString(" <a href='edit'>%1</a>").arg(editLink);
		mTagLabel->setText(text);
	}


	void updateSemanticInfoDialog() {
		mSemanticInfoDialog->mTagWidget->setEnabled(!that->contextManager()->selection().isEmpty());
		mSemanticInfoDialog->mTagWidget->setTagInfo(mTagInfo);
	}
};


SemanticInfoContextManagerItem::SemanticInfoContextManagerItem(ContextManager* manager, KActionCollection* actionCollection)
: AbstractContextManagerItem(manager)
, d(new SemanticInfoContextManagerItemPrivate) {
	d->that = this;
	d->mSideBar = 0;
	d->mGroup = 0;
	d->mActionCollection = actionCollection;
	d->mEditTagsAction = d->mActionCollection->addAction("edit_tags");
	Q_ASSERT(d->mEditTagsAction);
	d->mEditTagsAction->setText(i18nc("@action", "Edit Tags"));
	d->mEditTagsAction->setShortcut(Qt::CTRL | Qt::Key_T);
	connect(d->mEditTagsAction, SIGNAL(triggered()),
		SLOT(showSemanticInfoDialog()) );

	connect(contextManager(), SIGNAL(selectionChanged()),
		SLOT(updateSideBarContent()) );
	connect(contextManager(), SIGNAL(selectionDataChanged()),
		SLOT(updateSideBarContent()) );
	connect(contextManager(), SIGNAL(currentDirUrlChanged()),
		SLOT(updateSideBarContent()) );
}


SemanticInfoContextManagerItem::~SemanticInfoContextManagerItem() {
	delete d;
}


void SemanticInfoContextManagerItem::setSideBar(SideBar* sideBar) {
	d->mSideBar = sideBar;
	connect(sideBar, SIGNAL(aboutToShow()),
		SLOT(updateSideBarContent()) );

	d->mGroup = sideBar->createGroup(i18n("Semantic Information"));

	QWidget* container = new QWidget;
	d->setupUi(container);
	container->layout()->setMargin(0);
	d->mGroup->addWidget(container);

	d->mRatingWidget->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	d->mRatingWidget->setHalfStepsEnabled(false);
	d->mRatingWidget->setMaxRating(5);
	connect(d->mRatingWidget, SIGNAL(ratingChanged(int)),
		SLOT(slotRatingChanged(int)));

	connect(d->mDescriptionLineEdit, SIGNAL(editingFinished()),
		SLOT(storeDescription()));

	connect(d->mTagLabel, SIGNAL(linkActivated(const QString&)),
		d->mEditTagsAction, SLOT(trigger()) );

	d->setupShortcuts();
}


inline int ratingForVariant(const QVariant& variant) {
	if (variant.isValid()) {
		return variant.toInt();
	} else {
		return 0;
	}
}


void SemanticInfoContextManagerItem::updateSideBarContent() {
	if (!d->mSideBar->isVisible()) {
		return;
	}

	KFileItemList itemList = contextManager()->selection();

	bool first = true;
	int rating = 0;
	QString description;
	SortedDirModel* dirModel = contextManager()->dirModel();

	// This hash stores for how many items the tag is present
	// If you have 3 items, and only 2 have the "Holiday" tag,
	// then tagHash["Holiday"] will be 2 at the end of the loop.
	typedef QHash<QString, int> TagHash;
	TagHash tagHash;

	Q_FOREACH(const KFileItem& item, itemList) {
		QModelIndex index = dirModel->indexForItem(item);

		QVariant value = dirModel->data(index, SemanticInfoDirModel::RatingRole);
		if (first) {
			rating = ratingForVariant(value);
		} else if (rating != ratingForVariant(value)) {
			// Ratings aren't the same, reset
			rating = 0;
		}

		QString indexDescription = index.data(SemanticInfoDirModel::DescriptionRole).toString();
		if (first) {
			description = indexDescription;
		} else if (description != indexDescription) {
			description.clear();
		}

		// Fill tagHash, incrementing the tag count if it's already there
		TagSet tagSet = TagSet::fromVariant(index.data(SemanticInfoDirModel::TagsRole));
		Q_FOREACH(const QString& tag, tagSet) {
			TagHash::Iterator it = tagHash.find(tag);
			if (it == tagHash.end()) {
				tagHash[tag] = 1;
			} else {
				++it.value();
			}
		}

		first = false;
	}
	d->mRatingWidget->setRating(rating);
	d->mDescriptionLineEdit->setText(description);

	// Init tagInfo from tagHash
	d->mTagInfo.clear();
	int itemCount = itemList.count();
	TagHash::ConstIterator
		it = tagHash.begin(),
		end = tagHash.end();
	for (; it!=end; ++it) {
		QString tag = it.key();
		int count = it.value();
		d->mTagInfo[tag] = count == itemCount;
	}

	d->mEditTagsAction->setEnabled(!contextManager()->selection().isEmpty());
	d->updateTagLabel();
	if (d->mSemanticInfoDialog) {
		d->updateSemanticInfoDialog();
	}
}


void SemanticInfoContextManagerItem::slotRatingChanged(int rating) {
	KFileItemList itemList = contextManager()->selection();

	SortedDirModel* dirModel = contextManager()->dirModel();
	Q_FOREACH(const KFileItem& item, itemList) {
		QModelIndex index = dirModel->indexForItem(item);
		dirModel->setData(index, rating, SemanticInfoDirModel::RatingRole);
	}
}


void SemanticInfoContextManagerItem::storeDescription() {
	QString description = d->mDescriptionLineEdit->text();
	KFileItemList itemList = contextManager()->selection();

	SortedDirModel* dirModel = contextManager()->dirModel();
	Q_FOREACH(const KFileItem& item, itemList) {
		QModelIndex index = dirModel->indexForItem(item);
		dirModel->setData(index, description, SemanticInfoDirModel::DescriptionRole);
	}
}


void SemanticInfoContextManagerItem::assignTag(const SemanticInfoTag& tag) {
	KFileItemList itemList = contextManager()->selection();

	SortedDirModel* dirModel = contextManager()->dirModel();
	Q_FOREACH(const KFileItem& item, itemList) {
		QModelIndex index = dirModel->indexForItem(item);
		TagSet tags = TagSet::fromVariant( dirModel->data(index, SemanticInfoDirModel::TagsRole) );
		if (!tags.contains(tag)) {
			tags << tag;
			dirModel->setData(index, tags.toVariant(), SemanticInfoDirModel::TagsRole);
		}
	}
}


void SemanticInfoContextManagerItem::removeTag(const SemanticInfoTag& tag) {
	KFileItemList itemList = contextManager()->selection();

	SortedDirModel* dirModel = contextManager()->dirModel();
	Q_FOREACH(const KFileItem& item, itemList) {
		QModelIndex index = dirModel->indexForItem(item);
		TagSet tags = TagSet::fromVariant( dirModel->data(index, SemanticInfoDirModel::TagsRole) );
		if (tags.contains(tag)) {
			tags.remove(tag);
			dirModel->setData(index, tags.toVariant(), SemanticInfoDirModel::TagsRole);
		}
	}
}


void SemanticInfoContextManagerItem::showSemanticInfoDialog() {
	if (!d->mSemanticInfoDialog) {
		d->mSemanticInfoDialog = new SemanticInfoDialog(d->mSideBar);
		d->mSemanticInfoDialog->setAttribute(Qt::WA_DeleteOnClose, true);

		connect(d->mSemanticInfoDialog->mPreviousButton, SIGNAL(clicked()),
			d->mActionCollection->action("go_previous"), SLOT(trigger()) );
		connect(d->mSemanticInfoDialog->mNextButton, SIGNAL(clicked()),
			d->mActionCollection->action("go_next"), SLOT(trigger()) );

		AbstractSemanticInfoBackEnd* backEnd = contextManager()->dirModel()->semanticInfoBackEnd();
		d->mSemanticInfoDialog->mTagWidget->setSemanticInfoBackEnd(backEnd);
		connect(d->mSemanticInfoDialog->mTagWidget, SIGNAL(tagAssigned(const SemanticInfoTag&)),
			SLOT(assignTag(const SemanticInfoTag&)) );
		connect(d->mSemanticInfoDialog->mTagWidget, SIGNAL(tagRemoved(const SemanticInfoTag&)),
			SLOT(removeTag(const SemanticInfoTag&)) );
	}
	d->updateSemanticInfoDialog();
	d->mSemanticInfoDialog->show();
}


} // namespace
