#pragma once
#include "../libs/VTFLib/VTFLib/VTFLib.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QWidget>

class ResourceWidget : public QWidget
{
	Q_OBJECT;

public:
	ResourceWidget( QWidget *parent = nullptr );

	void set_vtf( VTFLib::CVTFFile *file );

private:
	void setup_ui();

	QTableWidget *table_;
};
