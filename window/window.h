#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "ui_window.h"
#include "..\graphics\graphics.h"

class window : public QMainWindow
{
	Q_OBJECT

public:
	window(QWidget *parent = Q_NULLPTR);

private:
	Ui::windowClass ui;
	QTimer * timer_;
	std::unique_ptr<Seed::Graphics> graphics_;

private:
	unsigned int bb_;
	
private:
	unsigned int dsv_;

private:
	unsigned int vp_;

private:
	unsigned int sh_default_;
	unsigned int sh_9patch_;

private:
	unsigned int window_tex_;
	unsigned int tex_node_hole_;
	unsigned int tex_none_;

private:
	struct NinePatchBuffer
	{
		__declspec(align(16)) struct { DirectX::Vector2 size, scale; } geometry_;
		__declspec(align(16)) struct { float left, top, right, bottom; } border_;
	};

private:
	NinePatchBuffer np_cb_;
	
private:
	void window::Draw9Patch(const unsigned int & texture_id, const float & x, const float & y, const float & width, const float & height, const float & ang = 0.f);
	void window::DrawLine9Patch(const unsigned int & texture_id, const DirectX::Vector2 & start, const DirectX::Vector2 & end);

private slots:
	void Update(void);
};
