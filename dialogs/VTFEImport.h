#pragma once
#include "../common/VTFEImageFormat.h"
#include "../libs/QColorWheel/QtColorTriangle.h"
#include "../libs/VTFLib/VTFLib/VTFLib.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QGridLayout>

enum VTFErrorType
{
	NODATA,
	INVALIDIMAGE,
	SUCCESSS
};

class VTFEImport;

class GeneralTab : public QDialog
{
	friend class VTFEImport;
	QGridLayout *vMainLayout;
	void GeneralOptions();
	void GeneralResize();
	void GeneralMipMaps();
	void GeneralNormalMap();

public:
	GeneralTab( VTFEImport *parent );
	// General Options
	QComboBox *formatCombo_;
	QComboBox *typeCombo_;
	// Resize
	QCheckBox *resizeCheckbox_;
	QComboBox *resizeMethodCombo_;
	QComboBox *resizeFilterCombo_;
	QCheckBox *clampCheckbox_;
	QComboBox *clampWidthCombo_;
	QComboBox *clampHeightCombo_;
	// Mipmaps
	QCheckBox *generateMipmapsCheckbox_;
	QComboBox *mipmapFilterCombo_;
	// Normal Map
#ifdef NORMAL_GENERATION
	QCheckBox *generateNormalMapCheckbox_;
	QComboBox *kernelFilterCombo_;
	QComboBox *heightConversionCombo_;
	QComboBox *normalAlphaResultCombo_;
	QDoubleSpinBox *scaleSpinBox_;
	QCheckBox *checkbox5_;
#endif
};

class AdvancedTab : public QDialog
{
	friend class VTFEImport;
	QGridLayout *vMainLayout;
	void VersionMenu();
	void GammaCorrectionMenu();
	void Miscellaneous();
	void DTXCompression();
	void LuminanceWeights();
	void ColorCorrectionMenu();
	void UnsharpenMaskOptions();
	void XSharpenOptions();

public:
	AdvancedTab( VTFEImport *parent );
	static void HSVtoRGB( float H, float S, float V, int rgb[3] );
	// Version
	QComboBox *vtfVersionBox_;
#ifdef CHAOS_INITIATIVE
	QCheckBox *auxCompressionBox_;
	QComboBox *auxCompressionLevelBox_;
#endif
	// Gamma Correction
	QDoubleSpinBox *gammaCorrectionBox_;
	QCheckBox *gammaCorrectionCheckBox_;
	// Miscellaneous
	QCheckBox *computeReflectivityCheckBox_;
	QCheckBox *generateThumbnailCheckBox_;
	QCheckBox *generateSphereMapCheckBox_;
	// DTXCompression
	QComboBox *dtxCompressionQuality_;
	// LuminanceWeights
	QDoubleSpinBox *luminanceWeightRedBox_;
	QDoubleSpinBox *luminanceWeightGreenBox_;
	QDoubleSpinBox *luminanceWeightBlueBox_;
	// UnsharpenMaskOptions
	QDoubleSpinBox *unsharpenMaskRadiusBox_;
	QDoubleSpinBox *unsharpenMaskAmountBox_;
	QDoubleSpinBox *unsharpenMaskThresholdBox_;
	//	XSharpenOptions
	QDoubleSpinBox *xSharpenOptionsStrengthBox_;
	QDoubleSpinBox *xSharpenOptionsThresholdBox_;

	// ColorCorrectionMenu
#ifdef COLOR_CORRECTION
	QtColorTriangle *colorCorrectionDialog_;
	QDoubleSpinBox *colorCorrectionRedBox_;
	QDoubleSpinBox *colorCorrectionGreenBox_;
	QDoubleSpinBox *colorCorrectionBlueBox_;
	QDoubleSpinBox *colorCorrectionAlphaBox_;
#endif
};

class ResourceTab : public QDialog
{
	friend class VTFEImport;
	QGridLayout *vMainLayout;
	void LODControlResource();
	void InformationResource();

public:
	ResourceTab( VTFEImport *parent );
	// LODControlResource
	QDoubleSpinBox *controlResourceCrampUBox_;
	QDoubleSpinBox *controlResourceCrampVBox_;
	QCheckBox *lodControlResourceCheckBox_;
	// InformationResource
	QCheckBox *createInformationResourceCheckBox_;
	QLineEdit *informationResouceAuthor_;
	QLineEdit *informationResouceContact_;
	QLineEdit *informationResouceVersion_;
	QLineEdit *informationResouceModification_;
	QLineEdit *informationResouceDescription_;
	QLineEdit *informationResouceComments_;
};

class VTFEImport : public QDialog
{
	friend class GeneralTab;
	friend class AdvancedTab;
	friend class ResourceTab;
	VTFLib::CVTFFile *VTF;
	SVTFCreateOptions options = SVTFCreateOptions();
	vlUInt flags = 0;
	GeneralTab *pGeneralTab;
	AdvancedTab *pAdvancedTab;
	ResourceTab *pResourceTab;
	explicit VTFEImport( QWidget *pParent );
	VTFEImageFormat **images_;
	vlByte **pFFSArray;
	bool cancelled_ = true;
	int imageAmount_ = 0;
	void addImage( const QString &qString );
	void InitializeWidgets();

public:
	explicit VTFEImport( QWidget *pParent, const QString &filePath );
	explicit VTFEImport( QWidget *pParent, const QStringList &filePaths );
	VTFErrorType generateVTF();
	VTFLib::CVTFFile *getVTF();
	bool isCancelled() { return cancelled_; }

	static vlBool IsPowerOfTwo( vlUInt uiSize );
	static VTFEImport *fromVTF( QWidget *pParent, VTFLib::CVTFFile *pFile );
};