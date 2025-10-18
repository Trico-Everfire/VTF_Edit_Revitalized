#include <QDialog>
#include <QGroupBox>
#include <QImage>
#include <QString>
#include <vtfpp/VTF.h>

class QGridLayout;
class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QLineEdit;
class QLabel;

struct CVTFOptions
{
	enum VTFType : uint8_t
	{
		ANIMATED = 0,
		ENVIRONMENT,
		VOLUME,
		ANIMATED_ENVIRONMENT

	};

	vtfpp::ImageFormat textureFormat;
	vtfpp::ImageFormat alphaTextureFormat;
	VTFType imageType;
	uint8_t version;
	bool enable_compression;
	vtfpp::CompressionMethod compression_method;
	int8_t compression_level;
	bool srgb;
	bool generate_thumbnail;

	bool compute_reflectivity;
	double lumen_red;
	double lumen_green;
	double lumen_blue;

	vtfpp::ImageConversion::ResizeMethod resize_method;
	vtfpp::ImageConversion::ResizeFilter resize_filter;
	bool clamp;
	uint16_t max_width;
	uint16_t max_height;

	bool generate_mipmaps;
	vtfpp::ImageConversion::ResizeFilter mipmap_filter;

	bool lod_control_resource;
	double lod_control_strength;
	double lod_control_threshold;

	bool information_resource;
	QString information_author;
	QString information_contact;
	QString information_organization;
	QString information_version;
	QString information_modification;
	QString information_description;
	QString information_comments;
};

class CInverseGroupBox : public QGroupBox
{
	using QGroupBox::QGroupBox;
	void paintEvent( QPaintEvent *event ) override;
};

class CGeneralTab : public QWidget
{
	Q_OBJECT
	bool isEditingVTF = false;
	bool standalone = false;

public:
	explicit CGeneralTab( QWidget *parent, bool standalone );
	// General Options
	QComboBox *pFormatCombo;
	QCheckBox *pformatStandaloneCheckbox;
	QComboBox *pAlphaDetectedFormatCombo;
	QCheckBox *pAlphaFormatStandaloneCheckbox;
	QCheckBox *pGenerateThumbnailCheckBox;
	CInverseGroupBox *computeReflectivityBox;
	QComboBox *pTypeCombo;
	//	QCheckBox *pTypeComboStandaloneCheckbox;
	// Version
	QComboBox *pVtfVersionBox;
	QCheckBox *pVTFVersionStandaloneCheckbox;
	QGroupBox *CompressionBox;
	QComboBox *CompressionTypeBox;
	QCheckBox *pCompressionTypeStandaloneCheckbox;
	QComboBox *CompressionLevelBox;
	QCheckBox *pCompressionLevelStandaloneCheckbox;
	// Reflectivity
	QDoubleSpinBox *pLuminanceWeightRedBox;
	QCheckBox *pRedLumenStandaloneCheckbox;
	QDoubleSpinBox *pLuminanceWeightGreenBox;
	QCheckBox *pGreenLumenStandaloneCheckbox;
	QDoubleSpinBox *pLuminanceWeightBlueBox;
	QCheckBox *pBlueLumenStandaloneCheckbox;
	// Resize
	QGroupBox *vBoxResize;
	//	QCheckBox *pResizeCheckbox;
	QComboBox *pResizeMethodCombo;
	QComboBox *pResizeFilterCombo;
	QCheckBox *pClampCheckbox;
	QComboBox *pClampWidthCombo;
	QComboBox *pClampHeightCombo;
	// Mipmaps
	QGroupBox *generateMipmapBox;
	QComboBox *pMipmapFilterCombo;

	QCheckBox *pSRGBCheckBox;
	QLineEdit *mipmapCountTextBox;

	void getVTFOptions( CVTFOptions & ) const;
	void setVTFData( vtfpp::VTF *vtf );

signals:
	void setMipmapTextBoxText( const QString &txt );
	void versionSupportsStrata( bool supports );
	//	void resizeMethodChanged( vtfpp::ImageConversion::ResizeMethod method );
	//	void resizeMethodIsNone( bool isNone );
};

class CResourceTab : public QWidget
{
	Q_OBJECT

	bool isEditingVTF = false;
	bool standalone = false;

public:
	explicit CResourceTab( QWidget *parent, bool standalone );

	// LODControlResource
	QGroupBox *lodControlResourceBox;
	QDoubleSpinBox *pControlResourceCrampUBox;
	QDoubleSpinBox *pControlResourceCrampVBox;
	// InformationResource
	QLabel *pWarningLabel;
	QGroupBox *informationResourceBox;
	QCheckBox *pCreateInformationResourceCheckBox;
	//	QCheckBox *pCreateInformationStandaloneCheckbox;
	QLineEdit *pInformationResourceAuthor;
	QCheckBox *pAuthorStandaloneCheckbox;
	QLineEdit *pInformationResourceContact;
	QCheckBox *pContactStandaloneCheckbox;
	QLineEdit *pInformationResourceOrganization;
	QCheckBox *pOrganizationStandaloneCheckbox;
	QLineEdit *pInformationResourceVersion;
	QCheckBox *pVersionStandaloneCheckbox;
	QLineEdit *pInformationResourceModification;
	QCheckBox *pModificationStandaloneCheckbox;
	QLineEdit *pInformationResourceDescription;
	QCheckBox *pDescriptionStandaloneCheckbox;
	QLineEdit *pInformationResourceComments;

	QCheckBox *pCommentsStandaloneCheckbox;
	void getVTFOptions( CVTFOptions & ) const;

	void setVTFData( vtfpp::VTF *vtf );
public slots:
	void markResourceAsDangerous( bool mark );
};

class CVTFCreationDialog : public QDialog
{
	struct ImageContent
	{
		uint16_t width;
		uint16_t height;
		uint16_t frame;
		uint8_t face;
		uint16_t slice;
		vtfpp::ImageFormat format;
		std::vector<std::byte> data;
	};

	bool standalone = false;

	vtfpp::VTF *vtf = nullptr;
	QString fileName = "untitled";
	bool useImageData = true;
	bool alphaInImages = false;
	CGeneralTab *generalTabWidget;
	CResourceTab *resourceTabWidget;
	std::vector<ImageContent> imageList;
	//	std::map<ImageContent> imageList;

	void insertVTFData();

public:
	CVTFCreationDialog( QWidget *parent, vtfpp::VTF *vtf );
	bool addImage( std::span<std::byte> data, vtfpp::ImageFormat fmt, uint16_t width, uint16_t height, uint16_t frame = 1, uint16_t face = 1, uint8_t slice = 1, uint8_t mip = 1 );
	bool addImage( const QImage &iamge );
	bool addImage( const QString &qString );
	bool addImage( const QStringList &list );
	[[nodiscard]] QString getFileName() const;

	void setVTF( vtfpp::VTF *vtf );
	void applyChanges();

	int exec() override;
};
