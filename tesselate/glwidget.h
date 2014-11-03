// GLWidget class for managing rendering
// Originally built from Qt Toolkit example
// (c) James Gain, 2014

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "glheaders.h" // Must be included before QT opengl headers
#include <QGLWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPushButton>
#include <list>
#include <common/debug_vector.h>
#include <common/debug_list.h>

#include "view.h"
#include "scene.h"
#include "renderer.h"

//! [0]
using namespace std;

class Window;

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:

    GLWidget(const QGLFormat& format, QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    /// getter for currently active view
    View * getView(){ return &view; }

    /// getter for renderer
    Renderer * getRenderer(){ return renderer; }

    /// getter for scene geometry
    Scene * getScene(){ return &scene; }

    /// getter for intersection shape
    Mesh * getXSect(){ return &xsect; }

    /// setter for geometry updating
    void setGeometryUpdate(bool update){ updateGeometry = update; }

    /// setter for drawing intersection mesh
    void setMeshVisible(bool vis){ meshVisible = vis; setGeometryUpdate(true); }

    /// interesect mesh with scene
    void intersect(){  scene.packCubesInMesh(&xsect); setMeshVisible(false); repaint(); }

    /// Respond to key press events
    void keyPressEvent(QKeyEvent *event);

signals:
    void signalRepaintAllGL();
    
public slots:
    void animUpdate(); // animation step for change of focus
    void rotateUpdate(); // animation step for rotating around terrain center

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent * wheel);

private:

    // scene control
    Scene scene;                        ///< tesselation mesh
    Mesh xsect;                         ///< intersection mesh
    View view;
    vector<ShapeDrawData> drawParams;
    bool updateGeometry;                ///< recreate render buffers on change
    bool meshVisible;                   ///< render intersection mesh

    // render variables
    Renderer * renderer;

    // gui variables
    bool viewing;
    bool glewSetupDone;

    QPoint lastPos;
    QColor qtWhite;
    QTimer * atimer, * rtimer; // timers to control different types of animation
};

#endif
