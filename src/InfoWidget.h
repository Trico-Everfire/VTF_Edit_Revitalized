#pragma once

#include <QWidget>

namespace vtfpp
{
	class VTF;
	struct SpriteImagePositions;
} // namespace vtfpp
class QLabel;
class QCheckBox;
class QSlider;
class QLineEdit;
class QComboBox;
class QGroupBox;
class QSpinBox;
class QPushButton;

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

	void Animate();

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
	QGroupBox *spriteSheetGroupBox;
	QCheckBox *enableSpritesheetDisplay;
	QSpinBox *spriteSheetSequence;
	QSpinBox *spriteSheetFrame;
	vtfpp::VTF *vtfFile = nullptr;
	QSpinBox *spriteSheetPositions;
	QPushButton *spriteSheetAnimate;

	void canTriggerInternal();
signals:
	void spriteSheetInfoUpdated( vtfpp::SpriteImagePositions, bool );
};
