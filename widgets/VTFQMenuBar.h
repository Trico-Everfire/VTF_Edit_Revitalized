#pragma once

#include "../libs/VTFLib/VTFLib/VTFLib.h"

#include <QList>
#include <QMenuBar>
#include <QTabWidget>

enum VFileType
{
	VTFFile = 0,
#ifdef VMT_EDITOR
	VMTFIle,
#endif
	INVALID
};

class VFileQAction
{
protected:
	bool isReady = false;
	QString savePath;
	VFileQAction() = default;
	~VFileQAction() = default;

public:
	bool hasSavePath() { return !savePath.isEmpty(); };
	void setSavePath( QString sPath ) { savePath = sPath; };
	QString getSavePath() { return savePath; };
	virtual VFileType getType() = 0;
};
#ifdef VMT_EDITOR
class VMTQAction : public QWidget,
				   public VFileQAction
{
	Q_OBJECT
	VTFLib::CVMTFile *pVMTile = nullptr;

public:
	~VMTQAction()
	{
		// this place is the only logical place you should ever access VMTs from, therefore we take full ownership after it's been created elsewhere.
		delete pVMTile;
	}
	explicit VMTQAction( QWidget *parent );
	void setVMT( VTFLib::CVMTFile *pVTF );
	VTFLib::CVMTFile *getVMT();
	bool isVMTSet();
	VFileType getType() override { return VMTFIle; };

Q_SIGNALS:
	void triggeredVMT( VTFLib::CVMTFile *vmt, bool checked );
};
#endif

class VTFQAction : public QWidget,
				   public VFileQAction
{
	Q_OBJECT
	VTFLib::CVTFFile *pVTFFile = nullptr;

public:
	~VTFQAction()
	{
		// this place is the only logical place you should ever access VTFs from, therefore we take full ownership after it's been created elsewhere.
		delete pVTFFile;
	}
	explicit VTFQAction( QWidget *parent );
	void setVTF( VTFLib::CVTFFile *pVTF );
	VTFLib::CVTFFile *getVTF();
	bool isVTFSet();
	VFileType getType() override { return VTFFile; };

Q_SIGNALS:
	void triggeredVTF( VTFLib::CVTFFile *vtf, bool checked );
	void setFinalsDefault();
};

class VTFQMenuBar : public QTabWidget
{
	Q_OBJECT
public:
	explicit VTFQMenuBar( QWidget *parent );
	template <typename Func1>
	inline VTFQAction *addVTFAction( VTFLib::CVTFFile *vtf, const QString &text, Func1 slot )
	{
		auto result = new VTFQAction( this );
		result->setVTF( vtf );
		connect( result, &VTFQAction::triggeredVTF, slot );
		addTab( result, text );
		return result;
	}
#ifdef VMT_EDITOR
	template <typename Func1>
	inline VMTQAction *addVMTAction( VTFLib::CVMTFile *vmt, const QString &text, Func1 slot )
	{
		auto result = new VMTQAction( this );
		result->setVMT( vmt );
		connect( result, &VMTQAction::triggeredVMT, slot );
		addTab( result, text );
		return result;
	}
#endif
};
