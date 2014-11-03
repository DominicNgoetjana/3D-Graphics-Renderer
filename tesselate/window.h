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

#ifndef WINDOW_H
#define WINDOW_H

#include "glwidget.h"
#include <QWidget>
#include <QtWidgets>
#include <string>

class QAction;
class QMenu;
class QLineEdit;

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window();

    ~Window(){};

    QSize sizeHint() const;

    /// Getter for currently active camera
    View &getCamera() { return *perspectiveView->getView(); }
    const View &getCamera() const { return *perspectiveView->getView(); }

    /// Getter for perspective view
    GLWidget * getGL() { return perspectiveView; }

public slots:

    /// force re-rendering of scene
    void repaintAllGL();

    /// new file menu item
    void newFile();

    /// open intersection file menu item
    void open();

    /// save tesselation file menu item
    void saveFile();

    /// save tesselation file under user-inputted name
    void saveAs();

    /// intersect mesh and scene
    void xsectPress();

    /// make parameter panel visible
    void showParamOptions();

    /// deal with user changing parameter values
    void lineEditChange();

protected:
    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void optionsChanged();

private:
    GLWidget * perspectiveView; ///< openGL render view
    QWidget * paramPanel;       ///< side panel for user access to parameters

    // param panel sub components
    QLineEdit * radSphereEdit, * radCylEdit, * lenEdit, * widthEdit, * hghtEdit, * sideEdit; ///< text entry fields for parameters

    // menu widgets and actions
    QMenu *fileMenu;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QMenu *viewMenu;
    QAction *showParamAct;

    QString tessfilename; ///< name of tesselation file for output

    /// init menu
    void createActions();
    void createMenus();
};

#endif
