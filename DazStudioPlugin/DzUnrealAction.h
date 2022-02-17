#pragma once
#include <dzaction.h>
#include <dznode.h>
#include <dzgeometry.h>
#include <dzfigure.h>
#include <dzjsonwriter.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QUuid.h>
#include <DzBridgeAction.h>

class DzUnrealDialog;

class DzUnrealAction : public DzBridgeAction {
	Q_OBJECT
//	Q_PROPERTY(DzBasicDialog* wBridgeDialog READ getBridgeDialog WRITE setBridgeDialog)
public:
	 DzUnrealAction();

//	 Q_INVOKABLE DzUnrealDialog* getBridgeDialog() { return BridgeDialog; }
	 Q_INVOKABLE bool setBridgeDialog(DzBasicDialog* arg_dlg);

	 Q_INVOKABLE void resetToDefaults();
	 QString readGUIRootFolder();

protected:
	 int Port;
//	 DzUnrealDialog *BridgeDialog;

	 void executeAction();
	 Q_INVOKABLE void WriteConfiguration();
	 void SetExportOptions(DzFileIOSettings& ExportOptions);

};
