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
#include <QMessageBox>

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

    cpi = cpj = cpk = 0; // initial selected control point
    latdx = latdy = latdz = 3; // order of the ffd lattice in each dimension
    sliderange = 40.0f; // determines bounds of how far cp can be shifted

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

    getCamera().setForcedFocus(cgp::Point(0.0f, 0.0f, 0.0f));
    getCamera().setViewScale(1.0f);

    // param panel
    paramPanel = new QWidget;
    QVBoxLayout *paramLayout = new QVBoxLayout;

    // components on parameter panel
    // check box for display of scene
    checkModel = new QCheckBox(tr("Show Scene"));
    checkModel->setChecked(false);
    paramLayout->addWidget(checkModel);

    // check box for display of ffd lattice
    checkLat = new QCheckBox(tr("Show Lattice"));
    checkLat->setChecked(true);
    paramLayout->addWidget(checkLat);

    // control point index selection
    QGroupBox *selGroup = new QGroupBox(tr("Control Point Selection"));
    QGridLayout *selLayout = new QGridLayout;

    QLabel *iLabel = new QLabel(tr("i:"));
    iEdit = new QLineEdit;
    iEdit->setFixedWidth(25);
    iEdit->setValidator(new QIntValidator(0, latdx-1, iEdit));
    QLabel *jLabel = new QLabel(tr("j:"));
    jEdit = new QLineEdit;
    jEdit->setFixedWidth(25);
    jEdit->setValidator(new QIntValidator(0, latdy-1, jEdit));
    QLabel *kLabel = new QLabel(tr("k:"));
    kEdit = new QLineEdit;
    kEdit->setFixedWidth(25);
    kEdit->setValidator(new QIntValidator(0, latdz-1, kEdit));

    iEdit->setText(QString::number(cpi, 'i', 0));
    jEdit->setText(QString::number(cpj, 'i', 0));
    kEdit->setText(QString::number(cpk, 'i', 0));

    selLayout->addWidget(iLabel, 0, 0);
    selLayout->addWidget(iEdit, 0, 1);
    selLayout->addWidget(jLabel, 0, 2);
    selLayout->addWidget(jEdit, 0, 3);
    selLayout->addWidget(kLabel, 0, 4);
    selLayout->addWidget(kEdit, 0, 5);
    selGroup->setLayout(selLayout);
    paramLayout->addWidget(selGroup);

    // control point positioning
    QGroupBox *ffdGroup = new QGroupBox(tr("Control Point Position"));
    QVBoxLayout *ffdLayout = new QVBoxLayout;

    // add cp sliders with correct initial positions
    cgp::Point trs = perspectiveView->getDef()->getCP(cpi, cpj, cpk);

    // x translate
    ffdLayout->addWidget(new QLabel(tr("Trs X")));
    xtrslider = new QSlider(Qt::Horizontal);
    xtrslider->setMinimum(int(std::ceil(-20.0f * sliderange)));
    xtrslider->setMaximum(int(std::floor(20.0f * sliderange)));
    xtrslider->setPageStep(200);
    xtrslider->setSingleStep(1);
    xtrslider->setTracking(true);
    xtrslider->setValue(int(std::round(trs.x * sliderange)));
    ffdLayout->addWidget(xtrslider);

    // y translate
    ffdLayout->addWidget(new QLabel(tr("Trs Y")));
    ytrslider = new QSlider(Qt::Horizontal);
    ytrslider->setMinimum(int(std::ceil(-20.0f * sliderange)));
    ytrslider->setMaximum(int(std::floor(20.0f * sliderange)));
    ytrslider->setPageStep(200);
    ytrslider->setSingleStep(1);
    ytrslider->setTracking(true);
    ytrslider->setValue(int(std::round(trs.y * sliderange)));
    ffdLayout->addWidget(ytrslider);

    // z translate
    ffdLayout->addWidget(new QLabel(tr("Trs Z")));
    ztrslider = new QSlider(Qt::Horizontal);
    ztrslider->setMinimum(int(std::ceil(-20.0f * sliderange)));
    ztrslider->setMaximum(int(std::floor(20.0f * sliderange)));
    ztrslider->setPageStep(200);
    ztrslider->setSingleStep(1);
    ztrslider->setTracking(true);
    ztrslider->setValue(int(std::round(trs.z * sliderange)));
    ffdLayout->addWidget(ztrslider);

    ffdGroup->setLayout(ffdLayout);
    paramLayout->addWidget(ffdGroup);

    // button for loading models
    loadButton = new QPushButton(tr("Load STL"));
    loadButton->setEnabled(true);
    paramLayout->addWidget(loadButton);

    // button for loading voxel grids
    loadGridButton = new QPushButton(tr("Load Voxel Grid"));
    loadGridButton->setEnabled(true);
    paramLayout->addWidget(loadGridButton);

    // button for voxelising csg tree
    voxButton = new QPushButton(tr("Voxelize"));
    //voxButton->setEnabled(true);
    paramLayout->addWidget(voxButton);

    // button for marching cubes
    marchButton = new QPushButton(tr("Extract Isosurface"));
    marchButton->setEnabled(false);
    paramLayout->addWidget(marchButton);

    // button for smooth isosurface
    smoothButton = new QPushButton(tr("Smooth"));
    smoothButton->setEnabled(false);
    paramLayout->addWidget(smoothButton);

    // button for voxelising csg tree
    defButton = new QPushButton(tr("Deform"));
    defButton->setEnabled(false);
    paramLayout->addWidget(defButton);

    // button for shrinking mesh
    shrinkButton = new QPushButton(tr("Shrink"));
    shrinkButton->setEnabled(true);
    paramLayout->addWidget(shrinkButton);

    // button for demoing
    demoButton = new QPushButton(tr("Demo"));
    demoButton->setEnabled(true);
    paramLayout->addWidget(demoButton);

    // panel for demo label
    demoLabel = new QLabel();
    demoLabel->setText("");
    QFont f( "Arial", 10, QFont::Light);
    demoLabel->setFont(f);
    demoLabel->setMargin(10);
    demoLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    paramLayout->setSizeConstraint(QLayout::SetMinimumSize);
    paramLayout->addWidget(demoLabel);

    // signal to slot connections
    connect(perspectiveView, SIGNAL(signalRepaintAllGL()), this, SLOT(repaintAllGL()));
    connect(checkModel, SIGNAL(stateChanged(int)), this, SLOT(showModel(int)));
    connect(checkLat, SIGNAL(stateChanged(int)), this, SLOT(showLat(int)));
    connect(loadButton, &QPushButton::clicked, this, &Window::loadPress);
    connect(loadGridButton, &QPushButton::clicked, this, &Window::loadGridPress);
    connect(voxButton, &QPushButton::clicked, this, &Window::voxPress);
    connect(marchButton, &QPushButton::clicked, this, &Window::marchPress);
    connect(smoothButton, &QPushButton::clicked, this, &Window::smoothPress);
    connect(defButton, &QPushButton::clicked, this, &Window::defPress);
    connect(shrinkButton, &QPushButton::clicked, this, &Window::shrinkPress);
    connect(demoButton, &QPushButton::clicked, this, &Window::demoMode);
    connect(iEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(jEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(kEdit, SIGNAL(editingFinished()), this, SLOT(lineEditChange()));
    connect(xtrslider, SIGNAL(valueChanged(int)), this, SLOT(sliderChange(int)));
    connect(ytrslider, SIGNAL(valueChanged(int)), this, SLOT(sliderChange(int)));
    connect(ztrslider, SIGNAL(valueChanged(int)), this, SLOT(sliderChange(int)));

    paramPanel->setLayout(paramLayout);
    mainLayout->addWidget(perspectiveView, 0, 1);
    mainLayout->addWidget(paramPanel, 0, 0, Qt::AlignTop);

    createActions();
    createMenus();

    mainWidget->setLayout(mainLayout);
    setWindowTitle(tr("Tesselation Viewer"));
    mainWidget->setMouseTracking(true);
    setMouseTracking(true);
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

void Window::saveFile()
{
    if(!tessfilename.isEmpty()) // save directly if we already have a file name
    {
        std::string outfile = tessfilename.toUtf8().constData();
        if(!endsWith(outfile, ".stl"))
            outfile = outfile + ".stl";
        if(!perspectiveView->getScene()->getMesh()->writeSTL(outfile)) // error message
        {
            QMessageBox msgBox;
            msgBox.setText("Unable to save mesh to file");
            msgBox.exec();
        }
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
        if(!perspectiveView->getScene()->getMesh()->writeSTL(outfile)) // error message
        {
            QMessageBox msgBox;
            msgBox.setText("Unable to save mesh to file");
            msgBox.exec();
        }
    }
}

void Window::showModel(int show)
{
    perspectiveView->setMeshVisible(show == Qt::Checked);
    repaintAllGL();
}

void Window::showLat(int show)
{
    perspectiveView->setLatVisible(show == Qt::Checked);
    repaintAllGL();
}

void Window::sliderChange(int value)
{
    if(sender() == xtrslider)
    {
        cgp::Point trs = perspectiveView->getDef()->getCP(cpi, cpj, cpk);
        trs.x = (float) value / sliderange;
        perspectiveView->getDef()->setCP(cpi, cpj, cpk, trs);
    }
    else if(sender() == ytrslider)
    {
        cgp::Point trs = perspectiveView->getDef()->getCP(cpi, cpj, cpk);
        trs.y = (float) value / sliderange;
        perspectiveView->getDef()->setCP(cpi, cpj, cpk, trs);
    }
    else if(sender() == ztrslider)
    {
        cgp::Point trs = perspectiveView->getDef()->getCP(cpi, cpj, cpk);
        trs.z = (float) value / sliderange;
        perspectiveView->getDef()->setCP(cpi, cpj, cpk, trs);
    }
    perspectiveView->setGeometryUpdate(true);
    repaintAllGL();
}
void Window::lineEditChange()
{
    bool ok;
    int val;

    if(sender() == iEdit) // selected cp along x axis, 0..numdx-1
    {
        val = iEdit->text().toInt(&ok);
        if(ok)
        {
            // change cp highlighting
            perspectiveView->getDef()->deactivateAllCP();
            cpi = val;
            perspectiveView->getDef()->activateCP(cpi, cpj, cpk);
        }
    }
    else if(sender() == jEdit) // selected cp along x axis, 0..numdx-1
    {
        val = jEdit->text().toInt(&ok);
        if(ok)
        {
            // change cp highlighting
            perspectiveView->getDef()->deactivateAllCP();
            cpj = val;
            perspectiveView->getDef()->activateCP(cpi, cpj, cpk);
        }
    }
    else if(sender() == kEdit) // selected cp along x axis, 0..numdx-1
    {
        val = kEdit->text().toInt(&ok);
        if(ok)
        {
            // change cp highlighting
            perspectiveView->getDef()->deactivateAllCP();
            cpk = val;
            perspectiveView->getDef()->activateCP(cpi, cpj, cpk);
        }
    }

    // adjust sliders to match new cp position
    cgp::Point trs = perspectiveView->getDef()->getCP(cpi, cpj, cpk);
    xtrslider->blockSignals(true); // block signals to prevent slider signalling a change in value
    xtrslider->setValue(int(std::round(trs.x * sliderange)));
    xtrslider->blockSignals(false);
    ytrslider->blockSignals(true); // block signals to prevent slider signalling a change in value
    ytrslider->setValue(int(std::round(trs.y * sliderange)));
    ytrslider->blockSignals(false);
    ztrslider->blockSignals(true); // block signals to prevent slider signalling a change in value
    ztrslider->setValue(int(std::round(trs.z * sliderange)));
    ztrslider->blockSignals(false);

    perspectiveView->setGeometryUpdate(true);
    repaintAllGL();
}

void Window::showParamOptions()
{
    paramPanel->setVisible(showParamAct->isChecked());
}

void Window::voxPress()
{
    perspectiveView->getScene()->voxelise(0.1f);
    perspectiveView->setGeometryUpdate(true);
    marchButton->setEnabled(true); // only now can marching cubes be applied
    repaintAllGL();
}

void Window::marchPress()
{
    perspectiveView->getScene()->isoextract();
    perspectiveView->setGeometryUpdate(true);
    smoothButton->setEnabled(true); // only now can smoothing be applied
    repaintAllGL();
}

void Window::smoothPress()
{
    perspectiveView->getScene()->smooth();
    perspectiveView->setGeometryUpdate(true);
    defButton->setEnabled(true); // only now can deformation be applied
    repaintAllGL();
}

void Window::defPress()
{
    perspectiveView->getScene()->deform(perspectiveView->getDef());
    perspectiveView->setGeometryUpdate(true);
    repaintAllGL();
}

void Window::loadPress()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open STL model"), "",
                                                    tr("STL Models (*.stl *.STL);;All Files (*)"));
    perspectiveView->getScene()->expensiveScene(filename.toStdString());
    perspectiveView->setGeometryUpdate(true);
    voxButton->setEnabled(true);
    marchButton->setEnabled(false);
    smoothButton->setEnabled(false);
    defButton->setEnabled(false);
    repaintAllGL();
}

