#pragma once

#include <QHeaderView>
#include <QTableWidget>
#include <QWidget>
#include <vtfpp/vtfpp.h>

class ResourceWidget : public QWidget
{
	Q_OBJECT;

public:
	ResourceWidget( QWidget *parent = nullptr );

	void set_vtf( vtfpp::VTF *file );

private:
	void setup_ui();

	QTableWidget *table_;
};
