#include "ProcessVTF.h"

#include "ApplicationOptionsWidget.h"
#include "flagsandformats.hpp"

#include <QBuffer>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStylePainter>
#include <QTabWidget>
#include <kvpp/kvpp.h>

CVTFCreationDialog::CVTFCreationDialog( QWidget *parent, vtfpp::VTF *vtf ) :
	QDialog( parent ), vtf( vtf )
{
	if ( !vtf )
	{
		this->standalone = true;
	}

	this->generalTabWidget = new CGeneralTab( this, this->standalone );
	this->resourceTabWidget = new CResourceTab( this, this->standalone );

	auto creationLayout = new QVBoxLayout( this );
	auto creationTabsWidget = new QTabWidget( this );

	creationTabsWidget->addTab( this->generalTabWidget, tr( "General" ) );
	creationTabsWidget->addTab( this->resourceTabWidget, tr( "Resource" ) );

	creationLayout->addWidget( creationTabsWidget );

	auto buttonlayoutBox = new QDialogButtonBox( this );
	auto acceptButton = buttonlayoutBox->addButton( "Accept", QDialogButtonBox::AcceptRole );
	auto cancelButton = buttonlayoutBox->addButton( "Cancel", QDialogButtonBox::RejectRole );

	connect( acceptButton, &QPushButton::pressed, this, &CVTFCreationDialog::accept );
	connect( cancelButton, &QPushButton::pressed, this, &CVTFCreationDialog::close );
	creationLayout->addWidget( buttonlayoutBox );

	if ( vtf && vtf->hasImageData() )
		insertVTFData();
}

bool CVTFCreationDialog::addImage( const QImage &image )
{
	QByteArray arr;
	QBuffer buff( &arr );
	image.save( &buff, "PNG" );

	vtfpp::ImageFormat format;
	int width, height, frames;
	auto data = vtfpp::ImageConversion::convertFileToImageData( std::span( reinterpret_cast<const std::byte *>( arr.constData() ), reinterpret_cast<const std::byte *>( arr.constData() ) + arr.size() ), format, width, height, frames );
	return addImage( data, format, width, height, frames );
}

bool CVTFCreationDialog::addImage( const QString &str )
{
	vtfpp::ImageFormat format;
	int width, height, frames;
	auto data = vtfpp::ImageConversion::convertFileToImageData( sourcepp::fs::readFileBuffer( str.toStdString() ), format, width, height, frames );

	bool res = true;
	for ( int i = 0; i < frames; i++ )
	{
		uint32_t offset, size;
		bool possible = vtfpp::ImageFormatDetails::getDataPosition( offset, size, format, 0, 1, i, frames, 0, 1, width, height );
		res &= addImage( std::span( data.data() + offset, data.data() + offset + size ), format, width, height, 1 );
	}
	return res;
}

bool CVTFCreationDialog::addImage( const QStringList &list )
{
	bool res = true;
	for ( const auto &str : list )
		res &= addImage( str );
	return res;
}

QString CVTFCreationDialog::getFileName() const
{
	return fileName;
}

bool CVTFCreationDialog::addImage( const std::span<std::byte> data, vtfpp::ImageFormat fmt, uint16_t width, uint16_t height, uint16_t frames, uint16_t faces, uint8_t slices, uint8_t mips )
{
	alphaInImages += vtfpp::ImageFormatDetails::transparent( fmt );

	if ( imageList.empty() )
		this->generalTabWidget->setMipmapTextBoxText( QString::number( vtfpp::ImageDimensions::getRecommendedMipCountForDims( fmt, width, height ) ) );

	for ( int i = 0; i < frames; i++ )
		for ( int j = 0; j < faces; j++ )
			for ( int k = 0; k < slices; k++ )
				for ( int l = 0; l < mips; l++ )
				{
					this->imageList.emplace_back( width, height, i, j, k, fmt, std::vector<std::byte>( data.begin(), data.end() ) );
				}
	return true;
}

int CVTFCreationDialog::exec()
{
	if ( this->standalone && !vtf )
		return QDialog::exec();

	if ( !vtf )
		return QDialog::Rejected;

	if ( !vtf->hasImageData() && imageList.empty() )
		return QDialog::Rejected;

	this->useImageData = !( vtf->hasImageData() && imageList.empty() );

	auto result = QDialog::exec();
	if ( result == QDialog::Accepted )
		this->applyChanges();
	return result;
}

