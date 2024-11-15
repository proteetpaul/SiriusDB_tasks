#include "task1.grpc.pb.h"

#include <vector>
#include <string>
#include <memory>
#include <grpcpp/grpcpp.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <thread>
#include <cstdint>

class Task1ServiceImpl: public Task1::Service {
private:
    std::vector<uint32_t> vec;
    std::string* data_str;
    uint64_t num_total_bytes;
    uint64_t chunk_size;

public:
    Task1ServiceImpl(uint64_t num_total_bytes, uint64_t chunk_size)
            : num_total_bytes(num_total_bytes), chunk_size(chunk_size) {
        int size = chunk_size/sizeof(uint32_t);
        vec.resize(size);
        for (uint32_t i=0; i<vec.size(); i++) {
            vec[i] = i;
        }
        data_str = new std::string(reinterpret_cast<const char*>(vec.data()), chunk_size);
    }

    grpc::Status Get(grpc::ServerContext* context, const Empty* request, 
            grpc::ServerWriter<Data>* writer) override {
        std::cout << std::this_thread::get_id() << ": Received Get request\n";
        uint64_t i = 0ll;
        Data d;
        d.set_allocated_chunk(data_str);
        for (; i<num_total_bytes; i+=chunk_size) {
            writer->Write(d);
        }
        data_str = d.release_chunk();
        Data last;
        grpc::WriteOptions options;
        writer->WriteLast(last, options);
        std::cout << "Wrote " << i << " chunks:\n";

        return grpc::Status::OK;
    }

    virtual ~Task1ServiceImpl(){}
};

ABSL_FLAG(uint64_t, num_gigs, 1, "Size of dataset in GiB");
ABSL_FLAG(uint64_t, chunk_size, 64, "Chunk size in KB");

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    uint64_t num_gigs = absl::GetFlag(FLAGS_num_gigs);
    uint64_t chunk_size = absl::GetFlag(FLAGS_chunk_size) * 1ll * 1024;
    uint64_t total = num_gigs * 1ll * 1<<30;
    std::string server_address("node-1.proteet-230925.nextgendb-pg0.utah.cloudlab.us:50052");
    std::cout << "Total bytes: " << total << "\n";
    std::cout << "Chunk size: " << chunk_size << "\n";
    Task1ServiceImpl service(total, chunk_size);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}