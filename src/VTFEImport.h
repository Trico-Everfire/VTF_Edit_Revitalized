#pragma once
#include "../libs/QColorWheel/QtColorTriangle.h"
#include "../libs/VTFLib/VTFLib/VTFLib.h"
#include "VTFEImageFormat.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QGridLayout>

enum VTFErrorType
{
	NO_DATA = 0,
	INVALID_IMAGE,
	SUCCESS
};

class VTFEImport;

class GeneralTab : public QDialog
{
	friend class VTFEImport;
	QGridLayout *pMainLayout;
	void GeneralOptions();
	void GeneralResize();
	void GeneralMipMaps();
#ifdef NORMAL_GENERATION
	void GeneralNormalMap();
#endif

public:
	GeneralTab( VTFEImport *parent );
	// General Options
	QComboBox *pFormatCombo;
	QComboBox *pTypeCombo;
	QCheckBox *pSRGBCheckbox;
	// Resize
	QCheckBox *pResizeCheckbox;
	QComboBox *pResizeMethodCombo;
	QComboBox *pResizeFilterCombo;
	QCheckBox *pClampCheckbox;
	QComboBox *pClampWidthCombo;
	QComboBox *pClampHeightCombo;
	// Mipmaps
	QCheckBox *pGenerateMipmapsCheckbox;
	QComboBox *pMipmapFilterCombo;
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
	QGridLayout *pMainLayout;
	void VersionMenu();
	void GammaCorrectionMenu();
	void Miscellaneous();
	void DTXCompression();
	void LuminanceWeights();
#ifdef COLOR_CORRECTION
	void ColorCorrectionMenu();
#endif
	void UnsharpenMaskOptions();
	void XSharpenOptions();

public:
	AdvancedTab( VTFEImport *parent );
	static void HSVtoRGB( float H, float S, float V, int rgb[3] );
	// Version
	QComboBox *pVtfVersionBox;
#ifdef CHAOS_INITIATIVE
	QCheckBox *pAuxCompressionBox;
	QComboBox *pAuxCompressionLevelBox;
#endif
	// Gamma Correction
	QDoubleSpinBox *pGammaCorrectionBox;
	QCheckBox *pGammaCorrectionCheckBox;
	// Miscellaneous
	QCheckBox *pComputeReflectivityCheckBox;
	QCheckBox *pGenerateThumbnailCheckBox;
	QCheckBox *pGenerateSphereMapCheckBox;
	// DTXCompression
	QComboBox *pDtxCompressionQuality;
	// LuminanceWeights
	QDoubleSpinBox *pLuminanceWeightRedBox;
	QDoubleSpinBox *pLuminanceWeightGreenBox;
	QDoubleSpinBox *pLuminanceWeightBlueBox;
	// UnsharpenMaskOptions
	QDoubleSpinBox *pUnsharpenMaskRadiusBox;
	QDoubleSpinBox *pUnsharpenMaskAmountBox;
	QDoubleSpinBox *pUnsharpenMaskThresholdBox;
	//	XSharpenOptions
	QDoubleSpinBox *pXSharpenOptionsStrengthBox;
	QDoubleSpinBox *pXSharpenOptionsThresholdBox;

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
	QGridLayout *pMainLayout;
	void LODControlResource();
	void InformationResource();

public:
	ResourceTab( VTFEImport *parent );
	// LODControlResource
	QDoubleSpinBox *pControlResourceCrampUBox;
	QDoubleSpinBox *pControlResourceCrampVBox;
	QCheckBox *pLodControlResourceCheckBox;
	// InformationResource
	QCheckBox *pCreateInformationResourceCheckBox;
	QLineEdit *pInformationResourceAuthor;
	QLineEdit *pInformationResouceContact;
	QLineEdit *pInformationResouceVersion;
	QLineEdit *pInformationResouceModification;
	QLineEdit *pInformationResouceDescription;
	QLineEdit *pInformationResouceComments;
};

class VTFEImport : public QDialog
{
	friend class GeneralTab;
	friend class AdvancedTab;
	friend class ResourceTab;

	SVTFCreateOptions VTFCreateOptions {};
	vlUInt vtfImageFlags = 0;
	GeneralTab *pGeneralTab;
	AdvancedTab *pAdvancedTab;
	ResourceTab *pResourceTab;
	explicit VTFEImport( QWidget *pParent );
	QMap<int, VTFEImageFormat *> imageList;
	bool isCancelled = true;
	void AddImage( const QString &qString );
	void InitializeWidgets();

public:
	VTFEImport( QWidget *pParent, const QString &filePath, bool &hasData );
	VTFEImport( QWidget *pParent, const QStringList &filePaths, bool &hasData );
	~VTFEImport()
	{
		foreach( auto imageFormat, imageList )
			delete imageFormat;
	}
	VTFLib::CVTFFile *GenerateVTF( VTFErrorType &err );
	bool IsCancelled() const { return isCancelled; }

	static vlBool
	IsPowerOfTwo( vlUInt uiSize );
	static VTFEImport *FromVTF( QWidget *pParent, VTFLib::CVTFFile *pFile );
	void SetDefaults();
};