/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <graphene/chain/financing_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/khc/util.hpp>

namespace graphene { namespace chain {

void_result asset_investment_evaluator::do_evaluate( const asset_investment_operation& o )
{ try {
   database& d = db();
   const asset_object& khd_asset_object = d.get(o.amount.asset_id);
   KHC_EASSERT(khd_asset_object.symbol == KHD_ASSET_SYMBOL,"investment must be ${asset}",("asset",KHD_ASSET_SYMBOL));
   KHC_WASSERT(o.fee.amount >= 0,"invalid asset investment fee amount");
   KHC_WASSERT(o.amount.amount > 0,"invalid investment amount:${investment_amount}",
               ("investment_amount",khc::khc_amount_to_string(o.amount.amount.value,khd_asset_object.precision)));

   asset account_khd_asset = d.get_balance(d.get(o.account_id), khd_asset_object);
   KHC_WASSERT( account_khd_asset >= o.amount,"account amount(${account_amount}) less than inverstment amount(${investment_amount})",
                ("account_amount",khc::khc_amount_to_string(account_khd_asset.amount.value,khd_asset_object.precision))
                ("investment_amount",khc::khc_amount_to_string(o.amount.amount.value,khd_asset_object.precision)));

   const asset_object& asset_obj = d.get(o.investment_asset_id);
   KHC_WASSERT(!asset_obj.is_market_issued());
   KHC_WASSERT(o.investment_asset_id != asset_id_type(0),"can't investment ${asset}",("asset",GRAPHENE_SYMBOL));

   const auto& dpo = d.get_dynamic_global_properties();
   const auto& po = d.get_global_properties();
   uint32_t start_block_num = asset_obj.proj_options.ref_block_num;
   uint32_t end_block_num = asset_obj.proj_options.ref_block_num + (asset_obj.proj_options.financing_cycle/po.parameters.block_interval);
   uint32_t curr_block_num = dpo.head_block_number;
   KHC_WASSERT(curr_block_num>=asset_obj.proj_options.ref_block_num&&curr_block_num<=end_block_num,
               "current block num(${curr_block_num}) need betweent [${start_block_num},${end_block_num}]",
               ("curr_block_num",curr_block_num)("start_block_num",start_block_num)("end_block_num",end_block_num));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result asset_investment_evaluator::do_apply( const asset_investment_operation& o )
{ try {
   database& d = db();
   const auto& dpo = d.get_dynamic_global_properties();
   d.create<asset_investment_object>([&](asset_investment_object& s)
   {
       s.investment_account_id = o.account_id;
       s.investment_khd_amount = o.amount;
       s.investment_asset_id = o.investment_asset_id;
       s.investment_height = dpo.head_block_number;
       s.investment_timestamp = d.head_block_time();
       s.return_financing_flag = false;
   });
   const asset_object& mia = d.get(o.investment_asset_id);
   const asset_dynamic_data_object* asset_dyn_data = &mia.dynamic_asset_data_id(d);
   d.modify( *asset_dyn_data, [&]( asset_dynamic_data_object& data ){
        data.financing_current_supply += o.amount.amount;
        data.financing_confidential_supply += o.amount.amount;
   });

   d.adjust_balance( o.account_id , -o.amount );

   const asset_object& khd_asset_object = d.get(o.amount.asset_id);
   khc_dlog("account(${account}) investment asset(${asset}) ${investment_khd_amount} KHD.",
            ("account",o.account_id)("asset",o.investment_asset_id)
            ("investment_khd_amount",khc::khc_amount_to_string(o.amount.amount,khd_asset_object.precision)));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result issue_asset_and_get_financing_evaluator::do_evaluate( const issue_asset_and_get_financing_operation& o )
{ try {
   database& d = db();

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result issue_asset_and_get_financing_evaluator::do_apply( const issue_asset_and_get_financing_operation& o )
{ try {
   database& d = db();
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

} } // graphene::chain
