#include <QtGui/qcheckbox.h>
#include <QtGui/QMessageBox>
#include <QtNetwork/qudpsocket.h>
#include <QtNetwork/qabstractsocket.h>
#include <QUuid.h>

#include <dzapp.h>
#include <dzscene.h>
#include <dzmainwindow.h>
#include <dzshape.h>
#include <dzproperty.h>
#include <dzobject.h>
#include <dzpresentation.h>
#include <dznumericproperty.h>
#include <dzimageproperty.h>
#include <dzcolorproperty.h>
#include <dpcimages.h>
#include <dzfigure.h>
#include <dzfacetmesh.h>
#include <dzbone.h>
#include <dzcontentmgr.h>
//#include <dznodeinstance.h>
#include "idzsceneasset.h"
#include "dzuri.h"

#include "DzUnrealDialog.h"
#include "DzUnrealAction.h"
#include "DzBridgeMorphSelectionDialog.h"

DzUnrealAction::DzUnrealAction() :
	 DzRuntimePluginAction(tr("&Daz to Unreal"), tr("Send the selected node to Unreal."))
{
	 Port = 0;
	 BridgeDialog = nullptr;
     NonInteractiveMode = 0;
	 AssetType = QString("SkeletalMesh");
	 //Setup Icon
	 QString iconName = "icon";
	 QPixmap basePixmap = QPixmap::fromImage(getEmbeddedImage(iconName.toLatin1()));
	 QIcon icon;
	 icon.addPixmap(basePixmap, QIcon::Normal, QIcon::Off);
	 QAction::setIcon(icon);

	 m_bGenerateNormalMaps = true;
}

void DzUnrealAction::executeAction()
{
	 // Check if the main window has been created yet.
	 // If it hasn't, alert the user and exit early.
	 DzMainWindow* mw = dzApp->getInterface();
	 if (!mw)
	 {
         if (NonInteractiveMode == 0) 
		 {
             QMessageBox::warning(0, tr("Error"),
                 tr("The main window has not been created yet."), QMessageBox::Ok);
         }
		 return;
	 }

	 // Create and show the dialog. If the user cancels, exit early,
	 // otherwise continue on and do the thing that required modal
	 // input from the user.
    if (dzScene->getNumSelectedNodes() != 1)
    {
        if (NonInteractiveMode == 0) 
		{
            QMessageBox::warning(0, tr("Error"),
                tr("Please select one Character or Prop to send."), QMessageBox::Ok);
        }
        return;
    }

    // Create the dialog
	if (BridgeDialog == nullptr)
	{
		BridgeDialog = new DzUnrealDialog(mw);
	}

	// Prepare member variables when not using GUI
	if (NonInteractiveMode == 1)
	{
		if (RootFolder != "") BridgeDialog->getIntermediateFolderEdit()->setText(RootFolder);

		if (ScriptOnly_MorphList.isEmpty() == false)
		{
			ExportMorphs = true;
			MorphString = ScriptOnly_MorphList.join("\n1\n");
			MorphString += "\n1\n.CTRLVS\n2\nAnything\n0";
			if (m_morphSelectionDialog == nullptr)
			{
				m_morphSelectionDialog = DzBridgeMorphSelectionDialog::Get(BridgeDialog);
			}
			MorphMapping.clear();
			foreach(QString morphName, ScriptOnly_MorphList)
			{
				QString label = m_morphSelectionDialog->GetMorphLabelFromName(morphName);
				MorphMapping.insert(morphName, label);
			}
		}
		else
		{
			ExportMorphs = false;
			MorphString = "";
			MorphMapping.clear();
		}

	}

    // If the Accept button was pressed, start the export
    int dialog_choice = -1;
	if (NonInteractiveMode == 0)
	{
		dialog_choice = BridgeDialog->exec();
	}
    if (NonInteractiveMode == 1 || dialog_choice == QDialog::Accepted)
    {
		// Read in Custom GUI values
		Port = BridgeDialog->getPortEdit()->text().toInt();
		ExportMaterialPropertiesCSV = BridgeDialog->getExportMaterialPropertyCSVCheckBox()->isChecked();
		// Read in Common GUI values
		readGUI(BridgeDialog);

		exportHD();
    }
}

