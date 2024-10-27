#include "task1.grpc.pb.h"

#include <vector>
#include <string>
#include <memory>
#include <grpcpp/grpcpp.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <cstdint>

class Task1ServiceImpl: public Task1::Service {
private:
    std::vector<uint32_t> vec;
    std::string* data_str;
    uint64_t num_total_bytes;

public:
    Task1ServiceImpl(uint64_t num_total_bytes): num_total_bytes(num_total_bytes) {
        int size = 2048/sizeof(uint32_t);
        vec.resize(size);
        for (uint32_t i=0; i<vec.size(); i++) {
            vec[i] = i;
        }
        data_str = new std::string(reinterpret_cast<const char*>(vec.data()), 2048);
        std::cout << data_str->size() << "\n";
    }

    grpc::Status Get(grpc::ServerContext* context, const Empty* request, 
            grpc::ServerWriter<Data>* writer) override {
        uint64_t i = 0ll;
        Data d;
        d.set_allocated_chunk(data_str);
        for (; i<num_total_bytes; i+=2048) {
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

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    uint64_t num_gigs = absl::GetFlag(FLAGS_num_gigs);
    uint64_t total = num_gigs * 1ll * 1<<30;
    std::string server_address("localhost:50052");
    std::cout << "Total bytes: " << total << "\n";
    Task1ServiceImpl service(total);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}