// Viewer Widget

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <string>


#include "viewer.h"
#include "nifQGLWidget.h"

// Qt includes
#include <QWidget>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QLabel>
#include <QSignalMapper>
#include <QCheckBox>

using namespace std;

//Helper Function for deleting all of layout's children, taken from https://stackoverflow.com/questions/18934361/delete-all-children-from-qvboxlayout
void remove(QLayout* layout)
{
    QLayoutItem* child;
    while(layout->count()!=0)
    {
        child = layout->takeAt(0);
        if(child->layout() != 0)
        {
            remove(child->layout());
        }
        else if(child->widget() != 0)
        {
            delete child->widget();
        }

        delete child;
    }
}

Viewer::Viewer(QWidget* parent)
	: QWidget(parent)
{
	setWindowTitle(tr("nifRender"));
	//Object List
	objectList = new QComboBox;
	
	objectList->addItem("List of Current Nif Objects");
	//Object Creation Buttons
	QPushButton* GenRandomButton = new QPushButton("Create Random Object");
	GenRandomButton->setToolTip(tr("Creates an object in the scene in a random location. Iterates between a helmet, chair, and plate."));
	QPushButton* loadFileButton = new QPushButton("Load File..");
	loadFileButton->setToolTip(tr("click to choose a .nif file to place at the origin."));
	QPushButton* resetSceneButton = new QPushButton("Reset Scene");
	
	
	//Render Window
	nifRenderWindow = new nifQGLWidget;
	
	//Set up connections for buttons
	connect(GenRandomButton, SIGNAL(clicked(void)), nifRenderWindow, SLOT(generateRandomObject(void)));
	connect(loadFileButton, SIGNAL(clicked(void)), nifRenderWindow, SLOT(loadNifFile(void)));
	connect(resetSceneButton, SIGNAL(clicked(void)), nifRenderWindow, SLOT(resetScene(void)));
	
	//Connections for updating the object list
	connect(nifRenderWindow, SIGNAL(newObj(int, string)), this, SLOT(appendNifList(int, string)));
	connect(objectList, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(getObjectInfo(const QString&)));
	connect(nifRenderWindow, SIGNAL(clearedScene(void)), this, SLOT(resetObjectList()));
	
	//layout
	layout = new QGridLayout;
	layout->setColumnStretch(0,100);
	layout->setColumnMinimumWidth(0,75);
	layout->setRowStretch(4,100);
	
	//attach the Render Window widget
	layout->addWidget(nifRenderWindow,0,0,5,1);
	
	QGroupBox* creationBox = new QGroupBox("Object Creation");
	QGridLayout* creationLay = new QGridLayout;
	creationLay->addWidget(GenRandomButton, 0,0);
	creationLay->addWidget(loadFileButton, 1,0);
	creationLay->addWidget(resetSceneButton);
	creationBox->setLayout(creationLay);
	
	nifDataBox = new QGroupBox("File Data");
	nifDataLay = new QGridLayout;
	nifDataBox->setMaximumWidth(470);
	nifDataLay->addWidget(objectList,0,0);
	
	nifDataBox->setLayout(nifDataLay);
	
	shaderFlagBox = new QGroupBox("Shader Flags");
	shaderFlagLay = new QGridLayout;
	vector<string> title;
	title.push_back("diffuse");
	title.push_back("normal");
	title.push_back("glow");
	title.push_back("parallax");
	title.push_back("cube");
	title.push_back("mask");
	title.push_back("subsurface");
	title.push_back("nothing");
	title.push_back("breakthings");
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			shaderCheck[i+j*3] = new QCheckBox(QString(title[i+j*3].c_str()), shaderFlagBox);
			shaderCheck[i+j*3]->setChecked(true);
			shaderFlagLay->addWidget(shaderCheck[i+j*3],j,i);
			connect(shaderCheck[i+j*3],SIGNAL(stateChanged(int)),this,SLOT(toggledSF(int)));
		}
	}
	shaderFlagBox->setLayout(shaderFlagLay);
	
	layout->addWidget(nifDataBox,2,1);
	layout->addWidget(shaderFlagBox,1,1);
	layout->addWidget(creationBox,0,1);
	setLayout(layout);
};

