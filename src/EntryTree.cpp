#include "EntryTree.h"

#include "vpkpp/PackFile.h"
#include "vpkpp/format/VPK.h"

#include <QFileSystemModel>
#include <QStringView>

EntryTree::EntryTree( QWidget *pParent ) :
	QTreeView( pParent )
{
	TreeModel *model = new TreeModel( this );
	setModel( model );

	connect( this, &EntryTree::expanded, this, [this, model]( const QModelIndex &parent )
			 {
				 if ( TreeItem *item = reinterpret_cast<TreeItem *>( parent.internalPointer() ) )
				 {
					 model->fillItem( item );
					 model->sort( 0 );
					 model->layoutChanged();
				 }
			 } );
}

TreeItem::TreeItem( QVariantList data, TreeItem *parent, bool expandable ) :
	m_itemData( std::move( data ) ), m_parentItem( parent ), m_expandable( expandable )
{
}

void TreeItem::appendChild( std::unique_ptr<TreeItem> &&child )
{
	m_childItems.push_back( std::move( child ) );
}

TreeItem *TreeItem::child( int row )
{
	return row >= 0 && row < childCount() ? m_childItems.at( row ).get() : nullptr;
}

int TreeItem::childCount() const
{
	return int( m_childItems.size() );
}

int TreeItem::row() const
{
	if ( !m_parentItem )
		return 0;
	const auto it = std::find_if( m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
								  [this]( const std::unique_ptr<TreeItem> &treeItem )
								  {
									  return treeItem.get() == this;
								  } );

	if ( it != m_parentItem->m_childItems.cend() )
		return std::distance( m_parentItem->m_childItems.cbegin(), it );
	Q_ASSERT( false ); // should not happen
	return -1;
}

int TreeItem::columnCount() const
{
	return int( m_itemData.count() );
}

QVariant TreeItem::data( int column ) const
{
	return m_itemData.value( column );
}

void TreeItem::setData( int column, QVariant data )
{
	if ( column > m_itemData.count() )
		return;

	m_itemData[column] = data;
}

TreeItem *TreeItem::parentItem()
{
	return m_parentItem;
}
bool TreeItem::isExpandable() const
{
	return m_expandable;
}
TreeItem::DataDisplayType TreeItem::getDisplayType() const
{
	return m_displayType;
}
void TreeItem::setDisplayType( DataDisplayType type )
{
	m_displayType = type;
}
void TreeItem::setPath( const QString &path )
{
	m_path = path;
}
QString TreeItem::getPath() const
{
	return m_path;
}
TreeItem::ItemType TreeItem::getItemType() const
{
	return m_itemType;
}
void TreeItem::setItemType( TreeItem::ItemType type )
{
	m_itemType = type;
}
void TreeItem::setExpandable( bool expandable )
{
	m_expandable = expandable;
}
void TreeItem::setPakfile( std::unique_ptr<vpkpp::PackFile> &&pak )
{
	m_vpkFile = std::move( pak );
}
bool TreeItem::hasPakfile()
{
	return !!m_vpkFile;
}
vpkpp::PackFile *TreeItem::pakFile() const
{
	return m_vpkFile.get();
}
std::string TreeItem::getEntry()
{
	return m_entry;
}
void TreeItem::setEntry( std::string entryPath )
{
	m_entry = entryPath;
}

TreeModel::TreeModel( QObject *parent ) :
	QAbstractItemModel( parent ), rootItem( std::make_unique<TreeItem>( QVariantList { tr( "File System" ) } ) )
{
	rootItem->setPath( QDir::rootPath() );
	fillItem( rootItem.get() );
}

