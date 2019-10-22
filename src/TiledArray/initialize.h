//
// Created by Chong Peng on 7/27/18.
//

#ifndef TILEDARRAY_INITIALIZE_H__INCLUDED
#define TILEDARRAY_INITIALIZE_H__INCLUDED


#include <TiledArray/config.h>

#include <TiledArray/external/madness.h>
#ifdef TILEDARRAY_HAS_CUDA
#include <cutt.h>
#include <TiledArray/external/cuda.h>
#include <TiledArray/math/cublas.h>
#endif

namespace TiledArray{

#ifdef TILEDARRAY_HAS_CUDA
/// initialize cuda environment
inline void cuda_initialize(World& world) {
  /// initialize cudaGlobal
  cudaEnv::instance(world);
  //
  cuBLASHandlePool::handle();
  // initialize cuTT
  cuttInitialize();
}

/// finalize cuda environment
inline void cuda_finalize() {
  CudaSafeCall(cudaDeviceSynchronize());
  cuttFinalize();
  cublasDestroy(cuBLASHandlePool::handle());
  delete &cuBLASHandlePool::handle();
  cudaEnv::instance().reset(nullptr);
}
#endif

namespace detail {
  static bool& initialized_madworld_accessor() {
    static bool flag = false;
    return flag;
  }
  static bool initialized_madworld() {
    return initialized_madworld_accessor();
  }
  static bool& initialized_accessor() {
    static bool flag = false;
    return flag;
  }
  static bool& finalized_accessor() {
    static bool flag = false;
    return flag;
  }
}

/// @return true if TiledArray (and, necessarily, MADWorld runtime) is in an initialized state
static bool initialized() {
  return detail::initialized_accessor();
}

/// @return true if TiledArray has been finalized at least once
static bool finalized() {
  return detail::finalized_accessor();
}

/// @name TiledArray initialization.
///       These functions initialize TiledArray and (if needed) MADWorld runtime.
/// @note the default World object is set to the object returned by these.
/// @warning MADWorld can only be initialized/finalized once, hence if TiledArray initializes MADWorld
///          it can also be initialized/finalized only once.

/// @{

/// @throw TiledArray::Exception if TiledArray initialized MADWorld and TiledArray::finalize() had been called
inline World& initialize(int& argc, char**& argv, const SafeMPI::Intracomm& comm, bool quiet = true) {
  if (detail::initialized_madworld() && finalized())
    throw Exception("TiledArray finalized MADWorld already, cannot re-initialize MADWorld again");
  if (!initialized()) {
    if (!madness::initialized())
      detail::initialized_madworld_accessor() = true;
    else {  // if MADWorld initialized, we must assume that comm is its default World.
      if (madness::World::is_default(comm))
        throw Exception("MADWorld initialized before TiledArray::initialize(argc, argv, comm), but not initialized with comm");
    }
    auto& default_world =
        detail::initialized_madworld()
            ? madness::initialize(argc, argv, comm, quiet)
            : *madness::World::find_instance(comm);
    TiledArray::set_default_world(default_world);
#ifdef TILEDARRAY_HAS_CUDA
    TiledArray::cuda_initialize(default_world);
#endif
    madness::print_meminfo_disable();
    detail::initialized_accessor() = true;
    return default_world;
  }
  else
    throw Exception("TiledArray already initialized");
}

inline World& initialize(int& argc, char**& argv, bool quiet = true) {
  return TiledArray::initialize(argc, argv, SafeMPI::COMM_WORLD, quiet);
}

inline World& initialize(int& argc, char**& argv, const MPI_Comm& comm, bool quiet = true) {
  return TiledArray::initialize(argc, argv, SafeMPI::Intracomm(comm), quiet);
}

/// @}

/// Finalizes TiledArray (and MADWorld runtime, if it had not been initialized when TiledArray::initialize was called).
inline void finalize() {
#ifdef TILEDARRAY_HAS_CUDA
  TiledArray::cuda_finalize();
#endif
  TiledArray::get_default_world().gop.fence(); // TODO remove when madness::finalize() fences
  if (detail::initialized_madworld()) {
    madness::finalize();
  }
  TiledArray::reset_default_world();
  detail::initialized_accessor() = false;
  detail::finalized_accessor() = true;
}

}

#endif //TILEDARRAY_INITIALIZE_H__INCLUDED
