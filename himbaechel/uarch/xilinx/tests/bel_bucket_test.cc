/*
 *  nextpnr -- Next Generation Place and Route
 *
 *  Copyright (C) 2026  nextpnr contributors
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "command.h"
#include "gtest/gtest.h"
#include "nextpnr.h"
#include "uarch/xilinx/xilinx.h"
#define HIMBAECHEL_CONSTIDS "uarch/xilinx/constids.inc"
#include "himbaechel_constids.h"

USING_NEXTPNR_NAMESPACE

class XilinxBelBucketTest : public ::testing::Test
{
  protected:
    virtual void SetUp()
    {
        init_share_dirname();
        chipArgs.device = "xc7a50tcsg324-1";
        ctx = new Context(chipArgs);
        ctx->uarch->init(ctx);
        ctx->late_init();
    }

    virtual void TearDown() { delete ctx; }

    ArchArgs chipArgs;
    Context *ctx;
};

// Utilisation, --report and the placer's logs name cells and bels by their
// bucket.  prjxray names a bel "<site type>_<bel name>", so a chipdb bel type
// can be the primitive name twice (RAMB18E1_RAMB18E1); the bucket must be the
// primitive, else the utilisation table reads "RAMB18E1_RAMB18E1" and overflows
// its name column.
TEST_F(XilinxBelBucketTest, repeated_bel_type_buckets_as_the_primitive)
{
    EXPECT_EQ(ctx->getBelBucketForCellType(id_RAMB18E1_RAMB18E1), id_RAMB18E1);
    EXPECT_EQ(ctx->getBelBucketForCellType(id_FIFO18E1_FIFO18E1), id_FIFO18E1);
    EXPECT_EQ(ctx->getBelBucketForCellType(id_RAMB36E1_RAMB36E1), id_RAMB36E1);
    EXPECT_EQ(ctx->getBelBucketForCellType(id_DSP48E1_DSP48E1), id_DSP48E1);
    EXPECT_EQ(ctx->getBelBucketForCellType(id_MMCME2_ADV_MMCME2_ADV), id_MMCME2_ADV);
    EXPECT_EQ(ctx->getBelBucketForCellType(id_BUFG_BUFG), id_BUFG);
    EXPECT_EQ(ctx->getBelBucketForCellType(id_BUFR_BUFR), id_BUFR);
    EXPECT_EQ(ctx->getBelBucketForCellType(id_ISERDESE2_ISERDESE2), id_ISERDESE2);
}

TEST_F(XilinxBelBucketTest, bel_type_that_does_not_repeat_its_name_is_its_own_bucket)
{
    EXPECT_EQ(ctx->getBelBucketForCellType(id_SLICE_LUTX), id_SLICE_LUTX);
    EXPECT_EQ(ctx->getBelBucketForCellType(id_IOB33M_OUTBUF), id_IOB33M_OUTBUF);
    EXPECT_EQ(ctx->getBelBucketForCellType(id_BSCAN), id_BSCAN);
    EXPECT_EQ(ctx->getBelBucketForCellType(id_PAD), id_PAD);
}

TEST_F(XilinxBelBucketTest, bel_bucket_names_the_primitive)
{
    bool found_repeated_bel = false, found_plain_bel = false;
    for (auto bel : ctx->getBels()) {
        IdString bel_type = ctx->getBelType(bel);
        if (bel_type == id_RAMB18E1_RAMB18E1) {
            EXPECT_EQ(ctx->getBelBucketForBel(bel), id_RAMB18E1);
            found_repeated_bel = true;
        }
        if (bel_type == id_IOB33M_OUTBUF) {
            EXPECT_EQ(ctx->getBelBucketForBel(bel), id_IOB33M_OUTBUF);
            found_plain_bel = true;
        }
    }
    EXPECT_TRUE(found_repeated_bel);
    EXPECT_TRUE(found_plain_bel);
}