void TreeModel::fillItem( TreeItem *item )
{
	if ( item->childCount() > 0 )
		return;

	static QStringList supportedImageList { "bmp", "gif", "tif", "jpg", "jpeg", "png", "tga", "hdr" };

	if ( item->getItemType() == TreeItem::VPK_FILE )
	{
		if ( !item->hasPakfile() )
		{
			auto vpk = vpkpp::VPK::open( item->getPath().toStdString() );
			if ( !vpk )
			{
				// TODO: warning message.
				return;
			}
			item->setPakfile( std::move( vpk ) );
		}

		std::map<QString, TreeItem *> pList {};
		item->pakFile()->runForAllEntries( []( std::string p, vpkpp::Entry entry ) {

		} );
		//		for ( const auto &[directory, files] : item->pakFile()->getBakedEntries() )
		//		{
		//			TreeItem *current = item;
		//
		//			QString pathString {};
		//			for ( const auto &curr : QString( directory.data() ).split( "/" ) )
		//			{
		//				pathString += curr;
		//				if ( !pList.contains( pathString ) )
		//				{
		//					//					pList.emplace(pathString);
		//					auto uniqueTreeItem = std::make_unique<TreeItem>( QVariantList() << curr, current, true );
		//					uniqueTreeItem->setDisplayType( TreeItem::DISPLAY_FOLDER );
		//					//					uniqueTreeItem->setPath( iter.fileInfo().canonicalFilePath() );
		//					pList.insert( { pathString, uniqueTreeItem.get() } );
		//					current->appendChild( std::move( uniqueTreeItem ) );
		//				}
		//				current = pList[pathString];
		//			}
		//			// TODO: file insert
		//
		//			for ( const vpkpp::Entry &file : files )
		//			{
		//				if ( file.getExtension() != "vpk" &&
		//					 file.getExtension() != "vtf" &&
		//					 file.getExtension() != "ttf" &&
		//					 file.getExtension() != "otf" &&
		//					 !supportedImageList.contains( file.getExtension().c_str() ) )
		//				{
		//					continue;
		//				}
		//
		//				auto uniqueTreeItem = std::make_unique<TreeItem>( QVariantList() << file.getFilename().c_str(), current, false );
		//
		//				uniqueTreeItem->setEntry( file.path );
		//				if ( file.getExtension() == "vtf" )
		//					uniqueTreeItem->setDisplayType( TreeItem::DISPLAY_VTF );
		//				if ( file.getExtension() == "ttf" || file.getExtension() == "otf" )
		//					uniqueTreeItem->setDisplayType( TreeItem::DISPLAY_FONT );
		//				if ( supportedImageList.contains( file.getExtension().c_str() ) )
		//					uniqueTreeItem->setDisplayType( TreeItem::DISPLAY_IMAGE );
		//				uniqueTreeItem->setItemType( TreeItem::VPK_INTERNAL );
		//				current->appendChild( std::move( uniqueTreeItem ) );
		//			}
		//		}

		return;
	}

	QDirIterator iter( item->getPath(), {}, QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoDotDot );

	while ( iter.hasNext() )
	{
		if ( iter.fileInfo().isDir() )
		{
			if ( iter.fileName() == "." )
			{
				iter.next();
				continue;
			}
			auto dirCount = QDir( iter.fileInfo().canonicalFilePath() ).count();
			auto uniqueTreeItem = std::make_unique<TreeItem>( QVariantList() << iter.fileInfo().fileName(), item, dirCount > 0 );
			uniqueTreeItem->setDisplayType( TreeItem::DISPLAY_FOLDER );
			uniqueTreeItem->setPath( iter.fileInfo().canonicalFilePath() );
			item->appendChild( std::move( uniqueTreeItem ) );
		}
		else
		{
			if ( iter.fileInfo().suffix() != "vpk" &&
				 iter.fileInfo().suffix() != "vtf" &&
				 iter.fileInfo().suffix() != "ttf" &&
				 iter.fileInfo().suffix() != "otf" &&
				 !supportedImageList.contains( iter.fileInfo().suffix() ) )
			{
				iter.next();
				continue;
			}

			if ( iter.fileInfo().suffix() == "vpk" )
			{
				auto file = QFile( iter.fileInfo().canonicalFilePath() );
				file.open( QFile::ReadOnly );
				std::uint32_t signature = *reinterpret_cast<std::uint32_t *>( file.read( 4 ).data() );
				file.close();
				if ( signature != vpkpp::VPK_SIGNATURE )
				{
					iter.next();
					continue;
				} // Not a valid VPK.
			}

			auto uniqueTreeItem = std::make_unique<TreeItem>( QVariantList() << iter.fileInfo().fileName(), item );

			uniqueTreeItem->setItemType( TreeItem::REGULAR );

			if ( iter.fileInfo().suffix() == "vpk" )
			{
				uniqueTreeItem->setDisplayType( TreeItem::DISPLAY_VPK );
				uniqueTreeItem->setItemType( TreeItem::VPK_FILE );
				uniqueTreeItem->setExpandable( true );
			}
			if ( iter.fileInfo().suffix() == "vtf" )
				uniqueTreeItem->setDisplayType( TreeItem::DISPLAY_VTF );
			if ( iter.fileInfo().suffix() == "ttf" || iter.fileInfo().suffix() == "otf" )
				uniqueTreeItem->setDisplayType( TreeItem::DISPLAY_FONT );
			if ( supportedImageList.contains( iter.fileInfo().suffix() ) )
				uniqueTreeItem->setDisplayType( TreeItem::DISPLAY_IMAGE );

			uniqueTreeItem->setPath( iter.fileInfo().canonicalFilePath() );

			item->appendChild( std::move( uniqueTreeItem ) );
		}

		iter.next();
	}
}

