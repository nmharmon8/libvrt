#include <gtest/gtest.h>

#include <array>
#include <cstdint>

#include <vrt/vrt_common.h>
#include <vrt/vrt_write.h>

#include "hex.h"

class WriteFieldsTest : public ::testing::Test {
   protected:
    WriteFieldsTest() : h_(), f_(), buf_() {}

    void SetUp() override {
        vrt_init_header(&h_);
        vrt_init_fields(&f_);
        buf_.fill(0xBAADF00D);
    }

    vrt_header              h_;
    vrt_fields              f_;
    std::array<uint32_t, 8> buf_;
};

TEST_F(WriteFieldsTest, None) {
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 0), 0);
}

TEST_F(WriteFieldsTest, StreamId) {
    h_.packet_type = VRT_PT_IF_DATA_WITHOUT_STREAM_ID;
    f_.stream_id   = 0xFEDCBA98;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 1), 0);

    h_.packet_type = VRT_PT_IF_DATA_WITH_STREAM_ID;
    f_.stream_id   = 0xFEDCBA98;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 0), VRT_ERR);
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 1), 1);
    ASSERT_EQ(Hex(buf_[0]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[1]), Hex(0xBAADF00D));

    h_.packet_type = VRT_PT_EXT_DATA_WITHOUT_STREAM_ID;
    f_.stream_id   = 0xFEDCBA98;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 1), 0);

    h_.packet_type = VRT_PT_EXT_DATA_WITH_STREAM_ID;
    f_.stream_id   = 0xFEDCBA98;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 0), VRT_ERR);
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 1), 1);
    ASSERT_EQ(Hex(buf_[0]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[1]), Hex(0xBAADF00D));

    h_.packet_type = VRT_PT_IF_CONTEXT;
    f_.stream_id   = 0xFEDCBA98;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 0), VRT_ERR);
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 1), 1);
    ASSERT_EQ(Hex(buf_[0]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[1]), Hex(0xBAADF00D));

    h_.packet_type = VRT_PT_EXT_CONTEXT;
    f_.stream_id   = 0xFEDCBA98;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 0), VRT_ERR);
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 1), 1);
    ASSERT_EQ(Hex(buf_[0]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[1]), Hex(0xBAADF00D));
}

TEST_F(WriteFieldsTest, ClassId) {
    h_.has.class_id                    = true;
    f_.class_id.oui                    = 0xFFFEDCBA;
    f_.class_id.information_class_code = 0xFEDC;
    f_.class_id.packet_class_code      = 0xBA98;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 1), VRT_ERR);
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 2), 2);
    ASSERT_EQ(Hex(buf_[0]), Hex(0x00FEDCBA));
    ASSERT_EQ(Hex(buf_[1]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[2]), Hex(0xBAADF00D));
}

TEST_F(WriteFieldsTest, IntegerSecondsTimestamp) {
    h_.tsi                       = VRT_TSI_OTHER;
    f_.integer_seconds_timestamp = 0xFEDCBA98;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 1), 1);
    ASSERT_EQ(Hex(buf_[0]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[1]), Hex(0xBAADF00D));
}

TEST_F(WriteFieldsTest, FractionalSecondsTimestamp) {
    h_.tsf                          = VRT_TSF_SAMPLE_COUNT;
    f_.fractional_seconds_timestamp = 0xFEDCBA987654321F;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 2), 2);
    ASSERT_EQ(Hex(buf_[0]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[1]), Hex(0x7654321F));
    ASSERT_EQ(Hex(buf_[2]), Hex(0xBAADF00D));
}

TEST_F(WriteFieldsTest, EveryOther1) {
    h_.packet_type               = VRT_PT_EXT_DATA_WITH_STREAM_ID;
    h_.has.class_id              = false;
    h_.tsi                       = VRT_TSI_OTHER;
    h_.tsf                       = VRT_TSF_NONE;
    h_.packet_count              = 0xF;
    f_.stream_id                 = 0xDEADBEEF;
    f_.integer_seconds_timestamp = 0xFEDCBA98;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 1), VRT_ERR);
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 2), 2);
    ASSERT_EQ(Hex(buf_[0]), Hex(0xDEADBEEF));
    ASSERT_EQ(Hex(buf_[1]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[2]), Hex(0xBAADF00D));
}

TEST_F(WriteFieldsTest, EveryOther2) {
    h_.packet_type                     = VRT_PT_IF_DATA_WITHOUT_STREAM_ID;
    h_.has.class_id                    = true;
    h_.tsi                             = VRT_TSI_NONE;
    h_.tsf                             = VRT_TSF_FREE_RUNNING_COUNT;
    h_.packet_count                    = 0x0;
    f_.class_id.oui                    = 0x00FEDCBA;
    f_.class_id.information_class_code = 0xFEDC;
    f_.class_id.packet_class_code      = 0xBA98;
    f_.fractional_seconds_timestamp    = 0xFEDCBA987654321F;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 3), VRT_ERR);
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 4), 4);
    ASSERT_EQ(Hex(buf_[0]), Hex(0x00FEDCBA));
    ASSERT_EQ(Hex(buf_[1]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[2]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[3]), Hex(0x7654321F));
    ASSERT_EQ(Hex(buf_[4]), Hex(0xBAADF00D));
}

TEST_F(WriteFieldsTest, All) {
    h_.packet_type                     = VRT_PT_EXT_DATA_WITH_STREAM_ID;
    h_.has.class_id                    = true;
    h_.tsi                             = VRT_TSI_OTHER;
    h_.tsf                             = VRT_TSF_SAMPLE_COUNT;
    h_.packet_count                    = 0xC;
    f_.stream_id                       = 0xDEADBEEF;
    f_.class_id.oui                    = 0x00FEDCBA;
    f_.class_id.information_class_code = 0xFEDC;
    f_.class_id.packet_class_code      = 0xBA98;
    f_.integer_seconds_timestamp       = 0xFEDCBA98;
    f_.fractional_seconds_timestamp    = 0xFEDCBA987654321F;
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 5), VRT_ERR);
    ASSERT_EQ(vrt_write_fields(&h_, &f_, buf_.data(), 6), 6);
    ASSERT_EQ(Hex(buf_[0]), Hex(0xDEADBEEF));
    ASSERT_EQ(Hex(buf_[1]), Hex(0x00FEDCBA));
    ASSERT_EQ(Hex(buf_[2]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[3]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[4]), Hex(0xFEDCBA98));
    ASSERT_EQ(Hex(buf_[5]), Hex(0x7654321F));
    ASSERT_EQ(Hex(buf_[6]), Hex(0xBAADF00D));
}
