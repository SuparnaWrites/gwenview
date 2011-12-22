/*
Gwenview: an image viewer
Copyright 2007 Aurélien Gâteau <agateau@kde.org>

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#ifndef THUMBNAILLOADJOBTEST_H
#define THUMBNAILLOADJOBTEST_H

// Qt
#include <QColor>
#include <QHash>
#include <QObject>
#include <QSize>

class SandBox {
public:
	SandBox();
	void initDir();
	void fill();
	void createTestImage(const QString& name, int width, int height, const QColor& color);
	void copyTestImage(const QString& name, int width, int height);

	QHash<QString, QSize> mSizeHash;
	QString mPath;
};

class ThumbnailLoadJobTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
	void init();
	void initTestCase();
	void testLoadLocal();
	void testLoadRemote();
	void testUseEmbeddedOrNot();

private:
	SandBox mSandBox;
};

#endif // THUMBNAILLOADJOBTEST_H