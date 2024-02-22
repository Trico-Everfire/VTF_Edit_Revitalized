#include "VTFEImport.h"

#define STB_IMAGE_IMPLEMENTATION

#include "../libs/stb/stb_image.h"
#include "ImageSettingsWidget.h"
#include "MainWindow.h"
#include "flagsandformats.hpp"
#include "supported_formats/TiffSupport.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCommandLineParser>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <cmath>

VTFEImport::VTFEImport( QWidget *pParent, const QString &filePath, bool &hasData ) :
	QDialog( pParent )
{
	hasData = true;

	AddImage( filePath );

	if ( imageList.isEmpty() )
	{
		hasData = false;
		return;
	}

	SetDefaults();

	InitializeWidgets();

	pGeneralTab->pFormatCombo->setCurrentIndex( pGeneralTab->pFormatCombo->findData( imageList[0]->getFormat() ) );
}

VTFEImport::VTFEImport( QWidget *pParent, const QStringList &filePaths, bool &hasData ) :
	QDialog( pParent )
{
	hasData = true;

	for ( int i = 0; i < filePaths.count(); i++ )
		AddImage( filePaths[i] );

	if ( imageList.isEmpty() )
	{
		hasData = false;
		return;
	}

	SetDefaults();

	InitializeWidgets();

	pGeneralTab->pFormatCombo->setCurrentIndex( pGeneralTab->pFormatCombo->findData( imageList[0]->getFormat() ) );
	pGeneralTab->pAlphaDetectedFormatCombo->setCurrentIndex( pGeneralTab->pAlphaDetectedFormatCombo->findData( imageList[0]->getFormat() ) );
}

void VTFEImport::SetDefaults()
{
	this->setWindowTitle( tr( "VTF Options" ) );
	// filling default information.
	VTFCreateOptions.ImageFormat = VTFImageFormat::IMAGE_FORMAT_RGBA32323232F;
	VTFCreateOptions.uiVersion[0] = 7;
	VTFCreateOptions.uiVersion[1] = 5;
	VTFCreateOptions.uiStartFrame = 0;
	VTFCreateOptions.bResize = true;
	VTFCreateOptions.bMipmaps = true;
	VTFCreateOptions.ResizeMethod = VTFResizeMethod::RESIZE_NEAREST_POWER2;
	VTFCreateOptions.bResizeClamp = true;
	VTFCreateOptions.uiResizeClampWidth = 2048;
	VTFCreateOptions.uiResizeClampHeight = 2048;
}

void VTFEImport::AddImage( const QString &qString )
{
	const char *file = qString.toUtf8().constData();

	if ( qString.endsWith( ".tif" ) || qString.endsWith( ".tiff" ) )
	{
		//		ImageData_t data;
		//		memset( &data, 0, sizeof( ImageData_t ) );
		//		TiffSupport::Load_TIFF( file, &data );
		//
		//		//		if ( !data.bValidImageData )
		//		//			return;
		//
		//		tagVTFImageFormat format = IMAGE_FORMAT_NONE;
		//		int totalDataSize = data.nImageWidth * data.nImageHeight * data.nBitCountPerChannel * data.nCountChannels;
		//
		//		auto dat = std::vector<std::byte> {};
		//		dat.resize( totalDataSize );
		//
		//		switch ( data.nBitCountPerChannel )
		//		{
		//			case 8:
		//				format = data.bHasAlpha ? IMAGE_FORMAT_RGBA8888 : IMAGE_FORMAT_RGB888;
		//				memcpy( dat.data(), data.pImageData, totalDataSize );
		//				break;
		//			case 16:
		//				format = IMAGE_FORMAT_RGBA16161616F;
		//				if ( !data.bHasAlpha )
		//				{
		//					auto tempBuff = std::vector<uint16_t> {};
		//					// RGB16F does not exist in source, we are forced to upgrade to RGBA16F.
		//					for ( int i = 0; i < totalDataSize; i += 3 )
		//					{
		//						tempBuff.push_back( data.pImageData16f[i] );
		//						tempBuff.push_back( data.pImageData16f[i + 1] );
		//						tempBuff.push_back( data.pImageData16f[i + 2] );
		//						tempBuff.push_back( 65535 ); // Alpha set to max.
		//					}
		//					memcpy( dat.data(), tempBuff.data(), totalDataSize );
		//				}
		//				else
		//				{
		//					memcpy( dat.data(), data.pImageData16f, totalDataSize );
		//				}
		//				break;
		//			case 32:
		//				format = data.bHasAlpha ? IMAGE_FORMAT_RGBA32323232F : IMAGE_FORMAT_RGB323232F;
		//				memcpy( dat.data(), data.pImageData32f, totalDataSize );
		//				break;
		//			default:
		//				return;
		//		}

		TIFFFile tiffFIle;

		bool success = TiffSupport::Load_TIFF( file, tiffFIle );

		if ( !success || !tiffFIle.isValid )
			return;

		tagVTFImageFormat format = IMAGE_FORMAT_NONE;
		switch ( tiffFIle.type )
		{
			case 8:
				format = tiffFIle.hasAlpha ? IMAGE_FORMAT_RGBA8888 : IMAGE_FORMAT_RGB888;
				break;
			case 16:
				format = IMAGE_FORMAT_RGBA16161616F;
				//				{
				//					std::vector<short> tempSHRTData;
				//					std::vector<short> tempSHRTAlphaData {};
				//					//					tempSHRTData.resize( tiffFIle.imageData.size() );
				//					int newsize = tiffFIle.imageData.size() * 1.3;
				//					//					tempSHRTAlphaData.resize( newsize );
				//
				//					memcpy( tempSHRTData.data(), tiffFIle.imageData.data(), tiffFIle.imageData.size() );
				//
				//					for ( auto it = tempSHRTData.begin(); it != tempSHRTData.end(); it += 3 )
				//					{
				//						tempSHRTAlphaData.push_back( 32767 );
				//						tempSHRTAlphaData.push_back( 32767 );
				//						tempSHRTAlphaData.push_back( 32767 );
				//						tempSHRTAlphaData.push_back( 32767 );
				//					}
				//
				//					tiffFIle.imageData = std::vector<std::byte>();
				//					tiffFIle.imageData.resize( newsize );
				//					memcpy( tiffFIle.imageData.data(), tempSHRTAlphaData.data(), newsize );
				//				}
				break;
			case 32:
				format = tiffFIle.hasAlpha ? IMAGE_FORMAT_RGBA32323232F : IMAGE_FORMAT_RGB323232F;
				break;
			default:
				return;
		}

		imageList[imageList.size()] = new VTFEImageFormat(
			reinterpret_cast<vlByte *>( tiffFIle.imageData.data() ), tiffFIle.width, tiffFIle.height, 0, format );
		return;
	}

	int x, y, n;

	if ( !stbi_is_hdr( file ) )
	{
		vlByte *data = stbi_load( file, &x, &y, &n, 4 );

		if ( !data )
			return;

		imageList[imageList.size()] = new VTFEImageFormat(
			data, x, y, 0, IMAGE_FORMAT_RGBA8888 );

		stbi_image_free( data );
	}
	else
	{
		float *data = stbi_loadf( file, &x, &y, &n, 0 );

		if ( !data )
			return;

		auto convertedData = reinterpret_cast<vlByte *>( data );

		tagVTFImageFormat format = n > 3 ? IMAGE_FORMAT_RGBA32323232F : IMAGE_FORMAT_RGB323232F;

		imageList[imageList.size()] = new VTFEImageFormat(
			convertedData, x, y, 0, format );

		stbi_image_free( data );
	}
}