void CVTFCreationDialog::applyChanges()
{
	if ( !vtf )
		return;

	CVTFOptions options;
	this->generalTabWidget->getVTFOptions( options );
	this->resourceTabWidget->getVTFOptions( options );

	if ( !vtf->hasImageData() )
	{
		auto firstImage = imageList[0];
		vtf->setSize( firstImage.width, firstImage.height, options.resize_filter );
		uint16_t frames = 1;
		uint16_t faces = 1;
		uint16_t slices = 1;

		switch ( options.imageType )
		{
			case CVTFOptions::ANIMATED:
				frames = imageList.size();
				break;
			case CVTFOptions::ENVIRONMENT:
				faces = imageList.size() > 6 ? 6 : imageList.size();
				break;
			case CVTFOptions::VOLUME:
				slices = imageList.size();
				break;
			case CVTFOptions::ANIMATED_ENVIRONMENT:
				frames = imageList.size() / 6;
				faces = 6;
				break;
		}

		vtf->setFaceCount( faces != 1 );
		vtf->setDepth( slices );
		vtf->setFrameCount( frames );

		for ( int i = 0; i < frames; i++ )
			for ( int j = 0; j < faces; j++ )
				for ( int k = 0; k < slices; k++ )
				{
					auto image = imageList[i + j + k];
					vtf->setImage( image.data, image.format, image.width, image.height, options.resize_filter, 0, i, j, k );
				}
	}

	vtf->setVersion( options.version );

	if ( options.enable_compression )
	{
		vtf->setCompressionMethod( options.compression_method );
		vtf->setCompressionLevel( options.compression_level );
	}

	vtf->setImageResizeMethods( options.resize_method, options.resize_method );

	vtf->setSRGB( options.srgb );

	vtf->setFormat( alphaInImages ? options.alphaTextureFormat : options.textureFormat );

	if ( options.generate_thumbnail )
		vtf->computeThumbnail();

	if ( options.compute_reflectivity )
		vtf->computeReflectivity();
	else
		vtf->setReflectivity( { options.lumen_red, options.lumen_green, options.lumen_blue } );

	if ( options.generate_mipmaps )
		vtf->computeMips( options.mipmap_filter );

	vtf->removeLODResource();
	if ( options.lod_control_resource )
		vtf->setLODResource( options.lod_control_strength, options.lod_control_threshold );

	vtf->removeKeyValuesDataResource();
	if ( options.information_resource )
	{
		kvpp::KV1Writer informationKeyValue;
		auto root = &informationKeyValue.addChild( "Information" );

		root->addChild( "Author", options.information_author.toStdString() );
		root->addChild( "Contact", options.information_contact.toStdString() );
		root->addChild( "Organization", options.information_organization.toStdString() );
		root->addChild( "Version", options.information_version.toStdString() );
		root->addChild( "Modification", options.information_modification.toStdString() );
		root->addChild( "Description", options.information_description.toStdString() );
		root->addChild( "Comments", options.information_comments.toStdString() );

		vtf->setKeyValuesDataResource( informationKeyValue.bake() );
	}

	if ( !this->useImageData )
		return;
}
void CVTFCreationDialog::insertVTFData()
{
	alphaInImages = vtfpp::ImageFormatDetails::transparent( vtf->getFormat() );
	this->generalTabWidget->setVTFData( vtf );
	this->resourceTabWidget->setVTFData( vtf );
}
void CVTFCreationDialog::setVTF( vtfpp::VTF *vtf )
{
	this->vtf = vtf;
	this->generalTabWidget->setMipmapTextBoxText( QString::number( vtfpp::ImageDimensions::getRecommendedMipCountForDims( vtf->getFormat(), vtf->getWidth(), vtf->getHeight() ) ) );
}

