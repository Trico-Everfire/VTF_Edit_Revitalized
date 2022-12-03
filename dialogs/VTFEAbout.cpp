//
// Created by trico on 2-8-22.
//

#include "VTFEAbout.h"

#include <QBoxLayout>
#include <QGroupBox>

VTFEAbout::VTFEAbout( QWidget *parent ) :
	QDialog( parent )
{
	this->setWindowTitle( tr( "About" ) );
	this->resize( 340, 420 );
	this->setStyleSheet( "*{background-color: #ebe6da; color:black;} QGroupBox {  border: 1px solid black;}" );
	auto vBox = new QGroupBox( this );
	auto vBLayout = new QVBoxLayout( vBox );
	vBLayout->addWidget( labelledText(
		"VTFLib Version: 2.0.0 (custom)<br>"
		"VTFLib Authors: Neil Jedrzejewski, Ryan Gregg, Joshua-Ashton, JJL77<br><br>"
		"VTFEdit Revitalized Version: 0.2.0<br>"
		"VTFEdit Revitalized Authors: Trico Everfire<br><br>"
		"QT Version: 5.15.2<br><br>"
		"Chaos Initiative Widgets:<br>"
		"ImageSettingsWidget.cpp/hpp<br>"
		"ImageViewWidget.cpp/hpp<br>"
		"ResourceWidget.cpp/hpp<br>"
		"InfoWidget.cpp/hpp<br>"
		"enums.cpp/hpp<br>"
		"utils.hpp<br>"
		"contents of: flagsandformats.hpp<br>",
		this ) );
	QPixmap pmap;
	pmap.load( "vtf_edit_revitalised2.png" );
	pmap = pmap.scaled( 50, 50 );
	QLabel *l = labelledText( "", this );
	l->setFixedSize( 50, 50 );
	l->setPixmap( pmap );
	vBLayout->addWidget( l );
	vBLayout->setAlignment( l, Qt::AlignRight );

	auto vBLayout2 = new QVBoxLayout( this );
	vBLayout2->addWidget( vBox );
}

QLabel *VTFEAbout::labelledText( const QString &text, QWidget *parent )
{
	auto *pLabel = new QLabel( parent );
	pLabel->setWordWrap( true );
	pLabel->setMinimumSize( this->width() / 2, 1 );
	pLabel->setTextFormat( Qt::TextFormat::RichText );
	pLabel->setText( text );
	return pLabel;
}