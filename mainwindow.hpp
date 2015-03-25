#ifndef __MAINWINDOW_HPP__0E042426_E73E_4382_BC86_DE7F000B57CC
#define __MAINWINDOW_HPP__0E042426_E73E_4382_BC86_DE7F000B57CC

#include <QMainWindow>
#include <QPainterPath>

// forward declarations
class GraphicsNodeView;
class GraphicsNodeScene;


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

private:
	void addNodeViews();
	void addFakeContent();

	GraphicsNodeView *_view;
	GraphicsNodeScene *_scene;

	QPainterPath _path;
};

#endif /* __MAINWINDOW_HPP__0E042426_E73E_4382_BC86_DE7F000B57CC */

