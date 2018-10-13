#pragma once
#include <cstdint>
#include <graphene/chain/protocol/asset.hpp>

#define KHC_PROJECT_ASSET_MAX_NAME_LENGTH 30
#define KHC_PROJECT_ASSET_MIN_PROJECT_CYCLE 1
#define KHC_PROJECT_ASSET_MAX_PROJECT_CYCLE 36
#define KHC_PROJECT_ASSET_MIN_TRANSFER_RATIO 0
#define KHC_PROJECT_ASSET_MAX_TRANSFER_RATIO 49
#define KHC_100_PERCENT                 100
#define KHC_PROJECT_ASSET_MIN_FINANCING_CYCLE 1
#define KHC_PROJECT_ASSET_MAX_FINANCING_CYCLE 4

#define KHC_PROJECT_ASSET_FINANCING_CYCLE_UNIT (60*60*24*7) //1 month
#define KHC_PROJECT_ASSET_TEST_FINANCING_CYCLE_UNIT 60 //1 min

#define KHC_PROJECT_ASSET_PROJECT_CYCLE_UNIT (60*60*24*30) //1 week
#define KHC_PROJECT_ASSET_TEST_PROJECT_CYCLE_UNIT 60 //1 min

extern uint64_t g_khc_project_asset_financing_cycle_unit;
extern uint64_t g_khc_project_asset_project_cycle_unit;

#define KHC_POWER_PRECISION_DIGITS 8

#define KHC_POWER_CONVERT_KHD_RATIO 100
#define KHD_PRECISION_DIGITS 4
/*
#define KHC_POWER_GRADE_0_MAX_FUNDRAISING_QUOTA 1000
#define KHC_POWER_GRADE_0_MIN_POWER 51 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)
#define KHC_POWER_GRADE_0_MAX_POWER 99 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)

#define KHC_POWER_GRADE_1_MAX_FUNDRAISING_QUOTA 5000
#define KHC_POWER_GRADE_1_MIN_POWER 100 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)
#define KHC_POWER_GRADE_1_MAX_POWER 499 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)

#define KHC_POWER_GRADE_2_MAX_FUNDRAISING_QUOTA 20000
#define KHC_POWER_GRADE_2_MIN_POWER 500 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)
#define KHC_POWER_GRADE_2_MAX_POWER 1999 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)

#define KHC_POWER_GRADE_3_MAX_FUNDRAISING_QUOTA 100000
#define KHC_POWER_GRADE_3_MIN_POWER 2000 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)
#define KHC_POWER_GRADE_3_MAX_POWER 4999 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)

#define KHC_POWER_GRADE_4_MAX_FUNDRAISING_QUOTA 500000
#define KHC_POWER_GRADE_4_MIN_POWER 5000 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)
#define KHC_POWER_GRADE_4_MAX_POWER 9999 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)

#define KHC_POWER_GRADE_5_MAX_FUNDRAISING_QUOTA 1000000
#define KHC_POWER_GRADE_5_MIN_POWER 10000 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)
#define KHC_POWER_GRADE_5_MAX_POWER 49999 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)

//#define KHC_POWER_GRADE_6_MAX_FUNDRAISING_QUOTA int64_t(1000000000000000000ll)
#define KHC_POWER_GRADE_6_MIN_POWER 50000 * graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS)
#define KHC_POWER_GRADE_6_MAX_POWER 0 //0 means No upper limit
*/