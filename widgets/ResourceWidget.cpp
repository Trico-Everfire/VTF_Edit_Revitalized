#include "ResourceWidget.h"

#include "../common/enums.hpp"
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
	if ( !file )
		return;

	auto resources = file->GetResourceCount();
	table_->setRowCount( resources );
	for ( vlUInt i = 0; i < resources; ++i )
	{
		auto type = file->GetResourceType( i );
		vlUInt size;
		auto data = file->GetResourceData( type, size );

		table_->setItem( i, 0, new QTableWidgetItem( GetResourceName( type ) ) );

		auto typeItem = new QTableWidgetItem( fmt::format( FMT_STRING( "0x{:X}" ), type ).c_str() );
		table_->setItem( i, 1, typeItem );

		auto sizeItem =
			new QTableWidgetItem( fmt::format( FMT_STRING( "{:d} bytes ({:.2f} KiB)" ), size, size / 1024.f ).c_str() );
		table_->setItem( i, 2, sizeItem );
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