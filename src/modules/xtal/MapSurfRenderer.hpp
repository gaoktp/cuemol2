// -*-Mode: C++;-*-
//
// Generate/Render the contour surface of ScalarObject
//

#ifndef XTAL_MAP_SURF_RENDERER_HPP_INCLUDED
#define XTAL_MAP_SURF_RENDERER_HPP_INCLUDED

#include "xtal.hpp"
#include "MapRenderer.hpp"

#include <qlib/ByteMap.hpp>
#include <qsys/ScalarObject.hpp>
#include <qsys/ViewEvent.hpp>
#include <modules/molstr/molstr.hpp>
#include <modules/molstr/BSPTree.hpp>

#include <modules/surface/MolSurfObj.hpp>

class MapSurfRenderer_wrap;

namespace xtal {

  using gfx::DisplayContext;
  using qsys::ScalarObject;
  using molstr::SelectionPtr;
  using molstr::MolCoordPtr;
  using molstr::BSPTree;
  class DensityMap;

  class MapSurfRenderer : public MapRenderer,
                          public qsys::ViewEventListener
  {
    MC_SCRIPTABLE;
    MC_CLONEABLE;

  private:
    typedef MapRenderer super_t;
    friend class ::MapSurfRenderer_wrap;

    ///////////////////////////////////////////
    // properties

    /// Periodic boundary flag
    /// (default: false; set true, if map contains the entire of unit cell)
    bool m_bPBC;

    /// Automatically update the map center as view center
    /// (default: true)
    bool m_bAutoUpdate;

    /// Automatically update the map center as view center
    /// in both mouse-drag and mouse-up events
    /// (default: false)
    bool m_bDragUpdate;

  public:
    enum {
      MSRDRAW_FILL = 0,
      MSRDRAW_LINE = 1,
      MSRDRAW_POINT = 2,
    };
    
  private:
    /// Mesh-drawing mode
    int m_nDrawMode;

    /// Line width (used in LINE/POINT mode)
    double m_lw;

  public:
    int getDrawMode() const { return m_nDrawMode; }
    void setDrawMode(int n) {
      m_nDrawMode = n;
      invalidateDisplayCache();
    }
    
    void setLineWidth(double f) {
      m_lw = f;
      invalidateDisplayCache();
    }
    double getLineWidth() const { return m_lw; }
    

  private:
    /// cull face
    bool m_bCullFace;

  public:
    bool isCullFace() const { return m_bCullFace; }
    void setCullFace(bool b) {
      m_bCullFace = b;
      invalidateDisplayCache();
    }
    

  private:

    ///////////////////////////////////////////
    // work area

    /// size of map (copy from m_pMap)
    int m_nMapColNo, m_nMapRowNo, m_nMapSecNo;

    /// size of section array
    int m_nActCol, m_nActRow, m_nActSec;
    int m_nStCol, m_nStRow, m_nStSec;

    /// contour level (not a property)
    double m_dLevel;

    /// for debug
    std::deque<Vector4D> m_tmpv;
    
  public:

    ///////////////////////////////////////////
    // constructors / destructor

    /// default constructor
    MapSurfRenderer();

    /// destructor
    virtual ~MapSurfRenderer();

    ///////////////////////////////////////////

    virtual const char *getTypeName() const;

    //virtual void attachObj(qlib::uid_t obj_uid);
    virtual void setSceneID(qlib::uid_t nid);

    virtual qlib::uid_t detachObj();

    ///////////////////////////////////////////

    virtual void render(DisplayContext *pdl);
    virtual void preRender(DisplayContext *pdc);
    virtual void postRender(DisplayContext *pdc);

    // virtual bool isTransp() const { return true; }

    ///////////////////////////////////////////////////////////////

    virtual void viewChanged(qsys::ViewEvent &);

  protected:
    // We must override firePropertyChanged() to avoid destructing the display list,
    // when only the color was changed.
    // virtual void firePropertyChanged(qlib::PropChgEvent &ev);

  private:

    // cached ptr of target obj
    ScalarObject *m_pCMap;

    void makerange();

    void renderImpl(DisplayContext *pdl);

    void marchCube(DisplayContext *pdl, int fx, int fy, int fz, double *values, bool *bary);

    double getOffset(double fValue1, double fValue2, double fValueDesired);
    void getVertexColor(Vector4D &rfColor, Vector4D &rfPosition, Vector4D &rfNormal);
    Vector4D getNormal(const Vector4D &rfNormal,bool,bool,bool);

    inline double getDen(int x, int y, int z) const
    {
      // TO DO: support symop

      if (m_bPBC) {
        const int xx = (x+10000*m_nMapColNo)%m_nMapColNo;
        const int yy = (y+10000*m_nMapRowNo)%m_nMapRowNo;
        const int zz = (z+10000*m_nMapSecNo)%m_nMapSecNo;
        // return pMap->atByte(xx,yy,zz);
        return m_pCMap->atFloat(xx, yy, zz);
      }
      else {
        if (x<0||y<0||z<0)
          return 0.0;
        if (x>=m_nMapColNo||
            y>=m_nMapRowNo||
            z>=m_nMapSecNo)
          return 0.0;
        return m_pCMap->atFloat(x, y, z);
      }
      
    }

    Vector4D getGrdNorm(int ix, int iy, int iz);

  private:
    std::deque<surface::MSVert> m_msverts;
    Matrix4D m_xform;

    void addMSVert(const Vector4D &v, const Vector4D &n)
    {
      Vector4D vv(v);
      vv.w() = 1.0;
      m_xform.xform4D(vv);

      Vector4D nn(n);
      nn.w() = 0.0;
      m_xform.xform4D(nn);

      m_msverts.push_back( surface::MSVert(vv, nn) );
    }

  public:    
    qsys::ObjectPtr generateSurfObj();

  };

}

#endif