void DzUnrealAction::WriteConfiguration()
{
	 QString DTUfilename = DestinationPath + CharacterName + ".dtu";
	 QFile DTUfile(DTUfilename);
	 DTUfile.open(QIODevice::WriteOnly);
	 DzJsonWriter writer(&DTUfile);
	 writer.startObject(true);

	 writeDTUHeader(writer);

	 if (AssetType != "Environment")
	 {
		 QTextStream *pCVSStream = nullptr;
		 if (ExportMaterialPropertiesCSV)
		 {
			 QString filename = DestinationPath + CharacterName + "_Maps.csv";
			 QFile file(filename);
			 file.open(QIODevice::WriteOnly);
			 pCVSStream = new QTextStream(&file);
			 *pCVSStream << "Version, Object, Material, Type, Color, Opacity, File" << endl;
		 }
		 writeAllMaterials(Selection, writer, pCVSStream);
		 writeAllMorphs(writer);
		 writeAllSubdivisions(writer);
		 writeAllDForceInfo(Selection, writer);

		 if (AssetType == "SkeletalMesh")
		 {
			 bool ExportDForce = true;
			 writer.startMemberArray("dForce WeightMaps", true);
			 if (ExportDForce)
			 {
				 WriteWeightMaps(Selection, writer);
			 }
			 writer.finishArray();
		 }
	 }

	 if (AssetType == "Pose")
	 {
		 writer.startMemberArray("Poses", true);

		for (QList<QString>::iterator i = PoseList.begin(); i != PoseList.end(); ++i)
		{
			writer.startObject(true);
			writer.addMember("Name", *i);
			writer.addMember("Label", MorphMapping[*i]);
			writer.finishObject();
		}

		 writer.finishArray();

	 }

	 if (AssetType == "Environment")
	 {
		 writer.startMemberArray("Instances", true);
		 QMap<QString, DzMatrix3> WritingInstances;
		 QList<DzGeometry*> ExportedGeometry;
		 WriteInstances(Selection, writer, WritingInstances, ExportedGeometry);
		 writer.finishArray();
	 }

	 writer.finishObject();

	 DTUfile.close();

	 if (AssetType != "Environment" && ExportSubdivisions)
	 {
		 QString CMD = "ImportFBXScene " + DTUfilename;
		 QByteArray array = CMD.toLocal8Bit();
		 char* cmd = array.data();
		 int res = system(cmd);
	 }

	 // Send a message to Unreal telling it to start an import
	 QUdpSocket* sendSocket = new QUdpSocket(this);
	 QHostAddress* sendAddress = new QHostAddress("127.0.0.1");

	 sendSocket->connectToHost(*sendAddress, Port);
	 sendSocket->write(DTUfilename.toUtf8());
}

// Setup custom FBX export options
void DzUnrealAction::SetExportOptions(DzFileIOSettings& ExportOptions)
{

}

void DzUnrealAction::WriteInstances(DzNode* Node, DzJsonWriter& Writer, QMap<QString, DzMatrix3>& WritenInstances, QList<DzGeometry*>& ExportedGeometry, QUuid ParentID)
{
	DzObject* Object = Node->getObject();
	DzShape* Shape = Object ? Object->getCurrentShape() : NULL;
	DzGeometry* Geometry = Shape ? Shape->getGeometry() : NULL;
	DzBone* Bone = qobject_cast<DzBone*>(Node);

	if (Bone == nullptr && Geometry)
	{
		ExportedGeometry.append(Geometry);
		ParentID = WriteInstance(Node, Writer, ParentID);
	}

	for (int ChildIndex = 0; ChildIndex < Node->getNumNodeChildren(); ChildIndex++)
	{
		DzNode* ChildNode = Node->getNodeChild(ChildIndex);
		WriteInstances(ChildNode, Writer, WritenInstances, ExportedGeometry, ParentID);
	}
}

QUuid DzUnrealAction::WriteInstance(DzNode* Node, DzJsonWriter& Writer, QUuid ParentID)
{
	QString Path = Node->getAssetFileInfo().getUri().getFilePath();
	QFile File(Path);
	QString FileName = File.fileName();
	QStringList Items = FileName.split("/");
	QStringList Parts = Items[Items.count() - 1].split(".");
	QString AssetID = Node->getAssetUri().getId();
	QString Name = AssetID.remove(QRegExp("[^A-Za-z0-9_]"));
	QUuid Uid = QUuid::createUuid();

	Writer.startObject(true);
	Writer.addMember("Version", 1);
	Writer.addMember("InstanceLabel", Node->getLabel());
	Writer.addMember("InstanceAsset", Name);
	Writer.addMember("ParentID", ParentID.toString());
	Writer.addMember("Guid", Uid.toString());
	Writer.addMember("TranslationX", Node->getWSPos().m_x);
	Writer.addMember("TranslationY", Node->getWSPos().m_y);
	Writer.addMember("TranslationZ", Node->getWSPos().m_z);

	DzQuat RotationQuat = Node->getWSRot();
	DzVec3 Rotation;
	RotationQuat.getValue(Node->getRotationOrder(), Rotation);
	Writer.addMember("RotationX", Rotation.m_x);
	Writer.addMember("RotationY", Rotation.m_y);
	Writer.addMember("RotationZ", Rotation.m_z);

	DzMatrix3 Scale = Node->getWSScale();

	Writer.addMember("ScaleX", Scale.row(0).length());
	Writer.addMember("ScaleY", Scale.row(1).length());
	Writer.addMember("ScaleZ", Scale.row(2).length());
	Writer.finishObject();

	return Uid;
}

// Overrides baseclass implementation with Unreal specific resets
// Resets Default Values but Ignores any saved settings
void DzUnrealAction::resetToDefaults()
{
	DzRuntimePluginAction::resetToDefaults();

	// Must Instantiate BridgeDialog so that we can override any saved states
	if (BridgeDialog == nullptr)
	{
		DzMainWindow* mw = dzApp->getInterface();
		BridgeDialog = new DzUnrealDialog(mw);
	}
	BridgeDialog->resetToDefaults();

	if (m_subdivisionDialog != nullptr)
	{
		foreach(QObject * obj, m_subdivisionDialog->getSubdivisionCombos())
		{
			QComboBox* combo = qobject_cast<QComboBox*>(obj);
			if (combo)
				combo->setCurrentIndex(0);
		}
	}
	// reset morph selection
	//DzBridgeMorphSelectionDialog::Get(nullptr)->PrepareDialog();

}

bool DzUnrealAction::setBridgeDialog(DzBasicDialog* arg_dlg) 
{
	BridgeDialog = qobject_cast<DzUnrealDialog*>(arg_dlg); 

	if (BridgeDialog == nullptr && arg_dlg != nullptr)
	{
		return false;
	}

	return true;
}

#include "moc_DzUnrealAction.cpp"
