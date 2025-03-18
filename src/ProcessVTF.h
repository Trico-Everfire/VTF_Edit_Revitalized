#pragma once

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <vtfpp/VTF.h>

class VTFEImageContainer;
class QComboBox;
class QCheckBox;
class QShortcut;
class ProcessVTF;

class RemovableItemDelegate : public QStyledItemDelegate
{
public:
	RemovableItemDelegate( QObject *parent = nullptr );

	void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

	QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

	void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
	void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

	void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

private:
};

class SharedTabWidget : public QListView
{
	//		using QListView::QListView;
	uint16_t maximum = 65535;
	bool deleteOnDragSuccess = true;
	void dropEvent( QDropEvent *event ) override;
	void dragMoveEvent( QDragMoveEvent *e ) override;

public:
	SharedTabWidget( QWidget *parent );
	void setMaximum( uint16_t max ) { this->maximum = max; }
	[[nodiscard]] uint16_t getMaximum() const { return maximum; }
	[[nodiscard]] bool shouldDeleteOnDrag() const { return deleteOnDragSuccess; };
	void setDeleteOnDrag( bool should ) { deleteOnDragSuccess = should; }
};

class DropButton : public QPushButton
{
	Q_OBJECT
	ProcessVTF *dropListProvider;

public:
	explicit DropButton( ProcessVTF *listProvider, QWidget *parent = nullptr ) :
		QPushButton( parent ), dropListProvider( listProvider )
	{
		this->setAcceptDrops( true );
		connect( this, &QPushButton::clicked, this, [&]
				 {
					 this->setIcon( QIcon() );
				 } );
	};
	explicit DropButton( ProcessVTF *listProvider, const QString &text, QWidget *parent = nullptr ) :
		QPushButton( text, parent ), dropListProvider( listProvider )
	{
		this->setAcceptDrops( true );
		connect( this, &QPushButton::clicked, this, [&]
				 {
					 this->setIcon( QIcon() );
				 } );
	};
	DropButton( ProcessVTF *listProvider, const QIcon &icon, const QString &text, QWidget *parent = nullptr ) :
		QPushButton( icon, text, parent ), dropListProvider( listProvider )
	{
		this->setAcceptDrops( true );
		connect( this, &QPushButton::clicked, this, [&]
				 {
					 this->setIcon( QIcon() );
				 } );
	};

	void dragEnterEvent( QDragEnterEvent *event ) override;
	void dropEvent( QDropEvent *event ) override;

	int imageIndex = 0;

signals:
	void onImageInserted();
	void onImageRemoved();
};

class ProcessVTF : public QDialog
{
	Q_OBJECT
	enum class TextureType : uint8_t
	{
		SINGLE_IMAGE = 0,
		ANIMATED_TEXTURE,
		CUBEMAP,
		ANIMATED_CUBEMAP,
		VOLUMETRIC_TEXTURES
	};

	uint16_t imageID = 0;
	vtfpp::VTF *processVtf = nullptr;
	QString fileName;
	QShortcut *pasteShortcut = nullptr;
	QShortcut *exitShortcut = nullptr;

	SharedTabWidget *imageListList {};
	SharedTabWidget *baseImageList;
	SharedTabWidget *frameList;
	SharedTabWidget *faceList;
	SharedTabWidget *sliceList;
	SharedTabWidget *mipList;
	QPushButton *applButton;
	QComboBox *pFormatCombo;
	QComboBox *typeComboBox;
	QCheckBox *mipmapCheckBox;
	DropButton *singleFrameContainerButton;

	void setupUI();
	bool readyUI();

	void applyChanges();

public:
	ProcessVTF( QWidget *parent, vtfpp::VTF *vtf );
	~ProcessVTF();

	bool addImage( const QImage &image );
	bool addImage( const QString &imagePath );
	bool addImage( const QStringList &imagePaths );
	bool addImage( const std::byte *data, size_t size, uint16_t width, uint16_t height, vtfpp::ImageFormat format );
	bool addImage( const std::vector<std::byte> &data, uint16_t width, uint16_t height, vtfpp::ImageFormat format );

	int exec() override;
	QString getFileName() const { return fileName; };
	void dragEnterEvent( QDragEnterEvent *event ) override;
	void dropEvent( QDropEvent *event ) override;
	QMap<uint16_t, VTFEImageContainer *> imageList;
public slots:
	void onPaste();
};