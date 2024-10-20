// #include "normal_rpc_co_client_module/normal_rpc_co_client_module.h"

#include "normal_rpc_co_client_module.h"
#include "aimrt_module_cpp_interface/co/aimrt_context.h"
#include "aimrt_module_cpp_interface/co/inline_scheduler.h"
#include "aimrt_module_cpp_interface/co/on.h"
#include "aimrt_module_cpp_interface/co/schedule.h"
#include "aimrt_module_cpp_interface/co/sync_wait.h"
#include "aimrt_module_protobuf_interface/util/protobuf_tools.h"

#include "yaml-cpp/yaml.h"

#include <iostream>
#include <fstream>
#include <string>

// #include <grpcpp/grpcpp.h>  // 包含 gRPC 相关的头文件

namespace my_protobuf_rpc_namespace::normal_rpc_co_client_module {

bool NormalRpcCoClientModule::Initialize(aimrt::CoreRef core) {
  core_ = core;

  try {
    // Read cfg
    std::string file_path = std::string(core_.GetConfigurator().GetConfigFilePath());
    if (!file_path.empty()) {
      YAML::Node cfg_node = YAML::LoadFile(file_path);
      rpc_frq_ = cfg_node["rpc_frq"].as<double>();
    }


    // Get executor handle
    //获取并验证执行器（executor），用于后续的异步任务调度。
    executor_ = core_.GetExecutorManager().GetExecutor("work_thread_pool");
    AIMRT_CHECK_ERROR_THROW(executor_ && executor_.SupportTimerSchedule(),
                            "Get executor 'work_thread_pool' failed.");

    // Get rpc handle
    //获取 RPC 句柄
    auto rpc_handle = core_.GetRpcHandle();
    AIMRT_CHECK_ERROR_THROW(rpc_handle, "Get rpc handle failed.");

    // Register rpc client
    //注册 RPC 客户端 。
    bool ret = my_package_name::Registermy_serviceClientFunc(rpc_handle);
    AIMRT_CHECK_ERROR_THROW(ret, "Register client failed.");

    // Create rpc proxy
    //创建一个 RPC 代理（proxy_），用于后续的 RPC 调用。
    proxy_ = std::make_shared<my_package_name::my_serviceCoProxy>(rpc_handle);

    //注册两个过滤器（RegisterFilter），在发起 RPC 请求和处理响应时，记录调试信息和计算 RPC 调用的耗时。

    // Register filter
    proxy_->RegisterFilter([this](aimrt::rpc::ContextRef ctx,
                                  const void* req_ptr, void* rsp_ptr,
                                  const aimrt::rpc::CoRpcHandle& next)
                               -> aimrt::co::Task<aimrt::rpc::Status> {
      // debuglog: 记录RPC请求的上下文和请求内容
      AIMRT_INFO("Client start new rpc call. context: {}, req: {}",
                 ctx.ToString(), aimrt::Pb2CompactJson(*static_cast<const google::protobuf::Message*>(req_ptr)));
      //1. 例如：Client start new rpc call. context: Client context, timeout: 3000ms, meta: {{"aimrt-function_name":"pb:/aimrt.protocols.example.ExampleService/GetFooData"},{"aimrt-serialization_type":"pb"}}, req: {"msg":"hello world foo, count 112"}
      const auto& status = co_await next(ctx, req_ptr, rsp_ptr);//实际执行 RPC 调用的操作，next 是下一个协程操作，它会发起 RPC 请求并等待响应。
      
      //用co_await，在这里等待RPC调用的结果的返回...

      if (status.OK()) {
        // 执行真正的RPC调用，并等待结果
        AIMRT_INFO("Client get rpc ret, status: {}, rsp: {}", status.ToString(),
                   aimrt::Pb2CompactJson(*static_cast<const google::protobuf::Message*>(rsp_ptr)));
      //2. 例如：Client get rpc ret, status: suc, code 0, msg: OK, rsp: {"code":"0","msg":"echo hello world foo, count 112"}
      } else {
        AIMRT_WARN("Client get rpc error ret, status: {}", status.ToString());
      }

      co_return status;// 返回RPC调用的状态
    });

    proxy_->RegisterFilter([this](aimrt::rpc::ContextRef ctx,
                                  const void* req_ptr, void* rsp_ptr,
                                  const aimrt::rpc::CoRpcHandle& next)
                               -> aimrt::co::Task<aimrt::rpc::Status> {
      // timecost count
      auto begin_time = std::chrono::steady_clock::now();
      const auto& status = co_await next(ctx, req_ptr, rsp_ptr);//等待 RPC 调用的结果或任务调度完成。
      
      // 用co_await，在这里等待RPC调用的结果的返回...

      auto end_time = std::chrono::steady_clock::now();

      AIMRT_INFO("Client rpc time cost {} us",
                 std::chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time).count());
      //3. 例如：Client rpc time cost 11522 us
      co_return status;
    });

  } catch (const std::exception& e) {
    AIMRT_ERROR("Init failed, {}", e.what());
    return false;
  }

