//
// Molecular model coordinates class (geometry related implementations)
//
// $Id: MolCoordGeomImpl.cpp,v 1.6 2011/04/16 14:32:28 rishitani Exp $

#include <common.h>

#include "MolCoord.hpp"

#include "MolChain.hpp"
#include "MolResidue.hpp"
#include "MolAtom.hpp"
#include "AtomIterator.hpp"
#include "ResidIterator.hpp"
#include "MolXformEditInfo.hpp"
#include "TopparManager.hpp"
#include "TopoBuilder.hpp"

#include <qlib/Box3D.hpp>
#include <qsys/View.hpp>
#include <qsys/Scene.hpp>
#include <qsys/ObjectEvent.hpp>
#include <qsys/UndoManager.hpp>

using namespace molstr;

using qlib::Vector4D;
using qlib::Matrix4D;
using qlib::Box3D;

using qsys::UndoManager;

qlib::Vector4D MolCoord::getCenterPos(bool fselect) const
{
  qlib::Vector4D pos;
  int natom=0;

  SelectionPtr psel;
  if (fselect)
    psel = getSelection();

  //
  // calculate center of selected atoms
  //
  // lock();
  {
    AtomIterator iter(MolCoordPtr(const_cast<MolCoord *>(this)), psel);
    for (iter.first(); iter.hasMore(); iter.next()) {
      MolAtomPtr pAtom = iter.get();
      MB_ASSERT(!pAtom.isnull());
      pos += pAtom->getPos();
      natom++;
    }
  }
  //unlock();
  
  if (natom>0)
    pos /= natom;
  else
    pos = qlib::Vector4D(0,0,0);
  
  return pos;
}

qlib::Vector4D MolCoord::getBoundBoxMin(bool fselect) const
{
  qlib::Box3D pos;
  int natom=0;

  SelectionPtr psel;
  if (fselect)
    psel = getSelection();

  //
  // calculate center of selected atoms
  //
  // lock();
  {
    AtomIterator iter(MolCoordPtr(const_cast<MolCoord *>(this)), psel);
    for (iter.first(); iter.hasMore(); iter.next()) {
      MolAtomPtr pAtom = iter.get();
      MB_ASSERT(!pAtom.isnull());
      pos.merge(pAtom->getPos());
    }
  }
  //unlock();
  
  return pos.vstart;
}

qlib::Vector4D MolCoord::getBoundBoxMax(bool fselect) const
{
  qlib::Box3D pos;
  int natom=0;

  SelectionPtr psel;
  if (fselect)
    psel = getSelection();

  //
  // calculate center of selected atoms
  //
  // lock();
  {
    AtomIterator iter(MolCoordPtr(const_cast<MolCoord *>(this)), psel);
    for (iter.first(); iter.hasMore(); iter.next()) {
      MolAtomPtr pAtom = iter.get();
      MB_ASSERT(!pAtom.isnull());
      pos.merge(pAtom->getPos());
    }
  }
  //unlock();
  
  return pos.vend;
}

void MolCoord::fitView(bool fselect, qsys::ViewPtr pView) const
{
  qlib::LQuat rotq = pView->getRotQuat();
  Matrix4D rmat = Matrix4D::makeRotMat(rotq);
  Matrix4D invmat = rmat.invert();
  //Matrix4D invmat = Matrix4D::makeRotMat(rotq.inv());

  int natom=0;
  Box3D bbox;

  SelectionPtr psel;
  if (fselect)
    psel = getSelection();

  {
    AtomIterator iter(MolCoordPtr(const_cast<MolCoord *>(this)), psel);
    for (iter.first(); iter.hasMore(); iter.next()) {
      MolAtomPtr pAtom = iter.get();
      MB_ASSERT(!pAtom.isnull());

      Vector4D vpos = rmat.mulvec(pAtom->getPos());
      bbox.merge(vpos);
    }
  }
  
  {
    Vector4D dv = (bbox.vend - bbox.vstart).scale(0.2);
    bbox.vend += dv;
    bbox.vstart -= dv;
  }

  Vector4D cen = invmat.mulvec(bbox.center());
  pView->setViewCenter(cen);

  int cx = pView->getWidth();
  int cy = pView->getHeight();
  double fasp = double(cx)/double(cy);
  double mx = (bbox.vend.x()-bbox.vstart.x());
  double my = (bbox.vend.y()-bbox.vstart.y());
  double masp = mx / my;

  // MB_DPRINTLN("mx: %f", mx);
  // MB_DPRINTLN("my: %f", my);
  // MB_DPRINTLN("fasp: %f", fasp);
  // MB_DPRINTLN("masp: %f", masp);

  double zoom;
  if (fasp>1.0) {
    if (masp>fasp) {
      zoom = mx/fasp;
    }
    else {
      zoom = my;
    }
  }
  else {
    if (masp>fasp) {
      zoom = mx/fasp;
    }
    else {
      zoom = my;
    }
  }

  // MB_DPRINTLN("Zoom: %f", zoom);
  pView->setZoom(zoom);
  pView->setSlabDepth(bbox.vend.z()-bbox.vstart.z());
}

