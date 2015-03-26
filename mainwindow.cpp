#include "mainwindow.hpp"
#include <QPen>
#include <QColor>
#include <QFont>
#include <QBrush>
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QColorDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QStatusBar>
#include <QResizeEvent>

#include <iostream>

#include "graphicsnodescene.hpp"
#include "graphicsnodeview.hpp"
#include "graphicsnodeview.hpp"
#include "graphicsnode.hpp"
#include "graphicsbezieredge.hpp"


MainWindow::MainWindow()
: _view(nullptr)
, _scene(nullptr)
{

	// create and configure scene
	_scene = new GraphicsNodeScene(this);
	_scene->setSceneRect(-32000, -32000, 64000, 64000);

	//  view setup
	_view = new GraphicsNodeView(this);
	_view->setScene(_scene);
	this->setCentralWidget(_view);


	// add some content
	//addFakeContent()
	addNodeViews();
}

void MainWindow::
resizeEvent(QResizeEvent *event)
{
	QMainWindow::resizeEvent(event);
}



void MainWindow::
addFakeContent()
{
	// fill with some content
	QBrush greenBrush(Qt::green);
	QPen outlinePen(Qt::black);
	outlinePen.setWidth(2);

	// populate with a of lines
	auto gridColor = QColor::fromRgbF(0.4, 0.4, 0.4, 1.0);
	QBrush gridBrush(gridColor);
	QPen gridPen(gridColor);

	// populate with 'content'
	auto rect = _scene->addRect(100, 0, 80, 100, outlinePen, greenBrush);
	rect->setFlag(QGraphicsItem::ItemIsMovable);

	auto text = _scene->addText("scene01", QFont("Ubuntu Mono"));
	text->setDefaultTextColor(QColor::fromRgbF(1.0, 1.0, 1.0, 1.0));
	text->setFlag(QGraphicsItem::ItemIsMovable);
	text->setPos(0, -25);

	auto widget1 = new QPushButton("Hello World");
	auto proxy1 = _scene->addWidget(widget1);
	proxy1->setPos(0, 30);

	auto widget2 = new QTextEdit();
	auto proxy2 = _scene->addWidget(widget2);
	proxy2->setPos(0, 60);
}


void MainWindow::
addNodeViews()
{
	GraphicsNode *n1, *n2, *n3, *n4;

	n1 = new GraphicsNode();
	n1->setPos(20, 40);
	n1->add_sink("sink 1.1");
	n1->add_sink("sink 1.2");
	n1->add_source("source 1.1");
	n1->add_source("source 1.2");
	n1->add_source("source 1.3");


	n2 = new GraphicsNode();
	n2->setPos(350, 250);
	n2->add_sink("sink 2.1");
	n2->add_sink("sink 2.2");
	n2->add_sink("sink 2.3");
	n2->add_source("source 2.1");


	n3 = new GraphicsNode();
	n3->setPos(350, 40);
	n3->add_sink("sink 3.1");
	n3->add_source("source 3.1");
	n3->add_source("source 3.2");

	n4 = new GraphicsNode();
	n4->setPos(550, 140);
	n4->add_sink("sink 4.1");
	n4->add_sink("sink 4.2");

	_scene->addItem(n1);
	_scene->addItem(n2);
	_scene->addItem(n3);
	_scene->addItem(n4);

	GraphicsBezierEdge *e;

	// build up all edges
	e = new GraphicsBezierEdge(n1, 0, n3, 0);
	_scene->addItem(e);

	e = new GraphicsBezierEdge(n1, 1, n2, 1);
	_scene->addItem(e);

	e = new GraphicsBezierEdge(n1, 2, n2, 2);
	_scene->addItem(e);

	e = new GraphicsBezierEdge(n2, 0, n4, 1);
	_scene->addItem(e);

	e = new GraphicsBezierEdge(n3, 0, n4, 0);
	_scene->addItem(e);

	e = new GraphicsBezierEdge(n3, 1, n2, 0);
	_scene->addItem(e);
}

