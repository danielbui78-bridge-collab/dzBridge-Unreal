#pragma once
#include <dzaction.h>
#include <dznode.h>
#include <dzgeometry.h>
#include <dzfigure.h>
#include <dzjsonwriter.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QUuid.h>
#include <DzRuntimePluginAction.h>

class DzUnrealDialog;

class DzUnrealAction : public DzRuntimePluginAction {
	Q_OBJECT
	Q_PROPERTY(DzBasicDialog* wBridgeDialog READ getBridgeDialog WRITE setBridgeDialog)
public:
	 DzUnrealAction();

	 Q_INVOKABLE DzUnrealDialog* getBridgeDialog() { return BridgeDialog; }
	 Q_INVOKABLE bool setBridgeDialog(DzBasicDialog* arg_dlg);

	 Q_INVOKABLE void resetToDefaults();

protected:
	 int Port;
	 DzUnrealDialog *BridgeDialog;

	 void executeAction();
	 void WriteMaterials(DzNode* Node, DzJsonWriter& Writer, QTextStream& Stream);
	 void WriteInstances(DzNode* Node, DzJsonWriter& Writer, QMap<QString, DzMatrix3>& WritenInstances, QList<DzGeometry*>& ExportedGeometry, QUuid ParentID = QUuid());
	 QUuid WriteInstance(DzNode* Node, DzJsonWriter& Writer, QUuid ParentID);
	 Q_INVOKABLE void WriteConfiguration();
	 void SetExportOptions(DzFileIOSettings& ExportOptions);

};
