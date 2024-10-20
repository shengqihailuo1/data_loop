// Copyright (c) 2023, AgiBot Inc.
// All rights reserved.

#pragma once

#include <atomic>
#include <memory>

#include "aimrt_module_cpp_interface/co/async_scope.h"
#include "aimrt_module_cpp_interface/co/task.h"
#include "aimrt_module_cpp_interface/module_base.h"
#include "aimrt_module_cpp_interface/rpc/rpc_co_filter.h"

// #include "rpc.aimrt_rpc.pb.h"
#include "my_image.aimrt_rpc.pb.h"

namespace my_protobuf_rpc_namespace::normal_rpc_co_client_module {

class NormalRpcCoClientModule : public aimrt::ModuleBase {
 public:
  NormalRpcCoClientModule() = default;
  ~NormalRpcCoClientModule() override = default;

  aimrt::ModuleInfo Info() const override {
    return aimrt::ModuleInfo{.name = "NormalRpcCoClientModule"};
  }

  bool Initialize(aimrt::CoreRef core) override;

  bool Start() override;

  void Shutdown() override;

 private:
  auto GetLogger() { return core_.GetLogger(); }

  aimrt::co::Task<void> MainLoop();

  // co::Task<void> SendSensorData();//zcw添加

 private:
  aimrt::CoreRef core_;
  aimrt::executor::ExecutorRef executor_;

  aimrt::co::AsyncScope scope_;
  std::atomic_bool run_flag_ = true;

  double rpc_frq_ = 1.0;
  std::shared_ptr<my_package_name::my_serviceCoProxy> proxy_;
};

}  // namespace aimrt::examples::cpp::protobuf_rpc::normal_rpc_co_client_module