void MolCoord::fitView2(SelectionPtr pSel, qsys::ViewPtr pView) const
{
  MolCoord *pthis = const_cast<MolCoord *>(this);
  SelectionPtr pOldSel = getSelection();
  pthis->setSelection(pSel);
  fitView(true, pView);
  pthis->setSelection(pOldSel);
}

void MolCoord::xformByMat(const Matrix4D &mat, SelectionPtr pSel)
{
  MolCoordPtr pthis = MolCoordPtr(this);

  // Record undo info
  MolXformEditInfo *pPEI = NULL;
  UndoManager *pUM = NULL;
  qsys::ScenePtr cursc = getScene();
  if (!cursc.isnull()) {
    pUM = cursc->getUndoMgr();
    if (pUM->isOK()) {
      // record property changed undo/redo info
      pPEI = MB_NEW MolXformEditInfo();
      pPEI->saveBeforePos(pthis, pSel);
    }
  }
  
  ///////////////////

  AtomIterator iter(pthis, pSel);
  Vector4D pos;

  for (iter.first(); iter.hasMore(); iter.next()) {
    MolAtomPtr pAtom = iter.get();
    pos = pAtom->getPos();
    pos.w() = 1.0;
    pos = mat.mulvec(pos);
    pos.w() = 0.0;
    pAtom->setPos(pos);
  }

  ///////////////////

  // Record redo info
  if (pPEI!=NULL) {
    MB_ASSERT(pUM!=NULL);
    MB_ASSERT(pUM->isOK());
    // record property changed undo/redo info
    pPEI->saveAfterPos(pthis, pSel);
    pUM->addEditInfo(pPEI);
  }
  
  setModifiedFlag(true);
}

void MolCoord::fireAtomsMoved()
{
  // notify update of structure
  qsys::ObjectEvent obe;
  obe.setType(qsys::ObjectEvent::OBE_CHANGED);
  obe.setTarget(getUID());
  obe.setDescr("atomsMoved");
  fireObjectEvent(obe);
}

void MolCoord::fireTopologyChanged()
{
  // notify update of structure
  qsys::ObjectEvent obe;
  obe.setType(qsys::ObjectEvent::OBE_CHANGED);
  obe.setTarget(getUID());
  obe.setDescr("topologyChanged");
  fireObjectEvent(obe);
}

////////////////////////////////////////////////

void MolCoord::applyTopology(bool bAutoBuild/*=true*/)
{
  MolCoordPtr pmol(this);
  TopparManager *pTM = TopparManager::getInstance();
  TopoDB *pTopoDB = pTM->getTopoDB();
  if (pTopoDB!=NULL) {
    TopoBuilder tb(pTopoDB);
    tb.attachMol(pmol);
    if (!bAutoBuild)
      tb.setAutogenMode(TopoBuilder::AUTOGEN_NONE);
    tb.applyTopology();
  }
}

/// copy all props of pRes2 to pRes
static void copyResProps(MolResiduePtr pRes2, MolResiduePtr pRes)
{
  std::set<LString> names;
  pRes2->getResPropNames(names);

  BOOST_FOREACH (const LString &nm, names) {
    LString value;
    if (!pRes2->getPropStr(nm, value))
      continue;
    pRes->setPropStr(nm, value);
  }
}

