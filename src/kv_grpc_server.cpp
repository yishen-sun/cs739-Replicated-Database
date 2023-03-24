#include "./kv_grpc_server.h"


namespace fs = std::filesystem;
using namespace std;
using namespace raft;

using raft::Raft;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;  // https://grpc.github.io/grpc/core/md_doc_statuscodes.html

/** mimics `stat`/`lstat` functionality */

Status GRPC_Server::Put(ServerContext* context, const PutRequest* request, PutResponse* response){
    std::cout << "GPRC_Server::put" << std::endl;
    std::string k(request->key());
    std::string v(request->value());
    inmem_store.Put(k, v);
    response->set_success(0);
    return Status::OK;
}


Status GRPC_Server::Get(ServerContext* context, const GetRequest* request, GetResponse* response){
    std::cout << "GPRC_Server::get" << std::endl;
    std::string v(inmem_store.Get(request->key()));
    response->set_success(0);
    response->set_value(v);
    return Status::OK;
}

// Status GRPC_Server::getFileAttributes(ServerContext* context, const Path*
// request, Attributes* response) {
//   //std::cout << yellow << "GRPC_Server::getFileAttributes" << reset <<
//   std::endl;

//   struct stat attributes;
//   string path = Utility::concatenatePath(serverDirectory, request->path());
//   /*
//   if (lstat(path.c_str(), &attributes) == -1) {
//     response->set_status(-1);
//     response->set_errornum(errno);
//     return Status::OK;
//   }
//   */
//   if (lstat(path.c_str(), &attributes) != 0) {
//     // if (errno == ENOENT) {
//     //   cout << red << "`lstat` errno: " << errno << " doesn't exist for
//     path: " << path << reset << endl;
//     // } else if (errno == EACCES) {
//     //   cout << red << "`lstat` errno: " << errno << " no permission to know
//     for path: " << path << reset << endl;
//     // } else {
//     //   cout << red << "`lstat` errno: " << errno << " general error for
//     path: " << path << reset << endl;
//     // }

//     response->set_status(-1);
//     response->set_errornum(errno);
//     return Status::OK;
//   }
//   auto it = serverclock.find(path);
//   if (it == serverclock.end()) {
//     serverclock.insert(std::make_pair(path, 0));
//   }

//   response->set_status(0);
//   // map satus info to equivalent grpc message structure
//   response->set_grpc_st_dev(attributes.st_dev);
//   response->set_grpc_st_ino(attributes.st_ino);
//   response->set_grpc_st_mode(attributes.st_mode);
//   response->set_grpc_st_nlink(attributes.st_nlink);
//   response->set_grpc_st_uid(attributes.st_uid);
//   response->set_grpc_st_gid(attributes.st_gid);
//   response->set_grpc_st_rdev(attributes.st_rdev);
//   response->set_grpc_st_size(attributes.st_size);
//   response->set_grpc_st_blksize(attributes.st_blksize);
//   response->set_grpc_st_blocks(attributes.st_blocks);
//   response->set_grpc_st_atime(attributes.st_atime);
//   response->set_grpc_st_mtime(attributes.st_mtime);
//   response->set_grpc_st_ctime(attributes.st_ctime);
//   response->set_logical_clock(serverclock[path]);

//   return Status::OK;
// }

// Status GRPC_Server::getFileContents(ServerContext* context, const
// ReadRequest* request, ServerWriter<ReadReply>* writer) {
//   //std::cout << yellow << "GRPC_Server::getFileContents" << reset <<
//   std::endl; int numOfBytes = 0; int res; struct timespec spec; std::string
//   path = Utility::concatenatePath(serverDirectory, request->path());

//   ReadReply* reply = new ReadReply();

//   std::ifstream is;
//   is.exceptions(std::ifstream::failbit | std::ifstream::badbit);
//   try {
//     // read data to buffer with offset and size
//     is.open(path.c_str(), std::ios::binary | std::ios::ate);

//     is.seekg(0, is.end);
//     int length = is.tellg();
//     is.seekg(0, is.beg);
//     /*
//     if (offset + size > is.tellg()) {
//       //std::cout << "The offset + size is greater then the file size.\n"
//                 << "file size: " << is.tellg() << "\n"
//                 << "offset: " << offset << "\n"
//                 << "size: " << size << std::endl;
//     }
//     */

//     if (length == 0) {
//       clock_gettime(CLOCK_REALTIME, &spec);
//       reply->set_buf("");
//       reply->set_numbytes(0);
//       reply->set_err(0);
//       reply->set_timestamp(spec.tv_sec);
//       writer->Write(*reply);
//       // //std::cout << "File is empty." << std::endl;
//       is.close();
//       return Status::OK;
//     }