VTFLib::CVTFFile *VTFEImport::GenerateVTF( VTFErrorType &err )
{
	if ( imageList.isEmpty() )
	{
		err = VTFErrorType::NO_DATA;
		return nullptr;
	}

	auto vFile = new VTFLib::CVTFFile;

	VTFCreateOptions.ImageFormat = static_cast<tagVTFImageFormat>( VTFLib::CVTFFile::GetImageFormatInfo( imageList[0]->getFormat() ).uiAlphaBitsPerPixel == 0 ? pGeneralTab->pFormatCombo->currentData().toInt() : pGeneralTab->pAlphaDetectedFormatCombo->currentData().toInt() );
	VTFCreateOptions.uiVersion[0] = 7;
	VTFCreateOptions.uiVersion[1] = pAdvancedTab->pVtfVersionBox->currentData().toInt();
	VTFCreateOptions.bResize = ( pGeneralTab->pResizeMethodCombo->isEnabled() && pGeneralTab->pResizeCheckbox->isChecked() );
	VTFCreateOptions.bMipmaps =
		( pGeneralTab->pGenerateMipmapsCheckbox->isEnabled() && pGeneralTab->pGenerateMipmapsCheckbox->isChecked() );
	VTFCreateOptions.MipmapFilter = static_cast<VTFMipmapFilter>( pGeneralTab->pMipmapFilterCombo->currentData().toInt() );
	VTFCreateOptions.ResizeMethod = static_cast<VTFResizeMethod>( pGeneralTab->pResizeMethodCombo->currentData().toInt() );
	VTFCreateOptions.bResizeClamp = ( pGeneralTab->pClampCheckbox->isEnabled() && pGeneralTab->pClampCheckbox->isChecked() );
	;
	VTFCreateOptions.uiResizeClampWidth = pGeneralTab->pClampWidthCombo->currentData().toInt();
	VTFCreateOptions.uiResizeClampHeight = pGeneralTab->pClampHeightCombo->currentData().toInt();
	VTFCreateOptions.bReflectivity =
		( pAdvancedTab->pComputeReflectivityCheckBox->isEnabled() &&
		  pAdvancedTab->pComputeReflectivityCheckBox->isChecked() );
	VTFCreateOptions.sReflectivity[0] = pAdvancedTab->pLuminanceWeightRedBox->value();
	VTFCreateOptions.sReflectivity[1] = pAdvancedTab->pLuminanceWeightGreenBox->value();
	VTFCreateOptions.sReflectivity[2] = pAdvancedTab->pLuminanceWeightBlueBox->value();
	VTFCreateOptions.bThumbnail =
		( pAdvancedTab->pGenerateThumbnailCheckBox->isEnabled() &&
		  pAdvancedTab->pGenerateThumbnailCheckBox->isChecked() );
	VTFCreateOptions.bGammaCorrection =
		( pAdvancedTab->pGammaCorrectionCheckBox->isEnabled() && pAdvancedTab->pGammaCorrectionCheckBox->isChecked() );
	VTFCreateOptions.sGammaCorrection = pAdvancedTab->pGammaCorrectionBox->value();
	VTFCreateOptions.bSphereMap =
		( pAdvancedTab->pGenerateSphereMapCheckBox->isEnabled() &&
		  pAdvancedTab->pGenerateSphereMapCheckBox->isChecked() );

	VTFCreateOptions.bSRGB = pGeneralTab->pSRGBCheckbox->isChecked();

	if ( vtfImageFlags != 0 )
	{
		VTFCreateOptions.uiFlags = vtfImageFlags;
	}

	auto pFFSArray = new vlByte *[imageList.size()];

	for ( int i = 0; i < imageList.size(); i++ )
	{
		vlByte *imgData;
		if ( !( VTFCreateOptions.ImageFormat == IMAGE_FORMAT_RGBA32323232F || VTFCreateOptions.ImageFormat == IMAGE_FORMAT_RGB323232F || VTFCreateOptions.ImageFormat == IMAGE_FORMAT_RGBA16161616F || VTFCreateOptions.ImageFormat == IMAGE_FORMAT_R32F ) )
		{
			imgData = new vlByte[VTFLib::CVTFFile::ComputeImageSize( imageList[i]->getWidth(), imageList[i]->getHeight(), 1, IMAGE_FORMAT_RGBA8888 )];
			VTFLib::CVTFFile::Convert( imageList[i]->getData(), imgData, imageList[i]->getWidth(), imageList[i]->getHeight(), imageList[i]->getFormat(), IMAGE_FORMAT_RGBA8888 );
		}
		else
		{
			imgData = new vlByte[imageList[i]->getSize()];
			memcpy( imgData, imageList[i]->getData(), imageList[i]->getSize() );
		}

#ifdef COLOR_CORRECTION
		for ( int s = 0; s < images_[i]->getSize(); s += 4 )
		{
			//			int rgb1[3] = {0, 0, 0};
			//			int rgb2[3] = {0, 0, 0};
			//			int rgb3[3] = {0, 0, 0};
			//			auto currentColor = new QColor(imgData[s],imgData[s + 1],imgData[s + 2]);
			//			auto currentCMYK = currentColor->toCmyk();
			//			float r = pAdvancedTab->colorCorrectionDialog_->color().saturationF();

			//			currentColor->setCmyk(((currentCMYK.cyan()*(1 - r)) + (CMYK.cyan() * r)), ((currentCMYK.magenta()*(1 - r)) + (CMYK.magenta() * r)),((currentCMYK.yellow()*(1 - r)) - (CMYK.yellow() * r)), ((currentCMYK.black()*(1 - r)) - (CMYK.black() * r)) );
			//			auto currentRGB = currentCMYK.toRgb();
			//			auto RGB = CMYK.toRgb();

			//			Advanced::HSVtoRGB((HSV.hueF() ) * 360,HSV.saturationF() * 100,HSV.valueF() * 100, rgb1);
			//			Advanced::HSVtoRGB(HSV.hueF() * 360,HSV.saturationF() * 100,HSV.valueF() * 100, rgb2);
			//			Advanced::HSVtoRGB(HSV.hueF() * 360,HSV.saturationF() * 100,HSV.valueF() * 100, rgb3);

			//			float hue = currentColor->hueF() + pAdvancedTab->colorCorrectionDialog_->color().hueF();
			//			hue /= (hue / 2);
			//			float saturation = currentColor->saturationF() + pAdvancedTab->colorCorrectionDialog_->color().saturationF();
			//			saturation /= (saturation / 2);
			//			float value = currentColor->valueF() + pAdvancedTab->colorCorrectionDialog_->color().valueF();
			//			value /= (value / 2);
			//			currentColor->setHsvF(hue, saturation, value);

			//			currentColor->setHsvF()

			//			imgData[s] = currentColor->red();
			//			imgData[s+1] = currentColor->green();
			//			imgData[s+2] = currentColor->blue();
			// imgData[s+3] = (imgData[s + 3] - HSV.alpha()) * 2;
		}
#endif

		pFFSArray[i] = const_cast<vlByte *>( imgData );
	}

	int frames = pGeneralTab->pTypeCombo->currentIndex() == 0 ? imageList.size() : 1;
	int faces = pGeneralTab->pTypeCombo->currentIndex() == 1 ? imageList.size() : 1;
	int slices = pGeneralTab->pTypeCombo->currentIndex() == 2 ? imageList.size() : 1;

	if ( !vFile->Create( imageList[0]->getWidth(), imageList[0]->getHeight(), frames, faces, slices, pFFSArray, VTFCreateOptions, imageList[0]->getFormat() ) )
	{
		err = VTFErrorType::INVALID_IMAGE;
		delete vFile;
		return nullptr;
	};

	vFile->SetFlag( VTFImageFlag::TEXTUREFLAGS_SRGB, VTFCreateOptions.bSRGB );

	if ( !vFile->IsLoaded() )
	{
		err = VTFErrorType::INVALID_IMAGE;
		delete vFile;
		return nullptr;
	}

	if ( vFile->GetSupportsResources() )
	{
		bool bResult = true;

		if ( pResourceTab->pLodControlResourceCheckBox->isChecked() )
		{
			SVTFTextureLODControlResource LODControlResource;
			memset( &LODControlResource, 0, sizeof( SVTFTextureLODControlResource ) );
			LODControlResource.ResolutionClampU = pResourceTab->pControlResourceCrampUBox->value();
			LODControlResource.ResolutionClampV = pResourceTab->pControlResourceCrampVBox->value();

			bResult &= vFile->SetResourceData( VTF_RSRC_TEXTURE_LOD_SETTINGS, sizeof( SVTFTextureLODControlResource ), &LODControlResource ) != nullptr;
		}

		if ( pResourceTab->pCreateInformationResourceCheckBox->isChecked() )
		{
			auto pVMTFile = new VTFLib::CVMTFile();

			pVMTFile->Create( "Information" );
			if ( pResourceTab->pInformationResourceAuthor->text().length() > 0 )
			{
				pVMTFile->GetRoot()->AddStringNode( "Author", pResourceTab->pInformationResourceAuthor->text().toUtf8().constData() );
			}
			if ( pResourceTab->pInformationResouceContact->text().length() > 0 )
			{
				pVMTFile->GetRoot()->AddStringNode( "Contact", pResourceTab->pInformationResouceContact->text().toUtf8().constData() );
			}
			if ( pResourceTab->pInformationResouceVersion->text().length() > 0 )
			{
				pVMTFile->GetRoot()->AddStringNode( "Version", pResourceTab->pInformationResouceVersion->text().toUtf8().constData() );
			}
			if ( pResourceTab->pInformationResouceModification->text().length() > 0 )
			{
				pVMTFile->GetRoot()->AddStringNode( "Modification", pResourceTab->pInformationResouceModification->text().toUtf8().constData() );
			}
			if ( pResourceTab->pInformationResouceDescription->text().length() > 0 )
			{
				pVMTFile->GetRoot()->AddStringNode( "Description", pResourceTab->pInformationResouceDescription->text().toUtf8().constData() );
			}
			if ( pResourceTab->pInformationResouceComments->text().length() > 0 )
			{
				pVMTFile->GetRoot()->AddStringNode( "Comments", pResourceTab->pInformationResouceComments->text().toUtf8().constData() );
			}

			vlUInt uiSize = 0;
			vlByte lpBuffer[65536];
			if ( pVMTFile->Save( lpBuffer, sizeof( lpBuffer ), uiSize ) )
			{
				bResult &= vFile->SetResourceData( VTF_RSRC_KEY_VALUE_DATA, uiSize, lpBuffer ) != nullptr;
			}

			delete pVMTFile;
		}

		if ( !bResult )
		{
			QMessageBox::warning( this, "Failed to apply resources", "Unable to apply resources. ", QMessageBox::Ok );
		}
	}

#ifdef NORMAL_GENERATION
	if ( pGeneralTab->generateNormalMapCheckbox_->isEnabled() && pGeneralTab->generateNormalMapCheckbox_->isChecked() )
	{
		vFile->SetFlag( TEXTUREFLAGS_NORMAL, true );

		if ( !vFile->IsLoaded() )
			return VTFErrorType::INVALIDIMAGE;

		if ( vFile->GetFlags() & TEXTUREFLAGS_ENVMAP )
		{
			VTFLib::LastError.Set( "Image is an enviroment map." );
			return VTFErrorType::INVALIDIMAGE;
		}

		if ( !vFile->GetHasImage() )
		{
			VTFLib::LastError.Set( "No image data to generate normal map from." );
			return VTFErrorType::INVALIDIMAGE;
		}

		vlByte *lpData = vFile->GetData( 0, 0, 0, 0 );

		// Will hold frame's converted image data.
		vlByte *lpSource =
			new vlByte[vFile->ComputeImageSize( vFile->GetWidth(), vFile->GetHeight(), 1, IMAGE_FORMAT_RGBA32323232F )];

		// Get the frame's image data.
		if ( !vFile->Convert(
				 lpData, lpSource, vFile->GetWidth(), vFile->GetHeight(), vFile->GetFormat(),
				 IMAGE_FORMAT_RGBA32323232F ) )
		{
			//			delete []lpSource;
			return VTFErrorType::INVALIDIMAGE;
		}

		// Will hold normal image data.
		vlByte *lpDest =
			new vlByte[vFile->ComputeImageSize( vFile->GetWidth(), vFile->GetHeight(), 1, vFile->GetFormat() )];

		// toGreyScale(lpSource, vFile->ComputeImageSize(vFile->GetWidth(), vFile->GetHeight(), 1,
		// IMAGE_FORMAT_RGBA8888) ,255, 255, 255, 255); lpDest = TEScO::generateFormattedHeightmap(lpSource,
		// vFile->GetWidth(), vFile->GetHeight());

		// Set the frame's image data.
		if ( !vFile->Convert(
				 lpSource /*lpDest*/, lpDest, vFile->GetWidth(), vFile->GetHeight(), IMAGE_FORMAT_RGBA32323232F,
				 vFile->GetFormat() ) )
		{
			//			delete []lpSource;	// Moved from above.
			//			delete []lpDest;
			return VTFErrorType::INVALIDIMAGE;
		}
		vFile->SetData( 0, 0, 0, 0, lpDest );
		//		delete []lpSource;	// Moved from above.
		//		delete []lpDest;

		// qInfo() <<
		// vFile->GenerateNormalMap(static_cast<VTFKernelFilter>(pGeneralTab->kernelFilterCombo_->currentData().toInt()),
		// static_cast<VTFHeightConversionMethod>(pGeneralTab->heightConversionCombo_->currentData().toInt()),static_cast<VTFNormalAlphaResult>(pGeneralTab->normalAlphaResultCombo_->currentData().toInt()));
	}
#endif

#ifdef CHAOS_INITIATIVE
	if ( pAdvancedTab->pAuxCompressionBox->isEnabled() && pAdvancedTab->pAuxCompressionBox->isChecked() )
		vFile->SetAuxCompressionLevel( pAdvancedTab->pAuxCompressionLevelBox->currentData().toInt() );
#endif

	for ( int i = 0; i < imageList.size(); i++ )
		delete[] pFFSArray[i];

	delete[] pFFSArray;

	err = VTFErrorType::SUCCESS;
	return vFile;
}

