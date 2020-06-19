#ifndef WK_RUNTIMEDISPATCH_H
#define WK_RUNTIMEDISPATCH_H

#include "CPU.h"
#include "Log/Log.h"

namespace Wikinger {

template<typename F>
struct Implementation {
  F* func;
  uint64_t flags;
};

template<typename F>
class RuntimeDispatch {
private:
  F* getBest(std::initializer_list<Implementation<F>> impl) {
    F* cand = nullptr;
    CPU c;
    for(auto iter = impl.begin(); iter != impl.end(); iter++) {
      if((iter->flags & c.GetISA()) == iter->flags) {
        cand = iter->func;
        break;
      }
    }

    if(cand == nullptr) {
      WK_FATAL("RuntimeDispatch: No implementation available for this CPU, exiting application");
      exit(-1);
      return nullptr;
    }
    else {
      return cand;
    }
  }

public:
  RuntimeDispatch(std::initializer_list<Implementation<F>> list) {
    active = getBest(list);
  }

  template<typename... Args>
  auto& operator()(Args&&... args) const {
    return active(args...);
  }

private:
  F* active = nullptr;
};

}

#endif// WK_RUNTIMEDISPATCH_H