void Viewer::toggledSF(int state){
	(void)state;
	float sf[9];
	for(int i = 0; i < 9; i++){
		if(shaderCheck[i]->isChecked()){
			sf[i] = 1;
		}else{
			sf[i] = 0;
		}
	}
	nifRenderWindow->toggleShaderFlags(sf);
}
void Viewer::appendNifList(int index, string filename)
{
	ostringstream oss;
	oss << index << ": " << filename;
	QString output = oss.str().c_str();
	objectList->addItem(output);
	objectList->setCurrentIndex(index+1);
}

void Viewer::getObjectInfo(const QString& str){
	QString index = str.split(":")[0];
	if(index.toStdString().compare("L") == 0){
		return;
	}
	
	//Show which one is currently selected by showing a bounding box
	nifRenderWindow->toggleBBoxAllButOne(index.toInt());

	QString nifInfo(nifRenderWindow->getInfoFromIndex(index.toInt()).c_str());
	QStringList lines = nifInfo.split('\n');
	
	vector< vector<float> > transRot; // Index 1 is trishape, Index 2 is tX,tY,tZ,rAngle,rX,rY,rZ
	vector< vector<float> > boundBox;

	for(int i = 0; i < lines.size(); i++){
		QString line = lines[i];
		if(i == 0){
			QStringList token = line.split(','); //General Info
			
		}else if(i == 1){
			QStringList token = line.split(','); //Texture File Names
		}else if(i == 2){
			QStringList token = line.split(';'); //Translation / Rotation Data (tX, tY, tZ, rA, rX, rY, rZ)
			
			for( int j = 0; j < token.size()-1; j++){	
				QStringList subtoken = token[j].split(',');
				vector<float> transRotTemp;
				for(int k = 0; k < subtoken.size(); k++){
					transRotTemp.push_back(subtoken[k].toFloat());
				}
				transRot.push_back(transRotTemp);
			}
			
		}else if(i == 3){
			QStringList token = line.split(';'); //Bounding Box Information (MinX,MinY,MinZ,MaxX,MaxY,MaxZ)
			for( int j = 0; j < token.size(); j++){	
				QStringList subtoken = token[j].split(',');
				vector<float> boundBoxTemp;
				for(int k = 0; k < subtoken.size(); k++){
					boundBoxTemp.push_back(subtoken[k].toFloat());
				}
				boundBox.push_back(boundBoxTemp);
			}
			
		}
	}
	
	//Parse the info string and dump it to the UI
	nifDataBox->hide();
	delete nifDataLay;
	nifDataLay = new QGridLayout;
	
	
	nifDataLay->addWidget(objectList,0,0,1,5);
	
	for(unsigned int i = 0; i < transRot.size(); i++){
		//add a new box for each trishape in the object
		ostringstream oss;
		oss << "Trishape " << i;
		QGroupBox* triShapeInfoBox = new QGroupBox(oss.str().c_str());
		
		
		QDoubleSpinBox* transX = new QDoubleSpinBox;
		QDoubleSpinBox* transY = new QDoubleSpinBox;
		QDoubleSpinBox* transZ = new QDoubleSpinBox;
		
		//Set values, range, step size, decimal count for Translation/Rotation sliders
		transX->setValue(transRot[i][0]); transX->setDecimals(3); transX->setRange(-200,200); transX->setSingleStep(1.0); 
		transY->setValue(transRot[i][1]); transY->setDecimals(3); transY->setRange(-200,200); transY->setSingleStep(1.0);
		transX->setValue(transRot[i][2]); transZ->setDecimals(3); transZ->setRange(-200,200); transZ->setSingleStep(1.0);
		
		QDoubleSpinBox* rotAngle = new QDoubleSpinBox;
		QDoubleSpinBox* rotX = new QDoubleSpinBox;
		QDoubleSpinBox* rotY = new QDoubleSpinBox;
		QDoubleSpinBox* rotZ = new QDoubleSpinBox;
		rotAngle->setValue(transRot[i][3]); rotAngle->setDecimals(4); rotAngle->setRange(0,360); rotAngle->setSingleStep(1.0);
		
		rotX->setValue(transRot[i][4]); rotX->setDecimals(3); rotX->setRange(-1,1); rotX->setSingleStep(0.1);
		rotY->setValue(transRot[i][5]); rotY->setDecimals(3); rotY->setRange(-1,1); rotY->setSingleStep(0.1);
		rotZ->setValue(transRot[i][6]); rotZ->setDecimals(3); rotZ->setRange(-1,1); rotZ->setSingleStep(0.1);
		
		
		QLabel* transLabel = new QLabel("Translation");
		QGridLayout* triShapeInfoLay = new QGridLayout;
		triShapeInfoLay->addWidget(transLabel,0,0);
		
		triShapeInfoLay->addWidget(transX,0,1); triShapeInfoLay->addWidget(transY,0,2); triShapeInfoLay->addWidget(transZ,0,3);
		QLabel* rotLabel = new QLabel("Rotation");
		triShapeInfoLay->addWidget(rotLabel,1,0);
		triShapeInfoLay->addWidget(rotAngle,1,1); triShapeInfoLay->addWidget(rotX,1,2); triShapeInfoLay->addWidget(rotY,1,3); triShapeInfoLay->addWidget(rotZ,1,4);
		
		triShapeInfoBox->setLayout(triShapeInfoLay);
		
		nifDataLay->addWidget(triShapeInfoBox,i+1,0);
		
		QSignalMapper* signalMapper = new QSignalMapper(nifDataLay); //Create a signal mapper with nifDataLay is its parent so it's deleted when nifDataLay is
		
		connect(transX, SIGNAL(valueChanged(double)), signalMapper, SLOT(map()));
		connect(transY, SIGNAL(valueChanged(double)), signalMapper, SLOT(map()));
		connect(transZ, SIGNAL(valueChanged(double)), signalMapper, SLOT(map()));
		connect(rotAngle, SIGNAL(valueChanged(double)), signalMapper, SLOT(map()));
		connect(rotX, SIGNAL(valueChanged(double)), signalMapper, SLOT(map()));
		connect(rotY, SIGNAL(valueChanged(double)), signalMapper, SLOT(map()));
		connect(rotZ, SIGNAL(valueChanged(double)), signalMapper, SLOT(map()));
		
		signalMapper->setMapping(transX, transRotToQString(i).prepend("tX,"));
		signalMapper->setMapping(transY, transRotToQString(i).prepend("tY,"));
		signalMapper->setMapping(transZ, transRotToQString(i).prepend("tZ,"));
		signalMapper->setMapping(rotAngle, transRotToQString(i).prepend("rA,"));
		signalMapper->setMapping(rotX, transRotToQString(i).prepend("rX,"));
		signalMapper->setMapping(rotY, transRotToQString(i).prepend("rY,"));
		signalMapper->setMapping(rotZ, transRotToQString(i).prepend("rZ,"));
		//connect the mapper to the rottrans parsing slot
		connect(signalMapper, SIGNAL(mapped(const QString)), this, SLOT(setNifTransRot(const QString)));
		
	}
	
	//Translation/Rotation Sliders
	//nifDataLay->addWidget(new QLabel("Translation"),1,0);
	//nifDataLay->addWidget(transX,1,1); nifDataLay->addWidget(transY,1,2); nifDataLay->addWidget(transZ,1,3);
	//nifDataLay->addWidget(new QLabel("Rotation"),2,0);
	//nifDataLay->addWidget(rotAngle,2,1); nifDataLay->addWidget(rotX,2,2); nifDataLay->addWidget(rotY,2,3); nifDataLay->addWidget(rotZ,2,4);
	nifDataBox->setLayout(nifDataLay);
	nifDataBox->show();
}