///  Copy the selected part of pmol2 into this mol
///  Copy will fail and return false, if there is name collision (in chain, res, atom).
bool MolCoord::copyAtoms(MolCoordPtr pmol2, SelectionPtr psel2)
{
  // if (psel2.isnull())
  // return true;

  // check name collision
  AtomIterator iter(pmol2, psel2);
  std::set<int> atmset;
  for (iter.first(); iter.hasMore(); iter.next()) {
    int atomid = iter.getID();
    MolAtomPtr pAtom = pmol2->getAtom(atomid);
    MB_ASSERT(!pAtom.isnull());
    const LString &aname = pAtom->getName();
    ResidIndex nresid = pAtom->getResIndex();
    const LString &cname = pAtom->getChainName();
    if (!getResidue(cname, nresid).isnull())
      return false;
    if (!getAtom(cname, nresid, aname).isnull())
      return false;
    atmset.insert(atomid);
  }

  // copy
  {
    std::set<int>::const_iterator it2 = atmset.begin();
    std::set<int>::const_iterator end = atmset.end();
    for (; it2!=end; it2++) {
      int atomid = *it2;
      MolAtomPtr pAtom = pmol2->getAtom(atomid);
      //MB_ASSERT(pAtom!=NULL);
      if (pAtom.isnull()) continue;
      
      MolAtomPtr pNewAtom(pAtom->clone_cast<MolAtom>());
      
      // This should not fail.
      int res = appendAtom(pNewAtom);
      MB_ASSERT(res>=0);
    }
  }
  
  applyTopology();

  // Copy residue property
  // TO DO: copy once per residue (skip already copied residues)
  {
    // std::set<MolResiduePtr> copiedSet;
    std::set<int>::const_iterator it2 = atmset.begin();
    std::set<int>::const_iterator end = atmset.end();
    for (; it2!=end; it2++) {
      int atomid = *it2;
      MolAtomPtr pAtom = pmol2->getAtom(atomid);
      if (pAtom.isnull()) continue;
      
      MolResiduePtr pRes2 = pAtom->getParentResidue();
      //if (copiedSet.find(pRes2)!=copiedSet.end())
      //continue;

      const LString &cname = pRes2->getChainName();
      ResidIndex nresid = pRes2->getIndex();
      
      MolResiduePtr pRes = getResidue(cname, nresid);
      if (pRes.isnull()) {
        LOG_DPRINTLN("copy resid prop failed");
        continue;
      }
      
      copyResProps(pRes2, pRes);
      //copiedSet.insert(pRes2);
    }
  }

  return true;
}


///  Delete the selected part of this mol (inv. op. of copy())
bool MolCoord::deleteAtoms(SelectionPtr psel)
{
  MolCoordPtr pmol(this);

  // Collect atom IDs to remove
  //  (We cannot remove atom safely, with using its iterator!)
  AtomIterator iter(pmol, psel);
  std::set<int> atmset;
  for (iter.first(); iter.hasMore(); iter.next())
    atmset.insert(iter.getID());

  // remove the selected atoms
  std::set<int>::const_iterator it2 = atmset.begin();
  std::set<int>::const_iterator end = atmset.end();
  for (; it2!=end; it2++) {
    int atomid = *it2;
    // MolAtomPtr pAtom = getAtom(atomid);
    // MB_ASSERT(!pAtom.isnull());
    removeAtom(atomid);
    //delete pAtom;
  }

  return true;
}

////////////////////////////////////////////////

LString MolCoord::getChainNameCandsJSON() const
{
  LString rval = "[";
  
  ChainIter ci = begin();
  ChainIter cie = end();
  bool f=false;
  for (; ci!=cie; ++ci) {
    if (f)
      rval += ",";

    f = true;
    rval += LString('"') + ci->first + LString('"');
  }

  rval += "]";
  return rval;
}

static
LString makeNameSet(const std::set<LString> &nameset) {
  LString rval = "[";
  
  std::set<LString>::const_iterator ii = nameset.begin();
  std::set<LString>::const_iterator ie = nameset.end();
  bool f=false;
  for (; ii!=ie; ++ii) {
    if (f)
      rval += ",";

    f = true;
    rval += LString('"') + (*ii) + LString('"');
  }
  

  rval += "]";
  return rval;
}

LString MolCoord::getResidNameCandsJSON() const
{
  std::set<LString> nameset;

  ResidIterator ri(MolCoordPtr(const_cast<MolCoord *>(this)));
  for (ri.first(); ri.hasMore(); ri.next()) {
    nameset.insert(ri.get()->getName());
  }

  return makeNameSet(nameset);
}