vlBool VTFEImport::IsPowerOfTwo( vlUInt uiSize )
{
	return uiSize > 0 && ( uiSize & ( uiSize - 1 ) ) == 0;
}

VTFEImport::VTFEImport( QWidget *pParent ) :
	QDialog( pParent )
{
	this->setWindowTitle( tr( "VTF Options" ) );
}

void VTFEImport::InitializeWidgets()
{
	auto vBLayout = new QGridLayout( this );
	auto widget = new QTabWidget( this );

	pGeneralTab = new GeneralTab( this );
	pAdvancedTab = new AdvancedTab( this );
	pResourceTab = new ResourceTab( this );

	widget->addTab( pGeneralTab, tr( "General" ) );
	widget->addTab( pAdvancedTab, tr( "Advanced" ) );
	widget->addTab( pResourceTab, tr( "Resource" ) );
	vBLayout->addWidget( widget, 0, 0, 1, 2 );
	auto pPreviewButton = new QPushButton( this );
	pPreviewButton->setText( tr( "Preview" ) );
	connect(
		pPreviewButton, &QPushButton::pressed,
		[&]()
		{
			auto dialog = new QDialog();
			auto vRLayout = new QGridLayout( dialog );

			auto scrollArea = new ui::ZoomScrollArea( dialog );
			auto vIVW = new ImageViewWidget( scrollArea );
			scrollArea->setWidget( vIVW );
			auto vISW = new ImageSettingsWidget( vIVW, dialog );
			vRLayout->addWidget( vISW, 0, 0 );
			vRLayout->addWidget( scrollArea, 0, 1, Qt::AlignCenter );

			connect( scrollArea, &ui::ZoomScrollArea::onScrollUp, dialog, [vIVW]
					 {
						 vIVW->zoom( 0.1 );
					 } );

			connect( scrollArea, &ui::ZoomScrollArea::onScrollDown, dialog, [vIVW]
					 {
						 vIVW->zoom( -0.1 );
					 } );

			auto pMainMenuBar = new QMenuBar( dialog );

			auto pViewMenu = pMainMenuBar->addMenu( "View" );
			auto redBox = ui::CMainWindow::createCheckableAction( "Red", pViewMenu );
			auto greenBox = ui::CMainWindow::createCheckableAction( "Green", pViewMenu );
			auto blueBox = ui::CMainWindow::createCheckableAction( "Blue", pViewMenu );
			auto alphaBox = ui::CMainWindow::createCheckableAction( "Alpha", pViewMenu );

			connect( redBox, &QAction::triggered, dialog, [vIVW]( bool checked )
					 {
						 vIVW->set_red( checked );
					 } );
			connect( greenBox, &QAction::triggered, dialog, [vIVW]( bool checked )
					 {
						 vIVW->set_green( checked );
					 } );
			connect( blueBox, &QAction::triggered, dialog, [vIVW]( bool checked )
					 {
						 vIVW->set_blue( checked );
					 } );
			connect( alphaBox, &QAction::triggered, dialog, [vIVW]( bool checked )
					 {
						 vIVW->set_alpha( checked );
					 } );

			pViewMenu->addAction( redBox );
			pViewMenu->addAction( greenBox );
			pViewMenu->addAction( blueBox );
			pViewMenu->addAction( alphaBox );

			vRLayout->setMenuBar( pMainMenuBar );

			VTFErrorType err;
			auto vtfFile = GenerateVTF( err );
			if ( err == SUCCESS )
			{
				vIVW->set_vtf( vtfFile );
				vISW->set_vtf( vtfFile );
			}
			scrollArea->resize( dialog->width() + vISW->width(), dialog->height() );
			dialog->setAttribute( Qt::WA_DeleteOnClose );
			dialog->exec();
			delete vtfFile;
		} );
	vBLayout->addWidget( pPreviewButton, 1, 0, Qt::AlignLeft );

	auto blayoutBox = new QDialogButtonBox( this );
	auto accept = blayoutBox->addButton( "Accept", QDialogButtonBox::AcceptRole );
	auto cancelled = blayoutBox->addButton( "Cancel", QDialogButtonBox::RejectRole );
	connect(
		accept, &QPushButton::pressed,
		[this]
		{
			isCancelled = false;
			close();
		} );
	connect(
		cancelled, &QPushButton::pressed,
		[this]
		{
			close();
		} );
	vBLayout->addWidget( blayoutBox, 1, 1, Qt::AlignRight );
}