  AIMRT_INFO("Init succeeded.");

  return true;
}

bool NormalRpcCoClientModule::Start() {
  try {
    //使用协程调度器 InlineScheduler 启动一个协程任务（MainLoop），避免阻塞主线程。
    //注意：这里的MainLoop()本身就是一个协程函数。
    
    //InlineScheduler调度的工作过程：在当前线程上立即调度并执行协程，所有的操作都在同一线程中进行。。
    scope_.spawn(aimrt::co::On(aimrt::co::InlineScheduler(), MainLoop()));
  } catch (const std::exception& e) {
    AIMRT_ERROR("Start failed, {}", e.what());
    return false;
  }

  AIMRT_INFO("Start succeeded.");
  return true;
}

void NormalRpcCoClientModule::Shutdown() {
  try {
    run_flag_ = false;
    //使用 SyncWait 等待协程范围（scope_）中的所有任务完成。
    aimrt::co::SyncWait(scope_.complete());
  } catch (const std::exception& e) {
    AIMRT_ERROR("Shutdown failed, {}", e.what());
    return;
  }

  AIMRT_INFO("Shutdown succeeded.");
}


aimrt::co::Task<void> NormalRpcCoClientModule::MainLoop() {
  try {
    AIMRT_INFO("Start MainLoop.");
    aimrt::co::AimRTScheduler work_thread_pool_scheduler(executor_);

    uint32_t count = 0;

    // 打开视频文件，作为示例
    std::ifstream file("/home/sqhl/65s.mp4", std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法打开视频文件");
    }

    // 读取整个视频文件内容并分块
    const std::size_t chunk_size = 1024 * 1024;  // 1MB 一块作为示例
    std::vector<std::string> video_chunks;
    std::string video_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    for (std::size_t i = 0; i < video_data.size(); i += chunk_size) {
        video_chunks.push_back(video_data.substr(i, chunk_size));
    }

    const int total_chunks = video_chunks.size();
    const int frame_rate = 30; // 假设 30fps
    const std::string format = "mp4";

    while (run_flag_) {
      co_await aimrt::co::ScheduleAfter(work_thread_pool_scheduler,
          std::chrono::milliseconds(static_cast<uint32_t>(1000 / rpc_frq_)));
      count++;
      AIMRT_INFO("Loop count : {} -------------------------", count);

      // 创建上下文对象，设置超时时间为 3 秒
      auto ctx_ptr = proxy_->NewContextSharedPtr();
      ctx_ptr->SetTimeout(std::chrono::seconds(3));

      // 使用代理对象，开始流式传输
      auto stream = proxy_->my_service_function(ctx_ptr);

      // 逐个发送视频切块
      for (int i = 0; i < total_chunks; ++i) {
        my_package_name::SensorDataReq req;
        req.set_video_chunk_data(video_chunks[i]);
        req.set_chunk_index(i);
        req.set_total_chunks(total_chunks);
        req.set_frame_rate(frame_rate);
        req.set_format(format);

        // 发送视频切块
        auto status = co_await stream.Write(req);
        if (!status.OK()) {
          AIMRT_WARN("Error sending chunk index {}: {}", i, status.ToString());
          break;
        }
      }

      // 关闭流式传输
      stream.Finish();

      // 等待响应
      my_package_name::SensorDataRsp rsp;
      auto status = co_await stream.Read(rsp);
      if (status.OK()) {
        AIMRT_INFO("Client received response: {}", aimrt::Pb2CompactJson(rsp));
      } else {
        AIMRT_WARN("Client encountered error: {}", status.ToString());
      }
    }

    AIMRT_INFO("Exit MainLoop.");
  } catch (const std::exception& e) {
    AIMRT_ERROR("Exit MainLoop with exception, {}", e.what());
  }

  co_return;
}


}  