LString MolCoord::getAtomNameCandsJSON() const
{
  std::set<LString> nameset;

  AtomIter ai = beginAtom();
  AtomIter ae = endAtom();
  for (; ai!=ae; ++ai) {
    nameset.insert(ai->second->getName());
  }

  return makeNameSet(nameset);
}

LString MolCoord::getElemNameCandsJSON() const
{
  std::set<LString> nameset;

  AtomIter ai = beginAtom();
  AtomIter ae = endAtom();
  for (; ai!=ae; ++ai) {
    nameset.insert(ai->second->getElementName());
  }

  return makeNameSet(nameset);
}

#if 0

double MolCoord::getDistMin(const Vector3D &pos)
{
  // calculate center of selected atoms
  AtomIterator iter(this, getSelect());

  double dist2=1.0E100, diff;
  bool fOK = false;

  lock(); {
    for (iter.first(); iter.hasMore(); iter.next()) {
      MolAtom *pAtom = iter.get();
      diff = (pAtom->pos - pos).sqlen();
      if (diff<dist2) {
        dist2 = diff;
        fOK = true;
      }
    }
  } unlock();

  if (!fOK) {
    // TO DO: throw exception
    return 0.0;
  }

  return ::sqrt(dist2);
}
void MolCoord::transf(const Matrix4D &mat, SelectionPtr psel)
{
  if (psel.isnull())
    return;
   
  Vector3D vtmp;

  lock(); {
    AtomIterator iter(this, psel);
    for (iter.first(); iter.hasMore(); iter.next()) {
      MolAtom *pAtom = iter.get();
      vtmp = pAtom->pos;
      mat.xform(vtmp);
      pAtom->pos = vtmp;
    }
  } unlock();

  return;
}

#endif

#if 0
/**
  move the selected part of pmol2 into this mol.
 */
bool MolCoord::merge(MolCoord *pmol2)
{
  SelectionPtr psel2 = pmol2->getSelect();
  if (psel2.isnull())
    return true;
  pmol2->popSelect();

  //
  // check overlapping
  //
  AtomIterator iter(pmol2, psel2);
  set<int> atmset;
  for (iter.first(); iter.hasMore(); iter.next()) {
    int atomid = iter.getID();
    MolAtom *pAtom = pmol2->getAtom(atomid);
    MB_ASSERT(pAtom!=NULL);
    const LString &aname = pAtom->getName();
    int nresid = pAtom->getResIndex();
    const LString &cname = pAtom->getChainName();
    if (getAtom(cname, nresid, aname)!=NULL)
      return false;
    atmset.insert(atomid);
  }

  //
  // merge
  //
  set<int>::iterator it2 = atmset.begin();
  for (; it2!=atmset.end(); it2++) {
    int atomid = *it2;
    MolAtom *pAtom = pmol2->getAtom(atomid);
    //MB_ASSERT(pAtom!=NULL);
    if (pAtom==NULL) continue;

    pmol2->removeAtom(atomid);
    appendAtom(pAtom);
  }

  // select the incorporated atoms
  pushSelect(psel2);

  return true;
}

//////////////////////////////////////////////////

#include <qlib/BinStream.hpp>
#include <qlib/StringStream.hpp>

void MolCoord::writeObj(qlib::ObjOutStream &dos) const
{
  MbObject::writeObj(dos);
  MolCoord *pthis = (MolCoord *)this;

  lock(); {
    qlib::StrOutStream sos;
    qlib::BinOutStream bos(sos);
    // write all atoms
    bos.write<int>(m_atomPool.size());
    AtomPool::const_iterator iter = beginAtom();
    int i=0;
    for ( ; iter!=endAtom(); iter++, i++) {
      const MolAtom *p = (*iter).second;
      p->writeBin(bos);
    }
    bos.close();
    sos.close();

    int nsize;
    char *parray = sos.getData(nsize);
    dos.writeData("szatom", nsize);
    dos.writeArray("atoms", parray, nsize);
    if (nsize>0)
      delete [] parray;
  } unlock();
  
  // write selection stack
  m_lockSelstk.lock(); {
    //dos.writeData("selstk", m_selstk);

    SelStack::const_iterator pos = m_selstk.begin();
    int i=0, n=m_selstk.size();
    dos.writeData("size", n);
    for (; pos!=m_selstk.end(); ++pos, ++i) {
      const SelectionPtr value = *pos;
      LString propname = LString::format("[%d]", i);
      dos.writeObj(propname, value.get());
    }

  } m_lockSelstk.unlock();
  
  {
    // write residue properties
    ResidIterator iter(pthis);
    for (iter.first(); iter.hasMore(); iter.next()) {
      MolResidue *pRes = iter.get();
      MB_ASSERT(pRes!=NULL);
      LString key = LString::format("resid %s %d",
                                    (const char *)pRes->getChainName(),
                                    pRes->getIndex());
      dos.writeData(key, *pRes);
    }
  }

}

