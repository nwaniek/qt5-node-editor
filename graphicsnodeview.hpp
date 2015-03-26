#ifndef __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718
#define __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718

#include <QGraphicsView>
#include <QPoint>

class QResizeEvent;
class GraphicsBezierEdge;
class GraphicsNodeSocket;

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


	enum edge_mode {
		none = 0,
		sink_to_source,
		source_to_sink
	};


	GraphicsBezierEdge *_tmp_edge;
	GraphicsNodeSocket *_sock_source;
	GraphicsNodeSocket *_sock_sink;
	edge_mode _mode;
};

#endif /* __GRAPHICSNODEVIEW_HPP__59C6610F_3283_42A1_9102_38A7065DB718 */

