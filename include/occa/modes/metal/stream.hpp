#include <occa/defines.hpp>

#if OCCA_METAL_ENABLED
#  ifndef OCCA_MODES_METAL_STREAM_HEADER
#  define OCCA_MODES_METAL_STREAM_HEADER

#include <occa/core/stream.hpp>
#include <occa/modes/metal/headers.hpp>

namespace occa {
  namespace metal {
    class stream : public occa::modeStream_t {
    public:
      metalCommandQueue_t metalCommandQueue;

      stream(modeDevice_t *modeDevice_,
             const occa::properties &properties_,
             metalCommandQueue_t metalCommandQueue_);

      virtual ~stream();
    };
  }
}

#  endif
#endif