#pragma once

#include <QFileSystemModel>
#include <QTreeWidget>
#include <vpkpp/PackFile.h>
class EntryTree : public QTreeView
{
	Q_OBJECT
public:
	EntryTree( QWidget *pParent );
};

class TreeItem
{
public:
	enum DataDisplayType
	{
		DISPLAY_FOLDER = 0,
		DISPLAY_VTF,
		DISPLAY_VPK,
		DISPLAY_FONT,
		DISPLAY_IMAGE,
		DISPLAY_NONE
	};

	enum ItemType
	{
		REGULAR = 0,
		VPK_FILE,
		VPK_INTERNAL
	};

public:
	explicit TreeItem( QVariantList data, TreeItem *parentItem = nullptr, bool isExpandable = false );

	void appendChild( std::unique_ptr<TreeItem> &&child );

	TreeItem *child( int row );
	int childCount() const;
	int columnCount() const;
	void setExpandable( bool expandable );
	bool isExpandable() const;
	QVariant data( int column ) const;
	int row() const;
	TreeItem *parentItem();
	void setDisplayType( DataDisplayType type );
	DataDisplayType getDisplayType() const;
	ItemType getItemType() const;
	void setItemType( ItemType type );
	void setPath( const QString &path );
	QString getPath() const;
	bool hasPakfile();
	void setPakfile( std::unique_ptr<vpkpp::PackFile> &&pak );
	vpkpp::PackFile *pakFile() const;
	std::string getEntry();
	void setEntry( std::string entryPath );

	void setData( int column, QVariant data );

private:
	std::vector<std::unique_ptr<TreeItem>> m_childItems;
	QVariantList m_itemData;
	TreeItem *m_parentItem;
	std::unique_ptr<vpkpp::PackFile> m_vpkFile;
	std::string m_entry;
	bool m_expandable;
	DataDisplayType m_displayType;
	ItemType m_itemType;
	QString m_path;
};

class TreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	Q_DISABLE_COPY_MOVE( TreeModel )

	explicit TreeModel( QObject *parent );
	~TreeModel() override;

	QVariant data( const QModelIndex &index, int role ) const override;
	Qt::ItemFlags flags( const QModelIndex &index ) const override;
	QVariant headerData( int section, Qt::Orientation orientation,
						 int role ) const override;

	bool hasChildren( const QModelIndex &parent ) const override;
	QModelIndex index( int row, int column,
					   const QModelIndex &parent = {} ) const override;
	QModelIndex parent( const QModelIndex &index ) const override;
	int rowCount( const QModelIndex &parent = {} ) const override;
	int columnCount( const QModelIndex &parent = {} ) const override;

	void fillItem( TreeItem *item );

private:
	static void setupModelData( const QList<QStringView> &lines, TreeItem *parent );

	std::unique_ptr<TreeItem> rootItem;
};
