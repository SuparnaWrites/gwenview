// vim: set tabstop=4 shiftwidth=4 noexpandtab
/*
Gwenview - A simple image viewer for KDE
Copyright 2000-2004 Aur�lien G�teau

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


#ifndef GVSCROLLPIXMAPVIEW_H
#define GVSCROLLPIXMAPVIEW_H

// Qt includes
#include <qmap.h>
#include <qscrollview.h>
#include <qtimer.h>

#include "gvbusylevelmanager.h"
#include "gvimageutils/gvimageutils.h"

class QEvent;
class QLabel;
class QMouseEvent;
class QPainter;
class QTimer;
class QWheelEvent;
class KAction;
class KActionCollection;
class KConfig;
class KToggleAction;

class GVDocument;

class GVScrollPixmapView;

// see GVScrollPixmapView ctor
class GVScrollPixmapViewFilter : public QObject {
Q_OBJECT
public:
	GVScrollPixmapViewFilter( GVScrollPixmapView* parent );
	virtual bool eventFilter( QObject*, QEvent* );
};

class GVScrollPixmapView : public QScrollView {
Q_OBJECT

public:
	class ToolController;
	class ScrollToolController;
	class ZoomToolController;
	friend class GVScrollPixmapViewFilter;

	enum Tool { SCROLL, ZOOM };
	enum OSDMode { NONE, PATH, COMMENT, PATH_AND_COMMENT, FREE_OUTPUT };
	typedef QMap<ButtonState,Tool> ButtonStateToolMap;
	typedef QMap<Tool,ToolController*> ToolControllers;

	GVScrollPixmapView(QWidget* parent,GVDocument*,KActionCollection*);
	~GVScrollPixmapView();
	void readConfig(KConfig* config, const QString& group);
	void writeConfig(KConfig* config, const QString& group) const;

	// Properties
	KToggleAction* autoZoom() const { return mAutoZoom; }
	KAction* zoomIn() const { return mZoomIn; }
	KAction* zoomOut() const { return mZoomOut; }
	KAction* resetZoom() const { return mResetZoom; }
	KToggleAction* lockZoom() const { return mLockZoom; }
	double zoom() const { return mZoom; }
	void setZoom(double zoom, int centerX=-1, int centerY=-1);
	bool fullScreen() const { return mFullScreen; }
	void setFullScreen(bool);

	// we use "normal"BackgroundColor because backgroundColor() already exists
	QColor normalBackgroundColor() const { return mBackgroundColor; }
	void setNormalBackgroundColor(const QColor&);

	OSDMode osdMode() const { return mOSDMode; }
	void setOSDMode(OSDMode);
	QString freeOutputFormat() const { return mFreeOutputFormat; }
	void setFreeOutputFormat(const QString& outFormat) { mFreeOutputFormat = outFormat; }
	GVImageUtils::SmoothAlgorithm smoothAlgorithm() const { return mSmoothAlgorithm; }
	void setSmoothAlgorithm(GVImageUtils::SmoothAlgorithm);
	bool doDelayedSmoothing() const { return m_DelayedSmoothing && mSmoothAlgorithm != GVImageUtils::SMOOTH_NONE; }
	bool configuredDelayedSmoothing() const { return m_DelayedSmoothing; }
	void setDelayedSmoothing(bool);
	bool enlargeSmallImages() const { return mEnlargeSmallImages; }
	void setEnlargeSmallImages(bool);
	bool showScrollBars() const { return mShowScrollBars; }
	void setShowScrollBars(bool);
	bool mouseWheelScroll() const { return mMouseWheelScroll; }
	void setMouseWheelScroll(bool);


public slots:
	// File operations
	void showFileProperties();
	void renameFile();
	void copyFile();
	void moveFile();
	void deleteFile();

signals:
	void selectPrevious();
	void selectNext();
	void zoomChanged(double);

protected:
	virtual void openContextMenu(const QPoint&);
	virtual void contentsDragEnterEvent(QDragEnterEvent*);
	virtual void contentsDropEvent(QDropEvent*);
	
private:
	// contentsX/contentsY are not applied, use them manually
	int imageToWidgetX( int x ) const;
	int imageToWidgetY( int y ) const;
	QPoint imageToWidget( const QPoint& p ) const;
	QRect imageToWidget( const QRect& r ) const;
	int widgetToImageX( int x ) const;
	int widgetToImageY( int y ) const;
	QPoint widgetToImage( const QPoint& p ) const;
	QRect widgetToImage( const QRect& r ) const;
	QRect widgetToImageBounding( const QRect& r ) const;

	void addPendingPaint( bool smooth, QRect rect = QRect());
	void addPendingPaintInternal( bool smooth, QRect rect = QRect());
	void performPaint( QPainter* painter, int clipx, int clipy, int clipw, int cliph, bool smooth );
	struct PendingPaint;
	void limitPaintSize( PendingPaint& paint );
	void fullRepaint();
	void cancelPending();
	enum Operation { CHECK_OPERATIONS = 0, SMOOTH_PASS = 1 << 0 };
	void scheduleOperation( Operation operation );
	void checkPendingOperationsInternal();
	void updateBusyLevels();

	double computeZoom(bool in) const;
	double computeAutoZoom() const;
	
	void updateImageOffset();
	void updateScrollBarMode();
	void updateContentSize();
	void updateFullScreenLabel();
	void updateZoomActions();
	void selectTool(ButtonState, bool force);
	void restartAutoHideTimer();

	// Used by the browse tool controller
	void emitSelectPrevious() { emit selectPrevious(); }
	void emitSelectNext() { emit selectNext(); }

private slots:
	void slotLoaded();
	void slotModified();
	void slotZoomIn();
	void slotZoomOut();
	void slotResetZoom();
	void setAutoZoom(bool);
	void hideCursor();
	void slotImageSizeUpdated();
	void slotImageRectUpdated(const QRect&);
	void checkPendingOperations();
	void loadingStarted();
	void slotBusyLevelChanged(GVBusyLevel);
	
protected:
	// Overloaded methods
	bool eventFilter(QObject*, QEvent*);
	void viewportMousePressEvent(QMouseEvent*);
	void viewportMouseMoveEvent(QMouseEvent*);
	void viewportMouseReleaseEvent(QMouseEvent*);
	bool viewportKeyEvent(QKeyEvent*); // This one is not inherited, it's called from the eventFilter
	void wheelEvent(QWheelEvent* event);
	void resizeEvent(QResizeEvent* event);
	void drawContents(QPainter* p,int clipx,int clipy,int clipw,int cliph);

private:
	GVDocument* mDocument;
	QTimer* mAutoHideTimer;
	QLabel* mFullScreenLabel;
	
	QColor mBackgroundColor;
	OSDMode mOSDMode;
	QString mFreeOutputFormat;
	GVImageUtils::SmoothAlgorithm mSmoothAlgorithm;
	bool m_DelayedSmoothing;
	bool mEnlargeSmallImages;
	bool mShowScrollBars;
	bool mMouseWheelScroll;
	ButtonStateToolMap mButtonStateToolMap;
	ToolControllers mToolControllers;

	Tool mTool;

	// Offset to center images
	int mXOffset,mYOffset;

	// Zoom info
	double mZoom;

	// Our actions
	KToggleAction* mAutoZoom;
	KAction* mZoomIn;
	KAction* mZoomOut;
	KAction* mResetZoom;
	KToggleAction* mLockZoom;
	KActionCollection* mActionCollection;

	// Object state info
	bool mFullScreen;
	bool mOperaLikePrevious; // Flag to avoid showing the popup menu on Opera like previous
	double mZoomBeforeAuto;
	int mXCenterBeforeAuto, mYCenterBeforeAuto;
	
	GVScrollPixmapViewFilter mFilter;

	struct PendingPaint {
		PendingPaint( bool s, const QRect& r ) : rect( r ), smooth( s ) {};
		PendingPaint() {}; // stupid Qt containers
		QRect rect;
		bool smooth;
	};
	QMap< long long, PendingPaint > mPendingPaints;
	QRegion mPendingNormalRegion;
	QRegion mPendingSmoothRegion;
	int mPendingOperations;
	QTimer mPendingPaintTimer;
	bool mSmoothingSuspended;
	bool mEmptyImage;
	int mMaxRepaintSize;
	int mMaxScaleRepaintSize;
	int mMaxSmoothRepaintSize;
};

// see coordinates description in the .cpp file
inline
int GVScrollPixmapView::imageToWidgetX( int x ) const {
	if( mZoom == 1.0 ) return x + mXOffset;
	return int( round( x * mZoom )) + mXOffset;
}

inline
int GVScrollPixmapView::imageToWidgetY( int y ) const {
	if( mZoom == 1.0 ) return y + mYOffset;
	return int( round( y * mZoom )) + mYOffset;
}

inline
QPoint GVScrollPixmapView::imageToWidget( const QPoint& p ) const {
	return QPoint( imageToWidgetX( p.x()), imageToWidgetY( p.y()));
}

inline
QRect GVScrollPixmapView::imageToWidget( const QRect& r ) const {
	return QRect( imageToWidget( r.topLeft()), imageToWidget( r.bottomRight()));
}

inline
int GVScrollPixmapView::widgetToImageX( int x ) const {
	if( mZoom == 1.0 ) return x - mXOffset;
	return int( round( ( x - mXOffset ) / mZoom ));
}

inline
int GVScrollPixmapView::widgetToImageY( int y ) const {
	if( mZoom == 1.0 ) return y - mYOffset;
	return int( round( ( y - mYOffset ) / mZoom ));
}

inline
QPoint GVScrollPixmapView::widgetToImage( const QPoint& p ) const {
	return QPoint( widgetToImageX( p.x()), widgetToImageY( p.y()));
}

inline
QRect GVScrollPixmapView::widgetToImage( const QRect& r ) const {
	return QRect( widgetToImage( r.topLeft()), widgetToImage( r.bottomRight()));
}

inline
QRect GVScrollPixmapView::widgetToImageBounding( const QRect& r ) const {
	int extra = mZoom == 1.0 ? 0 : int( ceil( 1 / mZoom ));
	QRect ret( widgetToImage( r.topLeft()), widgetToImage( r.bottomRight()));
	ret.addCoords( -extra, -extra, extra, extra );
	return ret;
}

#endif