//     std::string buffer(length, '\0');
//     is.read(&buffer[0], length);
//     // //std::cout << "buffer" << buffer << std::endl;
//     // send data chunk to client
//     int bytesRead = 0;
//     int minSize = std::min(CHUNK_SIZE, length);

//     while (bytesRead < is.tellg()) {
//       clock_gettime(CLOCK_REALTIME, &spec);
//       std::string subBuffer = buffer.substr(bytesRead, minSize);
//       // if (subBuffer.find("SERVER_READ_CRASH") != std::string::npos) {
//       //   //std::cout << "Killing server process in read\n";
//       //   kill(getpid(), SIGINT);
//       // }
//       reply->set_buf(subBuffer);
//       reply->set_numbytes(minSize);
//       reply->set_err(0);
//       reply->set_timestamp(spec.tv_sec);
//       bytesRead += minSize;
//       writer->Write(*reply);
//     }

//     is.close();

//   } catch (std::ifstream::failure e) {
//     reply->set_buf("");
//     reply->set_numbytes(0);
//     reply->set_err(-1);
//     reply->set_timestamp(-1);
//     writer->Write(*reply);
//     //std::cout << "Caught a failure when read.\n Explanatory string: " <<
//     e.what() << "'\nError code: " << e.code() << std::endl; is.close();
//   }

//   return Status::OK;

//   /*
//   int fd = open(path.c_str(), O_RDONLY);
//   if (fd == -1) {
//     reply->set_err(-errno);
//     reply->set_numbytes(INT_MIN);
//     return Status::OK;
//   }
//   std::string buf;
//   buf.resize(size);
//   int bytesRead = pread(fd, &buf[0], size, offset);
//   if (bytesRead != size) {
//     printf("Read Send: PREAD didn't read %d bytes from offset %d\n", size,
//             offset);
//   }
//   if (bytesRead == -1) {
//     reply->set_err(-errno);
//     reply->set_numbytes(INT_MIN);
//   }
//   int curr = 0;
//   while (bytesRead > 0) {
//     if (buf.find("crash1") != std::string::npos) {
//       //std::cout << "Killing server process in read\n";
//       kill(getpid(), SIGINT);
//     }
//     clock_gettime(CLOCK_REALTIME, &spec);
//     reply->set_buf(buf.substr(curr, std::min(CHUNK_SIZE, bytesRead)));
//     reply->set_numbytes(std::min(CHUNK_SIZE, bytesRead));
//     reply->set_err(0);
//     reply->set_timestamp(spec.tv_sec);
//     curr += std::min(CHUNK_SIZE, bytesRead);
//     bytesRead -= std::min(CHUNK_SIZE, bytesRead);
//     writer->Write(*reply);
//       }
//   if (fd > 0) {
//     printf("Read Send: Calling close()\n");
//     close(fd);
//   }
//   */
// }

// Status GRPC_Server::readDirectory(ServerContext* context, const Path*
// request, ServerWriter<afs::ReadDirResponse>* writer) {
//   //std::cout << yellow << "GRPC_Server::readDirectory" << reset <<
//   std::endl; string path = Utility::concatenatePath(serverDirectory,
//   request->path()); std::vector<std::string> alldata; struct dirent* de;

//   ReadDirResponse* response = new ReadDirResponse();

//   DIR* dp = opendir(path.c_str());
//   if (dp == NULL)
//     return Status::OK;

//   while ((de = readdir(dp)) != NULL) {
//     std::string data;
//     data.resize(sizeof(struct dirent));
//     memcpy(&data[0], de, sizeof(struct dirent));
//     alldata.push_back(data);
//   }

//   closedir(dp);

//   for (auto entry : alldata) {
//     response->set_buf(entry);
//     writer->Write(*response);
//   }

//   return Status::OK;
// }

// Status GRPC_Server::createDirectory(ServerContext* context, const
// MkDirRequest* request, Response* response) {
//   //std::cout << yellow << "GRPC_Server::createDirectory" << reset <<
//   std::endl; std::string s(request->path()); string path =
//   Utility::concatenatePath(serverDirectory, s); mode_t mode =
//   (mode_t)request->modet();

//   int ret = mkdir(path.c_str(), mode);
//   if (ret != 0)
//     response->set_erronum(errno);

//   response->set_status(ret);

//   return Status::OK;
// }

// Status GRPC_Server::removeDirectory(ServerContext* context, const Path*
// request, Response* response) {
//   //std::cout << yellow << "GRPC_Server::removeDirectory" << reset <<
//   std::endl; string path = Utility::concatenatePath(serverDirectory,
//   request->path()); std::error_code errorCode;

