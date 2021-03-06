// -*-Mode: C++;-*-
//
// TTY dummy View implementation
//

#include <common.h>

#include <qsys/qsys.hpp>
#include <qsys/View.hpp>
#include <qsys/Scene.hpp>
#include <gfx/DisplayContext.hpp>

namespace {

  class TTYDisplayContext : public gfx::DisplayContext
  {
  private:
    typedef gfx::DisplayContext super_t;

  public:
    TTYDisplayContext() {}
    virtual ~TTYDisplayContext() {}

    virtual bool setCurrent() { return true; }
    virtual bool isCurrent() const { return true; }
    virtual bool isFile() const { return true; }

    virtual void vertex(const qlib::Vector4D &) {}
    virtual void normal(const qlib::Vector4D &) {}
    virtual void color(const gfx::ColorPtr &c) {}

    virtual void pushMatrix() {}
    virtual void popMatrix() {}
    virtual void multMatrix(const qlib::Matrix4D &mat) {}
    virtual void loadMatrix(const qlib::Matrix4D &mat) {}

    virtual void setPolygonMode(int id) {}
    virtual void startPoints() {}
    virtual void startPolygon() {}
    virtual void startLines() {}
    virtual void startLineStrip() {}
    virtual void startTriangles() {}
    virtual void startTriangleStrip() {}
    virtual void startTriangleFan() {}
    virtual void startQuadStrip() {}
    virtual void startQuads() {}
    virtual void end() {}

  };
  

  class TTYView : public qsys::View
  {
  private:
    TTYDisplayContext *m_pCtxt;
  public:

    TTYView() : m_pCtxt(new TTYDisplayContext()) {}

    TTYView(const TTYView &r) {}

    virtual ~TTYView() {}
  
    //////////
  
  public:
    virtual LString toString() const { return LString("TTYView"); }

    /// Setup the projection matrix for stereo (View interface)
    virtual void setUpModelMat(int nid) {}
    
    /// Setup projection matrix (View interface)
    virtual void setUpProjMat(int w, int h) {}
    
    /// Draw current scene
    virtual void drawScene()
    {
      qsys::ScenePtr pScene = getScene();
      if (pScene.isnull()) {
	MB_DPRINTLN("DrawScene: invalid scene %d !!", getSceneID());
	return;
      }
      
      gfx::DisplayContext *pdc = getDisplayContext();
      pdc->setCurrent();
      pScene->display(pdc);
    }
    
    virtual gfx::DisplayContext *getDisplayContext() { return m_pCtxt; }

  };

}

namespace qsys {
  //static
  qsys::View *View::createView()
  {
    qsys::View *pret = MB_NEW TTYView();
    MB_DPRINTLN("TTYView created (%p, ID=%d)", pret, pret->getUID());
    return pret;
  }
}
