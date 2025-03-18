#pragma once
#include <QObject>
#include <qcommandlineoption.h>
class QCommandLineParser;
class CLIProcessor
{
	enum TYPE : int8_t
	{
		CREATE = 1 << 0,
		EDIT = 1 << 1,
		EXTRACT = 1 << 2,
		CREATEANDEDIT = CREATE | EDIT,
		ALL = CREATE | EDIT | EXTRACT,
		NONE = -1
	};

	struct Option
	{
		TYPE type;
		bool required;
		QStringList cmds;
	};

	static TYPE getType( const QString &str );
	inline static std::map<TYPE, std::vector<Option>> specificOptions;
	static void addCLIOption( QCommandLineParser *parser, Option option, const QString &description );
	static bool initOptions( QCommandLineParser *pParser, const QStringList & );
	//	static void createSpecificCLIOptions( QCommandLineParser *parser, const QStringList &cmdList, const QString &description );
	//	static void editSpecificCLIOptions( QCommandLineParser *parser, const QStringList &cmdList, const QString &description );
	//	static void extractSpecificCLIOptions( QCommandLineParser *parser, const QStringList &cmdList, const QString &description );

public:
	static bool processCLI( QCommandLineParser *parser, const QStringList & );
};
