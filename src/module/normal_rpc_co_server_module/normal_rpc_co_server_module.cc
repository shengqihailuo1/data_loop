// Copyright (c) 2023, AgiBot Inc.
// All rights reserved.

// #include "normal_rpc_co_server_module/normal_rpc_co_server_module.h"

#include "normal_rpc_co_server_module.h"
#include "filter.h"
#include "global.h"
// #include "normal_rpc_co_server_module/filter.h"
// #include "normal_rpc_co_server_module/global.h"

namespace my_protobuf_rpc_namespace::normal_rpc_co_server_module {

bool NormalRpcCoServerModule::Initialize(aimrt::CoreRef core) {
  core_ = core;

  SetLogger(core_.GetLogger());

  try {
    // 创建了服务的实例 service_ptr_，这个实例代表服务的具体实现，它是实际处理 RPC 请求的类。
    service_ptr_ = std::make_shared<my_serviceImpl>();

    //这些过滤器将在 RPC 请求处理过程中执行，主要用于日志记录和性能监控
    service_ptr_->RegisterFilter(my_protobuf_rpc_namespace::normal_rpc_co_server_module::DebugLogServerFilter);
    service_ptr_->RegisterFilter(my_protobuf_rpc_namespace::normal_rpc_co_server_module::TimeCostLogServerFilter);

    // 将服务注册到 RPC 系统中，这样服务端就可以接收来自客户端的 RPC 调用了。
    bool ret = core_.GetRpcHandle().RegisterService(service_ptr_.get());
    AIMRT_CHECK_ERROR_THROW(ret, "Register service failed.");

    AIMRT_INFO("Register service succeeded.");

  } catch (const std::exception& e) {
    AIMRT_ERROR("Init failed, {}", e.what());
    return false;
  }

  AIMRT_INFO("Init succeeded.");

  return true;
}

bool NormalRpcCoServerModule::Start() { return true; }

void NormalRpcCoServerModule::Shutdown() {}

}  // namespace aimrt::examples::cpp::protobuf_rpc::normal_rpc_co_server_module
