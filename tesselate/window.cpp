/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/



#include "glwidget.h"
#include "window.h"
#include "vecpnt.h"
#include "common/str.h"

#include <cmath>
#include <string>

using namespace std;

QSize Window::sizeHint() const
{
    return QSize(1152, 864);
}

Window::Window()
{
    QWidget *mainWidget = new QWidget;
    QGridLayout *mainLayout = new QGridLayout;

    setCentralWidget(mainWidget);
    mainLayout->setColumnStretch(0, 0);
    mainLayout->setColumnStretch(1, 1);

    // OpenGL widget
    // Specify an OpenGL 3.2 format.
    QGLFormat glFormat;
    glFormat.setVersion( 3, 2 );
    glFormat.setProfile( QGLFormat::CoreProfile );
    glFormat.setSampleBuffers( false );

    perspectiveView = new GLWidget(glFormat);

    getCamera().setForcedFocus(vpPoint(0.0f, 0.0f, 0.0f));
    getCamera().setViewScale(1.0f);

    // param panel
    paramPanel = new QWidget;
    QVBoxLayout *paramLayout = new QVBoxLayout;

    // components on parameter panel

    // radius settings
    QGroupBox *radGroup = new QGroupBox(tr("Wireframe Settings"));
    QLabel *radSphereLabel = new QLabel(tr("Vertex Radius:"));
    radSphereEdit = new QLineEdit;
    radSphereEdit->setFixedWidth(75);
    radSphereEdit->setValidator(new QDoubleValidator(0.02, 5.0, 2, radSphereEdit));
    QLabel *radCylLabel = new QLabel(tr("Edge Radius:"));
    radCylEdit = new QLineEdit;
    radCylEdit->setFixedWidth(75);
    radCylEdit->setValidator(new QDoubleValidator(0.02, 5.0, 2, radCylEdit));

    // set initial radius values in panel
    radSphereEdit->setText(QString::number(perspectiveView->getScene()->getSphereRad(), 'g', 2));
    radCylEdit->setText(QString::number(perspectiveView->getScene()->getCylRad(), 'g', 2));

    QGridLayout *radLayout = new QGridLayout;
    radLayout->addWidget(radSphereLabel, 0, 0);
    radLayout->addWidget(radSphereEdit, 0, 1);
    radLayout->addWidget(radCylLabel, 1, 0);
    radLayout->addWidget(radCylEdit, 1, 1);
    radGroup->setLayout(radLayout);
    paramLayout->addWidget(radGroup);

    // volume settings
    QGroupBox *volGroup = new QGroupBox(tr("Volume Settings"));
    QLabel *lenLabel = new QLabel(tr("Length:"));
    lenEdit = new QLineEdit;
    lenEdit->setFixedWidth(75);
    lenEdit->setValidator(new QDoubleValidator(1.0, 50.0, 2, lenEdit));
    QLabel *widthLabel = new QLabel(tr("Width:"));
    widthEdit = new QLineEdit;
    widthEdit->setFixedWidth(75);
    widthEdit->setValidator(new QDoubleValidator(1.0, 50.0, 2, widthEdit));
    QLabel *hghtLabel = new QLabel(tr("Height:"));
    hghtEdit = new QLineEdit;
    hghtEdit->setFixedWidth(75);
    hghtEdit->setValidator(new QDoubleValidator(1.0, 50.0, 2, hghtEdit));
    QLabel *sideLabel = new QLabel(tr("Element Side:"));
    sideEdit = new QLineEdit;
    sideEdit->setFixedWidth(75);
    sideEdit->setValidator(new QDoubleValidator(0.2, 50.0, 2, sideEdit));

    // set initial volume values in panel
    Vector diag = perspectiveView->getScene()->getBoundDiag();
    lenEdit->setText(QString::number(diag.i, 'g', 2));
    widthEdit->setText(QString::number(diag.k, 'g', 2));
    hghtEdit->setText(QString::number(diag.j, 'g', 2));
    sideEdit->setText(QString::number(perspectiveView->getScene()->getTessLength(), 'g', 2));

    QGridLayout *volLayout = new QGridLayout;
    volLayout->addWidget(lenLabel, 0, 0);
    volLayout->addWidget(lenEdit, 0, 1);
    volLayout->addWidget(widthLabel, 1, 0);
    volLayout->addWidget(widthEdit, 1, 1);
    volLayout->addWidget(hghtLabel, 2, 0);
    volLayout->addWidget(hghtEdit, 2, 1);
    volLayout->addWidget(sideLabel, 3, 0);
    volLayout->addWidget(sideEdit, 3, 1);

    volGroup->setLayout(volLayout);
    paramLayout->addWidget(volGroup);

    // mesh settings
    QGroupBox *meshGroup = new QGroupBox(tr("Mesh Settings"));
    QLabel *scfLabel = new QLabel(tr("Scale:"));
    scfEdit = new QLineEdit;
    scfEdit->setFixedWidth(75);
    scfEdit->setValidator(new QDoubleValidator(0.0, 100.0, 2, scfEdit));
    QLabel *trsLabel = new QLabel(tr("Translation:"));
    txEdit = new QLineEdit;
    txEdit->setFixedWidth(75);
    txEdit->setValidator(new QDoubleValidator(0.0, 100.0, 2, txEdit));
    tyEdit = new QLineEdit;
    tyEdit->setFixedWidth(75);
    tyEdit->setValidator(new QDoubleValidator(0.0, 100.0, 2, tyEdit));
    tzEdit = new QLineEdit;
    tzEdit->setFixedWidth(75);
    tzEdit->setValidator(new QDoubleValidator(0.0, 100.0, 2, tzEdit));
    QLabel *rotLabel = new QLabel(tr("Rotation:"));
    rxEdit = new QLineEdit;
    rxEdit->setFixedWidth(75);
    rxEdit->setValidator(new QDoubleValidator(0.0, 100.0, 2, rxEdit));
    ryEdit = new QLineEdit;
    ryEdit->setFixedWidth(75);
    ryEdit->setValidator(new QDoubleValidator(0.0, 100.0, 2, ryEdit));
    rzEdit = new QLineEdit;
    rzEdit->setFixedWidth(75);
    rzEdit->setValidator(new QDoubleValidator(0.0, 100.0, 2, rzEdit));

    // set initial mesh values in panel
    scfEdit->setText(QString::number(perspectiveView->getXSect()->getScale(), 'g', 2));
    Vector trs = perspectiveView->getXSect()->getTranslation();
    txEdit->setText(QString::number(trs.i, 'g', 2));
    tyEdit->setText(QString::number(trs.j, 'g', 2));
    tzEdit->setText(QString::number(trs.k, 'g', 2));
    float ax, ay, az;
    perspectiveView->getXSect()->getRotations(ax, ay, az);
    rxEdit->setText(QString::number(ax, 'g', 2));
    ryEdit->setText(QString::number(ay, 'g', 2));
    rzEdit->setText(QString::number(ay, 'g', 2));

    QGridLayout *meshLayout = new QGridLayout;
    meshLayout->addWidget(scfLabel, 0, 0);
    meshLayout->addWidget(scfEdit, 0, 1);
    meshLayout->addWidget(trsLabel, 1, 0);
    meshLayout->addWidget(txEdit, 1, 1);
    meshLayout->addWidget(tyEdit, 1, 2);
    meshLayout->addWidget(tzEdit, 1, 3);
    meshLayout->addWidget(rotLabel, 2, 0);
    meshLayout->addWidget(rxEdit, 2, 1);
    meshLayout->addWidget(ryEdit, 2, 2);
    meshLayout->addWidget(rzEdit, 2, 3);

    meshGroup->setLayout(meshLayout);
    paramLayout->addWidget(meshGroup);

    // deformation settings
    QGroupBox *defGroup = new QGroupBox(tr("Deformation Settings"));
    QLabel *defLabel = new QLabel(tr("Magnitude:"));
    defEdit = new QLineEdit;
    defEdit->setFixedWidth(75);
    defEdit->setValidator(new QDoubleValidator(0.0, 100.0, 2, defEdit));

    // set initial deformation values in panel
    AxialDef axdef;
    float axval;
    perspectiveView->getScene()->getDefParam(axdef, axval);
    defEdit->setText(QString::number(axval, 'g', 2));

    QGridLayout *defLayout = new QGridLayout;
    defLayout->addWidget(defLabel, 0, 0);
    defLayout->addWidget(defEdit, 0, 1);

    defGroup->setLayout(defLayout);
    paramLayout->addWidget(defGroup);

    // button for intersecting mesh with scene
    QPushButton *xsectButton = new QPushButton(tr("1. Intersect"));
    paramLayout->addWidget(xsectButton);

    // button for generating a 3d print - not connected yet
    QPushButton *voxButton = new QPushButton(tr("2. Voxelise"));
    paramLayout->addWidget(voxButton);

    // signal to slot connections
    connect(perspectiveView, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
    connect(radSphereEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(radCylEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(lenEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(widthEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(hghtEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(sideEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(scfEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(txEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(tyEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(tzEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(rxEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(ryEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(rzEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(defEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(xsectButton, &QPushButton::clicked, this, &Window::xsectPress);
    connect(voxButton, &QPushButton::clicked, this, &Window::voxPress);

    paramPanel->setLayout(paramLayout);
    mainLayout->addWidget(perspectiveView, 0, 1);
    mainLayout->addWidget(paramPanel, 0, 0, Qt::AlignTop);

    createActions();
    createMenus();

    mainWidget->setLayout(mainLayout);
    setWindowTitle(tr("Tesselation Viewer"));
    mainWidget->setMouseTracking(true);
    setMouseTracking(true);

    paramPanel->hide();
}

void Window::keyPressEvent(QKeyEvent *e)
{
    // pass to render window
    perspectiveView->keyPressEvent(e);
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    QWidget *child=childAt(event->pos());
    QGLWidget *glwidget = qobject_cast<QGLWidget *>(child);
    if(glwidget) {
        QMouseEvent *glevent=new QMouseEvent(event->type(),glwidget->mapFromGlobal(event->globalPos()),event->button(),event->buttons(),event->modifiers());
        QCoreApplication::postEvent(glwidget,glevent);
    }
}

void Window::repaintAllGL()
{
    perspectiveView->repaint();
}

void Window::newFile()
{
    // clear everything and reset
    // this does not reset the volume parameters
    perspectiveView->getXSect()->clear();
    perspectiveView->setMeshVisible(false);
    perspectiveView->setGeometryUpdate(true);
}

void Window::open()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QImage cap;

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Intersection File"),
                                                    "~/",
                                                    tr("STL Files (*.stl)"),
                                                    &selectedFilter,
                                                    options);
    if (!fileName.isEmpty())
    {
        std::string infile = fileName.toUtf8().constData();

        // use file extension to determine action
        if(endsWith(infile, ".stl"))
        {
            perspectiveView->getXSect()->readSTL(infile);
            perspectiveView->getXSect()->boxFit(10.0f);
            perspectiveView->setMeshVisible(true);
            repaintAllGL();
        }
        else
        {
            cerr << "Error Window::open: attempt to open unrecognized file format" << endl;
        }
    }
}

void Window::saveFile()
{
    if(!tessfilename.isEmpty()) // save directly if we already have a file name
    {
        std::string outfile = tessfilename.toUtf8().constData();
        if(!endsWith(outfile, ".stl"))
            outfile = outfile + ".stl";
        // TO DO
        // perspectiveView->saveScene(outfile);
    }
    else
    {
        saveAs();
    }
}

void Window::saveAs()
{
    QFileDialog::Options options;
    QString selectedFilter;
    tessfilename = QFileDialog::getSaveFileName(this,
                                                tr("Save Tesselation"),
                                                "~/",
                                                tr("STL File (*.stl)"),
                                                &selectedFilter,
                                                options);
    if (!tessfilename.isEmpty())
    {
        std::string outfile = tessfilename.toUtf8().constData();
        if(!endsWith(outfile, ".stl"))
            outfile = outfile + ".stl";
        // TO DO
        // perspectiveView->saveScene(outfile);
    }
}

void Window::xsectPress()
{
    if(!perspectiveView->getXSect()->empty())
        perspectiveView->intersect();
    else // error message
        cerr << "Error Window::xsectPress: must load a shape to intersect against first." << endl;

}

void Window::voxPress()
{
    perspectiveView->getScene()->voxelise(0.05f);
    perspectiveView->setGeometryUpdate(true);
}

void Window::showParamOptions()
{
    paramPanel->setVisible(showParamAct->isChecked());
}

void Window::lineEditChange()
{
    bool ok;
    float val;

    if(sender() == radSphereEdit) // sphere radius
    {
        val = radSphereEdit->text().toFloat(&ok);
        if(ok)
        {
            perspectiveView->getScene()->setSphereRad(val);
            perspectiveView->setGeometryUpdate(true);
        }
    }
    if(sender() == radCylEdit) // cylinder radius
    {
        val = radCylEdit->text().toFloat(&ok);
        if(ok)
        {
            perspectiveView->getScene()->setCylRad(val);
            perspectiveView->setGeometryUpdate(true);
        }
    }
    if(sender() == lenEdit) // volume x-length
    {
        val = lenEdit->text().toFloat(&ok);
        if(ok)
        {
            Vector diag = perspectiveView->getScene()->getBoundDiag();
            diag.i = val;
            perspectiveView->getScene()->setBoundDiag(diag);
            perspectiveView->setGeometryUpdate(true);
        }
    }
    if(sender() == widthEdit) // volume z-length
    {
        val = widthEdit->text().toFloat(&ok);
        if(ok)
        {
            Vector diag = perspectiveView->getScene()->getBoundDiag();
            diag.k = val;
            perspectiveView->getScene()->setBoundDiag(diag);
            perspectiveView->setGeometryUpdate(true);
        }
    }
    if(sender() == hghtEdit) // volume y-length
    {
        val = hghtEdit->text().toFloat(&ok);
        if(ok)
        {
            Vector diag = perspectiveView->getScene()->getBoundDiag();
            diag.j = val;
            perspectiveView->getScene()->setBoundDiag(diag);
            perspectiveView->setGeometryUpdate(true);
        }
    }
    if(sender() == sideEdit) // tesselation element side length
    {
        val = sideEdit->text().toFloat(&ok);
        if(ok)
        {
            perspectiveView->getScene()->setTessLength(val);
            perspectiveView->setGeometryUpdate(true);
        }
    }
    if(sender() == scfEdit) // tesselation element side length
    {
        val = scfEdit->text().toFloat(&ok);
        if(ok)
        {
            perspectiveView->getXSect()->setScale(val);
            perspectiveView->setMeshVisible(true);
        }
    }
    if(sender() == txEdit) // tesselation element side length
    {
        val = txEdit->text().toFloat(&ok);
        if(ok)
        {
            Vector trs = perspectiveView->getXSect()->getTranslation();
            trs.i = val;
            perspectiveView->getXSect()->setTranslation(trs);
            perspectiveView->setMeshVisible(true);
        }
    }
    if(sender() == tyEdit) // tesselation element side length
    {
        val = tyEdit->text().toFloat(&ok);
        if(ok)
        {
            Vector trs = perspectiveView->getXSect()->getTranslation();
            trs.j = val;
            perspectiveView->getXSect()->setTranslation(trs);
            perspectiveView->setMeshVisible(true);
        }
    }
    if(sender() == tzEdit) // tesselation element side length
    {
        val = tzEdit->text().toFloat(&ok);
        if(ok)
        {
            Vector trs = perspectiveView->getXSect()->getTranslation();
            trs.k = val;
            perspectiveView->getXSect()->setTranslation(trs);
            perspectiveView->setMeshVisible(true);
        }
    }
    if(sender() == rxEdit) // tesselation element side length
    {
        val = rxEdit->text().toFloat(&ok);
        if(ok)
        {
            float ax, ay, az;
            perspectiveView->getXSect()->getRotations(ax, ay, az);
            ax = val;
            perspectiveView->getXSect()->setRotations(ax, ay, az);
            perspectiveView->setMeshVisible(true);
        }
    }
    if(sender() == ryEdit) // tesselation element side length
    {
        val = ryEdit->text().toFloat(&ok);
        if(ok)
        {
            float ax, ay, az;
            perspectiveView->getXSect()->getRotations(ax, ay, az);
            ay = val;
            perspectiveView->getXSect()->setRotations(ax, ay, az);
            perspectiveView->setMeshVisible(true);
        }
    }
    if(sender() == rzEdit) // tesselation element side length
    {
        val = rzEdit->text().toFloat(&ok);
        if(ok)
        {
            float ax, ay, az;
            perspectiveView->getXSect()->getRotations(ax, ay, az);
            az = val;
            perspectiveView->getXSect()->setRotations(ax, ay, az);
            perspectiveView->setMeshVisible(true);
        }
    }
    if(sender() == defEdit) // deformation axis value
    {
        val = defEdit->text().toFloat(&ok);
        if(ok)
        {
            AxialDef axdef;
            float axval;
            perspectiveView->getScene()->getDefParam(axdef, axval);
            perspectiveView->getScene()->setDefParam(axdef, val);
            perspectiveView->setGeometryUpdate(true);
        }
    }

    repaintAllGL();
}

void Window::createActions()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(tr("&Open"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save a file"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

    saveAsAct = new QAction(tr("Save as"), this);
    saveAsAct->setStatusTip(tr("Save a file under name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    showParamAct = new QAction(tr("Show Parameters"), this);
    showParamAct->setCheckable(true);
    showParamAct->setChecked(false);
    showParamAct->setStatusTip(tr("Hide/Show Parameters"));
    connect(showParamAct, SIGNAL(triggered()), this, SLOT(showParamOptions()));
}

void Window::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(showParamAct);
}
