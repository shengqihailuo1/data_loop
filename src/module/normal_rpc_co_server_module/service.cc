
// Copyright (c) 2023, AgiBot Inc.
// All rights reserved.

// #include "normal_rpc_co_server_module/service.h"
// #include "normal_rpc_co_server_module/global.h"

#include "service.h"
#include "aimrt_module_protobuf_interface/util/protobuf_tools.h"
#include "global.h"

namespace my_protobuf_rpc_namespace::normal_rpc_co_server_module {

aimrt::co::Task<aimrt::rpc::Status> my_serviceImpl::my_service_function(
    aimrt::rpc::ContextRef ctx,
    const ::my_package_name::SensorDataReq& req,
    //grpc::ServerReader<::my_package_name::SensorDataReq>* reader, // 注意这里的类型是ServerReader
    ::my_package_name::SensorDataRsp& rsp
) {
    std::vector<std::string> video_chunks;
    my_package_name::SensorDataReq req;

    // 逐个接收视频切块
    while (reader->Read(&req)) {
        AIMRT_INFO("Received chunk index: {}", req.chunk_index());
        video_chunks.push_back(req.video_chunk_data());
    }

    // 将视频块拼接成完整的文件
    std::string complete_video;
    for (const auto& chunk : video_chunks) {
        complete_video.append(chunk);
    }

    // 假设将视频保存到某个路径
    std::ofstream output_file("/home/sqhl/received_video.mp4", std::ios::binary);
    output_file.write(complete_video.data(), complete_video.size());
    output_file.close();

    rsp.set_msg("Successfully received video data");
    rsp.set_result_bool(true);

    co_return aimrt::rpc::Status();
}


}  