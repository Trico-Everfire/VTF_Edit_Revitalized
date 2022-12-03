#pragma once

#include <QWidget>
class VTFQColorWheel : public QWidget
{
	explicit VTFQColorWheel( QWidget *parent );
	float r;
	float g;
	float b;
	float a;
};