VTFEImport *VTFEImport::FromVTF( QWidget *pParent, VTFLib::CVTFFile *pFile )
{
	auto vVTFImport = new VTFEImport( pParent );

	int type = 0;
	vlUInt fImageAmount = pFile->GetFrameCount();
	if ( pFile->GetFaceCount() > fImageAmount )
	{
		fImageAmount = pFile->GetFaceCount();
		type = 1;
	}
	if ( pFile->GetDepth() > fImageAmount )
	{
		fImageAmount = pFile->GetDepth();
		type = 2;
	}

	for ( int i = 0; i < fImageAmount; i++ )
	{
		vlUInt frames = type == 0 ? i + 1 : 1;
		vlUInt faces = type == 1 ? i + 1 : 1;
		vlUInt slices = type == 2 ? i + 1 : 1;

		vVTFImport->imageList[vVTFImport->imageList.size()] =
			new VTFEImageFormat( pFile->GetData( frames, faces, slices, 0 ), pFile->GetWidth(), pFile->GetHeight(), pFile->GetDepth(), pFile->GetFormat() );
	}

	vVTFImport->InitializeWidgets();

	vVTFImport->pAdvancedTab->pVtfVersionBox->setCurrentIndex( pFile->GetMinorVersion() );
	emit vVTFImport->pAdvancedTab->pVtfVersionBox->currentTextChanged( "7." + QString::number( pFile->GetMinorVersion() ) );
#ifdef CHAOS_INITIATIVE
	vVTFImport->pAdvancedTab->pAuxCompressionBox->setChecked( pFile->GetAuxCompressionLevel() > 0 );
	emit vVTFImport->pAdvancedTab->pAuxCompressionBox->clicked( pFile->GetAuxCompressionLevel() > 0 );
	if ( pFile->GetAuxCompressionLevel() > 0 )
		vVTFImport->pAdvancedTab->pAuxCompressionLevelBox->setCurrentIndex( pFile->GetAuxCompressionLevel() );
#endif
	vVTFImport->pGeneralTab->pGenerateMipmapsCheckbox->setChecked( pFile->GetMipmapCount() > 0 );
	emit vVTFImport->pGeneralTab->pGenerateMipmapsCheckbox->clicked( pFile->GetMipmapCount() > 0 );
	vVTFImport->pGeneralTab->pTypeCombo->setCurrentIndex( type );
	vVTFImport->pGeneralTab->pTypeCombo->currentTextChanged( QString::number( type ) );
	vVTFImport->pGeneralTab->pFormatCombo->setCurrentIndex(
		vVTFImport->pGeneralTab->pFormatCombo->findData( pFile->GetFormat() ) );
	vVTFImport->pGeneralTab->pAlphaDetectedFormatCombo->setCurrentIndex(
		vVTFImport->pGeneralTab->pAlphaDetectedFormatCombo->findData( pFile->GetFormat() ) );
	vlSingle r;
	vlSingle g;
	vlSingle b;
	pFile->GetReflectivity( r, g, b );
	vVTFImport->pAdvancedTab->pLuminanceWeightRedBox->setValue( r );
	vVTFImport->pAdvancedTab->pLuminanceWeightGreenBox->setValue( g );
	vVTFImport->pAdvancedTab->pLuminanceWeightBlueBox->setValue( b );

	vVTFImport->pGeneralTab->pSRGBCheckbox->setChecked( pFile->GetFlag( VTFImageFlag::TEXTUREFLAGS_SRGB ) );

	vVTFImport->vtfImageFlags = pFile->GetFlags();

	return vVTFImport;
}

GeneralTab::GeneralTab( VTFEImport *parent ) :
	QDialog( parent )
{
	pMainLayout = new QGridLayout( this );
	pMainLayout->setAlignment( Qt::AlignTop );
	GeneralOptions();
	GeneralResize();
	GeneralMipMaps();
	GeneralCustomMipmaps();
#ifdef NORMAL_GENERATION
	GeneralNormalMap();
#endif
}

