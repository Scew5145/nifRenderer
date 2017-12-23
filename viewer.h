#ifndef VIEWER_H
#define VIEWER_H

#include "nifQGLWidget.h"

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <string>

#include <QWidget>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>


using namespace std;

class Viewer : public QWidget
{
Q_OBJECT
private:
	QComboBox* objectList;
	nifQGLWidget* nifRenderWindow;
	QGridLayout* nifDataLay;
	QGroupBox* nifDataBox;
	QString transRotToQString(int triIndex); //Helper function for updating translation/rotation
public:
	Viewer(QWidget* parent=0);
private slots:
	void appendNifList(int, string);
	void getObjectInfo(const QString&);
	void setNifTransRot(const QString);
	void resetObjectList(void);
};

#endif //VIEWER_H
