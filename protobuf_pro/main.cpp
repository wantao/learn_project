#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <mutex>
#include <utility>  // for pair
#include "order.pb.h"

// 全局锁：多线程安全
std::mutex g_proto_mutex;

/**
 * @brief 创建测试订单数据
 */
order_server::OrderRequest CreateTestOrder() {
    order_server::OrderRequest req;

    // 基础字段
    req.set_order_id(10086);
    req.set_status(order_server::PAID);
    req.set_create_time(time(NULL));

    // 用户信息
    order_server::User* user = req.mutable_user();
    user->set_user_id(20010);
    user->set_phone("13800138000");
    user->set_email("demo@test.com");

    // 商品1
    order_server::Product* p1 = req.add_products();
    p1->set_product_id(1001);
    p1->set_name("C++ 服务端开发书籍");
    p1->set_price(99.8);
    p1->set_count(1);

    // 商品2
    order_server::Product* p2 = req.add_products();
    p2->set_product_id(1002);
    p2->set_name("Protobuf 高性能实战");
    p2->set_price(69.8);
    p2->set_count(2);

    // 扩展字段（proto2 兼容）
    order_server::ExtInfo* ext1 = req.add_ext_info();
    ext1->set_key("channel");
    ext1->set_value("PC");

    order_server::ExtInfo* ext2 = req.add_ext_info();
    ext2->set_key("version");
    ext2->set_value("v2.1");

    return req;
}

/**
 * @brief 序列化
 */
std::pair<char*, size_t> SerializeOrder(const order_server::OrderRequest& req) {
    std::lock_guard<std::mutex> lock(g_proto_mutex);
    
    // 老版本 protobuf 使用 ByteSize()
    size_t size = req.ByteSize();
    char* buffer = new char[size];
    
    if (!req.SerializeToArray(buffer, size)) {
        delete[] buffer;
        return std::make_pair((char*)NULL, 0);
    }
    return std::make_pair(buffer, size);
}

/**
 * @brief 反序列化
 */
bool DeserializeOrder(const char* buffer, size_t size, order_server::OrderRequest& out_req) {
    std::lock_guard<std::mutex> lock(g_proto_mutex);
    return out_req.ParseFromArray(buffer, size);
}

/**
 * @brief 性能测试
 */
void PerformanceTest() {
    order_server::OrderRequest req = CreateTestOrder();
    auto start = std::chrono::high_resolution_clock::now();

    const int TEST_COUNT = 100000;
    for (int i = 0; i < TEST_COUNT; ++i) {
        // 老式 pair 获取（兼容 GCC4.8.5）
        std::pair<char*, size_t> res = SerializeOrder(req);
        char* buf = res.first;
        size_t len = res.second;
        delete[] buf;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\n===== 性能测试结果 =====" << std::endl;
    std::cout << "10万次序列化耗时：" << duration.count() << " ms" << std::endl;
}

int main() {
    // 1. 创建数据
    order_server::OrderRequest req = CreateTestOrder();
    std::cout << "===== 原始订单数据 =====" << std::endl;
    std::cout << "订单ID：" << req.order_id() << "\n用户ID：" << req.user().user_id() << std::endl;

    // 2. 序列化
    std::pair<char*, size_t> send_res = SerializeOrder(req);
    char* send_buf = send_res.first;
    size_t buf_len = send_res.second;

    if (!send_buf) {
        std::cerr << "序列化失败！" << std::endl;
        return -1;
    }
    std::cout << "\n序列化完成，字节长度：" << buf_len << std::endl;

    // 3. 反序列化
    order_server::OrderRequest parse_req;
    if (!DeserializeOrder(send_buf, buf_len, parse_req)) {
        std::cerr << "反序列化失败！" << std::endl;
        delete[] send_buf;
        return -1;
    }
    std::cout << "\n===== 反序列化结果 =====" << std::endl;
    std::cout << "订单状态：" << parse_req.status() << "\n商品数量：" << parse_req.products_size() << std::endl;

    // 4. 性能测试
    PerformanceTest();

    delete[] send_buf;
    return 0;
}