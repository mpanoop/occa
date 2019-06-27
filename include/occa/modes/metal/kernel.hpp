#include <occa/defines.hpp>

#if OCCA_METAL_ENABLED
#  ifndef OCCA_MODES_METAL_KERNEL_HEADER
#  define OCCA_MODES_METAL_KERNEL_HEADER

#include <occa/core/launchedKernel.hpp>
#include <occa/modes/metal/headers.hpp>

namespace occa {
  namespace metal {
    class device;

    class kernel : public occa::launchedModeKernel_t {
      friend class device;

    private:
      metalDevice_t metalDevice;
      metalKernel_t metalKernel;

    public:
      kernel(modeDevice_t *modeDevice_,
             const std::string &name_,
             const std::string &sourceFilename_,
             const occa::properties &properties_);

      ~kernel();

      int maxDims() const;
      dim maxOuterDims() const;
      dim maxInnerDims() const;

      void deviceRun() const;
    };
  }
}

#  endif
#endif