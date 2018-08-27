#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_window.h"

class window : public QMainWindow
{
	Q_OBJECT

public:
	window(QWidget *parent = Q_NULLPTR);

private:
	Ui::windowClass ui;
};
