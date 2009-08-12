// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/*
Gwenview: an image viewer
Copyright 2009 Aurélien Gâteau <agateau@kde.org>

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
#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

// Qt

// KDE
#include <kurl.h>
#include <kmainwindow.h>

// Local

namespace Gwenview {


class ImportDialogPrivate;
class ImportDialog : public KMainWindow {
	Q_OBJECT
public:
	ImportDialog();
	~ImportDialog();

public Q_SLOTS:
	void setSourceUrl(const KUrl&);

protected:
	virtual QSize sizeHint() const;

private Q_SLOTS:
	void startImport();
	void slotImportFinished();

private:
	ImportDialogPrivate* const d;
};


} // namespace

#endif /* IMPORTDIALOG_H */