void GeneralTab::GeneralOptions()
{
	auto vBoxGeneralOptions = new QGroupBox( tr( "General Options" ), this );
	auto vBLayout = new QGridLayout( vBoxGeneralOptions );
	auto label1 = new QLabel();
	label1->setText( tr( "Texture Format:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pFormatCombo = new QComboBox( this );
	for ( auto &fmt : IMAGE_FORMATS )
	{
		if ( VTFLib::CVTFFile::GetImageFormatInfo( fmt.format ).bIsSupported )
			pFormatCombo->addItem( tr( fmt.name ), (int)fmt.format );
	}
	vBLayout->addWidget( pFormatCombo, 0, 1, Qt::AlignRight );

	auto alphaDetectedLabel = new QLabel();
	alphaDetectedLabel->setText( tr( "Alpha Texture Format:" ) );
	vBLayout->addWidget( alphaDetectedLabel, 1, 0, Qt::AlignLeft );
	pAlphaDetectedFormatCombo = new QComboBox( this );
	for ( auto &fmt : IMAGE_FORMATS )
	{
		if ( VTFLib::CVTFFile::GetImageFormatInfo( fmt.format ).bIsSupported )
			pAlphaDetectedFormatCombo->addItem( tr( fmt.name ), (int)fmt.format );
	}
	vBLayout->addWidget( pAlphaDetectedFormatCombo, 1, 1, Qt::AlignRight );

	auto label2 = new QLabel();
	label2->setText( tr( "Texture Type:" ) );
	pTypeCombo = new QComboBox( this );
	pTypeCombo->addItem( tr( "Animated Texture" ) );
	pTypeCombo->addItem( tr( "Environment Map" ) );
	pTypeCombo->addItem( tr( "Volume Texture" ) );

	auto vParent = static_cast<VTFEImport *>( this->parent() );
	connect(
		pTypeCombo, &QComboBox::currentTextChanged, this->parent(),
		[this, vParent]()
		{
			bool shouldCheck = pTypeCombo->currentIndex() == 0;
#ifdef NORMAL_GENERATION
			generateNormalMapCheckbox_->setDisabled( !shouldCheck );
			emit generateNormalMapCheckbox_->clicked( shouldCheck && generateNormalMapCheckbox_->isChecked() );
#endif
			if ( vParent )
			{
				vParent->pAdvancedTab->pGenerateSphereMapCheckBox->setDisabled( shouldCheck );
			}
		} );

	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	vBLayout->addWidget( pTypeCombo, 2, 1, Qt::AlignRight );
	pSRGBCheckbox = new QCheckBox( "sRGB Color Space", this );
	vBLayout->addWidget( pSRGBCheckbox, 3, 0, 1, 2, Qt::AlignLeft );
	pMainLayout->addWidget( vBoxGeneralOptions, 0, 0 );
}

void GeneralTab::GeneralResize()
{
	auto vBoxResize = new QGroupBox( tr( "Resize" ), this );
	auto vBLayout = new QGridLayout( vBoxResize );
	pResizeCheckbox = new QCheckBox( this );
	pResizeCheckbox->setText( tr( "Resize" ) );
	auto parent = static_cast<VTFEImport *>( this->parent() );
	vlBool b1 = parent->IsPowerOfTwo( parent->imageList[0]->getWidth() );
	vlBool b2 = parent->IsPowerOfTwo( parent->imageList[0]->getHeight() );
	pResizeCheckbox->setChecked( !( b1 && b2 ) );
	pResizeCheckbox->setDisabled( !( b1 && b2 ) );
	if ( !( b1 && b2 ) )
		pResizeCheckbox->setToolTip( tr( "Image is not in power of 2 and therefore needs resizing." ) );
	vBLayout->addWidget( pResizeCheckbox, 0, 0, Qt::AlignLeft );
	auto label1 = new QLabel();
	label1->setText( tr( "Resize Method:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	pResizeMethodCombo = new QComboBox( this );
	pResizeMethodCombo->addItem( tr( "Nearest Power Of 2" ), (int)RESIZE_NEAREST_POWER2 );
	pResizeMethodCombo->addItem( tr( "Biggest Power Of 2" ), (int)RESIZE_BIGGEST_POWER2 );
	pResizeMethodCombo->addItem( tr( "Smallest Power Of 2" ), (int)RESIZE_SMALLEST_POWER2 );
	pResizeMethodCombo->setCurrentIndex( 1 );
	vBLayout->addWidget( pResizeMethodCombo, 1, 1, Qt::AlignRight );
	auto label2 = new QLabel();
	label2->setText( tr( "Resize Filter:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	pResizeFilterCombo = new QComboBox( this );
	pResizeFilterCombo->addItem( tr( "Box" ), (int)MIPMAP_FILTER_BOX );
	pResizeFilterCombo->addItem( tr( "Triangle" ), (int)MIPMAP_FILTER_TRIANGLE );
	pResizeFilterCombo->addItem( tr( "Quadratic" ), (int)MIPMAP_FILTER_QUADRATIC );
	pResizeFilterCombo->addItem( tr( "Cubic" ), (int)MIPMAP_FILTER_CUBIC );
	pResizeFilterCombo->addItem( tr( "Catrom" ), (int)MIPMAP_FILTER_CATROM );
	pResizeFilterCombo->addItem( tr( "Mitchell" ), (int)MIPMAP_FILTER_MITCHELL );
	pResizeFilterCombo->addItem( tr( "Gaussian" ), (int)MIPMAP_FILTER_GAUSSIAN );
	pResizeFilterCombo->addItem( tr( "Sine Cardinal" ), (int)MIPMAP_FILTER_SINC );
	pResizeFilterCombo->addItem( tr( "Bessel" ), (int)MIPMAP_FILTER_BESSEL );
	pResizeFilterCombo->addItem( tr( "Hanning" ), (int)MIPMAP_FILTER_HANNING );
	pResizeFilterCombo->addItem( tr( "Hamming" ), (int)MIPMAP_FILTER_HAMMING );
	pResizeFilterCombo->addItem( tr( "Blackman" ), (int)MIPMAP_FILTER_BLACKMAN );
	pResizeFilterCombo->addItem( tr( "Kaiser" ), (int)MIPMAP_FILTER_KAISER );
	vBLayout->addWidget( pResizeFilterCombo, 2, 1, Qt::AlignRight );
	pClampCheckbox = new QCheckBox( this );
	pClampCheckbox->setText( tr( "Clamp" ) );

	vBLayout->addWidget( pClampCheckbox, 3, 0, Qt::AlignLeft );
	auto label3 = new QLabel();
	label3->setText( tr( "Maximum Width:" ) );
	pClampWidthCombo = new QComboBox( this );
	vBLayout->addWidget( label3, 4, 0, Qt::AlignLeft );
	vBLayout->addWidget( pClampWidthCombo, 4, 1, Qt::AlignRight );

	auto label4 = new QLabel();
	label4->setText( tr( "Maximum Height:" ) );
	pClampHeightCombo = new QComboBox( this );
	for ( int i = 1; i <= 4096; i *= 2 )
	{
		pClampHeightCombo->addItem( QString::number( i ), i );
		pClampWidthCombo->addItem( QString::number( i ), i );
	}

	pClampHeightCombo->setCurrentIndex( pClampHeightCombo->count() - 2 );
	pClampWidthCombo->setCurrentIndex( pClampWidthCombo->count() - 2 );

	label3->setDisabled( true );
	label4->setDisabled( true );
	pClampHeightCombo->setDisabled( true );
	pClampWidthCombo->setDisabled( true );

	connect(
		pClampCheckbox, &QCheckBox::clicked, this->parent(),
		[label3, label4, this]( bool checked )
		{
			label3->setDisabled( !checked );
			label4->setDisabled( !checked );
			pClampHeightCombo->setDisabled( !checked );
			pClampWidthCombo->setDisabled( !checked );
		} );

	if ( !pResizeCheckbox->isChecked() )
	{
		label1->setDisabled( true );
		label2->setDisabled( true );
		pResizeMethodCombo->setDisabled( true );
		pResizeFilterCombo->setDisabled( true );
		pClampCheckbox->setDisabled( true );
	}
	connect(
		pResizeCheckbox, &QCheckBox::clicked, this->parent(),
		[label1, label2, label3, label4, this]( bool checked )
		{
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			pResizeMethodCombo->setDisabled( !checked );
			pResizeFilterCombo->setDisabled( !checked );
			pClampCheckbox->setDisabled( !checked );
			bool secondChecked = pClampCheckbox->isChecked();
			label3->setDisabled( !checked || !secondChecked );
			label4->setDisabled( !checked || !secondChecked );
			pClampHeightCombo->setDisabled( !checked || !secondChecked );
			pClampWidthCombo->setDisabled( !checked || !secondChecked );
		} );

	vBLayout->addWidget( label4, 5, 0, Qt::AlignLeft );
	vBLayout->addWidget( pClampHeightCombo, 5, 1, Qt::AlignRight );
	pMainLayout->addWidget( vBoxResize, 1, 0 );
}

void GeneralTab::GeneralMipMaps()
{
	auto vBoxMipMaps = new QGroupBox( tr( "Mipmaps" ), this );
	auto vBLayout = new QGridLayout( vBoxMipMaps );
	pGenerateMipmapsCheckbox = new QCheckBox( this );
	pGenerateMipmapsCheckbox->setText( tr( "Generate Mipmaps" ) );
	vBLayout->addWidget( pGenerateMipmapsCheckbox, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel();
	label1->setText( tr( "Mipmap Filter:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );

	pMipmapFilterCombo = new QComboBox( this );
	pMipmapFilterCombo->addItem( tr( "Box" ), (int)MIPMAP_FILTER_BOX );
	pMipmapFilterCombo->addItem( tr( "Triangle" ), (int)MIPMAP_FILTER_TRIANGLE );
	pMipmapFilterCombo->addItem( tr( "Quadratic" ), (int)MIPMAP_FILTER_QUADRATIC );
	pMipmapFilterCombo->addItem( tr( "Cubic" ), (int)MIPMAP_FILTER_CUBIC );
	pMipmapFilterCombo->addItem( tr( "Catrom" ), (int)MIPMAP_FILTER_CATROM );
	pMipmapFilterCombo->addItem( tr( "Mitchell" ), (int)MIPMAP_FILTER_MITCHELL );
	pMipmapFilterCombo->addItem( tr( "Gaussian" ), (int)MIPMAP_FILTER_GAUSSIAN );
	pMipmapFilterCombo->addItem( tr( "Sine Cardinal" ), (int)MIPMAP_FILTER_SINC );
	pMipmapFilterCombo->addItem( tr( "Bessel" ), (int)MIPMAP_FILTER_BESSEL );
	pMipmapFilterCombo->addItem( tr( "Hanning" ), (int)MIPMAP_FILTER_HANNING );
	pMipmapFilterCombo->addItem( tr( "Hamming" ), (int)MIPMAP_FILTER_HAMMING );
	pMipmapFilterCombo->addItem( tr( "Blackman" ), (int)MIPMAP_FILTER_BLACKMAN );
	pMipmapFilterCombo->addItem( tr( "Kaiser" ), (int)MIPMAP_FILTER_KAISER );
	vBLayout->addWidget( pMipmapFilterCombo, 1, 1, Qt::AlignRight );

	label1->setDisabled( true );
	pMipmapFilterCombo->setDisabled( true );

	connect(
		pGenerateMipmapsCheckbox, &QCheckBox::clicked, this->parent(),
		[this, label1]( bool checked )
		{
			label1->setDisabled( !checked );
			pMipmapFilterCombo->setDisabled( !checked );
			vBoxCustomMipMaps->setDisabled( checked );
		} );

	pMainLayout->addWidget( vBoxMipMaps, 0, 1 );
}

void GeneralTab::GeneralCustomMipmaps()
{
	vBoxCustomMipMaps = new QGroupBox( tr( "Custom mipmaps" ), this );
	auto vBLayout = new QGridLayout( vBoxCustomMipMaps );

	auto pScrollArea = new QScrollArea( vBoxCustomMipMaps );
	auto pMipMapDialogLayout = new QVBoxLayout();

	auto parent = dynamic_cast<VTFEImport *>( this->parent() );

	vlUInt maxCubemaps = VTFLib::CVTFFile::ComputeMipmapCount( parent->imageList[0]->getWidth(), parent->imageList[0]->getHeight(), 1 );

	auto pFrameBox = new QSpinBox( pScrollArea );
	pFrameBox->setPrefix( "Frame: " );
	vBLayout->addWidget( pFrameBox, 0, 0 );
	auto pFaceBox = new QSpinBox( pScrollArea );
	pFaceBox->setPrefix( "Face: " );
	vBLayout->addWidget( pFaceBox, 0, 1 );
	auto pSliceBox = new QSpinBox( pScrollArea );
	pSliceBox->setPrefix( "Slice: " );
	vBLayout->addWidget( pSliceBox, 0, 2 );

	for ( int i = 1; i < maxCubemaps; i++ )
	{
		vlUInt uiMipWidth, uiMipHeight, uiMipDepth;
		VTFLib::CVTFFile::ComputeMipmapDimensions( parent->imageList[0]->getWidth(), parent->imageList[0]->getHeight(), 1, i, uiMipWidth, uiMipHeight, uiMipDepth );
		auto mipMapButton = new QPushButton( QApplication::style()->standardIcon( QStyle::SP_FileIcon ), QString::number( uiMipWidth ) + " X " + QString::number( uiMipHeight ) );

		connect( mipMapButton, &QPushButton::clicked, this, []() {

		} );

		pMipMapDialogLayout->addWidget( mipMapButton, Qt::AlignRight );
	}

	pScrollArea->setLayout( pMipMapDialogLayout );
	vBLayout->addWidget( pScrollArea, 1, 0, 1, 3 );

	pMainLayout->addWidget( vBoxCustomMipMaps, 1, 1 );
}

#ifdef NORMAL_GENERATION
void GeneralTab::GeneralNormalMap()
{
	auto vBoxCustomMipMaps = new QGroupBox( tr( "Normal Map" ), this );
	auto vBLayout = new QGridLayout( vBoxCustomMipMaps );
	generateNormalMapCheckbox_ = new QCheckBox( this );
	generateNormalMapCheckbox_->setText( tr( "Generate Normal Map" ) );
	vBLayout->addWidget( generateNormalMapCheckbox_, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Kernel Filter:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	kernelFilterCombo_ = new QComboBox( this );
	kernelFilterCombo_->addItem( tr( "4x" ), KERNEL_FILTER_4X );
	kernelFilterCombo_->addItem( tr( "3x3" ), KERNEL_FILTER_3X3 );
	kernelFilterCombo_->addItem( tr( "5x5" ), KERNEL_FILTER_5X5 );
	kernelFilterCombo_->addItem( tr( "7x7" ), KERNEL_FILTER_7X7 );
	kernelFilterCombo_->addItem( tr( "9x9" ), KERNEL_FILTER_9X9 );
	kernelFilterCombo_->addItem( tr( "DUDV" ), KERNEL_FILTER_DUDV );
	vBLayout->addWidget( kernelFilterCombo_, 1, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Height Source:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	heightConversionCombo_ = new QComboBox( this );
	heightConversionCombo_->addItem( tr( "Alpha Channel" ), (int)HEIGHT_CONVERSION_METHOD_ALPHA );
	heightConversionCombo_->addItem( tr( "Average RGB" ), (int)HEIGHT_CONVERSION_METHOD_AVERAGE_RGB );
	heightConversionCombo_->addItem( tr( "Biased RGB" ), (int)HEIGHT_CONVERSION_METHOD_BIASED_RGB );
	heightConversionCombo_->addItem( tr( "Red Channel" ), (int)HEIGHT_CONVERSION_METHOD_RED );
	heightConversionCombo_->addItem( tr( "Green Channel" ), (int)HEIGHT_CONVERSION_METHOD_GREEN );
	heightConversionCombo_->addItem( tr( "Blue Channel" ), (int)HEIGHT_CONVERSION_METHOD_BLUE );
	heightConversionCombo_->addItem( tr( "Max RGB" ), (int)HEIGHT_CONVERSION_METHOD_MAX_RGB );
	heightConversionCombo_->addItem( tr( "Colorspace" ), (int)HEIGHT_CONVERSION_METHOD_COLORSPACE );
	vBLayout->addWidget( heightConversionCombo_, 2, 1, Qt::AlignRight );
	auto label3 = new QLabel( this );
	label3->setText( tr( "Kernel Filter:" ) );
	vBLayout->addWidget( label3, 3, 0, Qt::AlignLeft );
	normalAlphaResultCombo_ = new QComboBox( this );
	normalAlphaResultCombo_->addItem( tr( "No Change" ), (int)NORMAL_ALPHA_RESULT_NOCHANGE );
	normalAlphaResultCombo_->addItem( tr( "Set To Height" ), (int)NORMAL_ALPHA_RESULT_HEIGHT );
	normalAlphaResultCombo_->addItem( tr( "Set To Black" ), (int)NORMAL_ALPHA_RESULT_BLACK );
	normalAlphaResultCombo_->addItem( tr( "Set To White" ), (int)NORMAL_ALPHA_RESULT_WHITE );
	vBLayout->addWidget( normalAlphaResultCombo_, 3, 1, Qt::AlignRight );

	auto label4 = new QLabel( this );
	label4->setText( tr( "Scale:" ) );
	vBLayout->addWidget( label4, 4, 0, Qt::AlignLeft );
	scaleSpinBox_ = new QDoubleSpinBox( this );
	scaleSpinBox_->setValue( 2.00 );
	scaleSpinBox_->setSingleStep( 0.05 );
	vBLayout->addWidget( scaleSpinBox_, 4, 1, Qt::AlignRight );

	checkbox5_ = new QCheckBox( this );
	checkbox5_->setText( tr( "Wrap Normal Map" ) );
	vBLayout->addWidget( checkbox5_, 5, 0, Qt::AlignLeft );

	kernelFilterCombo_->setDisabled( true );
	heightConversionCombo_->setDisabled( true );
	normalAlphaResultCombo_->setDisabled( true );
	scaleSpinBox_->setDisabled( true );
	checkbox5_->setDisabled( true );
	label1->setDisabled( true );
	label2->setDisabled( true );
	label3->setDisabled( true );
	label4->setDisabled( true );

	connect(
		generateNormalMapCheckbox_, &QCheckBox::clicked, this->parent(),
		[this, label1, label2, label3, label4]( bool checked )
		{
			kernelFilterCombo_->setDisabled( !checked );
			heightConversionCombo_->setDisabled( !checked );
			normalAlphaResultCombo_->setDisabled( !checked );
			scaleSpinBox_->setDisabled( !checked );
			checkbox5_->setDisabled( !checked );
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			label3->setDisabled( !checked );
			label4->setDisabled( !checked );
		} );

	vMainLayout->addWidget( vBoxCustomMipMaps, 1, 1 );
}
#endif
AdvancedTab::AdvancedTab( VTFEImport *parent ) :
	QDialog( parent )
{
	pMainLayout = new QGridLayout( this );
	pMainLayout->setAlignment( Qt::AlignTop );
	VersionMenu();
	GammaCorrectionMenu();
	Miscellaneous();
	// DTXCompression(); //doesn't seem to be used in modern VTFEdit or VTFLib.
	LuminanceWeights();
#ifdef COLOR_CORRECTION
	ColorCorrectionMenu();
#endif
	//	UnsharpenMaskOptions();
	//	XSharpenOptions(); //According to ~smead on discord, these are used by the old DTX library to apply filters to a
	// filter... new DTX library doesn't have this
}

void AdvancedTab::VersionMenu()
{
	auto vBoxVersion = new QGroupBox( tr( "Version" ), this );
	auto vBLayout = new QGridLayout( vBoxVersion );
	auto label1 = new QLabel( this );
	label1->setText( tr( "VTF Version:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pVtfVersionBox = new QComboBox( this );
#ifdef CHAOS_INITIATIVE
	for ( int i = 0; i <= VTF_MINOR_VERSION; i++ )
#else
	for ( int i = 0; i <= 5; i++ )
#endif
	{
		pVtfVersionBox->addItem( QString::number( VTF_MAJOR_VERSION ) + "." + QString::number( i ), i );
	}
	pVtfVersionBox->setCurrentIndex( pVtfVersionBox->count() - 2 );
	vBLayout->addWidget( pVtfVersionBox, 0, 1, Qt::AlignRight );
#ifdef CHAOS_INITIATIVE
	pAuxCompressionBox = new QCheckBox( this );
	pAuxCompressionBox->setText( tr( "AUX Compression" ) );
	vBLayout->addWidget( pAuxCompressionBox, 1, 0, Qt::AlignLeft );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Aux Compression Level:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	pAuxCompressionLevelBox = new QComboBox( this );
	for ( int i = 0; i <= 9; i++ )
	{
		pAuxCompressionLevelBox->addItem( QString::number( i ), i );
	}
	pAuxCompressionLevelBox->setCurrentIndex( pAuxCompressionLevelBox->count() - 1 );
	vBLayout->addWidget( pAuxCompressionLevelBox, 2, 1, Qt::AlignRight );

	pAuxCompressionBox->setDisabled( true );
	pAuxCompressionLevelBox->setDisabled( true );
	label2->setDisabled( true );

	connect(
		pVtfVersionBox, &QComboBox::currentTextChanged, this->parent(),
		[this, label2]( QString text )
		{
			bool iscompressCompatible = ( QString( text.at( 2 ).toLatin1() ).toInt() >= 6 );
			pAuxCompressionBox->setDisabled( !iscompressCompatible );
			bool isChecked = pAuxCompressionBox->isChecked();
			pAuxCompressionLevelBox->setDisabled( !iscompressCompatible || !isChecked );
			label2->setDisabled( !iscompressCompatible || !isChecked );
		} );
	connect(
		pAuxCompressionBox, &QCheckBox::clicked, this->parent(),
		[this, label2]( bool checked )
		{
			pAuxCompressionLevelBox->setDisabled( !checked );
			label2->setDisabled( !checked );
		} );
#endif
	pMainLayout->addWidget( vBoxVersion, 0, 0 );
}

void AdvancedTab::GammaCorrectionMenu()
{
	auto vBoxGammaCorrection = new QGroupBox( tr( "Gamma Correction" ), this );
	auto vBLayout = new QGridLayout( vBoxGammaCorrection );

	pGammaCorrectionCheckBox = new QCheckBox( this );
	pGammaCorrectionCheckBox->setText( tr( "Gamma Correction" ) );
	vBLayout->addWidget( pGammaCorrectionCheckBox, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Correction:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	pGammaCorrectionBox = new QDoubleSpinBox( this );

	pGammaCorrectionBox->setValue( 2.30 );
	vBLayout->addWidget( pGammaCorrectionBox, 1, 1, Qt::AlignRight );

	pGammaCorrectionBox->setDisabled( true );
	label1->setDisabled( true );

	connect(
		pGammaCorrectionCheckBox, &QCheckBox::clicked, this->parent(),
		[label1, this]( bool clicked )
		{
			pGammaCorrectionBox->setDisabled( !clicked );
			label1->setDisabled( !clicked );
		} );

	pMainLayout->addWidget( vBoxGammaCorrection, 1, 0 );
}

void AdvancedTab::Miscellaneous()
{
	auto vBoxMiscellaneous = new QGroupBox( tr( "Miscellaneous" ), this );
	auto vBLayout = new QGridLayout( vBoxMiscellaneous );
	pComputeReflectivityCheckBox = new QCheckBox( this );
	pComputeReflectivityCheckBox->setText( tr( "Compute Reflectivity" ) );
	vBLayout->addWidget( pComputeReflectivityCheckBox, 0, 0, Qt::AlignLeft );
	pGenerateThumbnailCheckBox = new QCheckBox( this );
	pGenerateThumbnailCheckBox->setText( tr( "Generate Thumbnail" ) );
	vBLayout->addWidget( pGenerateThumbnailCheckBox, 1, 0, Qt::AlignLeft );
	pGenerateSphereMapCheckBox = new QCheckBox( this );
	pGenerateSphereMapCheckBox->setText( tr( "Generate Sphere Map" ) );
	pGenerateSphereMapCheckBox->setDisabled( true );
	vBLayout->addWidget( pGenerateSphereMapCheckBox, 2, 0, Qt::AlignLeft );
	pMainLayout->addWidget( vBoxMiscellaneous, 2, 0 );
}

void AdvancedTab::DTXCompression()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "DTX Compression" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Quality:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pDtxCompressionQuality = new QComboBox( this );
	pDtxCompressionQuality->addItem( tr( "low" ) );
	pDtxCompressionQuality->addItem( tr( "medium" ) );
	pDtxCompressionQuality->addItem( tr( "high" ) );
	pDtxCompressionQuality->setCurrentIndex( pDtxCompressionQuality->count() - 1 );
	vBLayout->addWidget( pDtxCompressionQuality, 0, 1, Qt::AlignRight );

	pMainLayout->addWidget( vBoxDTXCompression, 3, 0 );
}

void AdvancedTab::LuminanceWeights()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "Luminance Weights" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Red:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pLuminanceWeightRedBox = new QDoubleSpinBox( this );
	pLuminanceWeightRedBox->setDecimals( 3 );
	pLuminanceWeightRedBox->setValue( 0.299 );
	vBLayout->addWidget( pLuminanceWeightRedBox, 0, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Green:" ) );
	vBLayout->addWidget( label2, 1, 0, Qt::AlignLeft );
	pLuminanceWeightGreenBox = new QDoubleSpinBox( this );
	pLuminanceWeightGreenBox->setDecimals( 3 );
	pLuminanceWeightGreenBox->setValue( 0.587 );
	vBLayout->addWidget( pLuminanceWeightGreenBox, 1, 1, Qt::AlignRight );

	auto label3 = new QLabel( this );
	label3->setText( tr( "Blue:" ) );
	vBLayout->addWidget( label3, 2, 0, Qt::AlignLeft );
	pLuminanceWeightBlueBox = new QDoubleSpinBox( this );
	pLuminanceWeightBlueBox->setDecimals( 3 );
	pLuminanceWeightBlueBox->setValue( 0.114 );
	vBLayout->addWidget( pLuminanceWeightBlueBox, 2, 1, Qt::AlignRight );

	pMainLayout->addWidget( vBoxDTXCompression, 0, 1 );
}
#ifdef COLOR_CORRECTION
void AdvancedTab::ColorCorrectionMenu()
{
	auto vBoxColorCorrection = new QGroupBox( tr( "Color Correction" ), this );
	auto vBLayout = new QGridLayout( vBoxColorCorrection );
	//	ColorCorrectionDialog_ = new QColorDialog(this);
	//	QColorDialog::ColorDialogOptions options;
	//	options.setFlag(QColorDialog::ShowAlphaChannel, true);
	//	options.setFlag(QColorDialog::NoButtons, true);
	//	//options.setFlag(QColorDialog::DontUseNativeDialog,true);
	//	ColorCorrectionDialog_->setOptions(options);
	//	ColorCorrectionDialog_->setFixedSize(20,20);
	colorCorrectionDialog_ = new QtColorTriangle( vBoxColorCorrection );
	QColor tmp;
	tmp.setHsv( 0, 0, 255 );
	colorCorrectionDialog_->setColor( tmp );
	colorCorrectionDialog_->setMinimumSize( 140, 140 );
	vBLayout->addWidget( colorCorrectionDialog_, 0, 0, 4, 1, Qt::AlignLeft );
	auto current = colorCorrectionDialog_->color();

	auto label1 = new QLabel( this );
	label1->setText( tr( "Hue:" ) );
	vBLayout->addWidget( label1, 0, 1, Qt::AlignLeft );
	colorCorrectionRedBox_ = new QDoubleSpinBox( this );
	colorCorrectionRedBox_->setDecimals( 3 );
	colorCorrectionRedBox_->setSingleStep( 0.01 );
	colorCorrectionRedBox_->setValue( current.redF() );
	colorCorrectionRedBox_->setRange( 0, 1 );
	vBLayout->addWidget( colorCorrectionRedBox_, 0, 2, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Saturation:" ) );
	vBLayout->addWidget( label2, 1, 1, Qt::AlignLeft );
	colorCorrectionGreenBox_ = new QDoubleSpinBox( this );
	colorCorrectionGreenBox_->setDecimals( 3 );
	colorCorrectionGreenBox_->setSingleStep( 0.01 );
	colorCorrectionGreenBox_->setValue( current.greenF() );
	colorCorrectionGreenBox_->setRange( 0, 1 );
	vBLayout->addWidget( colorCorrectionGreenBox_, 1, 2, Qt::AlignRight );

	auto label3 = new QLabel( this );
	label3->setText( tr( "Value:" ) );
	vBLayout->addWidget( label3, 2, 1, Qt::AlignLeft );
	colorCorrectionBlueBox_ = new QDoubleSpinBox( this );
	colorCorrectionBlueBox_->setDecimals( 3 );
	colorCorrectionBlueBox_->setSingleStep( 0.01 );

	colorCorrectionBlueBox_->setValue( current.blueF() );
	colorCorrectionBlueBox_->setRange( 0, 1 );
	vBLayout->addWidget( colorCorrectionBlueBox_, 2, 2, Qt::AlignRight );

	auto label4 = new QLabel( this );
	label4->setText( tr( "Alpha:" ) );
	vBLayout->addWidget( label4, 3, 1, Qt::AlignLeft );
	colorCorrectionAlphaBox_ = new QDoubleSpinBox( this );
	colorCorrectionAlphaBox_->setDecimals( 3 );
	colorCorrectionAlphaBox_->setSingleStep( 0.01 );
	colorCorrectionAlphaBox_->setValue( current.alphaF() );
	colorCorrectionAlphaBox_->setRange( 0, 1 );
	colorCorrectionAlphaBox_->setStepType( QDoubleSpinBox::AdaptiveDecimalStepType );
	vBLayout->addWidget( colorCorrectionAlphaBox_, 3, 2, Qt::AlignRight );

	AdvancedTab::connect(
		colorCorrectionRedBox_, QOverload<double>::of( &QDoubleSpinBox::valueChanged ), this,
		[this]( double value )
		{
			auto current = colorCorrectionDialog_->color().toHsv();
			current.setHsvF( value, current.saturationF(), current.valueF() );
			colorCorrectionDialog_->setColor( current );
		} );
	AdvancedTab::connect(
		colorCorrectionGreenBox_, QOverload<double>::of( &QDoubleSpinBox::valueChanged ), this,
		[this]( double value )
		{
			auto current = colorCorrectionDialog_->color().toHsv();
			current.setHsvF( current.hueF(), value, current.valueF() );
			colorCorrectionDialog_->setColor( current );
		} );
	AdvancedTab::connect(
		colorCorrectionBlueBox_, QOverload<double>::of( &QDoubleSpinBox::valueChanged ), this,
		[this]( double value )
		{
			auto current = colorCorrectionDialog_->color().toHsv();
			current.setHsvF( current.hueF(), current.saturationF(), value );
			colorCorrectionDialog_->setColor( current );
		} );

	connect(
		colorCorrectionDialog_, &QtColorTriangle::colorChanged,
		[this]( QColor c )
		{
			colorCorrectionRedBox_->setValue( c.hueF() );
			colorCorrectionGreenBox_->setValue( c.saturationF() );
			colorCorrectionBlueBox_->setValue( c.valueF() );
		} );

	vMainLayout->addWidget( vBoxColorCorrection, 1, 1 );
}
#endif
void AdvancedTab::HSVtoRGB( float H, float S, float V, int rgb[3] )
{
	if ( H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0 )
	{
		qInfo() << "The given HSV values are not in valid range" << Qt::endl;
		return;
	}
	float s = S / 100;
	float v = V / 100;
	float C = s * v;
	float X = C * ( 1 - abs( fmod( H / 60.0, 2 ) - 1 ) );
	float m = v - C;
	float r, g, b;
	if ( H >= 0 && H < 60 )
	{
		r = C, g = X, b = 0;
	}
	else if ( H >= 60 && H < 120 )
	{
		r = X, g = C, b = 0;
	}
	else if ( H >= 120 && H < 180 )
	{
		r = 0, g = C, b = X;
	}
	else if ( H >= 180 && H < 240 )
	{
		r = 0, g = X, b = C;
	}
	else if ( H >= 240 && H < 300 )
	{
		r = X, g = 0, b = C;
	}
	else
	{
		r = C, g = 0, b = X;
	}
	rgb[0] = ( r + m ) * 255;
	rgb[1] = ( g + m ) * 255;
	rgb[2] = ( b + m ) * 255;
}

void AdvancedTab::UnsharpenMaskOptions()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "Unsharpen Mask Options" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Radius:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pUnsharpenMaskRadiusBox = new QDoubleSpinBox( this );
	pUnsharpenMaskRadiusBox->setValue( 2 );
	vBLayout->addWidget( pUnsharpenMaskRadiusBox, 0, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Amount:" ) );
	vBLayout->addWidget( label2, 1, 0, Qt::AlignLeft );
	pUnsharpenMaskAmountBox = new QDoubleSpinBox( this );
	pUnsharpenMaskAmountBox->setValue( 0.5 );
	vBLayout->addWidget( pUnsharpenMaskAmountBox, 1, 1, Qt::AlignRight );

	auto label3 = new QLabel( this );
	label3->setText( tr( "Threshold:" ) );
	vBLayout->addWidget( label3, 2, 0, Qt::AlignLeft );
	pUnsharpenMaskThresholdBox = new QDoubleSpinBox( this );
	pUnsharpenMaskThresholdBox->setValue( 0 );
	vBLayout->addWidget( pUnsharpenMaskThresholdBox, 2, 1, Qt::AlignRight );

	pMainLayout->addWidget( vBoxDTXCompression, 1, 1 );
}

void AdvancedTab::XSharpenOptions()
{
	auto vBoxDTXCompression = new QGroupBox( tr( "X Sharpen Options" ), this );
	auto vBLayout = new QGridLayout( vBoxDTXCompression );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Strength:" ) );
	vBLayout->addWidget( label1, 0, 0, Qt::AlignLeft );
	pXSharpenOptionsStrengthBox = new QDoubleSpinBox( this );
	pXSharpenOptionsStrengthBox->setValue( 2 );
	vBLayout->addWidget( pXSharpenOptionsStrengthBox, 0, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Threshold:" ) );
	vBLayout->addWidget( label2, 1, 0, Qt::AlignLeft );
	pXSharpenOptionsThresholdBox = new QDoubleSpinBox( this );
	pXSharpenOptionsThresholdBox->setValue( 0.5 );
	vBLayout->addWidget( pXSharpenOptionsThresholdBox, 1, 1, Qt::AlignRight );

	pMainLayout->addWidget( vBoxDTXCompression, 2, 1 );
}

ResourceTab::ResourceTab( VTFEImport *parent ) :
	QDialog( parent )
{
	pMainLayout = new QGridLayout( this );
	pMainLayout->setAlignment( Qt::AlignTop );
	LODControlResource();
	InformationResource();
}

void ResourceTab::LODControlResource()
{
	auto vBoxLODControlResource = new QGroupBox( tr( "LOD Control Resource" ), this );
	auto vBLayout = new QGridLayout( vBoxLODControlResource );

	pLodControlResourceCheckBox = new QCheckBox( this );
	pLodControlResourceCheckBox->setText( tr( "Create LOD Control Resource" ) );
	vBLayout->addWidget( pLodControlResourceCheckBox, 0, 0, Qt::AlignLeft );

	auto label1 = new QLabel( this );
	label1->setText( tr( "Strength:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	pControlResourceCrampUBox = new QDoubleSpinBox( this );
	pControlResourceCrampUBox->setValue( 2 );
	vBLayout->addWidget( pControlResourceCrampUBox, 1, 1, Qt::AlignRight );

	auto label2 = new QLabel( this );
	label2->setText( tr( "Threshold:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	pControlResourceCrampVBox = new QDoubleSpinBox( this );
	pControlResourceCrampVBox->setValue( 0.5 );
	vBLayout->addWidget( pControlResourceCrampVBox, 2, 1, Qt::AlignRight );

	label1->setDisabled( true );
	label2->setDisabled( true );
	pControlResourceCrampUBox->setDisabled( true );
	pControlResourceCrampVBox->setDisabled( true );

	connect(
		pLodControlResourceCheckBox, &QCheckBox::clicked,
		[label1, label2, this]( bool checked )
		{
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			pControlResourceCrampUBox->setDisabled( !checked );
			pControlResourceCrampVBox->setDisabled( !checked );
		} );

	pMainLayout->addWidget( vBoxLODControlResource, 0, 0, Qt::AlignLeft );
}

void ResourceTab::InformationResource()
{
	auto vBoxInformationResource = new QGroupBox( tr( "Information Resource" ), this );
	auto vBLayout = new QGridLayout( vBoxInformationResource );
	pCreateInformationResourceCheckBox = new QCheckBox( this );
	pCreateInformationResourceCheckBox->setText( tr( "Create LOD Control Resource" ) );
	vBLayout->addWidget( pCreateInformationResourceCheckBox, 0, 0, Qt::AlignLeft );

	QLabel *label1 = new QLabel( this );
	label1->setText( tr( "Author:" ) );
	vBLayout->addWidget( label1, 1, 0, Qt::AlignLeft );
	pInformationResourceAuthor = new QLineEdit( this );
	vBLayout->addWidget( pInformationResourceAuthor, 1, 1, Qt::AlignLeft );

	QLabel *label2 = new QLabel( this );
	label2->setText( tr( "Contact:" ) );
	vBLayout->addWidget( label2, 2, 0, Qt::AlignLeft );
	pInformationResouceContact = new QLineEdit( this );
	vBLayout->addWidget( pInformationResouceContact, 2, 1, Qt::AlignLeft );

	QLabel *label3 = new QLabel( this );
	label3->setText( tr( "Version:" ) );
	vBLayout->addWidget( label3, 3, 0, Qt::AlignLeft );
	pInformationResouceVersion = new QLineEdit( this );
	vBLayout->addWidget( pInformationResouceVersion, 3, 1, Qt::AlignLeft );

	QLabel *label4 = new QLabel( this );
	label4->setText( tr( "Modification:" ) );
	vBLayout->addWidget( label4, 4, 0, Qt::AlignLeft );
	pInformationResouceModification = new QLineEdit( this );
	vBLayout->addWidget( pInformationResouceModification, 4, 1, Qt::AlignLeft );

	QLabel *label5 = new QLabel( this );
	label5->setText( tr( "Description:" ) );
	vBLayout->addWidget( label5, 5, 0, Qt::AlignLeft );
	pInformationResouceDescription = new QLineEdit( this );
	vBLayout->addWidget( pInformationResouceDescription, 5, 1, Qt::AlignLeft );

	QLabel *label6 = new QLabel( this );
	label6->setText( tr( "Comments:" ) );
	vBLayout->addWidget( label6, 6, 0, Qt::AlignLeft );
	pInformationResouceComments = new QLineEdit( this );
	vBLayout->addWidget( pInformationResouceComments, 6, 1, Qt::AlignLeft );

	label1->setDisabled( true );
	label2->setDisabled( true );
	label3->setDisabled( true );
	label4->setDisabled( true );
	label5->setDisabled( true );
	label6->setDisabled( true );
	pInformationResourceAuthor->setDisabled( true );
	pInformationResouceVersion->setDisabled( true );
	pInformationResouceContact->setDisabled( true );
	pInformationResouceModification->setDisabled( true );
	pInformationResouceDescription->setDisabled( true );
	pInformationResouceComments->setDisabled( true );

	connect(
		pCreateInformationResourceCheckBox, &QCheckBox::clicked,
		[label1, label2, label3, label4, label5, label6, this]( bool checked )
		{
			label1->setDisabled( !checked );
			label2->setDisabled( !checked );
			label3->setDisabled( !checked );
			label4->setDisabled( !checked );
			label5->setDisabled( !checked );
			label6->setDisabled( !checked );
			pInformationResourceAuthor->setDisabled( !checked );
			pInformationResouceVersion->setDisabled( !checked );
			pInformationResouceContact->setDisabled( !checked );
			pInformationResouceModification->setDisabled( !checked );
			pInformationResouceDescription->setDisabled( !checked );
			pInformationResouceComments->setDisabled( !checked );
		} );

	pMainLayout->addWidget( vBoxInformationResource, 1, 0, Qt::AlignLeft );
}