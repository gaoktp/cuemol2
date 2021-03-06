// -*-Mode: C++;-*-
//
//  Event manager for scripting interface
//

#include "qsys.hpp"

#include <qlib/SingletonBase.hpp>
#include <qlib/LScrObjects.hpp>
#include <qlib/LScrCallBack.hpp>
#include <qlib/LLogEvent.hpp>
#include <qlib/mcutils.hpp>
#include <qlib/IndexedTable.hpp>

#ifndef QSYS_SCR_EVENT_MANAGER_HPP_
#define QSYS_SCR_EVENT_MANAGER_HPP_

namespace qsys {

using qlib::LString;
class QsysEvent;
class ViewEvent;
class ObjectEvent;
class RendererEvent;
class SceneEvent;
class ViewEventListener;
class ObjectEventListener;
class RendererEventListener;
class SceneEventListener;

/**
    Event manager
 */
class QSYS_API ScrEventManager : public qlib::LSingletonScrObject,
  public qlib::SingletonBase<ScrEventManager>,
  public qlib::LLogEventListener
{
    MC_SCRIPTABLE;

public:
  ScrEventManager();
  virtual ~ScrEventManager();
  
  /////////////////////////////
  
private:
  /// callback ptr for scripting interface
  qlib::LSCBPtr m_pCb;
  
public:

  /// event source type ID
  enum {
    SEM_ANY = -1,
    SEM_LOG      = 0x0001,
    SEM_INDEV    = 0x0002,
    SEM_SCENE    = 0x0004,
    SEM_OBJECT   = 0x0008,
    SEM_RENDERER = 0x0010,
    SEM_VIEW     = 0x0020,
    SEM_CAMERA   = 0x0040,
    SEM_STYLE    = 0x0080,
    SEM_ANIM     = 0x0100,
    SEM_EXTND    = 0x8000
  };
  
  /// event type ID
  enum {
    SEM_ADDED = 1,
    SEM_REMOVING = 2,
    SEM_PROPCHG = 3,
    SEM_CHANGED = 4,
    SEM_OTHER = 9999
  };

  /// Notify script event listeners
  bool fireEventScript(const LString &aCatStr,
                       int aSrcType, int aEvtType,
                       qlib::uid_t aSrcID,
                       qlib::LEvent &event);

  bool fireViewEvent(ViewEvent &ev);

  inline bool fireEvent(QsysEvent &event) {
    return fireEventImpl(event);
  }
  //bool fireObjectEvent(ObjectEvent &ev);
  //bool fireRendererEvent(RendererEvent &ev);
  //bool fireSceneEvent(SceneEvent &ev);

  int addListener(qlib::LSCBPtr scb);
  bool removeListener(int nid);

private:

  bool fireEventImpl(QsysEvent &event);

  struct Entry {
    LString cat;
    int nTgtType;
    int nEvtType;
    qlib::uid_t nSrcUID;
  };
  typedef qlib::IndexedTable<Entry> SlotTab;
  SlotTab m_slot;

public:
  int append(const LString &category,int nTgtType, int nEvtType, int nSrcUID);
  bool remove(int nSlotID);

private:
  int searchSlot(const LString &category,
                 int nTgtType, int nEvtType, qlib::uid_t nSrcUID);

private:
  int m_nLogLsnID;

public:
  virtual void logAppended(qlib::LLogEvent &ev);

  static bool initClass(qlib::LClass *pcls);
  static void finiClass(qlib::LClass *pcls);

  ////////////////////////////////////////////
  // Native event manager
private:
  typedef std::pair<qlib::uid_t, ViewEventListener *> ViewETuple;
  typedef std::deque< ViewETuple > ViewListeners;
  ViewListeners m_viewListeners;

  bool fireNativeViewEvent(ViewEvent &event);

public:
  void addViewListener(qlib::uid_t nFilter,
                       ViewEventListener *pL);
  bool removeViewListener(ViewEventListener *pL);
};

}

#endif


