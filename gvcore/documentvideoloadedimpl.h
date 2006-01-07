// vim: set tabstop=4 shiftwidth=4 noexpandtab
/*
Gwenview - A simple image viewer for KDE
Copyright 2006 Aurelien Gateau
 
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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
*/
#ifndef DOCUMENTVIDEOLOADEDIMPL_H
#define DOCUMENTVIDEOLOADEDIMPL_H

// Local
#include "document.h"
#include "documentimpl.h"

namespace Gwenview {
class Document;

class DocumentVideoLoadedImpl : public DocumentImpl {
public:
	DocumentVideoLoadedImpl(Document* document)
	: DocumentImpl(document) {
		setImage(QImage(), false);
		setImageFormat(0);
	}

	void init() {
		emit finished(true);
	}

	Document::FileType fileType() const { return Document::FILE_VIDEO; }
};

} // namespace
#endif /* DOCUMENTVIDEOLOADEDIMPL_H */

