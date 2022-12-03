//
// Created by trico on 2-8-22.
//

#ifndef VTF_EDIT_REVITALIZED_VTFEABOUT_H
#define VTF_EDIT_REVITALIZED_VTFEABOUT_H

#include <QDialog>
#include <QLabel>
class VTFEAbout : public QDialog
{
	QLabel *labelledText( const QString &text, QWidget *parent );

public:
	explicit VTFEAbout( QWidget *parent = nullptr );
};

#endif // VTF_EDIT_REVITALIZED_VTFEABOUT_H