CGeneralTab::CGeneralTab( QWidget *parent, bool standalone ) :
	QWidget( parent ), standalone( standalone )
{
	auto settings = ApplicationOptions::getInstance()->get( IMPORT_MENU_SETTINGS, IMPORT_DEFAULTS ).toObject();
	auto generalTabLayout = new QGridLayout( this );

	auto generalOptionsBox = new QGroupBox( tr( "General Options" ), this );
	auto generalOptionsLayout = new QGridLayout( generalOptionsBox );
	auto textureFormatLabel = new QLabel( tr( "Texture Format:" ), generalOptionsBox );
	generalOptionsLayout->addWidget( textureFormatLabel, 0, 0 );
	pFormatCombo = new QComboBox( generalOptionsBox );

	for ( auto &fmt : IMAGE_FORMATS )
	{
		if ( vtfpp::ImageFormat::P8 != fmt.format )
			pFormatCombo->addItem( tr( fmt.name ), (int)fmt.format );
	}

	pFormatCombo->setCurrentIndex( pFormatCombo->findData( settings.value( "format" ).toInt() ) ); // vtfpp::ImageFormat::RGB888

	generalOptionsLayout->addWidget( pFormatCombo, 0, 1, Qt::AlignRight );

	pformatStandaloneCheckbox = new QCheckBox();
	pformatStandaloneCheckbox->setHidden( !this->standalone );
	generalOptionsLayout->addWidget( pformatStandaloneCheckbox, 0, 2 );

	auto alphaDetectedLabel = new QLabel( tr( "Alpha Texture Format:" ), generalOptionsBox );
	generalOptionsLayout->addWidget( alphaDetectedLabel, 1, 0, Qt::AlignLeft );
	pAlphaDetectedFormatCombo = new QComboBox( generalOptionsBox );
	for ( auto &fmt : IMAGE_FORMATS )
	{
		if ( vtfpp::ImageFormat::P8 != fmt.format )
			pAlphaDetectedFormatCombo->addItem( tr( fmt.name ), (int)fmt.format );
	}

	pAlphaDetectedFormatCombo->setCurrentIndex( pAlphaDetectedFormatCombo->findData( settings.value( "format_alpha" ).toInt() ) ); // vtfpp::ImageFormat::RGB888

	generalOptionsLayout->addWidget( pAlphaDetectedFormatCombo, 1, 1, Qt::AlignRight );

	pAlphaFormatStandaloneCheckbox = new QCheckBox();
	pAlphaFormatStandaloneCheckbox->setHidden( !this->standalone );
	generalOptionsLayout->addWidget( pAlphaFormatStandaloneCheckbox, 1, 2 );

	auto textureTypeLabel = new QLabel( tr( "Texture Type:" ), generalOptionsBox );
	pTypeCombo = new QComboBox( generalOptionsBox );
	pTypeCombo->addItem( tr( "Animated Texture" ), CVTFOptions::VTFType::ANIMATED );
	pTypeCombo->addItem( tr( "Environment Map" ), CVTFOptions::VTFType::ENVIRONMENT );
	pTypeCombo->addItem( tr( "Volume Texture" ), CVTFOptions::VTFType::VOLUME );
	pTypeCombo->setCurrentIndex( settings.value( "texture_type" ).toInt() );
	pTypeCombo->setDisabled( this->standalone );

	generalOptionsLayout->addWidget( textureTypeLabel, 2, 0, Qt::AlignLeft );
	generalOptionsLayout->addWidget( pTypeCombo, 2, 1, Qt::AlignRight );

	auto vtfVersionLabel = new QLabel( "VTF Version:", generalOptionsBox );
	generalOptionsLayout->addWidget( vtfVersionLabel, 3, 0, Qt::AlignLeft );
	pVtfVersionBox = new QComboBox( this );

	int vtfMax = ApplicationOptions::getInstance()->get( ADV_SRATA_SOURCE, false ).toBool() ? 6 : 5;

	for ( int i = 0; i <= vtfMax; i++ )
		pVtfVersionBox->addItem( "7." + QString::number( i ), i );

	pVtfVersionBox->setCurrentIndex( settings.value( "vtf_version" ).toInt() );
	generalOptionsLayout->addWidget( pVtfVersionBox, 3, 1, Qt::AlignRight );

	pVTFVersionStandaloneCheckbox = new QCheckBox();
	pVTFVersionStandaloneCheckbox->setHidden( !this->standalone );
	generalOptionsLayout->addWidget( pVTFVersionStandaloneCheckbox, 3, 2 );

	CompressionBox = new QGroupBox( tr( "Enable Compression." ), generalOptionsBox );
	CompressionBox->setCheckable( true );
	CompressionBox->setChecked( settings.value( "enable_compression" ).toBool() );
	generalOptionsLayout->addWidget( CompressionBox, 4, 0, Qt::AlignCenter );

	auto compressionBoxLayout = new QGridLayout( CompressionBox );

	auto compressionTypeLabel = new QLabel( tr( "Compression Type:" ), CompressionBox );
	compressionBoxLayout->addWidget( compressionTypeLabel, 0, 0, Qt::AlignLeft );

	CompressionTypeBox = new QComboBox( CompressionBox );
	CompressionTypeBox->addItem( tr( "Deflate" ), (int)vtfpp::CompressionMethod::DEFLATE );
	CompressionTypeBox->addItem( tr( "ZSTD" ), (int)vtfpp::CompressionMethod::ZSTD );

	CompressionTypeBox->setCurrentIndex( settings.value( "compression_method" ).toInt() );

	compressionBoxLayout->addWidget( CompressionTypeBox, 0, 1, Qt::AlignRight );

	pCompressionTypeStandaloneCheckbox = new QCheckBox();
	pCompressionTypeStandaloneCheckbox->setHidden( !this->standalone );
	compressionBoxLayout->addWidget( pCompressionTypeStandaloneCheckbox, 0, 2 );

	auto compressionLevelLabel = new QLabel( tr( "Compression Level:" ), CompressionBox );
	compressionBoxLayout->addWidget( compressionLevelLabel, 1, 0, Qt::AlignLeft );
	CompressionLevelBox = new QComboBox( CompressionBox );
	for ( int i = -1; i <= 9; i++ )
	{
		CompressionLevelBox->addItem( QString::number( i ), i );
	}

	connect( CompressionTypeBox, &QComboBox::currentIndexChanged, this, [&]( int ind )
			 {
				 CompressionLevelBox->clear();
				 if ( ind )
				 {
					 for ( int i = -1; i <= 22; i++ )
					 {
						 CompressionLevelBox->addItem( QString::number( i ), i );
					 }
					 return;
				 }
				 for ( int i = -1; i <= 9; i++ )
				 {
					 CompressionLevelBox->addItem( QString::number( i ), i );
				 }
			 } );

	CompressionLevelBox->setCurrentIndex( settings.value( "compression_level" ).toInt() );

	compressionBoxLayout->addWidget( CompressionLevelBox, 1, 1, Qt::AlignRight );

	pCompressionLevelStandaloneCheckbox = new QCheckBox();
	pCompressionLevelStandaloneCheckbox->setHidden( !this->standalone );
	compressionBoxLayout->addWidget( pCompressionLevelStandaloneCheckbox, 1, 2 );

	CompressionBox->setDisabled( settings.value( "vtf_version" ).toInt() < 6 );

	if ( vtfMax < 6 )
		CompressionBox->hide();
	else
		connect( pVtfVersionBox, &QComboBox::currentIndexChanged, CompressionBox, [&]( int index )
				 {
					 CompressionBox->setDisabled( index < 6 );
					 emit versionSupportsStrata( index > 5 );
				 } );

	pSRGBCheckBox = new QCheckBox( "sRGB Color Space", this );
	pSRGBCheckBox->setChecked( settings.value( "srgb" ).toBool() );
	if ( this->standalone )
		pSRGBCheckBox->setCheckState( Qt::PartiallyChecked );
	generalOptionsLayout->addWidget( pSRGBCheckBox, 5, 0, Qt::AlignLeft );

	pGenerateThumbnailCheckBox = new QCheckBox( tr( "Generate Thumbnail" ), this );
	if ( !this->standalone )
		pGenerateThumbnailCheckBox->setChecked( settings.value( "generate_thumbnail" ).toBool() );
	generalOptionsLayout->addWidget( pGenerateThumbnailCheckBox, 6, 0, Qt::AlignLeft );

	generalTabLayout->addWidget( generalOptionsBox, 0, 0, 2, 1 );

	computeReflectivityBox = new CInverseGroupBox( "Compute Reflectivity", this );
	computeReflectivityBox->setCheckable( true );
	if ( !this->standalone )
		computeReflectivityBox->setChecked( !settings.value( "generate_reflectivity" ).toBool() );
	generalTabLayout->addWidget( computeReflectivityBox, 2, 0, 1, 2 );

	auto computeReflectivityLayout = new QGridLayout( computeReflectivityBox );

	auto redLumenLabel = new QLabel( tr( "Red:" ), computeReflectivityBox );
	computeReflectivityLayout->addWidget( redLumenLabel, 0, 0, Qt::AlignLeft );
	pLuminanceWeightRedBox = new QDoubleSpinBox( computeReflectivityBox );
	pLuminanceWeightRedBox->setDecimals( 3 );
	pLuminanceWeightRedBox->setValue( settings.value( "red_lumen" ).toDouble() );
	computeReflectivityLayout->addWidget( pLuminanceWeightRedBox, 0, 1, Qt::AlignLeft );

	pRedLumenStandaloneCheckbox = new QCheckBox();
	pRedLumenStandaloneCheckbox->setHidden( !this->standalone );
	computeReflectivityLayout->addWidget( pRedLumenStandaloneCheckbox, 0, 2 );

	auto greenLumenLabel = new QLabel( tr( "Green:" ), computeReflectivityBox );
	computeReflectivityLayout->addWidget( greenLumenLabel, 1, 0, Qt::AlignLeft );
	pLuminanceWeightGreenBox = new QDoubleSpinBox( computeReflectivityBox );
	pLuminanceWeightGreenBox->setDecimals( 3 );
	pLuminanceWeightGreenBox->setValue( settings.value( "green_lumen" ).toDouble() );
	computeReflectivityLayout->addWidget( pLuminanceWeightGreenBox, 1, 1, Qt::AlignLeft );

	pGreenLumenStandaloneCheckbox = new QCheckBox();
	pGreenLumenStandaloneCheckbox->setHidden( !this->standalone );
	computeReflectivityLayout->addWidget( pGreenLumenStandaloneCheckbox, 1, 2 );

	auto blueLumenLabel = new QLabel( tr( "Blue:" ), computeReflectivityBox );
	computeReflectivityLayout->addWidget( blueLumenLabel, 2, 0, Qt::AlignLeft );
	pLuminanceWeightBlueBox = new QDoubleSpinBox( computeReflectivityBox );
	pLuminanceWeightBlueBox->setDecimals( 3 );
	pLuminanceWeightBlueBox->setValue( settings.value( "blue_lumen" ).toDouble() );
	computeReflectivityLayout->addWidget( pLuminanceWeightBlueBox, 2, 1, Qt::AlignLeft );

	pBlueLumenStandaloneCheckbox = new QCheckBox();
	pBlueLumenStandaloneCheckbox->setHidden( !this->standalone );
	computeReflectivityLayout->addWidget( pBlueLumenStandaloneCheckbox, 2, 2 );

	vBoxResize = new QGroupBox( tr( "Resize" ), this );
	vBoxResize->setCheckable( this->standalone );
	vBoxResize->setChecked( false );
	auto resizeBoxLayout = new QGridLayout( vBoxResize );

	auto resizeMethodLabel = new QLabel( tr( "Resize Method:" ), vBoxResize );
	resizeBoxLayout->addWidget( resizeMethodLabel, 0, 0, Qt::AlignLeft );
	pResizeMethodCombo = new QComboBox( vBoxResize );
	pResizeMethodCombo->addItem( tr( "Nearest Power Of 2" ), (int)vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_NEAREST );
	pResizeMethodCombo->addItem( tr( "Biggest Power Of 2" ), (int)vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_BIGGER );
	pResizeMethodCombo->addItem( tr( "Smallest Power Of 2" ), (int)vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_SMALLER );
	pResizeMethodCombo->addItem( tr( "None" ), (int)vtfpp::ImageConversion::ResizeMethod::NONE );
	pResizeMethodCombo->setCurrentIndex( settings.value( "resize_method" ).toInt() );
	connect( pResizeMethodCombo, &QComboBox::currentIndexChanged, this, [&]( int ind )
			 {
				 emit resizeMethodChanged( pResizeMethodCombo->itemData( ind ).value<vtfpp::ImageConversion::ResizeMethod>() );
			 } );

	resizeBoxLayout->addWidget( pResizeMethodCombo, 0, 1, Qt::AlignRight );

	auto resizeFilterLabel = new QLabel( tr( "Resize Filter:" ), vBoxResize );
	resizeBoxLayout->addWidget( resizeFilterLabel, 1, 0, Qt::AlignLeft );
	pResizeFilterCombo = new QComboBox( vBoxResize );
	pResizeFilterCombo->addItem( tr( "Default" ), (int)vtfpp::ImageConversion::ResizeFilter::DEFAULT );
	pResizeFilterCombo->addItem( tr( "Kaiser" ), (int)vtfpp::ImageConversion::ResizeFilter::KAISER );
	pResizeFilterCombo->addItem( tr( "Bi-linear" ), (int)vtfpp::ImageConversion::ResizeFilter::BILINEAR );
	pResizeFilterCombo->addItem( tr( "Nice" ), (int)vtfpp::ImageConversion::ResizeFilter::NICE );
	pResizeFilterCombo->addItem( tr( "Point Sample" ), (int)vtfpp::ImageConversion::ResizeFilter::POINT_SAMPLE );
	pResizeFilterCombo->addItem( tr( "Catmull Rom" ), (int)vtfpp::ImageConversion::ResizeFilter::CATMULL_ROM );
	pResizeFilterCombo->addItem( tr( "Cubic BSpline" ), (int)vtfpp::ImageConversion::ResizeFilter::CUBIC_BSPLINE );
	pResizeFilterCombo->addItem( tr( "Mitchell" ), (int)vtfpp::ImageConversion::ResizeFilter::MITCHELL );
	pResizeFilterCombo->addItem( tr( "Box" ), (int)vtfpp::ImageConversion::ResizeFilter::BOX );

	pResizeFilterCombo->setCurrentIndex( settings.value( "resize_filter" ).toInt() );

	resizeBoxLayout->addWidget( pResizeFilterCombo, 1, 1, Qt::AlignRight );
	pClampCheckbox = new QCheckBox( vBoxResize );
	pClampCheckbox->setText( tr( "Clamp" ) );

	pClampCheckbox->setChecked( settings.value( "should_clamp" ).toBool() );

	resizeBoxLayout->addWidget( pClampCheckbox, 2, 0, Qt::AlignLeft );

	auto clampWidthLabel = new QLabel( tr( "Maximum Width:" ), vBoxResize );
	resizeBoxLayout->addWidget( clampWidthLabel, 3, 0, Qt::AlignLeft );
	pClampWidthCombo = new QComboBox( vBoxResize );
	resizeBoxLayout->addWidget( pClampWidthCombo, 3, 1, Qt::AlignRight );

	auto clampHeightLabel = new QLabel( tr( "Maximum Height:" ), vBoxResize );
	resizeBoxLayout->addWidget( clampHeightLabel, 4, 0, Qt::AlignLeft );

	pClampHeightCombo = new QComboBox( vBoxResize );
	resizeBoxLayout->addWidget( pClampHeightCombo, 4, 1, Qt::AlignRight );

	for ( int i = 1; i <= 4096; i *= 2 )
	{
		pClampHeightCombo->addItem( QString::number( i ), i );
		pClampWidthCombo->addItem( QString::number( i ), i );
	}

	pClampWidthCombo->setCurrentIndex( settings.value( "clamp_width" ).toInt() );
	pClampHeightCombo->setCurrentIndex( settings.value( "clamp_height" ).toInt() );

	clampWidthLabel->setDisabled( true );
	clampHeightLabel->setDisabled( true );
	pClampHeightCombo->setDisabled( true );
	pClampWidthCombo->setDisabled( true );

	connect( pClampCheckbox, &QCheckBox::clicked, clampWidthLabel, &QWidget::setEnabled );
	connect( pClampCheckbox, &QCheckBox::clicked, clampHeightLabel, &QWidget::setEnabled );
	connect( pClampCheckbox, &QCheckBox::clicked, pClampHeightCombo, &QWidget::setEnabled );
	connect( pClampCheckbox, &QCheckBox::clicked, pClampWidthCombo, &QWidget::setEnabled );

	generalTabLayout->addWidget( vBoxResize, 0, 1 );

	generateMipmapBox = new QGroupBox( tr( "Generate Mipmaps" ), this );
	generateMipmapBox->setCheckable( true );
	generateMipmapBox->setChecked( false );
	if ( !this->standalone )
		generateMipmapBox->setChecked( settings.value( "gen_mipmaps" ).toBool() );

	auto mipmapBoxLayout = new QGridLayout( generateMipmapBox );

	auto mipmapCountLabel = new QLabel( tr( "Mipmap Count:" ), generateMipmapBox );
	mipmapBoxLayout->addWidget( mipmapCountLabel, 0, 0, Qt::AlignLeft );

	mipmapCountTextBox = new QLineEdit( generateMipmapBox );
	mipmapCountTextBox->setReadOnly( true );
	connect( this, &CGeneralTab::setMipmapTextBoxText, mipmapCountTextBox, &QLineEdit::setText );
	mipmapBoxLayout->addWidget( mipmapCountTextBox, 0, 1, Qt::AlignLeft );

	auto mipmapFilterLabel = new QLabel( tr( "Mipmap Filter:" ), generateMipmapBox );
	mipmapBoxLayout->addWidget( mipmapFilterLabel, 1, 0, Qt::AlignLeft );

	pMipmapFilterCombo = new QComboBox( generateMipmapBox );
	pMipmapFilterCombo->addItem( tr( "Default" ), (int)vtfpp::ImageConversion::ResizeFilter::DEFAULT );
	pMipmapFilterCombo->addItem( tr( "Kaiser" ), (int)vtfpp::ImageConversion::ResizeFilter::KAISER );
	pMipmapFilterCombo->addItem( tr( "Bi-linear" ), (int)vtfpp::ImageConversion::ResizeFilter::BILINEAR );
	pMipmapFilterCombo->addItem( tr( "Nice" ), (int)vtfpp::ImageConversion::ResizeFilter::NICE );
	pMipmapFilterCombo->addItem( tr( "Point Sample" ), (int)vtfpp::ImageConversion::ResizeFilter::POINT_SAMPLE );
	pMipmapFilterCombo->addItem( tr( "Catmull Rom" ), (int)vtfpp::ImageConversion::ResizeFilter::CATMULL_ROM );
	pMipmapFilterCombo->addItem( tr( "Cubic BSpline" ), (int)vtfpp::ImageConversion::ResizeFilter::CUBIC_BSPLINE );
	pMipmapFilterCombo->addItem( tr( "Mitchell" ), (int)vtfpp::ImageConversion::ResizeFilter::MITCHELL );
	pMipmapFilterCombo->addItem( tr( "Box" ), (int)vtfpp::ImageConversion::ResizeFilter::BOX );

	pMipmapFilterCombo->setCurrentIndex( settings.value( "mipmap_filter" ).toInt() );

	mipmapBoxLayout->addWidget( pMipmapFilterCombo, 1, 1, Qt::AlignRight );
	mipmapBoxLayout->setAlignment( Qt::AlignTop );

	generalTabLayout->addWidget( generateMipmapBox, 1, 1 );
}
void CGeneralTab::getVTFOptions( CVTFOptions &opts ) const
{
	opts.textureFormat = this->pFormatCombo->currentData().value<vtfpp::ImageFormat>();
	opts.alphaTextureFormat = this->pAlphaDetectedFormatCombo->currentData().value<vtfpp::ImageFormat>();
	opts.imageType = this->pTypeCombo->currentData().value<CVTFOptions::VTFType>();
	opts.version = this->pVtfVersionBox->currentData().value<uint8_t>();
	opts.enable_compression = this->CompressionBox->isChecked();
	opts.compression_method = this->CompressionTypeBox->currentData().value<vtfpp::CompressionMethod>();
	opts.compression_level = this->CompressionLevelBox->currentData().value<int8_t>();
	opts.srgb = this->pSRGBCheckBox->isChecked();
	opts.generate_thumbnail = this->pGenerateThumbnailCheckBox->isChecked();
	opts.compute_reflectivity = !this->computeReflectivityBox->isChecked();
	opts.lumen_red = this->pLuminanceWeightRedBox->value();
	opts.lumen_green = this->pLuminanceWeightGreenBox->value();
	opts.lumen_blue = this->pLuminanceWeightBlueBox->value();
	opts.resize_method = this->pResizeMethodCombo->currentData().value<vtfpp::ImageConversion::ResizeMethod>();
	opts.resize_filter = this->pResizeFilterCombo->currentData().value<vtfpp::ImageConversion::ResizeFilter>();
	opts.clamp = this->pClampCheckbox->isChecked();
	opts.max_width = this->pClampWidthCombo->currentData().value<uint16_t>();
	opts.max_height = this->pClampHeightCombo->currentData().value<uint16_t>();
	opts.generate_mipmaps = this->generateMipmapBox->isChecked();
	opts.mipmap_filter = this->pMipmapFilterCombo->currentData().value<vtfpp::ImageConversion::ResizeFilter>();
}
void CGeneralTab::setVTFData( vtfpp::VTF *vtf )
{
	vtfpp::ImageFormatDetails::transparent( vtf->getFormat() ) ?
		this->pAlphaDetectedFormatCombo->setCurrentIndex( this->pAlphaDetectedFormatCombo->findData( (int)vtf->getFormat() ) ) :
		this->pFormatCombo->setCurrentIndex( this->pFormatCombo->findData( (int)vtf->getFormat() ) );

	this->pVtfVersionBox->setCurrentIndex( vtf->getVersion() );
	bool comp = vtf->getCompressionLevel() > 0;
	this->CompressionBox->setChecked( comp );
	if ( comp )
	{
		this->CompressionTypeBox->setCurrentIndex( this->CompressionTypeBox->findData( (int)vtf->getCompressionMethod() ) );
		this->CompressionLevelBox->setCurrentIndex( vtf->getCompressionLevel() + 1 );
	}

	this->pSRGBCheckBox->setChecked( vtf->isSRGB() );

	auto lumen = vtf->getReflectivity();
	this->pLuminanceWeightRedBox->setValue( lumen[0] );
	this->pLuminanceWeightGreenBox->setValue( lumen[1] );
	this->pLuminanceWeightBlueBox->setValue( lumen[2] );

	// These 3 are set to false by default as we don't wanna re-generate the VTF's data without the user's consent.
	this->pGenerateThumbnailCheckBox->setChecked( false );
	this->generateMipmapBox->setChecked( false );
	this->computeReflectivityBox->setChecked( true ); // True = False in Inverse.
}

