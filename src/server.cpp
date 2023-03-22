int main(int argc, char** argv) {
    std::thread raft_thread(RunRaftServer);
    raft_thread.join();
    std::thread kv_thread(RunKVServer);
    kv_thread.join();

    return 0;
}