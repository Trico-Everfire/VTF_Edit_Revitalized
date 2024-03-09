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

	void set_vtf( VTFLib::CVTFFile *file );

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
	VTFLib::CVTFFile *file_ = nullptr;
	std::unordered_map<uint32_t, QCheckBox *> flagChecks_;
	bool settingFile_ = false;
};