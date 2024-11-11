#include "task1.grpc.pb.h"
#include <stdio.h>
#include <chrono>
#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
// #include <grpc/channel.h>

void connect_to_server(int *chunks, int idx) {
    std::unique_ptr<Task1::Stub> stub;
    auto channel_ptr = grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials());
    stub = std::move(std::make_unique<Task1::Stub>(channel_ptr));

    uint64_t total = 0ll;
    int iter = 1;
    int num_chunks;
    for (int i=0; i<iter; i++) {
        grpc::ClientContext ctx;
        Empty request;
        // auto start = std::chrono::system_clock::now();
        auto reader = stub->Get(&ctx, request);
        Data data;
        num_chunks = 0;
        while (reader->Read(&data)) {
            num_chunks++;
        }
        reader->Finish();
        // auto end = std::chrono::system_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        // total += duration.count();
        if (!idx && !i) {
            *chunks = num_chunks;
        }
    }
}

ABSL_FLAG(uint64_t, num_threads, 1, "Number of client threads");

int main(int argc, char **argv) {
    absl::ParseCommandLine(argc, argv);
    uint64_t num_client_threads = absl::GetFlag(FLAGS_num_threads);
    std::vector<std::thread> t;
    // std::vector<float> arr(num_client_threads);
    auto start = std::chrono::system_clock::now();
    int num_chunks;
    for (int i=0; i<num_client_threads; i++) {
        t.push_back(std::thread(std::bind(&connect_to_server, &num_chunks, i)));
    }
    for (auto &thread: t) {
        thread.join();
    }
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    float total_latency = 0ll;
    // for (float t: arr) total_latency += t;
    uint64_t total_bytes = num_chunks * 1ll * 2048 * num_client_threads;
    float throughput = (total_bytes/1e3)/(float)duration.count();
    std::cout << "Total bytes: " << total_bytes << "\n";
    std::cout << "Total throughput: " << throughput << " MBps\n";
}