//
// Created by trico on 1-9-22.
//

#include "VMTQSyntaxHighlighter.h"

#include "KeyValue.h"

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QToolTip>
VMTQSyntaxHighlighter::VMTQSyntaxHighlighter( QTextDocument *parent ) :
	QSyntaxHighlighter( parent )
{
	HighlightingRule rule;
	mainlineFormat.setForeground( QBrush( "#008080" ) );

	const HighlightPattern keywordPatterns[] = {
		HighlightPattern( QStringLiteral( "\\bLightMappedGeneric\\b" ), "Basic LightmappedGeneric material for brushes" ),
		HighlightPattern( QStringLiteral( "\\bVertexlitGeneric\\b" ), "Basic VertexlitGeneric material for Models" )

	};
	QList<HighlightPattern> def;
	auto file = QFile( QDir::currentPath() + "/test_json.json" );
	file.open( QIODevice::ReadOnly );
	auto data = file.readAll();
	file.close();
	QJsonDocument jsonDocument = QJsonDocument::fromJson( data );
	QJsonObject object = jsonDocument.object();

	keywordFormat.setForeground( QBrush( "#ffaf54" ) );

	for ( QJsonValue j : object.value( "default" ).toArray() )
	{
		QJsonObject matshader = j.toObject();
		QString slContext = QString( matshader.value( "name" ).toString() );
		auto slEscapedContext = ( QRegularExpression::escape( slContext ) );
		// qInfo() << slEscapedContext;
		QString description = !matshader.value( "description" ).isNull() ? matshader.value( "description" ).toString() : 0;
		description += !matshader.value( "wikiUri" ).isNull() ? "\n<a href='" + matshader.value( "wikiUri" ).toString() + "'>" + matshader.value( "name" ).toString() + "</a>" : 0;
		auto regexp = QRegularExpression( "\\B" + slEscapedContext + "\\b" );
		rule.pattern = regexp;
		rule.format = keywordFormat;
		rule.format.setToolTip( description );
		highlightingRules.append( rule );
	}

	for ( const HighlightPattern &hPatt : keywordPatterns )
	{
		rule.pattern = QRegularExpression( hPatt.pattern );
		rule.format = mainlineFormat;
		rule.format.setToolTip( hPatt.description );
		highlightingRules.append( rule );
	}

	//	classFormat.setFontWeight(QFont::Bold);
	//	classFormat.setForeground(Qt::magenta);
	//	rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
	//	rule.format = classFormat;
	//	highlightingRules.append(rule);

	//	quotationFormat.setForeground(Qt::green);
	//	rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
	//	rule.format = quotationFormat;
	//	highlightingRules.append(rule);

	//	functionFormat.setFontItalic(true);
	//	functionFormat.setForeground(Qt::yellow);
	//	rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
	//	rule.format = functionFormat;
	//	highlightingRules.append(rule);

	singleLineCommentFormat.setForeground( Qt::gray );
	rule.pattern = QRegularExpression( QStringLiteral( "//[^\n]*" ) );
	rule.format = singleLineCommentFormat;
	highlightingRules.append( rule );

	multiLineCommentFormat.setForeground( Qt::gray );

	commentStartExpression = QRegularExpression( QStringLiteral( "/\\*" ) );
	commentEndExpression = QRegularExpression( QStringLiteral( "\\*/" ) );
}

void VMTQSyntaxHighlighter::highlightBlock( const QString &text )
{
	for ( const HighlightingRule &rule : qAsConst( highlightingRules ) )
	{
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch( text );
		while ( matchIterator.hasNext() )
		{
			QRegularExpressionMatch match = matchIterator.next();
			setFormat( match.capturedStart(), match.capturedLength(), rule.format );
		}
	}

	setCurrentBlockState( 0 );

	int startIndex = 0;
	if ( previousBlockState() != 1 )
		startIndex = text.indexOf( commentStartExpression );

	while ( startIndex >= 0 )
	{
		QRegularExpressionMatch match = commentEndExpression.match( text, startIndex );
		int endIndex = match.capturedStart();
		int commentLength = 0;
		if ( endIndex == -1 )
		{
			setCurrentBlockState( 1 );
			commentLength = text.length() - startIndex;
		}
		else
		{
			commentLength = endIndex - startIndex + match.capturedLength();
		}
		setFormat( startIndex, commentLength, multiLineCommentFormat );
		startIndex = text.indexOf( commentStartExpression, startIndex + commentLength );
	}
}

VMTTextEdit::VMTTextEdit( QWidget *parent ) :
	QPlainTextEdit( parent )
{
	setMouseTracking( true );
}

bool VMTTextEdit::event( QEvent *event )
{
	if ( event->type() == QEvent::ToolTip )
	{
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>( event );
		QTextCursor cursor = cursorForPosition( helpEvent->pos() );

		for ( auto fmt : cursor.block().layout()->formats() )
		{
			if ( fmt.start < cursor.position() && fmt.start + fmt.length >= cursor.position() )
			{
				QToolTip::showText( helpEvent->globalPos(), fmt.format.toolTip() );
			}
		}

		//		cursor.select(QTextCursor::WordUnderCursor);
		//		if (!cursor.selectedText().isEmpty())
		//			QToolTip::showText(helpEvent->globalPos(), /*your text*/QString("%1 %2").arg(cursor.selectedText()).arg(cursor.selectedText().length()) );
		//
		//		else
		//			QToolTip::hideText();
		return true;
	}

	if ( event->type() == QEvent::Wheel )
	{
		auto mwheelEvent = static_cast<QWheelEvent *>( event );
		if ( mwheelEvent->modifiers().testFlag( Qt::KeyboardModifier::ControlModifier ) )
			if ( mwheelEvent->angleDelta().y() > 0 )
				this->zoomIn( 1 );
			else
				this->zoomOut( 1 );
	}
	return QPlainTextEdit::event( event );
}
void VFileScrollArea::wheelEvent( QWheelEvent *event )
{
	auto mwheelEvent = static_cast<QWheelEvent *>( event );
	if ( mwheelEvent->modifiers().testFlag( Qt::KeyboardModifier::ControlModifier ) )
	{
		return;
	}
	QScrollArea::wheelEvent( event );
}
VFileScrollArea::VFileScrollArea( QWidget *parent ) :
	QScrollArea( parent )
{
}