CResourceTab::CResourceTab( QWidget *parent, bool standalone ) :
	QWidget( parent ), standalone( standalone )
{
	auto settings = ApplicationOptions::getInstance()->get( IMPORT_MENU_SETTINGS, IMPORT_DEFAULTS ).toObject();

	auto resourceLayout = new QVBoxLayout( this );
	resourceLayout->setAlignment( Qt::AlignTop );

	lodControlResourceBox = new QGroupBox( tr( "LOD Control Resource" ), this );
	lodControlResourceBox->setCheckable( true );
	lodControlResourceBox->setChecked( settings.value( "lod_control_enabled" ).toBool() );
	auto lodControlResourceLayout = new QGridLayout( lodControlResourceBox );

	auto strengthLabel = new QLabel( tr( "Strength:" ), this );
	lodControlResourceLayout->addWidget( strengthLabel, 1, 0, Qt::AlignLeft );
	pControlResourceCrampUBox = new QDoubleSpinBox( this );
	pControlResourceCrampUBox->setValue( settings.value( "lod_strength" ).toDouble() );
	lodControlResourceLayout->addWidget( pControlResourceCrampUBox, 1, 1, Qt::AlignLeft );

	auto thresholdLabel = new QLabel( tr( "Threshold:" ), this );
	lodControlResourceLayout->addWidget( thresholdLabel, 2, 0, Qt::AlignLeft );
	pControlResourceCrampVBox = new QDoubleSpinBox( this );
	pControlResourceCrampVBox->setValue( settings.value( "lod_threshold" ).toDouble() );
	lodControlResourceLayout->addWidget( pControlResourceCrampVBox, 2, 1, Qt::AlignLeft );

	resourceLayout->addWidget( lodControlResourceBox );

	informationResourceBox = new QGroupBox( tr( "Information Resource" ), this );
	informationResourceBox->setCheckable( true );
	informationResourceBox->setChecked( settings.value( "information_enabled" ).toBool() );
	auto informationResourceLayout = new QGridLayout( informationResourceBox );

	pWarningLabel = new QLabel( tr( "WARNING: RESOURCE UNSUPPORTED!\nThis resource is not officially supported and is known to cause VTFs to not load by the engine.\nOnly the Strata Source Engine can reliably load VTFs with this resource." ) );
	informationResourceLayout->addWidget( pWarningLabel, 0, 0, 1, 3 );

	auto authorLabel = new QLabel( tr( "Author:" ), this );
	informationResourceLayout->addWidget( authorLabel, 1, 0, Qt::AlignLeft );
	pInformationResourceAuthor = new QLineEdit( this );
	pInformationResourceAuthor->setText( settings.value( "information_author" ).toString() );
	informationResourceLayout->addWidget( pInformationResourceAuthor, 1, 1, Qt::AlignLeft );

	auto contactLabel = new QLabel( tr( "Contact:" ), this );
	informationResourceLayout->addWidget( contactLabel, 2, 0, Qt::AlignLeft );
	pInformationResourceContact = new QLineEdit( this );
	pInformationResourceContact->setText( settings.value( "information_contact" ).toString() );
	informationResourceLayout->addWidget( pInformationResourceContact, 2, 1, Qt::AlignLeft );

	auto organizationLabel = new QLabel( tr( "Organization:" ), this );
	informationResourceLayout->addWidget( organizationLabel, 3, 0, Qt::AlignLeft );
	pInformationResourceOrganization = new QLineEdit( this );
	pInformationResourceOrganization->setText( settings.value( "information_organization" ).toString() );
	informationResourceLayout->addWidget( pInformationResourceOrganization, 3, 1, Qt::AlignLeft );

	auto versionLabel = new QLabel( tr( "Version:" ), this );
	informationResourceLayout->addWidget( versionLabel, 4, 0, Qt::AlignLeft );
	pInformationResourceVersion = new QLineEdit( this );
	informationResourceLayout->addWidget( pInformationResourceVersion, 4, 1, Qt::AlignLeft );

	auto modificationLabel = new QLabel( tr( "Modification:" ), this );
	informationResourceLayout->addWidget( modificationLabel, 5, 0, Qt::AlignLeft );
	pInformationResourceModification = new QLineEdit( this );
	informationResourceLayout->addWidget( pInformationResourceModification, 5, 1, Qt::AlignLeft );

	auto descriptionLabel = new QLabel( tr( "Description:" ), this );
	informationResourceLayout->addWidget( descriptionLabel, 6, 0, Qt::AlignLeft );
	pInformationResourceDescription = new QLineEdit( this );
	informationResourceLayout->addWidget( pInformationResourceDescription, 6, 1, Qt::AlignLeft );

	auto commentsLabel = new QLabel( tr( "Comments:" ), this );
	informationResourceLayout->addWidget( commentsLabel, 7, 0, Qt::AlignLeft );
	pInformationResourceComments = new QLineEdit( this );
	informationResourceLayout->addWidget( pInformationResourceComments, 7, 1, Qt::AlignLeft );

	resourceLayout->addWidget( informationResourceBox );
}

