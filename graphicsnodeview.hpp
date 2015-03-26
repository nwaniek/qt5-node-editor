#ifndef __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718
#define __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718

#include <QGraphicsView>

class QResizeEvent;

class GraphicsNodeView : public QGraphicsView
{
	Q_OBJECT
public:
	explicit GraphicsNodeView(QWidget *parent = nullptr);
	GraphicsNodeView(QGraphicsScene *scene, QWidget *parent = nullptr);

protected:
	virtual void wheelEvent(QWheelEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void resizeEvent(QResizeEvent *event);

private:
	bool _panning;
	QPoint _pan_cursor_pos;
};

#endif /* __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718 */

