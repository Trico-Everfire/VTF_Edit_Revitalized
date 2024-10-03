#include "ResourceWidget.h"

#include "enums.hpp"
#include "fmt/format.h"

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
	//		if ( type != VTF_RSRC_KEY_VALUE_DATA )
	//		{
	//			table_->setItem( count, 0, new QTableWidgetItem( GetResourceName( type ) ) );
	//
	//			auto typeItem = new QTableWidgetItem( fmt::format( FMT_STRING( "0x{:X}" ), type ).c_str() );
	//			table_->setItem( count, 1, typeItem );
	//
	//			auto sizeItem =
	//				new QTableWidgetItem( fmt::format( FMT_STRING( "{:d} bytes ({:.2f} KiB)" ), size, size / 1024.f ).c_str() );
	//			table_->setItem( count, 2, sizeItem );
	//			count++;
	//		}
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