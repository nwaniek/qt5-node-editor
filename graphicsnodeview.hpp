/* See LICENSE file for copyright and license details. */

#ifndef __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718
#define __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718

#include <QGraphicsView>
#include <QPoint>


class QResizeEvent;
class GraphicsNode;
class GraphicsDirectedEdge;
class GraphicsNodeSocket;


struct EdgeDragEvent
{
	// encode what the user is actually doing.
	enum drag_mode {
		// connect a new edge to a source or sink
		connect_to_source,
		connect_to_sink,

		// move an existing edge to another source/sink
		move_to_source,
		move_to_sink
	};

	GraphicsDirectedEdge *e;
	drag_mode mode;
};


struct NodeResizeEvent
{
	GraphicsNode *node;
	qreal orig_width, orig_height;
	QPoint pos;
};


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
	void middleMouseButtonPress(QMouseEvent *event);
	void leftMouseButtonPress(QMouseEvent *event);

	void middleMouseButtonRelease(QMouseEvent *event);
	void leftMouseButtonRelease(QMouseEvent *event);

	bool can_accept_edge(GraphicsNodeSocket *sock);
	GraphicsNodeSocket* socket_at(QPoint pos);

private:
	bool _panning;
	QPoint _pan_cursor_pos;

	GraphicsDirectedEdge *_tmp_edge;
	GraphicsNodeSocket *_sock_source;
	GraphicsNodeSocket *_sock_sink;

	EdgeDragEvent *_drag_event = nullptr;
	NodeResizeEvent *_resize_event = nullptr;
};

#endif /* __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718 */

