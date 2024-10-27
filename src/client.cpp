#include "task1.grpc.pb.h"
#include <stdio.h>
#include <chrono>
#include <memory>
#include <grpcpp/grpcpp.h>
// #include <grpc/channel.h>

int main() {
    std::unique_ptr<Task1::Stub> stub;
    auto channel_ptr = grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials());
    stub = std::move(std::make_unique<Task1::Stub>(channel_ptr));

    uint64_t total = 0ll;
    int num_chunks;
    for (int i=0; i<5; i++) {
        grpc::ClientContext ctx;
        Empty request;
        auto start = std::chrono::system_clock::now();
        auto reader = stub->Get(&ctx, request);
        Data data;
        num_chunks = 0;
        while (reader->Read(&data)) {
            num_chunks++;
        }
        reader->Finish();
        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        total += duration.count();
    }
    
    uint64_t total_bytes = num_chunks * 1ll * 2048;
    float time_per_iter = total/5.0f;
    std::cout << "Time taken: " << time_per_iter << "\n";
    std::cout << "Bytes read: " << total_bytes << "\nThroughput: " << ((float)total_bytes/1e3)/time_per_iter << "\n";
}