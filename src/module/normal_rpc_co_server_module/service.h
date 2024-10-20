
// Copyright (c) 2023, AgiBot Inc.
// All rights reserved.

#pragma once
#include "my_image.aimrt_rpc.pb.h"

namespace my_protobuf_rpc_namespace::normal_rpc_co_server_module {

class my_serviceImpl : public my_package_name::my_serviceCoService {
 public:
  my_serviceImpl() = default;
  ~my_serviceImpl() override = default;

  aimrt::co::Task<aimrt::rpc::Status> my_service_function(
      aimrt::rpc::ContextRef ctx,
      const ::my_package_name::SensorDataReq& req,
      //grpc::ServerReader<::my_package_name::SensorDataReq>* reader,
      ::my_package_name::SensorDataRsp& rsp) override;
};

} 