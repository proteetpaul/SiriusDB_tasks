#include "task1.grpc.pb.h"
#include <stdio.h>
#include <chrono>
#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
// #include <grpc/channel.h>

void connect_to_server(uint64_t *chunks, uint64_t *chunk_size, int idx) {
    std::unique_ptr<Task1::Stub> stub;
    auto channel_ptr = grpc::CreateChannel("node-1.proteet-230925.nextgendb-pg0.utah.cloudlab.us:50052", 
        grpc::InsecureChannelCredentials());
    stub = std::move(std::make_unique<Task1::Stub>(channel_ptr));

    uint64_t total = 0ll;
    int iter = 1;
    uint64_t num_chunks;
    for (int i=0; i<iter; i++) {
        grpc::ClientContext ctx;
        Empty request;
        // auto start = std::chrono::system_clock::now();
        auto reader = stub->Get(&ctx, request);
        Data data;
        num_chunks = 0;
        while (reader->Read(&data)) {
            num_chunks++;
            if (num_chunks == 1 && !idx) {
                *chunk_size = data.chunk().size();
            }
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
ABSL_FLAG(uint32_t, num_iter, 3, "Number of iterations");

int main(int argc, char **argv) {
    absl::ParseCommandLine(argc, argv);
    uint64_t num_client_threads = absl::GetFlag(FLAGS_num_threads);
    uint32_t num_iter = absl::GetFlag(FLAGS_num_iter);
    
    uint64_t chunk_size;
    float total_latency = 0ll;
    uint64_t num_chunks;
    // std::vector<float> arr(num_client_threads);
    for (int i=0; i<num_iter; i++) {
        std::vector<std::thread> t;
        auto start = std::chrono::system_clock::now();

        for (int i=0; i<num_client_threads; i++) {
            t.push_back(std::thread(std::bind(&connect_to_server, &num_chunks, &chunk_size, i)));
        }
        for (auto &thread: t) {
            thread.join();
        }
        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

        total_latency += (float)duration.count();
    }
    
    uint64_t total_bytes = num_chunks * 1ll * chunk_size * num_client_threads * num_iter;
    float throughput = (total_bytes/1e3)/(float)total_latency;
    std::cout << "Num chunks: " << num_chunks << "\n";
    std::cout << "Chunk size: " << chunk_size << "\n";
    // std::cout << "Total bytes: " << total_bytes << "\n";
    std::cout << "Total throughput: " << throughput << " MBps\n";
}