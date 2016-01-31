//
// Python CueMol module initialization
//

#include <Python.h>

#include <common.h>
#include <qlib/qlib.hpp>
#include <qlib/LProcMgr.hpp>
#include <qlib/EventManager.hpp>
#include <qsys/qsys.hpp>
#include <qsys/SceneManager.hpp>
#include <sysdep/sysdep.hpp>

#include "wrapper.hpp"

using namespace pybr;

/// DLL entrance routine for pymodule

PyMODINIT_FUNC
#if PY_MAJOR_VERSION >= 3
PyInit_cuemol()
#else
initcuemol()
#endif
{
  qlib::init();
  MB_DPRINTLN("CueMol2 pymodule : INITIALIZED");
  PyObject *module = Wrapper::init();

#if PY_MAJOR_VERSION >= 3
  return module;
#endif
}

////////////////////////////////////////////////

namespace render {
  extern bool init();
  extern void fini();
}

namespace molstr {
  extern bool init();
  extern void fini();
}

namespace molvis {
  extern bool init();
  extern void fini();
}

namespace xtal {
  extern bool init();
  extern void fini();
}

namespace molanl {
  extern bool init();
  extern void fini();
}

namespace surface {
  extern bool init();
  extern void fini();
}

namespace symm {
  extern bool init();
  extern void fini();
}

namespace lwview {
  extern bool init();
  extern void fini();
}

namespace anim {
  extern bool init();
  extern void fini();
}

namespace mdtools {
  extern bool init();
  extern void fini();
}

namespace importers {
  extern bool init();
  extern void fini();
}

namespace {
  class ProcMgrChkQueue : public qlib::IdleTask
  {
  public:
    void perform() {
      qlib::LProcMgr *pPM = qlib::LProcMgr::getInstance();
      pPM->checkQueue();
    }
  };

  class SceneMgrChkUpdate : public qlib::IdleTask
  {
  public:
    void perform() {
      qsys::SceneManager *pSM = qsys::SceneManager::getInstance();
      pSM->checkAndUpdateScenes();
    }
  };
}

namespace pybr {
  /// CueMol initialization routine
  PyObject *initCueMol(PyObject *self, PyObject *args)
  {
    const char *config;

    if (!PyArg_ParseTuple(args, "s", &config))
      return NULL;
  
    qsys::init(config);
    sysdep::init();

    // load other modules
    render::init();
    molstr::init();
    molvis::init();
    xtal::init();
    symm::init();
    surface::init();
    molanl::init();
    lwview::init();
    anim::init();
    mdtools::init();
    importers::init();

    // initTextRender();
    // MB_DPRINTLN("---------- initTextRender() OK");

    // setup timer
    
    qlib::EventManager *pEM = qlib::EventManager::getInstance();
    pEM->addIdleTask(new ProcMgrChkQueue, false);
    pEM->addIdleTask(new SceneMgrChkUpdate, true);

    return Py_BuildValue("");
  }
}
