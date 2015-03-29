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
	for (int i = 0; i < 5; i++) {
		auto n = new GraphicsNode();
		for (int j = i; j < 5; j++) {
			n->setPos(i * 25, i * 25);
			n->add_sink("sink");
			n->add_source("source");
		}
		_scene->addItem(n);
	}
}