//Helper function for combining the information needed to set the object into a passible QString
QString Viewer::transRotToQString(int triIndex){
	QString objectIndex = objectList->currentText().mid(0,1);
	QString output = objectIndex;
	output += QString(",");
	output += QString::number(triIndex);

	return output;
}

//Slot that takes in the value information and distributes it to the above setters
void Viewer::setNifTransRot(const QString input){
	QDoubleSpinBox* sBox;
	QStringList tokens = input.split(",");
	QGridLayout* triShapeInfoLay = dynamic_cast<QGridLayout*>(nifDataLay->itemAtPosition(tokens[2].toInt()+1,0)->widget()->layout());
	
	
	if(tokens[0].toStdString().compare("tX") == 0){
		sBox = dynamic_cast<QDoubleSpinBox*>(triShapeInfoLay->itemAtPosition(0,1)->widget());
		nifRenderWindow->setNifTransX(tokens[1].toInt(), tokens[2].toInt(), sBox->value());
		
	}else if(tokens[0].toStdString().compare("tY") == 0){
		sBox = (QDoubleSpinBox*)triShapeInfoLay->itemAtPosition(0,2)->widget();
		nifRenderWindow->setNifTransY(tokens[1].toInt(), tokens[2].toInt(), sBox->value());
		
	}else if(tokens[0].toStdString().compare("tZ") == 0){
		sBox = (QDoubleSpinBox*)triShapeInfoLay->itemAtPosition(0,3)->widget();
		nifRenderWindow->setNifTransZ(tokens[1].toInt(), tokens[2].toInt(), sBox->value());
		
	}else if(tokens[0].toStdString().compare("rA") == 0){
		sBox = (QDoubleSpinBox*)triShapeInfoLay->itemAtPosition(1,1)->widget();
		nifRenderWindow->setNifRotAngle(tokens[1].toInt(), tokens[2].toInt(), sBox->value());
		
	}else if(tokens[0].toStdString().compare("rX") == 0){
		sBox = (QDoubleSpinBox*)triShapeInfoLay->itemAtPosition(1,2)->widget();
		nifRenderWindow->setNifRotX(tokens[1].toInt(), tokens[2].toInt(), sBox->value());
		
	}else if(tokens[0].toStdString().compare("rY") == 0){
		sBox = (QDoubleSpinBox*)triShapeInfoLay->itemAtPosition(1,3)->widget();
		nifRenderWindow->setNifRotY(tokens[1].toInt(), tokens[2].toInt(), sBox->value());
		
	}else if(tokens[0].toStdString().compare("rZ") == 0){
		sBox = (QDoubleSpinBox*)triShapeInfoLay->itemAtPosition(1,4)->widget();
		nifRenderWindow->setNifRotZ(tokens[1].toInt(), tokens[2].toInt(), sBox->value());
	}

}

//Slot for resetting the objectList
void Viewer::resetObjectList(){
	delete nifDataBox;
	
	nifDataBox = new QGroupBox("File Data");
	nifDataLay = new QGridLayout;
	nifDataBox->setMaximumWidth(470);
	objectList = new QComboBox;
	objectList->addItem("List of Current Nif Objects");
	nifDataLay->addWidget(objectList,0,0);
	
	connect(objectList, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(getObjectInfo(const QString&)));
	connect(nifRenderWindow, SIGNAL(clearedScene(void)), this, SLOT(resetObjectList()));

	nifDataLay->addWidget(objectList,0,0);
	nifDataBox->setLayout(nifDataLay);
	layout->addWidget(nifDataBox,2,1);
	
}