void Window::loadGridPress()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open voxel grid model"), "",
                                                    tr("All files (*)"));
    perspectiveView->getScene()->voxelMeshScene(filename.toStdString());
    perspectiveView->setGeometryUpdate(true);
    voxButton->setEnabled(true);
    repaintAllGL();
}

void Window::shrinkPress()
{
    //perspectiveView->getScene()->pieceA1Scene(true);
    perspectiveView->getScene()->testShrinkScene(false);
    perspectiveView->setGeometryUpdate(true);
    repaintAllGL();
}

void Window::demoMode()
{
    /// basic demo

    // cube
    loadModelDemo("meshes/triangle/10mm_test_cube.stl", "Basic: Load cube");
    // sphere
    loadModelDemo("meshes/triangle/sphere.stl", "Basic: Load sphere");
    // hygrometer
    loadModelDemo("meshes/triangle/hygrometer.stl", "Basic: Load hygrometer");
    // barrel
    loadModelDemo("meshes/triangle/Single_Barrel.stl", "Basic: Load barrel");
    // bunny
    loadModelDemo("meshes/triangle/bunny.stl", "Basic: Load bunny");
}

void Window::loadModelDemo(string filename, string txtDisplay)
{
    perspectiveView->getScene()->loadSTLScene(filename);
    resetView();
    demoLabel->setText(txtDisplay.c_str());
    paramPanel->repaint();
    sleep(3);
}

void Window::resetView()
{
    perspectiveView->setGeometryUpdate(true);
    repaintAllGL();
}

void Window::createActions()
{
//    newAct = new QAction(tr("&New"), this);
//    newAct->setShortcuts(QKeySequence::New);
//    newAct->setStatusTip(tr("Create a new file"));
//    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

//    openAct = new QAction(tr("&Open"), this);
//    openAct->setShortcuts(QKeySequence::Open);
//    openAct->setStatusTip(tr("Open an existing file"));
//    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save a file"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

    saveAsAct = new QAction(tr("Save as"), this);
    saveAsAct->setStatusTip(tr("Save a file under name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    showParamAct = new QAction(tr("Show Parameters"), this);
    showParamAct->setCheckable(true);
    showParamAct->setChecked(true);
    showParamAct->setStatusTip(tr("Hide/Show Parameters"));
    connect(showParamAct, SIGNAL(triggered()), this, SLOT(showParamOptions()));
}

void Window::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    // fileMenu->addAction(newAct);
    // fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(showParamAct);
}
