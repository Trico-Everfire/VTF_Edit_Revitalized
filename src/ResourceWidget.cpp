#include "ResourceWidget.h"

#include "enums.hpp"
#include "fmt/format.h"

#include <QVBoxLayout>

using namespace VTFLib;

ResourceWidget::ResourceWidget( QWidget *parent ) :
	QWidget( parent )
{
	setup_ui();
}

void ResourceWidget::set_vtf( VTFLib::CVTFFile *file )
{
	table_->clear();
	table_->setRowCount( 0 );
	if ( !file )
	{
		return;
	}

	auto resources = file->GetResourceCount();

	int totalCount = resources;
	for ( vlUInt i = 0; i < resources; ++i )
	{
		auto type = file->GetResourceType( i );
		vlUInt size;

		auto data = file->GetResourceData( type, size );

		if ( type == VTF_RSRC_KEY_VALUE_DATA )
		{
			totalCount--;
			auto pVMTFile = new VTFLib::CVMTFile();

			if ( !pVMTFile->Load( data, size ) )
			{
				delete pVMTFile;
				continue;
			}

			totalCount += pVMTFile->GetRoot()->GetNodeCount();

			delete pVMTFile;
		}
	}

	table_->setRowCount( totalCount );
	resources = file->GetResourceCount();

	for ( vlUInt i = 0, count = 0; i < resources; ++i )
	{
		auto type = file->GetResourceType( i );
		vlUInt size;

		auto data = file->GetResourceData( type, size );

		if ( type != VTF_RSRC_KEY_VALUE_DATA )
		{
			table_->setItem( count, 0, new QTableWidgetItem( GetResourceName( type ) ) );

			auto typeItem = new QTableWidgetItem( fmt::format( FMT_STRING( "0x{:X}" ), type ).c_str() );
			table_->setItem( count, 1, typeItem );

			auto sizeItem =
				new QTableWidgetItem( fmt::format( FMT_STRING( "{:d} bytes ({:.2f} KiB)" ), size, size / 1024.f ).c_str() );
			table_->setItem( count, 2, sizeItem );
			count++;
		}
		else
		{
			auto pVMTFile = new VTFLib::CVMTFile();

			if ( !pVMTFile->Load( data, size ) )
			{
				delete pVMTFile;
				continue;
			}

			for ( int j = 0; j < pVMTFile->GetRoot()->GetNodeCount(); j++ )
			{
				auto String = static_cast<VTFLib::Nodes::CVMTStringNode *>( pVMTFile->GetRoot()->GetNode( j ) );

				auto itemName = new QTableWidgetItem( String->GetName() );
				table_->setItem( count, 0, itemName );

				auto typeItem = new QTableWidgetItem( String->GetValue() );
				table_->setItem( count, 1, typeItem );

				count++;
			}

			delete pVMTFile;
		}
	}
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