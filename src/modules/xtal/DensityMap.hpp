// -*-Mode: C++;-*-
//
//  Electron density map object (with 8bit precision)
//

#ifndef DENSITY_MAP_HPP_INCLUDED_
#define DENSITY_MAP_HPP_INCLUDED_

#include "xtal.hpp"

#include <qsys/ScalarObject.hpp>

#include <modules/symm/CrystalInfo.hpp>
#include <qlib/ByteMap.hpp>
#include <qlib/LDOM2Stream.hpp>

#define MAP_FLOAT_MIN (-1e10)
#define MAP_FLOAT_MAX (1e10)

namespace molstr {
  class MolCoord;
}

namespace xtal {

using symm::CrystalInfo;
using qlib::Vector4D;
  
///
///  Density map object for display.
///  The density data are stored with 8bit precision.
///  This object is not suitable for analytical purpose.
///
class XTAL_API DensityMap : public qsys::ScalarObject
{
  MC_SCRIPTABLE;

private:
 
  /// cell dimensions
  CrystalInfo m_xtalInfo;
  
  /// Numbers of grid points in the unit cell
  int m_nColInt;
  int m_nRowInt;
  int m_nSecInt;

  /// number of columns, rows, sections of this map
  int m_nCols;
  int m_nRows;
  int m_nSecs;

  /// number of first col, row, sec of this map
  int m_nStartCol;
  int m_nStartRow;
  int m_nStartSec;

  double m_dMinMap;
  double m_dMaxMap;
  double m_dMeanMap;
  double m_dRmsdMap;

  /// truncated map (8bit)
  qlib::ByteMap *m_pByteMap;
  double m_dLevelBase;
  double m_dLevelStep;

  ///////////////////////////////////////////////

public:
  /// default constructor
  DensityMap();

  /// destructor
  virtual ~DensityMap();

  ///////////////////////////////////////////////
  // Object interface

  // virtual qobj_inst *createInterpObj() const;
  // virtual bool isEmpty() const;
  // virtual void dump() const;

  ///////////////////////////////////////////////
  // ScalarObject interface

  virtual double getValueAt(const Vector4D &pos) const;
  virtual unsigned char atByte(int i, int j, int k) const;
  virtual double atFloat(int i, int j, int k) const;

  virtual bool isInRange(const Vector4D &pos) const;
  virtual bool isInBoundary(int i, int j, int k) const;

  virtual Vector4D getCenter() const;
  virtual Vector4D getOrigin() const;

  virtual double getRmsdDensity() const;
  virtual double getMinDensity() const { return m_dMinMap; }
  virtual double getMaxDensity() const { return m_dMaxMap; }
  virtual double getMeanDensity() const { return m_dMeanMap; }

  virtual double getLevelBase() const;
  virtual double getLevelStep() const;

  // get number of columns, rows, sections
  virtual int getColNo() const { return m_nCols; }
  virtual int getRowNo() const { return m_nRows; }
  virtual int getSecNo() const { return m_nSecs; }

  virtual int getStartCol() const { return m_nStartCol; }
  virtual int getStartRow() const { return m_nStartRow; }
  virtual int getStartSec() const { return m_nStartSec; }

  virtual double getColGridSize() const;
  virtual double getRowGridSize() const;
  virtual double getSecGridSize() const;

  virtual Vector4D convToOrth(const Vector4D &index) const;

  ///////////////////////////////////////////////
  // setup density map

  /// construct by float array
  /// axcol, ... specifiy the axis-order permutation
  void setMapFloatArray(const float *array,
			int ncol, int nrow, int nsect,
			int axcol, int axrow, int axsect);

  /// construct by byte array
  /// array must be sorted by the Fast-Medium-Slow order
  void setMapByteArray(const unsigned char*array,
                       int ncol, int nrow, int nsect,
                       double rhomin, double rhomax,
                       double mean, double sigma);
    
  /// setup column, row, section params
  void setMapParams(int stacol, int starow, int stasect,
		    int intcol, int introw, int intsect);

  /// setup crystal system's parameters
  void setXtalParams(double a, double b, double c,
                     double alpha, double beta, double gamma,
                     int nsg = 1);

  /// Histogram generation in JSON format
  // (can be moved to ScalarObj level??)
  LString getNormHistogramJSON();

  ///////////////////////////////////////////////////////////////
  // Get/set map properties.
  // Each Col, Row, Sec axis correspnds to X,Y,Z axis.

  int getColInterval() const { return m_nColInt; }
  int getRowInterval() const { return m_nRowInt; }
  int getSecInterval() const { return m_nSecInt; }

  const CrystalInfo &getXtalInfo() const { return m_xtalInfo; }

  ////////////////////////////////////////////
  // Data chunk serialization

  virtual bool isDataSrcWritable() const { return true; }
  virtual LString getDataChunkReaderName(int nQdfVer) const;
  virtual void writeDataChunkTo(qlib::LDom2OutStream &oos) const;

  ///////////////////////////////////////////////////////////////

private:
  /// helper method
  static void rotate(int &e0, int &e1, int &e2,
		     int ax0, int ax1, int ax2) {
    int r[3];
    r[ax0] = e0;
    r[ax1] = e1;
    r[ax2] = e2;
    e0 = r[0];
    e1 = r[1];
    e2 = r[2];
  }

  unsigned char getAtWithBndry(int nx, int ny, int nz) const;

};

} // namespace xtal

#endif