//   response->set_status(0);

//   int rc = rmdir(path.c_str());
//   if (rc != 0) {
//     response->set_status(-1);
//     response->set_erronum(errno);
//   }

//   return Status::OK;
// }

// Status GRPC_Server::removeFile(ServerContext* context, const Path* request,
// Response* response) {
//   //std::cout << yellow << "GRPC_Server::removeFile" << reset << std::endl;
//   string path = Utility::concatenatePath(serverDirectory, request->path());
//   std::error_code errorCode;

//   if (!fs::remove(path, errorCode)) {
//     response->set_status(1);
//     response->set_erronum(errorCode.value());
//   }
//   serverclock.erase(path);

//   return Status::OK;
// }

// Status GRPC_Server::createEmptyFile(ServerContext* context, const
// OpenRequest* request, OpenResponse* response) {
//   //std::cout << yellow << "GRPC_Server::createEmptyFile" << reset <<
//   std::endl; int rc; string path = Utility::concatenatePath(serverDirectory,
//   request->path());

//   rc = open(path.c_str(), request->mode(), S_IRWXG | S_IRWXO | S_IRWXU);
//   if (rc < 0) {
//     response->set_err(-errno);
//     return Status::OK;
//   }
//   struct timespec spec;
//   clock_gettime(CLOCK_REALTIME, &spec);
//   response->set_timestamp(spec.tv_sec);
//   response->set_err(0);
//   close(rc);
//   return Status::OK;
// }

// Status GRPC_Server::putFileContents(ServerContext* context,
// ServerReader<WriteRequest>* reader, WriteReply* reply) {
//   //std::cout << yellow << "GRPC_Server::putFileContents" << reset <<
//   std::endl; std::string path; WriteRequest request; std::string tempFilePath
//   = serverDirectory; int fd = -1; int res, size, offset, numOfBytes = 0;
//   reply->set_numbytes(numOfBytes);
//   //pthread_mutex_lock(&lock);
//   while (reader->Read(&request)) {
//     path = Utility::concatenatePath(serverDirectory, request.path());
//     size = request.size();
//     offset = request.offset();
//     std::string buf = request.buf();
//     if (numOfBytes == 0) {
//       tempFilePath = path + ".TMP";

//       fd = open(tempFilePath.c_str(), O_CREAT | O_SYNC | O_WRONLY, S_IRWXG |
//       S_IRWXO | S_IRWXU); if (fd == -1) {
//         reply->set_err(-errno);
//         reply->set_numbytes(INT_MIN);
//         return Status::OK;
//       }
//     }
//     res = pwrite(fd, &buf[0], size, offset);
//     fsync(fd);
//     // pwrite returns -1 when error, and store type in errno
//     if (res == -1) {
//       reply->set_err(-errno);
//       reply->set_numbytes(INT_MIN);
//       printf("putFileContents Send: Pwrite failed!");
//       return Status::OK;
//     }
//     numOfBytes += res;
//     if (numOfBytes == 0) break;
//   }
//   close(fd);

//   //pthread_mutex_unlock(&lock);
//   // error: cancel operation
//   if (context->IsCancelled()) {
//     fsync(fd);
//     close(fd);
//     reply->set_err(-errno);
//     reply->set_numbytes(INT_MIN);
//     return Status::CANCELLED;
//   }

//   if (fd > 0) {
//     fsync(fd);
//     close(fd);
//     // get new attributes in server timestamp
//     int res = rename(tempFilePath.c_str(), path.c_str());
//     if (res) {
//       reply->set_err(-errno);
//       reply->set_numbytes(INT_MIN);
//     }
//   }
//   serverclock[path] = serverclock[path] + 1;
//   struct stat stbuf;
//   int rc = stat(path.c_str(), &stbuf);
//   reply->set_logical_clock(serverclock[path]);
//   reply->set_numbytes(numOfBytes);
//   reply->set_err(0);

//   return Status::OK;
// }

// -------------------------------------------------------------------------------------------------
// EXAMPLE API
Status GRPC_Server::SayHello(ServerContext* context,
                             const HelloRequest* request, HelloReply* reply) {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
}

void RunServer(std::string address) {
    GRPC_Server service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

int main(int argc, char** argv) {
    struct stat info;

    // set defaults
    const std::string address("0.0.0.0:50051");
    
    // set configs from arguments
    // if (argc == 2) serverDirectory = argv[1];

    // std::cout << "serverDirectory: " << serverDirectory << std::endl;

    RunServer(address);

    return 0;
}