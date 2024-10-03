#pragma once

#include <QComboBox>
#include <QLineEdit>
#include <QWidget>
#include <vtfpp/vtfpp.h>

class QLabel;

class InfoWidget : public QWidget
{
	Q_OBJECT;

public:
	InfoWidget( QWidget *pParent = nullptr );

	/**
	 * Update the widget with info from the specified VTF file
	 */
	void update_info( vtfpp::VTF *file );

	QSlider *getSlider()
	{
		return this->slider;
	}

private:
	void setup_ui();
	inline QLineEdit *find( const std::string &l )
	{
		return fields_.find( l )->second;
	}

	std::unordered_map<std::string, QLineEdit *> fields_;
	QComboBox *formatCombo_ = nullptr;
	QSlider *slider;
	QLabel *sliderLabel;
};