void MolCoord::readObj(qlib::ObjInStream &dis)
{
  MbObject::readObj(dis);

  lock(); {
    int nsize;
    dis.readData<int>("szatom", nsize);
    char *parray = MB_NEW char[nsize];
    dis.readArray("atoms", parray, nsize);

    qlib::StrInStream sis(parray, nsize);
    qlib::BinInStream bis(sis);

    // read all atoms
    int natoms = bis.read<int>();
    int i=0;
    for ( ; i<natoms; i++) {
      MolAtom *p = MB_NEW MolAtom();
      p->readBin(bis);
      appendAtom(p);
    }


    delete [] parray;
  } unlock();
  
  // apply topology
  MolModule::getInstance()->applyTopology(this);
  
  //
  // read selection stack
  //

  // clear selection (if exists)
  m_lockSelstk.lock(); {
    m_selstk.clear();

    //dis.readData("selstk", m_selstk);
    int i, n;
    dis.readData("size", n);

    for (i=0; i<n; ++i) {
      LString propname = LString::format("[%d]", i);
      Serializable *pnew = dis.readObj(propname);
      MolSelection *psel = dynamic_cast<MolSelection *>(pnew);
      m_selstk.push_back(SelectionPtr(psel));
    }

  } m_lockSelstk.unlock();

  

  {
    // read residue properties
    ResidIterator iter(this);
    for (iter.first(); iter.hasMore(); iter.next()) {
      MolResidue *pRes = iter.get();
      MB_ASSERT(pRes!=NULL);

      LString key = LString::format("resid %s %d",
                                    (const char *)pRes->getChainName(),
                                    pRes->getIndex());
      dis.readData(key, *pRes);
    }
  }

}

/** create interpreter object */
qobj_inst *MolCoord::createInterpObj() const
{
  qobj_inst *p = MB_NEW molobj_inst();
  p->setObjName(getName());
  return p;
}

bool MolCoord::isCopyAvailable(bool bcut) const
{
  SelectionPtr psel = getSelect();
  if (psel.isnull() || psel->isEmpty())
    return false;

  return true;
}

MbObject *MolCoord::copyToCb(bool bcut)
{
  MolCoord *pnew = MB_NEW MolCoord;
  pnew->addRef();
  if (bcut) {
    //pnew->merge(this);
    MolModule *pmmod = MolModule::getInstance();
    pmmod->merge(pnew, this);
  }
  else
    pnew->copy(this);

  return pnew;
}

bool MolCoord::isPasteAvailable(MbObject *psrc) const
{
  MolCoord *psrcmol = dynamic_cast<MolCoord *>(psrc);
  if (psrcmol==NULL) return false;

  // TO DO: check overwrapping

  return true;
}

void MolCoord::pasteFromCb(MbObject *psrc)
{
  MolCoord *psrcmol = dynamic_cast<MolCoord *>(psrc);
  if (psrcmol==NULL) return;

  MolModule *pmmod = MolModule::getInstance();
  if (!pmmod->copy(this, psrcmol)) {
    // Error handling
    MB_THROW(ClipboardException, "clipboard contains atoms with the same properties");
  }
}
  
//#include "PDBFileWriter.hpp"

bool MolCoord::createCbTextData(LString &str)
{
  str = "CueMol MolCoord data\n";
  return true;
}

bool MolCoord::isDeleteAvailable() const
{
  return isCopyAvailable(true);
}

void MolCoord::deleteSelected()
{
  //pnew->merge(this);
  MolModule *pmmod = MolModule::getInstance();
  if (!pmmod->erase(this)) {
    MB_THROW(ClipboardException, "Cannot erase selected atoms!!");
  }
}

bool MolCoord::isEmpty() const
{
  return (getAtomSize()==0);
}

bool MolCoord::isSelectionSupported() const
{
  return true;
}

void MolCoord::selectAll()
{
  MolModule *pmmod = MolModule::getInstance();
  pmmod->select(getName());
}
#endif

