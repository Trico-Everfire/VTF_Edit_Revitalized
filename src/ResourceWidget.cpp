#include "ResourceWidget.h"

#include "enums.hpp"
#include "fmt/format.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <kvpp/kvpp.h>

ResourceWidget::ResourceWidget( QWidget *parent ) :
	QWidget( parent )
{
	setup_ui();
}

void ResourceWidget::set_vtf( vtfpp::VTF *file )
{
	table_->clear();
	table_->setRowCount( 0 );
	if ( !file )
	{
		return;
	}

	auto resources = file->getResources();

	int totalCount = resources.size();
	for ( auto resource : resources )
	{
		//		auto type = resource;
		//		uint32_t size;

		//		auto data = file->GetResourceData( type, size );

		if ( resource.type == vtfpp::Resource::TYPE_KEYVALUES_DATA )
		{
			totalCount--;
			auto pVMTFile = kvpp::KV1 { resource.getDataAsKeyValuesData() };

			if ( pVMTFile.isInvalid() )
			{
				continue;
			}

			totalCount += pVMTFile.getChildCount();
		}
	}

	table_->setRowCount( totalCount );
	//	resources = file->GetResourceCount();
	// TODO: properly add KV resources.

	//	for ( uint32_t i = 0, count = 0; i < resources; ++i )
	//	{
	//		auto type = file->GetResourceType( i );
	//		uint32_t size;
	//
	//		auto data = file->GetResourceData( type, size );
	//
	int i = 0;
	for ( auto resource : resources )
	{
		//		if ( resource.type != vtfpp::Resource::TYPE_KEYVALUES_DATA )
		{
			table_->setItem( i, 0, new QTableWidgetItem( GetResourceName( resource.type ) ) );

			auto typeItem = new QTableWidgetItem( fmt::format( FMT_STRING( "0x{}" ), (uint32_t)resource.type ).c_str() );
			table_->setItem( i, 1, typeItem );

			uint32_t size = resource.data.size_bytes();

			auto sizeItem =
				new QTableWidgetItem( fmt::format( FMT_STRING( "{} bytes ({} KiB)" ), size, size / 1024.f ).c_str() );
			table_->setItem( i, 2, sizeItem );
			i++;
		}
	}

	//	if ( auto kvResource = file->getResource( vtfpp::Resource::TYPE_KEYVALUES_DATA ) )
	//	{
	//		auto rawData = kvResource->getDataAsKeyValuesData();
	//		auto data = kvpp::KV1( rawData );
	//		for ( int j = 0; j < data.getChildCount(); j++, i++ )
	//		{
	//			// table_->setItem( i, 0, new QTableWidgetItem( "Key Value" ) );
	//			table_->setItem( i, 0, new QTableWidgetItem( std::string { data[j].getKey().data(), data[j].getKey().length() }.c_str() ) );
	//			table_->setItem( i, 1, new QTableWidgetItem( std::string { data[j].getValue().data(), data[j].getValue().length() }.c_str() ) );
	//		}
	//	}
	//		else
	//		{
	//			auto pVMTFile = new VTFLib::CVMTFile();
	//
	//			if ( !pVMTFile->Load( data, size ) )
	//			{
	//				delete pVMTFile;
	//				continue;
	//			}
	//
	//			for ( int j = 0; j < pVMTFile->GetRoot()->GetNodeCount(); j++ )
	//			{
	//				auto String = static_cast<VTFLib::Nodes::CVMTStringNode *>( pVMTFile->GetRoot()->GetNode( j ) );
	//
	//				auto itemName = new QTableWidgetItem( String->GetName() );
	//				table_->setItem( count, 0, itemName );
	//
	//				auto typeItem = new QTableWidgetItem( String->GetValue() );
	//				table_->setItem( count, 1, typeItem );
	//
	//				count++;
	//			}
	//
	//			delete pVMTFile;
	//		}
	//	}
}

void ResourceWidget::setup_ui()
{
	auto *layout = new QVBoxLayout( this );

	table_ = new QTableWidget( this );
	table_->setSelectionBehavior( QAbstractItemView::SelectRows );
	table_->verticalHeader()->hide();
	table_->setColumnCount( 3 );
	table_->horizontalHeader()->setStretchLastSection( true );
	table_->setHorizontalHeaderItem( 0, new QTableWidgetItem( "Resource Name" ) );
	table_->setHorizontalHeaderItem( 1, new QTableWidgetItem( "Resource Type" ) );
	table_->setHorizontalHeaderItem( 2, new QTableWidgetItem( "Data Size" ) );

	layout->addWidget( table_ );
}
AdditionalInformationWidget::AdditionalInformationWidget( QWidget *parent ) :
	QWidget( parent )
{
	informationLayout = new QGridLayout( this );
	informationLayout->setAlignment( Qt::AlignTop );
}

void AdditionalInformationWidget::set_vtf( vtfpp::VTF *file )
{
	for ( auto widget : informationWidgets )
		widget->deleteLater();
	informationWidgets.clear();

	if ( file )
	{
		auto resource = file->getResource( vtfpp::Resource::TYPE_KEYVALUES_DATA );
		if ( !resource )
			return;
		auto rawKV = resource->getDataAsKeyValuesData();
		auto preData = kvpp::KV1( rawKV );
		auto data = preData["Information"];

		for ( int j = 0; j < data.getChildCount(); j++ )
		{
			const auto &kv = data[j];
			auto key = std::string( kv.getKey() );
			key.append( ":" );
			auto value = std::string( kv.getValue() );
			auto resourceLabel = new QLabel( key.c_str(), this );
			informationWidgets.push_back( resourceLabel );
			this->informationLayout->addWidget( resourceLabel, j, 0, Qt::AlignTop );
			auto resourceTextBox = new QLineEdit( value.c_str(), this );
			resourceTextBox->setReadOnly( true );
			informationWidgets.push_back( resourceTextBox );
			this->informationLayout->addWidget( resourceTextBox, j, 1, Qt::AlignTop );
		}
	}
}
