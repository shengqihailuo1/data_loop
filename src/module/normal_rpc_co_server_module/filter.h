// Copyright (c) 2023, AgiBot Inc.
// All rights reserved.

#pragma once

#include "aimrt_module_cpp_interface/rpc/rpc_co_filter.h"
#include "aimrt_module_protobuf_interface/util/protobuf_tools.h"
// #include "normal_rpc_co_server_module/global.h"

#include "global.h"


namespace my_protobuf_rpc_namespace::normal_rpc_co_server_module {

inline aimrt::co::Task<aimrt::rpc::Status> DebugLogServerFilter(
    aimrt::rpc::ContextRef ctx, const void* req_ptr, void* rsp_ptr,
    const aimrt::rpc::CoRpcHandle& next) {
  // 1.
  AIMRT_INFO("Svr get new rpc call. context: {}, req: {}",
             ctx.ToString(), aimrt::Pb2CompactJson(*static_cast<const google::protobuf::Message*>(req_ptr)));

  const auto& status = co_await next(ctx, req_ptr, rsp_ptr);

  //3. 
  AIMRT_INFO("Svr handle rpc completed, status: {}, rsp: {}",
             status.ToString(),
             aimrt::Pb2CompactJson(*static_cast<const google::protobuf::Message*>(rsp_ptr)));
  co_return status;
}

inline aimrt::co::Task<aimrt::rpc::Status> TimeCostLogServerFilter(
    aimrt::rpc::ContextRef ctx, const void* req_ptr, void* rsp_ptr,
    const aimrt::rpc::CoRpcHandle& next) {
  auto begin_time = std::chrono::steady_clock::now();
  const auto& status = co_await next(ctx, req_ptr, rsp_ptr);
  auto end_time = std::chrono::steady_clock::now();

  // 4 .
  AIMRT_INFO("Svr rpc time cost {} us",
             std::chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time).count());

  co_return status;
}

}  // namespace aimrt::examples::cpp::protobuf_rpc::normal_rpc_co_server_module
