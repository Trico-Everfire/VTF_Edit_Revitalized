#include "VTFQMenuBar.h"

#include <QDebug>

VTFQMenuBar::VTFQMenuBar( QWidget *parent ) :
	QTabWidget( parent )
{
	setTabsClosable( true );
	setMovable( true );
	connect(
		this, &QTabWidget::currentChanged,
		[this]( int index )
		{
			if ( this->currentWidget() )
			{ // currentChanged will emit if it changes to -1, which is invalid, so we check this.
				auto vVFileWidget = dynamic_cast<VFileQAction *>( this->widget( index ) );
				if ( vVFileWidget->getType() == VTFFile )
				{
					auto vWidget = ( (VTFQAction *)this->widget( index ) );
					if ( vWidget->isVTFSet() )
					{
						emit vWidget->triggeredVTF( vWidget->getVTF(), false );
					}
				}
#ifdef VMT_EDITOR
				else if ( vVFileWidget->getType() == VMTFIle )
				{
					auto vWidget = ( (VMTQAction *)this->widget( index ) );
					if ( vWidget->isVMTSet() && vWidget->getVMT()->IsLoaded() )
					{
						emit vWidget->triggeredVMT( vWidget->getVMT(), false );
					}
				}
#endif
			}
		} );

	connect(
		this, &QTabWidget::tabCloseRequested,
		[this]( int index )
		{
			if ( this->count() == 1 && ( dynamic_cast<VFileQAction *>( this->currentWidget() ) )->getType() == VTFFile )
			{
				emit( (VTFQAction *)this->currentWidget() )->setFinalsDefault();
			}
			delete this->widget( index );
		} );
}

VTFQAction::VTFQAction( QWidget *parent ) :
	QWidget( parent ), VFileQAction()
{
}

void VTFQAction::setVTF( VTFLib::CVTFFile *pVTF )
{
	isReady = true;
	pVTFFile = pVTF;
}

VTFLib::CVTFFile *VTFQAction::getVTF()
{
	return pVTFFile;
}
bool VTFQAction::isVTFSet()
{
	return isReady;
}
#ifdef VMT_EDITOR
VMTQAction::VMTQAction( QWidget *parent ) :
	QWidget( parent ), VFileQAction()
{
}

void VMTQAction::setVMT( VTFLib::CVMTFile *pVMT )
{
	isReady = true;
	pVMTile = pVMT;
}

VTFLib::CVMTFile *VMTQAction::getVMT()
{
	return pVMTile;
}
bool VMTQAction::isVMTSet()
{
	return isReady;
}
#endif