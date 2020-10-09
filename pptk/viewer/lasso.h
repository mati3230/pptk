#ifndef __LASSO_H__
#define __LASSO_H__
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QPointF>
#include <QWindow>
#include "opengl_funcs.h"

class Lasso : protected OpenGLFuncs {
 public:
  enum SelectMode { ADD = 0, SUB = 1, NONE = 2 };

  Lasso(QWindow* window, QOpenGLContext* context)
      : _context(context), _window(window), _select_mode(NONE) {
    _context->makeCurrent(_window);
    initializeOpenGLFunctions();
    _context->doneCurrent();
    compileProgram();
    resetBounds();
  }

  void draw() {
    if (_select_mode == NONE) return;
    _program.bind();
    glBegin(GL_LINE_LOOP);
        for (QVector<QPointF>::iterator i = _path.begin(); i != _path.end(); ++i)
            glVertex2f(i->x(), i->y());
    glEnd();
  }

  void click(QPointF p, SelectMode select_mode) {
    _select_mode = select_mode;
    // std::cout << "select mode: " <<  _select_mode << std::endl;
    _path.append(p);
  }

  void drag(QPointF p) {
    _path.append(p);
    updateBounds(p);
    // std::cout << "bounds: (" << _bounds.x() << ", " << _bounds.y() << ", " << _bounds.z() << ", " << _bounds.w() << ")" << std::endl;
  }

  void release() {
    _select_mode = NONE;
    _path.clear();
    resetBounds();
  }

  bool active() const { return _select_mode != NONE; }

  bool empty() const { return _path.isEmpty(); }

  bool contains(QPointF p){
      bool result = inBounds(p);
      // std::cout << "p: (" << p.x() << ", " << p.y() << "), in bounds: " << result << std::endl;
      if (!result)
        return false;
      QPolygonF polygon(_path);
      result = polygon.containsPoint(p, Qt::OddEvenFill);
      // std::cout << "p: (" << p.x() << ", " << p.y() << "), in polygon: " << result << std::endl;
      return result;
  }

  const QVector<QPointF>& getPath() const { return _path; }

  SelectMode getType() const { return _select_mode; }

  QVector4D getBounds() const { return _bounds; }

 private:

  void compileProgram() {

    std::string fsCode =
          "void main()\n"
          "{\n"
          "   gl_FragColor = vec4(1, 1, 0, 1);\n"
          "}";
    _context->makeCurrent(_window);
    _program.addShaderFromSourceCode(QOpenGLShader::Fragment, fsCode.c_str());
    _program.link();
    _context->doneCurrent();
  }

  void updateBounds(QPointF p){
      if(p.x() < _bounds.x()) // x < x_min
        _bounds.setX(p.x());
      else if(p.x() > _bounds.y()) // x > x_max
        _bounds.setY(p.x());

      if(p.y() < _bounds.z()) // y < y_min
        _bounds.setZ(p.y());
      else if(p.y() > _bounds.w()) // y > y_max
        _bounds.setW(p.y());
  }

  bool inBounds(QPointF p){
      // is point in bounds that are formed by lasso?
      if (p.x() < _bounds.x()) // x < x_min
        return false;
      else if (p.x() > _bounds.y()) // x > x_max
        return false;
      if (p.y() < _bounds.z()) // y < y_min
        return false;
      else if (p.y() > _bounds.w()) // y > y_max
        return false;
      return true;
  }

  void resetBounds(){
    _bounds.setX(1);  // x_min - set max start value
    _bounds.setY(-1); // x_max - set min start value
    _bounds.setZ(1);  // y_min
    _bounds.setW(-1); // y_max
  }

  QOpenGLContext* _context;
  QWindow* _window;
  QOpenGLShaderProgram _program;

  SelectMode _select_mode;
  QVector<QPointF> _path;
  // use x = x_min, y = x_max, z = y_min, w = y_max
  QVector4D _bounds;
};

#endif  // __LASSO_H__
