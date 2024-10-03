#pragma once
#include "../libs/QColorWheel/QtColorTriangle.h"
// #include "../libs/VTFLib/VTFLib/VTFLib.h"
#include "VTFEImageContainer.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QListWidget>
#include <vtfpp/vtfpp.h>

enum VTFErrorType
{
	NO_DATA = 0,
	INVALID_IMAGE,
	SUCCESS
};

class VTFEImport;

class ImageProcessor : public QDialog
{
	Q_OBJECT

	friend class VTFEImport;

public:
	ImageProcessor( VTFEImport *parent );
	//	QGroupBox *ImageProcessorCustomMipmaps();
	//	QGroupBox *vBoxCustomMipMaps;

	class QMultiDragListWidget : public QListWidget
	{
	public:
		QMultiDragListWidget( QWidget *parent ) :
			QListWidget( parent ) {}

		bool base = false;

	protected:
		void dragMoveEvent( QDragMoveEvent *e ) override;
		void dropEvent( QDropEvent *event ) override;
	};
};

class GeneralTab : public QDialog
{
	Q_OBJECT

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
	QComboBox *pAlphaDetectedFormatCombo;
	QComboBox *pTypeCombo;
	QCheckBox *pSRGBCheckbox;
	// Resize
	QGroupBox *vBoxResize;
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
	QGroupBox *vBoxCustomMipMaps;
	void GeneralCustomMipmaps();
};

class AdvancedTab : public QDialog
{
	Q_OBJECT
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
	Q_OBJECT
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
	Q_OBJECT
	friend class ImageProcessor;
	friend class GeneralTab;
	friend class AdvancedTab;
	friend class ResourceTab;

	//	SVTFCreateOptions VTFCreateOptions {};
	vtfpp::VTF *editableVTF = nullptr;
	vtfpp::VTF::Flags vtfImageFlags = vtfpp::VTF::FLAG_NONE;
	ImageProcessor *pImageProcessor;
	GeneralTab *pGeneralTab;
	AdvancedTab *pAdvancedTab;
	ResourceTab *pResourceTab;
	explicit VTFEImport( QWidget *pParent );
	QMap<int, VTFEImageContainer *> imageList;
	QMap<int, QThread *> importThreads;
	int threadsImported = 0;
	//	std::vector<VTFEImageContainer *> imageList;
	bool isCancelled = true;
	void InitializeWidgets();
	void setVTF( vtfpp::VTF *vtf )
	{
		if ( vtf )
			this->editableVTF = vtf;
		else
			this->editableVTF = nullptr;
	};

public:
	VTFEImport( QWidget *pParent, const QString &filePath, bool &hasData );
	VTFEImport( QWidget *pParent, const QStringList &filePaths, bool &hasData );
	~VTFEImport()
	{
		foreach( auto imageFormat, imageList )
			delete imageFormat;
	}
	std::unique_ptr<vtfpp::VTF> GenerateVTF( VTFErrorType &err );
	[[nodiscard]] bool IsCancelled() const { return isCancelled; }

	static VTFEImport *FromVTF( QWidget *pParent, const vtfpp::VTF *pFile );
	static VTFEImport *FromFont( QWidget *pParent, std::byte *buff, int width, int height );
	static VTFEImport *Standalone( QWidget *pParent );
	void AddImage( const QString &qString );
	void clearImageList();
	[[nodiscard]] VTFEImageContainer *grabFirst() const
	{
		if ( imageList.empty() )
			return nullptr;
		return imageList[0];
	}
	void SetDefaults();
	bool editVTF( vtfpp::VTF *pFile );
};