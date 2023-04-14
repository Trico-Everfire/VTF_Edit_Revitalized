#pragma once
#include "../libs/VTFLib/VTFLib/VTFLib.h"

#include <QComboBox>
#include <QLineEdit>
#include <QWidget>

class InfoWidget : public QWidget
{
	Q_OBJECT;

public:
	InfoWidget( QWidget *pParent = nullptr );

	/**
	 * Update the widget with info from the specified VTF file
	 */
	void update_info( VTFLib::CVTFFile *file );

private:
	void setup_ui();
	inline QLineEdit *find( const std::string &l )
	{
		return fields_.find( l )->second;
	}

	std::unordered_map<std::string, QLineEdit *> fields_;
	QComboBox *formatCombo_ = nullptr;
};
