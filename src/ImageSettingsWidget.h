#pragma once
#include "ImageViewWidget.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QWidget>

class QPushButton;

class ImageSettingsWidget : public QWidget
{
	Q_OBJECT;

public:
	ImageSettingsWidget( ImageViewWidget *viewer, QWidget *parent = nullptr );

	void set_vtf( vtfpp::VTF *file );

	void set_frame( int frame )
	{
		if ( file_ )
			frame_->setValue( frame );
	}

signals:
	/**
	 * Invoked when the VTF is modified in some way
	 * ie by start frame being changed
	 */
	void fileModified();

private:
	void setup_ui( ImageViewWidget *viewer );

	QSpinBox *frame_ = nullptr;
	QSpinBox *face_ = nullptr;
	QSpinBox *mip_ = nullptr;
	QSpinBox *startFrame_ = nullptr;
	QPushButton *animateButton;
	vtfpp::VTF *file_ = nullptr;
	std::unordered_map<vtfpp::VTF::Flags, QCheckBox *> flagChecks_;
	bool settingFile_ = false;
};