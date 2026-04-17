#include <iostream>
#include <string>
#include <chrono>
#include <json/json.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "user.pb.h"

using namespace std;
using namespace chrono;

// 压力测试次数（百万次）
const int LOOP = 1000000;

void bench_protobuf() {
    string bin_data;
    {
        test::User u;
        u.set_id(1001);
        u.set_name("ZhangSan");
        u.set_age(20);
        u.set_phone("13800138000");
        u.SerializeToString(&bin_data);
    }

    // 序列化
    auto t1 = high_resolution_clock::now();
    for (int i = 0; i < LOOP; ++i) {
        test::User u;
        u.set_id(1001);
        u.set_name("ZhangSan");
        u.set_age(20);
        u.set_phone("13800138000");
        u.SerializeToString(&bin_data);
    }
    auto t2 = high_resolution_clock::now();

    // 反序列化
    for (int i = 0; i < LOOP; ++i) {
        test::User u;
        u.ParseFromString(bin_data);
    }
    auto t3 = high_resolution_clock::now();

    auto ser_us = duration_cast<microseconds>(t2 - t1).count();
    auto des_us = duration_cast<microseconds>(t3 - t2).count();

    cout << "[Protobuf]" << endl;
    cout << "size: " << bin_data.size() << " B" << endl;
    cout << "serialize: " << ser_us << " us (" << (double)ser_us / LOOP << " us/once)" << endl;
    cout << "deserialize: " << des_us << " us (" << (double)des_us / LOOP << " us/once)" << endl;
    cout << "total: " << ser_us + des_us << " us" << endl << endl;
}

void bench_json() {
    string json_str;
    {
        Json::Value v;
        v["id"] = 1001;
        v["name"] = "ZhangSan";
        v["age"] = 20;
        v["phone"] = "13800138000";
        Json::FastWriter w;
        json_str = w.write(v);
    }

    auto t1 = high_resolution_clock::now();
    for (int i = 0; i < LOOP; ++i) {
        Json::Value v;
        v["id"] = 1001;
        v["name"] = "ZhangSan";
        v["age"] = 20;
        v["phone"] = "13800138000";
        Json::FastWriter w;
        json_str = w.write(v);
    }
    auto t2 = high_resolution_clock::now();

    for (int i = 0; i < LOOP; ++i) {
        Json::Value v2;
        Json::Reader r;
        r.parse(json_str, v2);
    }
    auto t3 = high_resolution_clock::now();

    auto ser_us = duration_cast<microseconds>(t2 - t1).count();
    auto des_us = duration_cast<microseconds>(t3 - t2).count();

    cout << "[JSON]" << endl;
    cout << "size: " << json_str.size() << " B" << endl;
    cout << "serialize: " << ser_us << " us (" << (double)ser_us / LOOP << " us/once)" << endl;
    cout << "deserialize: " << des_us << " us (" << (double)des_us / LOOP << " us/once)" << endl;
    cout << "total: " << ser_us + des_us << " us" << endl << endl;
}

void bench_xml() {
    xmlChar *xml_buf = nullptr;
    int xml_size = 0;
    {
        xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
        xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "User");
        xmlDocSetRootElement(doc, root);
        xmlNewChild(root, NULL, BAD_CAST "id", BAD_CAST "1001");
        xmlNewChild(root, NULL, BAD_CAST "name", BAD_CAST "ZhangSan");
        xmlNewChild(root, NULL, BAD_CAST "age", BAD_CAST "20");
        xmlNewChild(root, NULL, BAD_CAST "phone", BAD_CAST "13800138000");
        xmlDocDumpMemory(doc, &xml_buf, &xml_size);
        xmlFreeDoc(doc);
    }

    auto t1 = high_resolution_clock::now();
    for (int i = 0; i < LOOP; ++i) {
        xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
        xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "User");
        xmlDocSetRootElement(doc, root);
        xmlNewChild(root, NULL, BAD_CAST "id", BAD_CAST "1001");
        xmlNewChild(root, NULL, BAD_CAST "name", BAD_CAST "ZhangSan");
        xmlNewChild(root, NULL, BAD_CAST "age", BAD_CAST "20");
        xmlNewChild(root, NULL, BAD_CAST "phone", BAD_CAST "13800138000");
        xmlFreeDoc(doc);
    }
    auto t2 = high_resolution_clock::now();

    for (int i = 0; i < LOOP; ++i) {
        xmlDocPtr doc = xmlParseDoc(xml_buf);
        xmlFreeDoc(doc);
    }
    auto t3 = high_resolution_clock::now();

    auto ser_us = duration_cast<microseconds>(t2 - t1).count();
    auto des_us = duration_cast<microseconds>(t3 - t2).count();

    cout << "[XML]" << endl;
    cout << "size: " << xml_size << " B" << endl;
    cout << "serialize: " << ser_us << " us (" << (double)ser_us / LOOP << " us/once)" << endl;
    cout << "deserialize: " << des_us << " us (" << (double)des_us / LOOP << " us/once)" << endl;
    cout << "total: " << ser_us + des_us << " us" << endl << endl;

    xmlFree(xml_buf);
    xmlCleanupParser();
}

int main() {
    cout << "===== Loop: " << LOOP << " times =====" << endl << endl;
    bench_protobuf();
    bench_json();
    bench_xml();
    return 0;
}