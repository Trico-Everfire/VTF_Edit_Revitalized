#pragma once

#include <QHeaderView>
#include <QTableWidget>
#include <QWidget>
#include <vtfpp/vtfpp.h>

class QGridLayout;

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

class AdditionalInformationWidget : public QWidget
{
	QGridLayout *informationLayout;
	std::vector<QWidget *> informationWidgets;

public:
	explicit AdditionalInformationWidget( QWidget *parent = nullptr );
	void set_vtf( vtfpp::VTF *file = nullptr );
};