void CResourceTab::getVTFOptions( CVTFOptions &opts ) const
{
	opts.lod_control_resource = this->lodControlResourceBox->isChecked();
	opts.lod_control_strength = this->pControlResourceCrampUBox->value();
	opts.lod_control_threshold = this->pControlResourceCrampVBox->value();
	opts.information_resource = this->informationResourceBox->isChecked();
	opts.information_author = this->pInformationResourceAuthor->text();
	opts.information_contact = this->pInformationResourceContact->text();
	opts.information_organization = this->pInformationResourceOrganization->text();
	opts.information_version = this->pInformationResourceVersion->text();
	opts.information_modification = this->pInformationResourceModification->text();
	opts.information_description = this->pInformationResourceDescription->text();
	opts.information_comments = this->pInformationResourceComments->text();
}
void CResourceTab::setVTFData( vtfpp::VTF *vtf )
{
	auto lodResource = vtf->getResource( vtfpp::Resource::TYPE_LOD_CONTROL_INFO );
	this->lodControlResourceBox->setChecked( lodResource );
	if ( lodResource )
	{
		auto [u, v, o, a] = lodResource->getDataAsLODControlInfo();
		this->pControlResourceCrampUBox->setValue( u );
		this->pControlResourceCrampVBox->setValue( v );
	}
	auto informationResource = vtf->getResource( vtfpp::Resource::TYPE_KEYVALUES_DATA );
	this->informationResourceBox->setChecked( informationResource );
	if ( informationResource )
	{
		auto rawKV = informationResource->getDataAsKeyValuesData();
		kvpp::KV1 kv { rawKV };
		const auto &info = kv["Information"];
		if ( info.isInvalid() )
			return;
		this->pInformationResourceAuthor->setText( std::string( info["Author"].getValue() ).data() );
		this->pInformationResourceContact->setText( std::string( info["Contact"].getValue() ).data() );
		this->pInformationResourceOrganization->setText( std::string( info["Organization"].getValue() ).data() );
		this->pInformationResourceVersion->setText( std::string( info["Version"].getValue() ).data() );
		this->pInformationResourceModification->setText( std::string( info["Modification"].getValue() ).data() );
		this->pInformationResourceDescription->setText( std::string( info["Description"].getValue() ).data() );
		this->pInformationResourceComments->setText( std::string( info["Comments"].getValue() ).data() );
	}
}
void CResourceTab::markResourceAsDangerous( bool mark )
{
	this->pWarningLabel->setHidden( mark );
}

void CInverseGroupBox::paintEvent( QPaintEvent *event )
{
	auto painter = QStylePainter( this );
	auto option = QStyleOptionGroupBox();
	this->initStyleOption( &option );
	if ( this->isCheckable() )
	{
		option.state &= QStyle::State_Off & QStyle::State_On;
		option.state |= ( this->isChecked() ? QStyle::State_Off : QStyle::State_On );
		painter.drawComplexControl( QStyle::CC_GroupBox, option );
	}
}
