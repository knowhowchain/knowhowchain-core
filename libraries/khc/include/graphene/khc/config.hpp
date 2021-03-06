#pragma once
#include <cstdint>
#include <graphene/chain/protocol/asset.hpp>

#define KHC_PROJECT_ASSET_MAX_NAME_LENGTH 30
#define KHC_PROJECT_ASSET_MIN_PROJECT_CYCLE 1
#define KHC_PROJECT_ASSET_MAX_PROJECT_CYCLE 36
#define KHC_PROJECT_ASSET_MIN_TRANSFER_RATIO 10
#define KHC_PROJECT_ASSET_MAX_TRANSFER_RATIO 75
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

#define KHC_POWER_CONVERT_KHD_RATIO 1000
#define KHD_PRECISION_DIGITS 4

#define KHC_FIRST_CLAIM_INVESTMENT_RATIO 30
#define KHC_SECOND_CLAIM_INVESTMENT_TATIO 30
#define KHC_THIRD_CLAIM_INVESTMENT_TATIO 40

#define KHC_PRIVATE_OFFERING 0   // financing type : private offering
#define KHC_PUBLIC_OFFERING 1    // financing type : public offering
