#pragma once
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSyntaxHighlighter>

class VMTQSyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	VMTQSyntaxHighlighter( QTextDocument *parent = nullptr );

protected:
	void highlightBlock( const QString &text ) override;

private:
	struct HighlightingRule
	{
		QRegularExpression pattern;
		QTextCharFormat format;
	};
	struct HighlightPattern
	{
		QString pattern;
		QString description;
		HighlightPattern( QString pat, QString desc = 0 ) :
			pattern( pat ), description( desc ) {};
	};

	QList<HighlightingRule> highlightingRules;

	QRegularExpression commentStartExpression;
	QRegularExpression commentEndExpression;

	QTextCharFormat mainlineFormat;
	QTextCharFormat keywordFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat multiLineCommentFormat;
	QTextCharFormat quotationFormat;
};

class VMTTextEdit : public QPlainTextEdit
{
public:
	VMTTextEdit( QWidget *parent = 0 );

	bool event( QEvent *event ) override;
};

class VFileScrollArea : public QScrollArea
{
public:
	VFileScrollArea( QWidget *parent = 0 );
	void wheelEvent( QWheelEvent *event ) override;
};