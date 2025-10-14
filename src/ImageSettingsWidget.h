#pragma once
#include "ImageViewWidget.h"
#include "VTFEImageContainer.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QWidget>

class QPushButton;

class ImageSettingsWidget : public QWidget
{
	Q_OBJECT;

public:
	ImageSettingsWidget( ImageViewWidget *viewer, QWidget *parent = nullptr );

	void set_vtf( const VTFContainer &file );

	void set_frame( int frame )
	{
		if ( file_ )
			frame_->setValue( frame );
	}

	bool aquireFFPS( int &frame, int &face, int &mip, int &slice );

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
	VTFContainer file_ = { nullptr };
	std::unordered_map<uint32_t, QCheckBox *> flagChecks_;
	bool settingFile_ = false;
};