TreeModel::~TreeModel() = default;

QModelIndex TreeModel::index( int row, int column, const QModelIndex &parent ) const
{
	if ( !hasIndex( row, column, parent ) )
		return {};

	TreeItem *parentItem = parent.isValid() ? static_cast<TreeItem *>( parent.internalPointer() ) : rootItem.get();

	if ( auto *childItem = parentItem->child( row ) )
		return createIndex( row, column, childItem );
	return {};
}

QModelIndex TreeModel::parent( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return {};

	auto *childItem = static_cast<TreeItem *>( index.internalPointer() );
	TreeItem *parentItem = childItem->parentItem();

	return parentItem != rootItem.get() ? createIndex( parentItem->row(), 0, parentItem ) : QModelIndex {};
}

int TreeModel::rowCount( const QModelIndex &parent ) const
{
	if ( parent.column() > 0 )
		return 0;

	const TreeItem *parentItem = parent.isValid() ? static_cast<const TreeItem *>( parent.internalPointer() ) : rootItem.get();

	return parentItem->childCount();
}

int TreeModel::columnCount( const QModelIndex &parent ) const
{
	if ( parent.isValid() )
		return static_cast<TreeItem *>( parent.internalPointer() )->columnCount();
	return rootItem->columnCount();
}

QVariant TreeModel::data( const QModelIndex &index, int role ) const
{
	const auto *item = static_cast<const TreeItem *>( index.internalPointer() );

	if ( role == Qt::DecorationRole )
	{
		switch ( item->getDisplayType() )
		{
			case TreeItem::DISPLAY_FOLDER:
				return QIcon::fromTheme( "folder" );
			case TreeItem::DISPLAY_VTF:
				return QIcon( ":/vtf.png" );
			case TreeItem::DISPLAY_VPK:
				return QIcon( ":/vpk.ico" );
			case TreeItem::DISPLAY_FONT:
				return QIcon::fromTheme( "font-x-generic" );
			case TreeItem::DISPLAY_IMAGE:
				return QIcon::fromTheme( "image-x-generic" );
			case TreeItem::DISPLAY_NONE:
				return {};
		}
	}

	if ( !index.isValid() || role != Qt::DisplayRole )
		return {};

	return item->data( index.column() );
}

Qt::ItemFlags TreeModel::flags( const QModelIndex &index ) const
{
	return index.isValid() ? QAbstractItemModel::flags( index ) : Qt::ItemFlags( Qt::NoItemFlags );
}

QVariant TreeModel::headerData( int section, Qt::Orientation orientation,
								int role ) const
{
	return orientation == Qt::Horizontal && role == Qt::DisplayRole ? rootItem->data( section ) : QVariant {};
}

void TreeModel::setupModelData( const QList<QStringView> &lines, TreeItem *parent )
{
}

bool TreeModel::hasChildren( const QModelIndex &parent ) const
{
	if ( const auto *item = reinterpret_cast<TreeItem *>( parent.internalPointer() ) )
	{
		/*
		 * Here you need to return true or false depending on whether
		 * or not any attached views should treat `item' as having one
		 * or more child items.  Presumably based on the same logic
		 * that governs `isExpandable' in the code you've shown.
		 */
		return item->isExpandable() ? true : QAbstractItemModel::hasChildren( parent );
	}
	return QAbstractItemModel::hasChildren( parent );
}
