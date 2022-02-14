#pragma once
#include "dzbasicdialog.h"
#include <QtGui/qcombobox.h>
#include <QtCore/qsettings.h>
#include "DzBridgeDialog.h"

class QPushButton;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QGroupBox;

class DzUnrealDialog : public DzBridgeDialog {
	Q_OBJECT
	Q_PROPERTY(QWidget* wIntermediateFolderEdit READ getIntermediateFolderEdit)
	Q_PROPERTY(QWidget* wPortEdit READ getPortEdit)
	Q_PROPERTY(QWidget* wExportMaterialPropertyCSVCheckBox READ getExportMaterialPropertyCSVCheckBox)
public:
	Q_INVOKABLE QLineEdit* getIntermediateFolderEdit() { return intermediateFolderEdit; }
	Q_INVOKABLE QLineEdit* getPortEdit() { return portEdit; }
	Q_INVOKABLE QCheckBox* getExportMaterialPropertyCSVCheckBox() { return exportMaterialPropertyCSVCheckBox; }

	/** Constructor **/
	 DzUnrealDialog(QWidget *parent=nullptr);

	/** Destructor **/
	virtual ~DzUnrealDialog() {}

	Q_INVOKABLE void resetToDefaults();

	QString getRootFolder();
protected slots:
	void HandleSelectIntermediateFolderButton();
	void HandlePortChanged(const QString& port);

protected:
	Q_INVOKABLE bool loadSavedSettings();

	QLineEdit* portEdit;
	QLineEdit* intermediateFolderEdit;
	QPushButton* intermediateFolderButton;
	QCheckBox* exportMaterialPropertyCSVCheckBox;

};
