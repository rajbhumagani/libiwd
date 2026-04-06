#include "libiwd/priority_store.hpp"

#include <cstdio>

TEST(priority_store_roundtrip_and_delete) {
    const std::string path = "/tmp/libiwd_priority_store_test.json";
    std::remove(path.c_str());

    libiwd::PriorityStore store(path);
    ASSERT_TRUE(store.load().success);
    ASSERT_TRUE(store.upsert("netA", {7, true, 123}).success);
    ASSERT_TRUE(store.upsert("netB", {2, false, 5}).success);

    libiwd::PriorityStore reloaded(path);
    ASSERT_TRUE(reloaded.load().success);
    auto netA = reloaded.get("netA");
    ASSERT_TRUE(netA.has_value());
    ASSERT_TRUE(netA->priority == 7);

    ASSERT_TRUE(reloaded.erase("netA").success);
    libiwd::PriorityStore afterDelete(path);
    ASSERT_TRUE(afterDelete.load().success);
    ASSERT_TRUE(!afterDelete.get("netA").has_value());
}
