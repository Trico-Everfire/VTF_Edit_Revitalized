#include "CLIProcessor.h"

#include "MainWindow.h"

#include <QCommandLineParser>
#include <QCoreApplication>

using namespace ui;
bool CLIProcessor::processCLI( QCommandLineParser *parser, const QStringList &args )
{
	if ( !CLIProcessor::initOptions( parser, args ) )
		return false;

	if ( parser->isSet( "version" ) )
		parser->showVersion();

	if ( !parser->isSet( "no-ui" ) )
		return true;

	if ( parser->positionalArguments().isEmpty() )
		return false;

	switch ( CLIProcessor::getType( parser->positionalArguments()[0] ) )
	{
		case CREATE:
		{
			for ( const auto &spec : CLIProcessor::specificOptions[CREATE] )
			{
				for ( const auto &cmd : parser->optionNames() )
				{
					if ( !parser->optionNames().contains( cmd ) )
						return false;

					//					if ()
				}
			}
			auto vtf = std::make_unique<vtfpp::VTF>();

			return vtf.get();
		};
		case EDIT:
			qInfo() << "Edit";
			break;
		case EXTRACT:
			qInfo() << "Extract";
			break;
		case NONE:
		default:
			return false;
	}

	return false;
}
CLIProcessor::TYPE CLIProcessor::getType( const QString &str )
{
	if ( str == "create" )
		return CLIProcessor::CREATE;
	if ( str == "edit" )
		return CLIProcessor::EDIT;
	if ( str == "extract" )
		return CLIProcessor::EXTRACT;

	return CLIProcessor::NONE;
}
void CLIProcessor::addCLIOption( QCommandLineParser *parser, Option option, const QString &description )
{
	auto types = option.type;
	auto cmdList = option.cmds;

	if ( types == NONE )
		return;
	QString exclusiveTo = "( ";

	if ( types & CREATE )
		exclusiveTo += "Create ";
	if ( types & EDIT )
		exclusiveTo += "Edit ";
	if ( types & EXTRACT )
		exclusiveTo += "Extract ";

	exclusiveTo += ") ";

	parser->addOption( { cmdList, exclusiveTo + description } );

	if ( types & CREATE )
		CLIProcessor::specificOptions[CREATE].push_back( option );
	if ( types & EDIT )
		CLIProcessor::specificOptions[EDIT].push_back( option );
	if ( types & EXTRACT )
		CLIProcessor::specificOptions[EXTRACT].push_back( option );
}
// void CLIProcessor::createSpecificCLIOptions( QCommandLineParser *parser, const QStringList &cmdList, const QString &description )
//{
//	parser->addOption( { cmdList, description } );
//	for ( const auto &cmd : cmdList )
//	{
//		CLIProcessor::specificOptions[CREATE].push_back( cmd );
//	}
// }
// void CLIProcessor::editSpecificCLIOptions( QCommandLineParser *parser, const QStringList &cmdList, const QString &description )
//{
//	parser->addOption( { cmdList, description } );
//	for ( const auto &cmd : cmdList )
//	{
//		CLIProcessor::specificOptions[EDIT].push_back( cmd );
//	}
// }
// void CLIProcessor::extractSpecificCLIOptions( QCommandLineParser *parser, const QStringList &cmdList, const QString &description )
//{
//	parser->addOption( { cmdList, description } );
//	for ( const auto &cmd : cmdList )
//	{
//		CLIProcessor::specificOptions[EXTRACT].push_back( cmd );
//	}
// }
bool CLIProcessor::initOptions( QCommandLineParser *pParser, const QStringList &list )
{
	pParser->setApplicationDescription( "VTF Forge Is a GUI/CLI tool for creating/editing/extracting Valve Texture Format files using SourcePP's VTFPP library." );
	pParser->addHelpOption();
	pParser->addVersionOption();
	//	addCLIOption( pParser, { ALL,   QStringList() << "multi-instance" }, "Allow VTF Forge to open another instance when one is already running." );
	//	addCLIOption( pParser, { ALL,  QStringList() << "no-ui" }, "Run VTF Forge without GUI. Used for a basic CLI mode." );
	//	pParser->addPositionalArgument( "operation", "create, edit, extract | Create a new VTF : Edit VTF file contents including frame/face/slice/mip data and VTF format. : Extract image files from VTF.", "create | edit | extract" );
	//	addCLIOption( pParser, ALL, QStringList() << "width" << "vw", "Set width of either the VTF or the output image." );
	//	addCLIOption( pParser, ALL, QStringList() << "height" << "vh", "Set height of either the VTF or the output image." );
	//	addCLIOption( pParser, { ALL,  QStringList() << "width-and-height" << "wh" }, "Set both width and height of either the VTF or the output image." );
	//
	//	addCLIOption( pParser, ALL, { QStringList() << "input" << "i" }, "Input file path." );
	//	addCLIOption( pParser, { ALL, QStringList() << "output" << "o" },
	//				  "Output file." );
	//	addCLIOption( pParser, ALL, QStringList() << "postfix", "Output file postfix" );
	//	addCLIOption( pParser, ALL, QStringList() << "prefix", "Output file prefix" );
	//
	//	addCLIOption( pParser, CREATEANDEDIT, QStringList() << "format" << "f", "Set the format of the VTF." );
	//	addCLIOption( pParser, CREATEANDEDIT, QStringList() << "alpha-format" << "af", "Set the format of the VTF." );
	//	addCLIOption( pParser, EXTRACT, QStringList() << "image-type" << "it", "Set the export image type (" + CMainWindow::supportedImageList.join( ',' ) + ")" );

	return pParser->parse( list );
}
